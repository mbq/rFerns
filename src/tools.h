/*   Shared C code

  Copyright 2011-2014 Miron B. Kursa

  This file is part of rFerns R package.

 rFerns is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 rFerns is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 You should have received a copy of the GNU General Public License along with rFerns. If not, see http://www.gnu.org/licenses/.
*/

#include <stdint.h>
#include <string.h> //For memcpy

typedef uint32_t uint;
typedef uint32_t mask;
typedef int32_t sint;
typedef double score_t;

//Structs
struct parameters{
 uint numClasses;
 uint D;
 uint twoToD;
 uint numFerns;
 uint repOobErrEvery;
 uint holdOobErr;
 uint calcImp;
 uint holdForest;
 uint multilabel;
};
typedef struct parameters params;
#define PARAMS_ params *P
#define SIMP_ uint numC,uint D,uint twoToD,uint multi
#define SIMPP_ SIMP_,uint numFerns
#define _SIMP numC,D,twoToD,multi
#define _SIMPP _SIMP,numFerns
#define _SIMPPQ(x) (x).numClasses,(x).D,(x).twoToD,(x).multilabel,(x).numFerns

union threshold{
 double value;
 sint intValue;
 mask selection;
};
typedef union threshold thresh;

struct ferns{
 //All in series of D elements representing single fern
 uint *splitAtts;
 thresh *thresholds;
 score_t *scores;
};
typedef struct ferns ferns;
#define FERN_ uint *restrict splitAtts,thresh *restrict thresholds,score_t *restrict scores
#define _FERN splitAtts,thresholds,scores
#define _thFERN(e) &((ferns->splitAtts)[(e)*D]),&((ferns->thresholds)[(e)*D]),&((ferns->scores)[(e)*twoToD*numC])

struct attribute{
 void *x;
 sint numCat; // =      0 --> x is numerical and double*
              // =     -1 --> x is numerical and sint*
              //otherwise --> x is sint* and max(x)=numCat-1
};
typedef struct attribute att;
#define DATASET_ att *X,uint nX,uint *Y,uint N
#define _DATASET X,nX,Y,N

#define PREDSET_ att *X,uint nX,uint N
#define _PREDSET X,nX,N

struct model{
 ferns *forest;
 double *oobErr;
 score_t *oobPreds;
 uint *oobOutOfBagC;
 double *imp;
 double *impSd;
};
typedef struct model model;

/*
 For speed, rFerns uses its own PRNG which is seeded from R's PRNG each time an rFerns model is built.
 It is somewhat worse then more computationally demanding PRNGs, though the way it is used makes the difference negligible.
*/
//Internal PRNG -- based on G. Marsaglia's not-better-idea generator
struct rng{
 uint32_t z;
 uint32_t w;
};
typedef struct rng rng_t;
#define znew (((rng->z)=36969*((rng->z)&65535)+((rng->z)>>16))<<16)
#define wnew (((rng->w)=18000*((rng->w)&65535)+((rng->w)>>16))&65535)
#define RINTEGER_MAX (~((uint32_t)0))
#define RINTEGER (znew+wnew)
//Gives a number from [0,1]
#define RUNIF_CLOSED (((double)RINTEGER)*(1./4294967295.))
//Gives a number from [0,1)
#define RUNIF_OPEN (((double)RINTEGER)*(1./4294967296.))
//Gives a number from 0 to upTo-1 (each value equally probable provided upTo is safely smaller than 1<<32)
#define RINDEX(upTo) ((uint32_t)(RUNIF_OPEN*((double)upTo)))
//Setting seed; give it two uint32s you like
#define SETSEED(a,b) rng->z=a; rng->w=b
#define RMASK(numCat) 1+RINDEX((1<<(numCat))-2)
#define R_ rng_t *rng
#define _R rng

