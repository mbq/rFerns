/*   Code handling fern ensembles -- creation, prediction, OOB, accuracy...

     Copyright 2011-2014 Miron B. Kursa

     This file is part of rFerns R package.

 rFerns is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 rFerns is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 You should have received a copy of the GNU General Public License along with rFerns. If not, see http://www.gnu.org/licenses/.
*/

//TODO: Make it multi-label (mean Hanning distance increase or mean quotient increase for good classes or ...)
double calcAccLoss(DATASET_,uint E,FERN_,uint *bag,uint *idx,score_t *curPreds,uint numC,uint D,R_,uint *idxC){
 uint count=0;
 double wrongDiff=0;
 memcpy(idxC,idx,sizeof(uint)*N);
 for(uint e=0;e<D;e++)
  if(splitAtts[e]==E){
   switch(X[E].numCat){
    case 0:{
     //Numerical split
     double *x=(double*)(X[E].x);
     double threshold=thresholds[e].value;
     for(uint ee=0;ee<N;ee++){
      sint mod=((x[RINDEX(N)]<threshold)-(x[ee]<threshold));
      idxC[ee]=idx[ee]+(1<<e)*mod;
     }
     break;
    }
    case -1:{
     //Integer split
     sint *x=(sint*)(X[E].x);
     sint threshold=thresholds[e].intValue;
     for(uint ee=0;ee<N;ee++){
      sint mod=((x[RINDEX(N)]<threshold)-(x[ee]<threshold));
      idxC[ee]=idx[ee]+(1<<e)*mod;
     }
     break;
    }
    default:{
     //Categorical split
     uint *x=(uint*)(X[E].x);
     mask mask=thresholds[e].selection;
     for(uint ee=0;ee<N;ee++){
      sint mod=(((mask&(1<<(x[RINDEX(N)])))>0)-((mask&(1<<(x[ee])))>0));
      idxC[ee]=idx[ee]+(1<<e)*mod;
     }
    }
   }
  }
 for(uint e=0;e<N;e++){
  wrongDiff+=
    (!(bag[e]))*
    (scores[idx[e]*numC+Y[e]]-scores[idxC[e]*numC+Y[e]]);
  count+=!(bag[e]);
 }
 return(wrongDiff/((double)count));
}

double calcOobError(score_t *oobPredsAcc,uint *oobPredsC,uint *Y,uint N,uint numC,uint multi){
 uint wrong=0;
 uint count=0;
 if(!multi){
  //Report 1-acc
  for(uint e=0;e<N;e++){
   score_t max=-INFINITY; uint whichMax=UINT_MAX;
   for(uint ee=0;ee<numC;ee++)
    if(oobPredsAcc[ee+numC*e]>max){
     max=oobPredsAcc[ee+numC*e];
     whichMax=ee;
    }
   uint ignore=!(oobPredsC[e]);
   wrong+=(!ignore)&&(Y[e]!=whichMax);
   count+=!ignore;
  }
 }else{
  //Report mean Hanning distance
  for(uint e=0;e<N;e++){
   uint ignore=!(oobPredsC[e]);
   for(uint ee=0;ee<numC;ee++){
    wrong+=(!ignore)&&(Y[ee*N+e]!=(oobPredsAcc[ee+numC*e]>0));
   }
   count+=!ignore;
  }
 }
 return ((double)wrong)/((double)count);
}

void killModel(model *x);

