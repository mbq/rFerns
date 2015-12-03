/*   Stand-alone test of rFerns' C code

     Copyright 2011-2014 Miron B. Kursa

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
 const double IrisByColumn[]={5.1,4.9,4.7,4.6,5,5.4,4.6,5,4.4,4.9,5.4,
 4.8,4.8,4.3,5.8,5.7,5.4,5.1,5.7,5.1,5.4,5.1,4.6,5.1,4.8,5,5,5.2,5.2,
 4.7,4.8,5.4,5.2,5.5,4.9,5,5.5,4.9,4.4,5.1,5,4.5,4.4,5,5.1,4.8,5.1,4.6,
 5.3,5,7,6.4,6.9,5.5,6.5,5.7,6.3,4.9,6.6,5.2,5,5.9,6,6.1,5.6,6.7,5.6,5.8,
 6.2,5.6,5.9,6.1,6.3,6.1,6.4,6.6,6.8,6.7,6,5.7,5.5,5.5,5.8,6,5.4,6,6.7,
 6.3,5.6,5.5,5.5,6.1,5.8,5,5.6,5.7,5.7,6.2,5.1,5.7,6.3,5.8,7.1,6.3,6.5,
 7.6,4.9,7.3,6.7,7.2,6.5,6.4,6.8,5.7,5.8,6.4,6.5,7.7,7.7,6,6.9,5.6,7.7,
 6.3,6.7,7.2,6.2,6.1,6.4,7.2,7.4,7.9,6.4,6.3,6.1,7.7,6.3,6.4,6,6.9,6.7,
 6.9,5.8,6.8,6.7,6.7,6.3,6.5,6.2,5.9,3.5,3,3.2,3.1,3.6,3.9,3.4,3.4,2.9,
 3.1,3.7,3.4,3,3,4,4.4,3.9,3.5,3.8,3.8,3.4,3.7,3.6,3.3,3.4,3,3.4,3.5,3.4,
 3.2,3.1,3.4,4.1,4.2,3.1,3.2,3.5,3.6,3,3.4,3.5,2.3,3.2,3.5,3.8,3,3.8,3.2,
 3.7,3.3,3.2,3.2,3.1,2.3,2.8,2.8,3.3,2.4,2.9,2.7,2,3,2.2,2.9,2.9,3.1,3,2.7,
 2.2,2.5,3.2,2.8,2.5,2.8,2.9,3,2.8,3,2.9,2.6,2.4,2.4,2.7,2.7,3,3.4,3.1,2.3,
 3,2.5,2.6,3,2.6,2.3,2.7,3,2.9,2.9,2.5,2.8,3.3,2.7,3,2.9,3,3,2.5,2.9,2.5,
 3.6,3.2,2.7,3,2.5,2.8,3.2,3,3.8,2.6,2.2,3.2,2.8,2.8,2.7,3.3,3.2,2.8,3,2.8,
 3,2.8,3.8,2.8,2.8,2.6,3,3.4,3.1,3,3.1,3.1,3.1,2.7,3.2,3.3,3,2.5,3,3.4,3,
 1.4,1.4,1.3,1.5,1.4,1.7,1.4,1.5,1.4,1.5,1.5,1.6,1.4,1.1,1.2,1.5,1.3,1.4,1.7,
 1.5,1.7,1.5,1,1.7,1.9,1.6,1.6,1.5,1.4,1.6,1.6,1.5,1.5,1.4,1.5,1.2,1.3,1.4,
 1.3,1.5,1.3,1.3,1.3,1.6,1.9,1.4,1.6,1.4,1.5,1.4,4.7,4.5,4.9,4,4.6,4.5,4.7,
 3.3,4.6,3.9,3.5,4.2,4,4.7,3.6,4.4,4.5,4.1,4.5,3.9,4.8,4,4.9,4.7,4.3,4.4,4.8,
 5,4.5,3.5,3.8,3.7,3.9,5.1,4.5,4.5,4.7,4.4,4.1,4,4.4,4.6,4,3.3,4.2,4.2,4.2,4.3,
 3,4.1,6,5.1,5.9,5.6,5.8,6.6,4.5,6.3,5.8,6.1,5.1,5.3,5.5,5,5.1,5.3,5.5,6.7,6.9,5,
 5.7,4.9,6.7,4.9,5.7,6,4.8,4.9,5.6,5.8,6.1,6.4,5.6,5.1,5.6,6.1,5.6,5.5,4.8,5.4,5.6,
 5.1,5.1,5.9,5.7,5.2,5,5.2,5.4,5.1,0.2,0.2,0.2,0.2,0.2,0.4,0.3,0.2,0.2,0.1,0.2,0.2,
 0.1,0.1,0.2,0.4,0.4,0.3,0.3,0.3,0.2,0.4,0.2,0.5,0.2,0.2,0.4,0.2,0.2,0.2,0.2,0.4,0.1,
 0.2,0.2,0.2,0.2,0.1,0.2,0.2,0.3,0.3,0.2,0.6,0.4,0.3,0.2,0.2,0.2,0.2,1.4,1.5,1.5,1.3,1.5,
 1.3,1.6,1,1.3,1.4,1,1.5,1,1.4,1.3,1.4,1.5,1,1.5,1.1,1.8,1.3,1.5,1.2,1.3,1.4,1.4,1.7,1.5,1,
 1.1,1,1.2,1.6,1.5,1.6,1.5,1.3,1.3,1.3,1.2,1.4,1.2,1,1.3,1.2,1.3,1.3,1.1,1.3,2.5,1.9,
 2.1,1.8,2.2,2.1,1.7,1.8,1.8,2.5,2,1.9,2.1,2,2.4,2.3,1.8,2.2,2.3,1.5,2.3,2,2,1.8,
 2.1,1.8,1.8,1.8,2.1,1.6,1.9,2,2.2,1.5,1.4,2.3,2.4,1.8,1.8,2.1,2.4,2.3,1.9,2.3,
 2.5,2.3,1.9,2,2.3,1.8};

 uint N=150;
 uint NN=4;

 params Q;
 Q.numClasses=3;
 Q.D=4;
 Q.twoToD=1<<(Q.D);
 Q.numFerns=100000;
 Q.holdOobErr=100;
 Q.repOobErrEvery=50000;
 Q.holdForest=1;
 Q.calcImp=1;
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

 //Fire classifier
 printf("Making fern forest...\n");
 model *M=makeModel(X,NN,y,N,ferns,&Q,_R);
 printf("Fern forest made.\n");
 uint *yp=malloc(sizeof(uint)*N);
 double *buf_sans=malloc(sizeof(double)*(Q.numClasses)*N);
 predictWithModelSimple(X,NN,N,M->forest,yp,_SIMPPQ(Q),buf_sans,_R);
 FREE(buf_sans);
 uint wrong=0;
 for(uint e=0;e<N;e++)
  wrong+=(y[e]!=yp[e]);
  //printf("%d %d %d\n",e,y[e],yp[e]);
 printf("Fit error %0.4f.\n",((double)wrong)/((double)N));

 if(M->imp){
  printf("Importances: (mean) (sd) (Z)\n");
  for(uint e=0;e<NN;e++)
   printf("Att%d %0.4f %0.4f %0.4f\n",e,M->imp[e],M->impSd[e],(M->imp[e])/(M->impSd[e]));
 }
 killModel(M); free(ferns);
 free(y);
 free(yp);
 for(uint e=0;e<NN;e++){
  free(X[e].x);
 }
 free(X);
}

uint main(uint argc,char **argv){
 srand(time(NULL));
 testByteStuff();
 rng_t rngdata;
 rng_t *rng=&rngdata;
 SETSEED(rand(),rand());
 irisTest(_R);
 return(0);
}
