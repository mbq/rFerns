/*   Code for making/predicting by single fern

     Copyright 2011-2015 Miron B. Kursa

     This file is part of rFerns R package.

 rFerns is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 rFerns is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 You should have received a copy of the GNU General Public License along with rFerns. If not, see http://www.gnu.org/licenses/.
*/

void makeFern(DATASET_,FERN_,uint *restrict bag,score_t *restrict oobPrMatrix,uint *restrict idx,SIMP_,R_){

 for(uint e=0;e<N;e++) idx[e]=0;

 for(uint e=0;e<D;e++){
  //Select an attribute to make a split on
  uint E=splitAtts[e]=RINDEX(nX);
  switch(X[E].numCat){
   case 0:{
    //Make numerical split
    double *restrict x=(double*)(X[E].x);
    double threshold=.5*(x[RINDEX(N)]+x[RINDEX(N)]);
    for(uint ee=0;ee<N;ee++)
     idx[ee]+=(1<<e)*(x[ee]<threshold);
    thresholds[e].value=threshold;
    break;
   }
   case -1:{
    //Make integer split
    sint *restrict x=(sint*)(X[E].x);
    sint threshold=x[RINDEX(N)];
    for(uint ee=0;ee<N;ee++)
     idx[ee]+=(1<<e)*(x[ee]<threshold);
    thresholds[e].intValue=threshold;
    break;
   }
   default:{
    //Make categorical split
    uint *restrict x=(uint*)(X[E].x);
    mask mask=RMASK(X[E].numCat);
    for(uint ee=0;ee<N;ee++)
     idx[ee]+=(1<<e)*((mask&(1<<(x[ee])))>0);
    thresholds[e].selection=mask;
   }
  }
 }
 //Calculate scores
 uint objInLeafPerClass[twoToD*numC]; //Counts of classes in a each leaf
 uint objInLeaf[twoToD]; //Counts of objects in each leaf
 uint objInBagPerClass[numC]; //Counts of classess in a bag
 for(uint e=0;e<numC;e++)
  objInBagPerClass[e]=0;
 for(uint e=0;e<twoToD*numC;e++)
  objInLeafPerClass[e]=0;
 for(uint e=0;e<twoToD;e++)
  objInLeaf[e]=0;

 if(!multi){
  //=Many-class case=
  //Count
  for(uint e=0;e<N;e++){
   objInLeafPerClass[Y[e]+idx[e]*numC]+=bag[e];
   objInLeaf[idx[e]]+=bag[e];
   objInBagPerClass[Y[e]]+=bag[e];
  }
  //Calculate the scores
  for(uint e=0;e<twoToD;e++)
   for(uint ee=0;ee<numC;ee++)
    scores[ee+e*numC]=log(
     ((double)objInLeafPerClass[ee+e*numC]+1)/((double)objInLeaf[e]+numC)
     *
     ((double)N+numC)/((double)objInBagPerClass[ee]+1)
    );
  //Fill the OOB scores
  for(uint e=0;e<N;e++)
   for(uint ee=0;ee<numC;ee++)
    oobPrMatrix[e*numC+ee]=scores[idx[e]*numC+ee];
 }else{
  //=Multi-class case=
  //Count
  for(uint e=0;e<N;e++){
   objInLeaf[idx[e]]+=bag[e];
   for(uint ee=0;ee<numC;ee++){
    uint toCount=bag[e]*Y[ee*N+e];
    objInLeafPerClass[ee+idx[e]*numC]+=toCount;
    objInBagPerClass[ee]+=toCount;
   }
  }
  //Calculate the quotient scores
  for(uint e=0;e<twoToD;e++)
   for(uint ee=0;ee<numC;ee++){
    scores[ee+e*numC]=log(
     ((double)objInLeafPerClass[ee+e*numC]+1)/((double)objInLeaf[e]-objInLeafPerClass[ee+e*numC]+1)
     *
     ((double)N-objInBagPerClass[ee]+1)/((double)objInBagPerClass[ee]+1)
    );
   }

  //Fill the OOB quotient scores
  for(uint e=0;e<N;e++)
   for(uint ee=0;ee<numC;ee++)
    oobPrMatrix[e*numC+ee]=scores[idx[e]*numC+ee];
 }
}