model *makeModel(DATASET_,ferns *ferns,params *P,R_){
 uint numC=P->numClasses;
 uint D=P->D;
 uint twoToD=P->twoToD;
 uint multi=P->multilabel;

 //=Allocations=//
 //Internal objects
 ALLOCN(curPreds,score_t,numC*N);
 ALLOCN(bag,uint,N);
 ALLOCN(idx,uint,N);

 //Output objects
 ALLOCN(ans,model,1);
 ans->oobErr=NULL;
 if(P->holdOobErr)
  ALLOC(ans->oobErr,double,P->numFerns);

 //OOB preds stuff
 ALLOCZ(ans->oobPreds,score_t,numC*N);
 score_t *oobPredsAcc=ans->oobPreds;
 ALLOCZ(ans->oobOutOfBagC,uint,N);
 uint *oobPredsC=ans->oobOutOfBagC;

 //Stuff for importance
 double *sumD=NULL; double *sumDD=NULL;
 uint *buf_idxC=NULL;
 ans->imp=NULL; ans->impSd=NULL;
 uint *useCount=NULL;

 if(P->calcImp){
  ALLOCZ(sumD,double,nX);
  ALLOCZ(sumDD,double,nX);
  ALLOCZ(useCount,uint,nX);
  ALLOCZ(ans->imp,double,nX);
  ALLOCZ(ans->impSd,double,nX);
  ALLOC(buf_idxC,uint,N);
 }

 /*
  When holdForest is FALSE, each fern is written to the index 0 of the fern vectors (which are obviously allocated to hold only 1 fern).
  To account for that, eM is introduced; eM*<fern index> is <fern index> when holdForest is TRUE and 0 when holdForest is FALSE.
 */
 uint eM=!(!(P->holdForest));
 ans->forest=ferns;

 //=Building model=//
 for(uint e=0;e<(P->numFerns);e++){
  CHECK_INTERRUPT; //Place to go though event loop, if such is present
  makeBagMask(bag,N,_R);
  makeFern(_DATASET,_thFERN(e*eM),bag,curPreds,idx,_SIMP,_R);

  //Accumulating OOB errors
  for(uint ee=0;ee<N;ee++){
   oobPredsC[ee]+=!(bag[ee]);
   for(uint eee=0;eee<numC;eee++)
    oobPredsAcc[eee+numC*ee]+=((double)(!(bag[ee])))*curPreds[eee+numC*ee];
  }

  //Reporting OOB error
  if(P->holdOobErr){
   ans->oobErr[e]=calcOobError(oobPredsAcc,oobPredsC,Y,N,numC,multi);
   if((e+1)%(P->repOobErrEvery)==0)
    PRINT("Done fern %u/%u; current OOB error %0.5f\n",(e+1),(P->numFerns),ans->oobErr[e]);
  }else{
   if((e+1)%(P->repOobErrEvery)==0){
    double err=calcOobError(oobPredsAcc,oobPredsC,Y,N,numC,multi);
    PRINT("Done fern %u/%u; current OOB error %0.5f\n",(e+1),(P->numFerns),err);
   }
  }
  if(P->calcImp){
   /*
    For importance, we want to know which unique attributes were used to build it.
    Their number will be placed in numAC, and attC[0..(numAC-1)] will contain their indices.
   */
   uint attC[16];
   attC[0]=(ferns->splitAtts)[e*D*eM];
   uint numAC=1;
   for(uint ee=1;ee<D;ee++){
    for(uint eee=0;eee<numAC;eee++)
     if((ferns->splitAtts)[e*D*eM+ee]==attC[eee]) goto isDuplicate;
    attC[numAC]=(ferns->splitAtts)[e*D*eM+ee]; numAC++;
    isDuplicate:
    continue;
   }

   /*
    For each unique attribute, we permute it and check how the fraction of scores for the right class decreases.
    This number is used to update sumD, sumDD and useCount cells for this attribute to later get average and SD over all ferns.
   */
   for(uint ee=0;ee<numAC;ee++){
    double impLoss=calcAccLoss(_DATASET,attC[ee],_thFERN(e*eM),bag,idx,curPreds,numC,D,_R,buf_idxC);
    sumD[attC[ee]]+=impLoss;
    sumDD[attC[ee]]+=impLoss*impLoss;
    useCount[attC[ee]]++;
   }
  }
 }

 //=Finishing up=//
 //Finishing importance
 if(P->calcImp){
  for(uint e=0;e<nX;e++){
   if(useCount[e]==0){
    ans->imp[e]=0.;
    ans->impSd[e]=NAN;
   }else{
    ans->imp[e]=((double)sumD[e])/((double)useCount[e]);
    ans->impSd[e]=sqrt(sumDD[e]-sumD[e]*sumD[e]/((double)useCount[e]))/((double)useCount[e]-1);
   }
  }
 }
 //Releasing memory
 FREE(bag); FREE(curPreds);
 FREE(sumD); FREE(sumDD); FREE(useCount); FREE(idx);
 FREE(buf_idxC);
 return(ans);

 #ifndef IN_R
  allocFailed:
  killModel(ans);
  IFFREE(curPreds); IFFREE(bag); IFFREE(sumD);
  IFFREE(sumDD); IFFREE(useCount); IFFREE(idx);
  IFFREE(buf_idxC);
  return(NULL);
 #endif
}

void predictWithModelSimple(PREDSET_,ferns *x,uint *ans,SIMPP_,double *sans,R_){
 ferns *ferns=x;
 for(uint e=0;e<numC*N;e++)
  sans[e]=0.;
 //Use ans memory as idx buffer
 uint *idx=ans;
 for(uint e=0;e<numFerns;e++){
  predictFernAdd(
   _PREDSET,
   _thFERN(e),
   sans,
   idx,
   _SIMP);
 }
 if(!multi){
  for(uint e=0;e<N;e++)
   ans[e]=whichMaxTieAware(&(sans[e*numC]),numC,_R);
 }else{
  for(uint e=0;e<numC;e++)
   for(uint ee=0;ee<N;ee++)
    ans[e*N+ee]=sans[ee*numC+e]>0.;
 }
}


void predictWithModelScores(PREDSET_,ferns *x,double *ans,SIMPP_,uint *idx){
 ferns *ferns=x;
 for(uint e=0;e<numC*N;e++)
  ans[e]=0.;
 for(uint e=0;e<numFerns;e++)
  predictFernAdd(
   _PREDSET,
   _thFERN(e),
   ans,
   idx,
   _SIMP);
}

void killModel(model *x){
 if(x){
  IFFREE(x->oobPreds);
  IFFREE(x->oobOutOfBagC);
  IFFREE(x->oobErr);
  IFFREE(x->imp);
  IFFREE(x->impSd);
  FREE(x);
 }
}