//Sync PRNG with R; R will feel only two numbers were generated
#define EMERGE_R_FROM_R \
 GetRNGstate(); \
 uint32_t a=(uint32_t)(((double)(~((uint32_t)0)))*unif_rand()); \
 uint32_t b=(uint32_t)(((double)(~((uint32_t)0)))*unif_rand()); \
 PutRNGstate(); \
  rng_t rngdata; \
 rng_t *rng=&rngdata; \
 SETSEED(a,b)

void makeBagMask(uint *bMask,uint N,R_){
 for(uint e=0;e<N;e++) bMask[e]=0;
 for(uint e=0;e<N;e++){
  bMask[RINDEX(N)]++;
 }
}

uint whichMax(double *where,uint N){
 double curMax=-INFINITY;
 uint ans=0;
 for(uint e=0;e<N;e++)
  if(where[e]>curMax){
   ans=e;
   curMax=where[e];
  }
 return ans;
}

uint whichMaxTieAware(score_t *where,uint N,R_){
 score_t curMax=-INFINITY;
 uint b[N];
 uint be;
 for(uint e=0;e<N;e++)
  if(where[e]>curMax){
   be=0;
   b[be]=e;
   curMax=where[e];
  } else if(where[e]==curMax){
   be++;
   b[be]=e;
  }
 if(!be) return(b[0]);
 return(b[RINDEX(be+1)]);
}

//Memory stuff
#ifdef IN_R
 #define ALLOCN(what,oftype,howmany) oftype* what=(oftype*)R_alloc(sizeof(oftype),(howmany))
 #define ALLOCNZ(what,oftype,howmany) oftype* what=(oftype*)R_alloc(sizeof(oftype),(howmany)); for(uint e_=0;e_<(howmany);e_++) what[e_]=(oftype)0
 #define ALLOC(what,oftype,howmany) what=(oftype*)R_alloc(sizeof(oftype),(howmany))
 #define ALLOCZ(what,oftype,howmany) {what=(oftype*)R_alloc(sizeof(oftype),(howmany));for(uint e_=0;e_<(howmany);e_++) what[e_]=(oftype)0;}
 #define IFFREE(x) //Nothing
 #define FREE(x) //Nothing
 #define CHECK_INTERRUPT R_CheckUserInterrupt()
#else
 #define ALLOCN(what,oftype,howmany) oftype* what=(oftype*)malloc(sizeof(oftype)*(howmany)); if(!(what)) goto allocFailed
 #define ALLOCNZ(what,oftype,howmany) oftype* what=(oftype*)malloc(sizeof(oftype)*(howmany)); if(!(what)) goto allocFailed; for(uint e_=0;e_<(howmany);e_++) what[e_]=(oftype)0
 #define ALLOC(what,oftype,howmany) {what=(oftype*)malloc(sizeof(oftype)*(howmany)); if(!(what)) goto allocFailed;}
 #define ALLOCZ(what,oftype,howmany) {what=(oftype*)malloc(sizeof(oftype)*(howmany)); if(!(what)) goto allocFailed;for(uint e_=0;e_<(howmany);e_++) what[e_]=(oftype)0;}
 #define IFFREE(x) if((x)) free((x))
 #define FREE(x) free(x)
 #define CHECK_INTERRUPT //Nothing
#endif

ferns *allocateFernForest(params *P){
 size_t sizeA=sizeof(ferns);
 size_t sizeB=(P->numFerns)*sizeof(uint)*(P->D);
 size_t sizeC=(P->numFerns)*sizeof(thresh)*(P->D);
 size_t sizeD=(P->numFerns)*sizeof(score_t)*(P->numClasses)*(P->twoToD);
 ferns *ans=(ferns*)malloc(sizeA+sizeB+sizeC+sizeD);
 if(!ans) return NULL;
 ans->splitAtts=(uint*)((char*)ans+sizeA);
 ans->thresholds=(thresh*)((char*)ans+sizeA+sizeB);
 ans->scores=(score_t*)((char*)ans+sizeA+sizeB+sizeC);
 return ans;
}

void killFernForest(ferns *x,params *P){
 IFFREE(x);
}
