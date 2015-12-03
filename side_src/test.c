/*   Stand-alone test of rFerns' C code

     Copyright 2011-2015 Miron B. Kursa

     This file is a redundant addition to the rFerns R package.

 rFerns is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 rFerns is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 You should have received a copy of the GNU General Public License along with rFerns. If not, see http://www.gnu.org/licenses/.
*/


#include <math.h>
#include <stdio.h>
#include <float.h>
#include <stdlib.h>
#include <time.h>

#define PRINT printf

#include "../src/tools.h"
#include "../src/fern.h"
#include "../src/forest.h"

void testByteStuff(){
 uint errors=0;
 if(((uint)(7>6))!=1) errors++;
 if(((uint)(7>=6))!=1) errors++;
 if(((uint)(7==7))!=1) errors++;
 if(((uint)(!0))!=1) errors++;
 if((1<<3)!=8) errors++;
 uint b=(1<<9)+(1<<7);
 if(!(b&(1<<7))) errors++;
 if(errors>0) printf("!!! Your machine or compiler does something pesky about byte operations, so the final code would not be working.\nPlease contact the author.\n");
}

void irisTest(R_){
 #include "iris.h"
 uint N=150;
 uint NN=8;

 params Q;
 Q.numClasses=3;
 Q.D=5;
 Q.twoToD=1<<(Q.D);
 Q.numFerns=500;
 Q.holdOobErr=100;
 Q.repOobErrEvery=50000;
 Q.holdForest=1;
 Q.calcImp=2;
 Q.multilabel=0;

 uint64_t cs;
 ((uint32_t*)&cs)[0]=RINTEGER;
 ((uint32_t*)&cs)[1]=RINTEGER;
 Q.consSeed=cs;
 //Make dataset
 struct attribute *X=malloc(sizeof(struct attribute)*NN);
 uint *y=malloc(sizeof(uint)*N);
 for(uint e=0;e<NN;e++){
  X[e].numCat=0;
  X[e].x=malloc(sizeof(double)*N);
  for(uint ee=0;ee<N;ee++){
    ((double*)(X[e].x))[ee]=IrisByColumn[N*e+ee];
  }
 }
 for(uint e=0;e<50;e++) y[e]=0;
 for(uint e=50;e<100;e++) y[e]=1;
 for(uint e=100;e<150;e++) y[e]=2;

 ferns *ferns=allocateFernForest(&Q);
 uint *yp=malloc(sizeof(uint)*N);


 //** Shadow importance **//
 {

  //Fire classifier
  printf("Making fern forest...\n");
  model *M=makeModel(X,NN,y,N,ferns,&Q,_R);
  printf("Fern forest made.\n");
  double *buf_sans=malloc(sizeof(double)*(Q.numClasses)*N);
  predictWithModelSimple(X,NN,N,M->forest,yp,_SIMPPQ(Q),buf_sans,_R);
  FREE(buf_sans);
  uint wrong=0;
  for(uint e=0;e<N;e++)
   wrong+=(y[e]!=yp[e]);
   //printf("%d %d %d\n",e,y[e],yp[e]);
  printf("Fit error %0.4f.\n",((double)wrong)/((double)N));

  if(M->imp && Q.calcImp==2){
   printf("Imp:\t(sLoss)\t(sha)\t(tries)\n");
   for(uint e=0;e<NN;e++){
    printf("Att%d\t%0.4f\t%0.4f\t%0.0f\n",e,
     M->imp[e],M->shimp[e],M->try[e]);
    if(e==3) printf("=====================================\n");
   }
  }else{
   if(M->imp && Q.calcImp==1){
    printf("Imp:\t(sLoss)\t(tries)\n");
    for(uint e=0;e<NN;e++)
     printf("Att%d\t%0.4f\t%0.0f\n",e,M->imp[e],M->try[e]);
   }
  }
  killModel(M);
 }

 //** Normal importance **//

 {
  Q.calcImp=1;
  //Fire classifier
  printf("Making fern forest...\n");
  model *M=makeModel(X,NN,y,N,ferns,&Q,_R);
  printf("Fern forest made.\n");

  double *buf_sans=malloc(sizeof(double)*(Q.numClasses)*N);
  predictWithModelSimple(X,NN,N,M->forest,yp,_SIMPPQ(Q),buf_sans,_R);
  FREE(buf_sans);
  uint wrong=0;
  for(uint e=0;e<N;e++)
   wrong+=(y[e]!=yp[e]);
   //printf("%d %d %d\n",e,y[e],yp[e]);
  printf("Fit error %0.4f.\n",((double)wrong)/((double)N));

  if(M->imp && Q.calcImp==2){
   printf("Imp:\t(sLoss)\t(sha)\t(tries)\n");
   for(uint e=0;e<NN;e++){
    printf("Att%d\t%0.4f\t%0.4f\t%0.0f\n",e,
     M->imp[e],M->shimp[e],M->try[e]);
    if(e==3) printf("=====================================\n");
   }
  }else{
   if(M->imp && Q.calcImp==1){
    printf("Imp:\t(sLoss)\t(tries)\n");
    for(uint e=0;e<NN;e++){
     printf("Att%d\t%0.4f\t%0.0f\n",e,M->imp[e],M->try[e]);
     if(e==3) printf("=====================================\n");
    }
   }
  }
  killModel(M);
 }

 free(ferns);
 free(y);
 free(yp);
 for(uint e=0;e<NN;e++){
  free(X[e].x);
 }
 free(X);
}

int main(int argc,char **argv){
 srand(time(NULL));
 testByteStuff();
 rng_t rngdata;
 rng_t *rng=&rngdata;
 SETSEED(rand(),rand());
 irisTest(_R);
 return(0);
}