void predictFernAdd(PREDSET_,FERN_,double *restrict ans,uint *restrict idx,SIMP_){
 for(uint e=0;e<N;e++) idx[e]=0;
 //ans is a matrix of N columns of length numC
 for(uint e=0;e<D;e++){
  uint E=splitAtts[e];
  switch(X[E].numCat){
   case 0:{
    //Make numerical split
    double *restrict x=(double*)(X[E].x);
    double threshold=thresholds[e].value;
    for(uint ee=0;ee<N;ee++)
     idx[ee]+=(1<<e)*(x[ee]<threshold);
    break;
   }
   case -1:{
    //Make integer split
    sint *restrict x=(sint*)(X[E].x);
    sint threshold=thresholds[e].intValue;
    for(uint ee=0;ee<N;ee++)
     idx[ee]+=(1<<e)*(x[ee]<threshold);
    break;
   }
   default:{
    //Make categorical split
    uint *restrict x=(uint*)(X[E].x);
    mask mask=thresholds[e].selection;
    for(uint ee=0;ee<N;ee++)
     idx[ee]+=(1<<e)*((mask&(1<<(x[ee])))>0);
   }
  }
 }
 //Fill ans with actual predictions
 for(uint e=0;e<N;e++)
  for(uint ee=0;ee<numC;ee++)
   ans[e*numC+ee]+=scores[idx[e]*numC+ee];
}

accLoss calcAccLossConsistent(DATASET_,uint E,FERN_,uint *bag,uint *idx,score_t *curPreds,uint numC,uint D,R_,uint64_t *rngStates,uint *idxP,uint *idxPP){
 //Generate idxP. To this end, implicitly generate a permuted version of the attribute E and build split on it; then
 //replace this split within idx to make a copy of the fern as if it was grown on a permuted E.
 //We also make idxPP as idxP in a plain importance calculation.
 //...yet RINDEX is consistent, i.e. returns the same permutation for the same E; threshold is not
 rng_t zw;
 rng_t *rng2=&zw;
 rng_t *rngO=rng;

 for(uint e=0;e<N;e++) idxPP[e]=(idxP[e]=idx[e]);
 for(uint e=0;e<D;e++) if(splitAtts[e]==E){
  //Re-seed
  ((uint64_t*)rng2)[0]=rngStates[E];

  //Back to business
  switch(X[E].numCat){
   case 0:{
    //Numerical split
    double *x=(double*)(X[E].x);
    double threshold=thresholds[e].value;
    for(uint ee=0;ee<N;ee++){
     rng=rng2;
     idxP[ee]=SET_BIT(idxP[ee],e,x[RINDEX(N)]<threshold);
     rng=rngO;
     idxPP[ee]=SET_BIT(idxPP[ee],e,x[RINDEX(N)]<threshold);
    }
    rng=rngO;
    break;
   }
   case -1:{
    //Integer split
    sint *x=(sint*)(X[E].x);
    sint threshold=thresholds[e].intValue;
    for(uint ee=0;ee<N;ee++){
     rng=rng2;
     idxP[ee]=SET_BIT(idxP[ee],e,x[RINDEX(N)]<threshold);
     rng=rngO;
     idxPP[ee]=SET_BIT(idxPP[ee],e,x[RINDEX(N)]<threshold);
    }
    rng=rngO;
    break;
   }
   default:{
    //Categorical split
    uint *x=(uint*)(X[E].x);
    mask mask=thresholds[e].selection;
    for(uint ee=0;ee<N;ee++){
     rng=rng2;
     idxP[ee]=SET_BIT(idxP[ee],e,GET_BIT(mask,x[RINDEX(N)]));
     rng=rngO;
     idxPP[ee]=SET_BIT(idxPP[ee],e,GET_BIT(mask,x[RINDEX(N)]));
    }
    rng=rngO;
   }
  }
 }
 rng=rngO;

 //Calculate leaves for this permuted fern; first init, ...
 uint twoToD=1<<(D);
 uint objInLeafPerClassP[twoToD*numC]; //Counts of classes in a each leaf
 uint objInLeafP[twoToD]; //Counts of objects in each leaf
 uint objInBagPerClassP[numC]; //Counts of classess in a bag
 for(uint e=0;e<numC;e++) objInBagPerClassP[e]=0;
 for(uint e=0;e<twoToD*numC;e++) objInLeafPerClassP[e]=0;
 for(uint e=0;e<twoToD;e++) objInLeafP[e]=0;
 //...then fill.
 for(uint e=0;e<N;e++){
  objInLeafPerClassP[Y[e]+idxP[e]*numC]+=bag[e];
  objInLeafP[idxP[e]]+=bag[e];
  objInBagPerClassP[Y[e]]+=bag[e];
 }

 //Combine into importance scores
 uint objInBag=0;
 double sumScoreOrig=0.;
 double sumScoreMixed=0.;
 double sumPermScore=0.;
 double sumPermScoreMixed=0.;
 for(uint e=0;e<N;e++){
  //Finish the score on the good class from
  double scoreTrueClassOrig=scores[idx[e]*numC+Y[e]];
  double scoreTrueClassMixed=scores[idxPP[e]*numC+Y[e]];
  double permScoreTrueClass=log(
   ((double)objInLeafPerClassP[Y[e]+idxP[e]*numC]+1)/((double)objInLeafP[idxP[e]]+numC)
   *
   ((double)N+numC)/((double)objInBagPerClassP[Y[e]]+1)
  );
  double permScoreTrueClassMixed=log(
   ((double)objInLeafPerClassP[Y[e]+idxPP[e]*numC]+1)/((double)objInLeafP[idxPP[e]]+numC)
   *
   ((double)N+numC)/((double)objInBagPerClassP[Y[e]]+1)
  );

  sumScoreOrig+=(!(bag[e]))*scoreTrueClassOrig;
  sumScoreMixed+=(!(bag[e]))*scoreTrueClassMixed;
  sumPermScore+=(!(bag[e]))*permScoreTrueClass;
  sumPermScoreMixed+=(!(bag[e]))*permScoreTrueClassMixed;
  objInBag+=!(bag[e]);
 }
 accLoss ans;
 ans.direct=(sumScoreOrig-sumScoreMixed)/((double)objInBag); //The same as in regular importance
 ans.shadow=(sumPermScore-sumPermScoreMixed)/((double)objInBag);
 return(ans);
}

