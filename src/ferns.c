/*   R frontend to C code

     Copyright 2011-2014 Miron B. Kursa

     This file is part of rFerns R package.

 rFerns is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 rFerns is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 You should have received a copy of the GNU General Public License along with rFerns. If not, see http://www.gnu.org/licenses/.
*/

#include <R.h>
#include <Rdefines.h>
#include <Rinternals.h>
#include <R_ext/Utils.h>

#define PRINT Rprintf
#define IN_R 7

#include "tools.h"
#include "fern.h"
#include "forest.h"

void loadAttributes(SEXP sAttributes,struct attribute **X,uint *nAtt,uint *nObj){
 //We assume sAttributes is a data.frame, so a list of attributes
 nAtt[0]=length(sAttributes);
 nObj[0]=length(VECTOR_ELT(sAttributes,0));
 X[0]=(struct attribute*)R_alloc(sizeof(struct attribute),nAtt[0]);
 for(uint e=0;e<nAtt[0];e++){
  SEXP xe=VECTOR_ELT(sAttributes,e);
  switch(TYPEOF(xe)){
   case REALSXP:
    X[0][e].numCat=0;
    X[0][e].x=(void*)REAL(xe);
   break;
   case INTSXP:
    X[0][e].numCat=length(getAttrib(xe,R_LevelsSymbol));
    if(X[0][e].numCat==0) X[0][e].numCat=-1;
    X[0][e].x=(void*)INTEGER(xe);
   break;
   default:
   error("Bad input in predictors!");
  }
 }

}

