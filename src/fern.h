/*   Code for making/predicting by single fern

     Copyright 2011-2014 Miron B. Kursa

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
  objInBagPerClass[e]=1;
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
