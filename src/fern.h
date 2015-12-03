/*   Code for making/predicting by single fern

     Copyright 2011,2012 Miron B. Kursa

     This file is part of rFerns R package.
 
 rFerns is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 rFerns is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 You should have received a copy of the GNU General Public License along with rFerns. If not, see http://www.gnu.org/licenses/.
*/

void makeFern(DATASET_,FERN_,uint *restrict bag,score_t *restrict oobPrMatrix,uint *restrict idx,SIMP_,R_){
	uint counts[twoToD*numC];
	uint objInLeaf[twoToD];
	for(uint e=0;e<N;e++) idx[e]=0;
	//counts is a matrix of 2^D columns of length nClass
	for(uint e=0;e<D;e++){
		//Select an attribute to make a split on
		uint E=splitAtts[e]=RINDEX(nX);
		switch(X[E].numCat){
			case 0:{
				//Make numerical split
				double  * restrict x=(double*)(X[E].x);
				double threshold=.5*(x[RINDEX(N)]+x[RINDEX(N)]);
				for(uint ee=0;ee<N;ee++)
					idx[ee]+=(1<<e)*(x[ee]<threshold);
				thresholds[e].value=threshold;
				break;
			}
			case -1:{
				//Make integer split
				sint  *restrict x=(sint*)(X[E].x);
				sint threshold=x[RINDEX(N)];
				for(uint ee=0;ee<N;ee++)
					idx[ee]+=(1<<e)*(x[ee]<threshold);
				thresholds[e].intValue=threshold;
				break;
			}
			default:{
				//Make categorical split
				uint  *restrict x=(uint*)(X[E].x);
				mask mask=RMASK(X[E].numCat);
				for(uint ee=0;ee<N;ee++)
					idx[ee]+=(1<<e)*((mask&(1<<(x[ee])))>0);
				thresholds[e].selection=mask;
			}
		}		
	}
	//Count classes' distribution in each leaf
        uint apriori[numC];
        for(uint e=0;e<numC;e++) apriori[e]=1; //Dirichlet prior
	for(uint e=0;e<twoToD*numC;e++) counts[e]=1; //Dirichlet prior
	for(uint e=0;e<twoToD;e++) objInLeaf[e]=numC; //An effect of Dirichlet prior
	for(uint e=0;e<N;e++){
		counts[Y[e]+idx[e]*numC]+=bag[e];
		objInLeaf[idx[e]]+=bag[e];
                apriori[Y[e]]+=bag[e];
	}
	uint aprioriAll=0;
        for(uint e=0;e<numC;e++) aprioriAll+=apriori[e];
	for(uint e=0;e<twoToD;e++)
		for(uint ee=0;ee<numC;ee++)
			scores[ee+e*numC]=log(((double)counts[ee+e*numC])/((double)objInLeaf[e])*((double)numC)/((double)apriori[ee])*((double)aprioriAll));
	
	for(uint e=0;e<N;e++)
		for(uint ee=0;ee<numC;ee++)
			oobPrMatrix[e*numC+ee]=scores[idx[e]*numC+ee];
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