SEXP random_ferns(SEXP sAttributes,SEXP sDecision,SEXP sD,SEXP sNumFerns,SEXP sCalcImp,SEXP sOobErrEvery,SEXP sHoldForest,SEXP sMultilabel){
 struct attribute *X;
 uint nAtt,nObj,nClass,*Y;
 uint multi=INTEGER(sMultilabel)[0];
 loadAttributes(sAttributes,&X,&nAtt,&nObj);
 if(!multi){
  nClass=length(getAttrib(sDecision,R_LevelsSymbol));
  //Sadly, we need to copy
  Y=(uint*)R_alloc(sizeof(uint),nObj);
  for(uint e=0;e<nObj;e++)
   Y[e]=INTEGER(sDecision)[e]-1;
 }else{
  nClass=length(sDecision)/nObj;
  Y=(uint*)R_alloc(sizeof(uint),nObj*nClass);
  for(uint e=0;e<nObj*nClass;e++)
   Y[e]=LOGICAL(sDecision)[e];
 }

 //Data loaded, time to load parameters
 params Q;
 Q.numClasses=nClass;
 Q.D=INTEGER(sD)[0];
 Q.twoToD=1<<(Q.D);
 Q.numFerns=INTEGER(sNumFerns)[0];
 Q.repOobErrEvery=abs(INTEGER(sOobErrEvery)[0]);
 Q.holdOobErr=INTEGER(sOobErrEvery)[0]>0;
 Q.calcImp=INTEGER(sCalcImp)[0];
 Q.holdForest=INTEGER(sHoldForest)[0];
 Q.multilabel=multi;

 //Allocating fern forest; the whole space is controlled by R
 ferns ferns;
 SEXP sfSplitAtts;
 SEXP sfScores;
 if(Q.holdForest){
  //To store the forest, we allocate vectors which will contain it
  // and build ferns out of their buffers. The rest is in saving forest.
  PROTECT(sfSplitAtts=allocVector(INTSXP,(Q.D)*(Q.numFerns)));
  ferns.splitAtts=INTEGER(sfSplitAtts);
  ferns.thresholds=(thresh*)R_alloc(sizeof(thresh),(Q.D)*(Q.numFerns));
  PROTECT(sfScores=allocVector(REALSXP,(Q.twoToD)*(Q.numClasses)*(Q.numFerns)));
  ferns.scores=(score_t*)REAL(sfScores);
 }else{
  //In the opposite case, we allocate a chunk for 1-fern forest on GC heap
  size_t sizeA=sizeof(uint)*(Q.D);
  size_t sizeB=sizeof(thresh)*(Q.D);
  size_t sizeC=sizeof(double)*(Q.numClasses)*(Q.twoToD);
  void *buf=R_alloc(1,sizeA+sizeB+sizeC);
  ferns.splitAtts=(uint*)((void*)buf);
  ferns.thresholds=(thresh*)((void*)buf+sizeA);
  ferns.scores=(score_t*)((void*)buf+sizeA+sizeB);
 }
 //Now, let's make the RNG and seed from R's RNG
 EMERGE_R_FROM_R;

 //Fire the code
 model *M=makeModel(X,nAtt,Y,nObj,&ferns,&Q,_R);

 //Start composing answer
 SEXP sAns; PROTECT(sAns=allocVector(VECSXP,5));

 //Saving forest
 if(Q.holdForest){
  SEXP sfThreReal; PROTECT(sfThreReal=allocVector(REALSXP,(Q.D)*(Q.numFerns)));
  SEXP sfThreInt; PROTECT(sfThreInt=allocVector(INTSXP,(Q.D)*(Q.numFerns)));
  for(uint e=0;e<(Q.D)*(Q.numFerns);e++){
   if(X[ferns.splitAtts[e]].numCat!=0){
    INTEGER(sfThreInt)[e]=ferns.thresholds[e].selection;
    REAL(sfThreReal)[e]=NAN;
   }else{
    INTEGER(sfThreInt)[e]=-1;
    REAL(sfThreReal)[e]=ferns.thresholds[e].value;
   }
  }
  SEXP sModel; PROTECT(sModel=allocVector(VECSXP,4));
  SET_VECTOR_ELT(sModel,0,sfSplitAtts);
  SET_VECTOR_ELT(sModel,1,sfThreReal);
  SET_VECTOR_ELT(sModel,2,sfThreInt);
  SET_VECTOR_ELT(sModel,3,sfScores);
  SEXP sModelNames; PROTECT(sModelNames=NEW_CHARACTER(4));
  SET_STRING_ELT(sModelNames,0,mkChar("splitAttIdxs"));
  SET_STRING_ELT(sModelNames,1,mkChar("threReal"));
  SET_STRING_ELT(sModelNames,2,mkChar("threInteger"));
  SET_STRING_ELT(sModelNames,3,mkChar("scores"));
  setAttrib(sModel,R_NamesSymbol,sModelNames);
  SET_VECTOR_ELT(sAns,0,sModel);
  UNPROTECT(6);
 }else{
  SET_VECTOR_ELT(sAns,0,R_NilValue);
 }

 //Currently it always happens
 if(M->oobPreds){
  //Build score matrix for R, with NAs for object which were never OOB
  SEXP sOobScores; PROTECT(sOobScores=allocVector(REALSXP,(Q.numClasses)*nObj));
  SEXP sOobDim; PROTECT(sOobDim=allocVector(INTSXP,2));
  INTEGER(sOobDim)[0]=Q.numClasses;
  INTEGER(sOobDim)[1]=nObj;
  double *tmp=REAL(sOobScores);
  for(uint e=0;e<nObj;e++)
   if(M->oobOutOfBagC[e])
    for(uint ee=0;ee<Q.numClasses;ee++)
     tmp[e*Q.numClasses+ee]=M->oobPreds[e*Q.numClasses+ee];
   else
    for(uint ee=0;ee<Q.numClasses;ee++)
     tmp[e*Q.numClasses+ee]=NA_REAL;
  setAttrib(sOobScores,R_DimSymbol,sOobDim);
  SET_VECTOR_ELT(sAns,1,sOobScores);
  UNPROTECT(2);

  if(!multi){
   //Do actual voting on this matrix; push NA for never-oobs and
   //random-of-max for ties.
   SEXP sOobPreds; PROTECT(sOobPreds=allocVector(INTSXP,nObj));
   sint *winningClass=INTEGER(sOobPreds);

   uint bestIdx[Q.numClasses];
   for(uint e=0;e<nObj;e++)
    if(M->oobOutOfBagC[e]){
     winningClass[e]=whichMaxTieAware(&(M->oobPreds[e*Q.numClasses]),Q.numClasses,_R);
    } else winningClass[e]=NA_INTEGER;

   SET_VECTOR_ELT(sAns,4,sOobPreds);
   UNPROTECT(1);
  }else{
   SET_VECTOR_ELT(sAns,4,R_NilValue);
  }
 }else{
  SET_VECTOR_ELT(sAns,1,R_NilValue);
  SET_VECTOR_ELT(sAns,4,R_NilValue);
 }

 if(M->oobErr){
  SEXP sOobErr; PROTECT(sOobErr=allocVector(REALSXP,(Q.numFerns)));
  double *tmp=REAL(sOobErr);
  for(uint e=0;e<(Q.numFerns);e++)
   tmp[e]=M->oobErr[e];
  SET_VECTOR_ELT(sAns,2,sOobErr);
  UNPROTECT(1);
 }else{
  SET_VECTOR_ELT(sAns,2,R_NilValue);
 }

 if(M->imp){
  SEXP sImp; PROTECT(sImp=allocVector(REALSXP,nAtt*2));
  double *tmp=REAL(sImp);
  for(uint e=0;e<nAtt;e++)
   tmp[e]=M->imp[e];
  for(uint e=0;e<nAtt;e++)
   tmp[e+nAtt]=M->impSd[e];
  SET_VECTOR_ELT(sAns,3,sImp);
  UNPROTECT(1);
 }else{
  SET_VECTOR_ELT(sAns,3,R_NilValue);
 }

 //Set names
 SEXP sAnsNames;
 PROTECT(sAnsNames=NEW_CHARACTER(5));
 SET_STRING_ELT(sAnsNames,0,mkChar("model"));
 SET_STRING_ELT(sAnsNames,1,mkChar("oobScores"));
 SET_STRING_ELT(sAnsNames,2,mkChar("oobErr"));
    SET_STRING_ELT(sAnsNames,3,mkChar("importance"));
    SET_STRING_ELT(sAnsNames,4,mkChar("oobPreds"));
 setAttrib(sAns,R_NamesSymbol,sAnsNames);
 UNPROTECT(2);

 killModel(M);
 return(sAns);
}