accLoss calcAccLoss(DATASET_,uint E,FERN_,uint *bag,uint *idx,score_t *curPreds,uint numC,uint D,R_,uint *idxPerm){
 //Generate idxPerm, idx for permuted values of an attribute E
 for(uint e=0;e<N;e++) idxPerm[e]=idx[e];
 for(uint e=0;e<D;e++) if(splitAtts[e]==E){
  switch(X[E].numCat){
   case 0:{
    //Numerical split
    double *x=(double*)(X[E].x);
    double threshold=thresholds[e].value;
    for(uint ee=0;ee<N;ee++)
     idxPerm[ee]=SET_BIT(idxPerm[ee],e,x[RINDEX(N)]<threshold);
    break;
   }
   case -1:{
    //Integer split
    sint *x=(sint*)(X[E].x);
    sint threshold=thresholds[e].intValue;
    for(uint ee=0;ee<N;ee++)
     idxPerm[ee]=SET_BIT(idxPerm[ee],e,x[RINDEX(N)]<threshold);
    break;
   }
   default:{
    //Categorical split
    uint *x=(uint*)(X[E].x);
    mask mask=thresholds[e].selection;
    for(uint ee=0;ee<N;ee++)
     idxPerm[ee]=SET_BIT(idxPerm[ee],e,GET_BIT(mask,x[RINDEX(N)]));
   }
  }
 }

 uint objInBag=0;
 double wrongDiff=0;
 for(uint e=0;e<N;e++){
  wrongDiff+=
    (!(bag[e]))*
    (scores[idx[e]*numC+Y[e]]-scores[idxPerm[e]*numC+Y[e]]);
  objInBag+=!(bag[e]);
 }

 accLoss ans;
 ans.direct=wrongDiff/((double)objInBag);
 return(ans);
}