SEXP random_ferns_predict(SEXP sAttributes,SEXP sModel,SEXP sD,SEXP sNumFerns,SEXP sNumClasses,SEXP sMode,SEXP sMultilabel){
 struct attribute *X;
 uint nAtt,nObj;
 loadAttributes(sAttributes,&X,&nAtt,&nObj);

 //Data loaded, time to load parameters
 params Q;
 uint nClass=INTEGER(sNumClasses)[0];
 uint multi=INTEGER(sMultilabel)[0];
 Q.numClasses=nClass;
 Q.D=INTEGER(sD)[0];
 Q.twoToD=1<<(Q.D);
 Q.numFerns=INTEGER(sNumFerns)[0];
 Q.multilabel=multi;

 //Deciphering model -- WARNING, order of Model list is SIGNIFICANT!
 ferns ferns;
 ferns.splitAtts=INTEGER(VECTOR_ELT(sModel,0));
 ferns.scores=(score_t*)REAL(VECTOR_ELT(sModel,3));
 sint *tI=INTEGER(VECTOR_ELT(sModel,2));
 double *tR=REAL(VECTOR_ELT(sModel,1));
 ferns.thresholds=(thresh*)R_alloc(sizeof(thresh),(Q.D)*(Q.numFerns));
 for(uint e=0;e<(Q.D)*(Q.numFerns);e++)
  if(!ISNAN(tR[e]))
   ferns.thresholds[e].value=tR[e];
  else
   ferns.thresholds[e].selection=tI[e];
 if(INTEGER(sMode)[0]==0 && !multi){
  EMERGE_R_FROM_R;

  //TODO: if(multi) allocate more for answer and don't use random

  SEXP sAns; PROTECT(sAns=allocVector(INTSXP,nObj));
  sint *yp=INTEGER(sAns);
  double *buf_sans=(double*)R_alloc(sizeof(double),(Q.numClasses)*nObj);
  predictWithModelSimple(X,nAtt,nObj,&ferns,(uint*)yp,_SIMPPQ(Q),buf_sans,_R);
  UNPROTECT(1);
  return(sAns);
 }else{
  SEXP sAns; PROTECT(sAns=allocVector(REALSXP,nObj*(Q.numClasses)));
  double *yp=REAL(sAns);
  uint *buf_idx=(uint*)R_alloc(sizeof(double),nObj);
  predictWithModelScores(X,nAtt,nObj,&ferns,(double*)yp,_SIMPPQ(Q),buf_idx);
  UNPROTECT(1);
  return(sAns);
 }
}
