/*   Code for making/predicting by single fern

     Copyright 2011 Miron B. Kursa

     This file is part of rFerns R package.
 
 rFerns is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 rFerns is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 You should have received a copy of the GNU General Public License along with rFerns. If not, see http://www.gnu.org/licenses/.
*/

void makeFern(DATASET_,FERN_,uint *bag,score_t *oobPrMatrix,uint *idx,SIMP_,R_){
	uint counts[twoToD*numC];
	uint objInLeaf[twoToD];
	for(uint e=0;e<N;e++) idx[e]=0;
	//counts is a matrix of 2^D columns of length nClass
	for(uint e=0;e<D;e++){
		//Select an attribute to make a split on
		uint E=splitAtts[e]=RINDEX(nX);
		if(X[E].numCat==0){
			//Make numerical split
			double *x=(double*)(X[E].x);
			double threshold=.5*(x[RINDEX(N)]+x[RINDEX(N)]);
			for(uint ee=0;ee<N;ee++)
				idx[ee]+=(1<<e)*(x[ee]<threshold);
			thresholds[e].value=threshold;
		}else{
			//Make categorical split
			uint *x=(uint*)(X[E].x);
			mask mask=RMASK(X[E].numCat);
			for(uint ee=0;ee<N;ee++)
				idx[ee]+=(1<<e)*((mask&(1<<(x[ee])))>0);
			thresholds[e].selection=mask;
		}
	}
	//Count classes' distribution in each leaf
	for(int e=0;e<twoToD*numC;e++) counts[e]=1; //Dirichlet prior
	for(int e=0;e<twoToD;e++) objInLeaf[e]=numC; //An effect of Dirichlet prior
	for(int e=0;e<N;e++){
		counts[Y[e]+idx[e]*numC]+=bag[e];
		objInLeaf[idx[e]]+=bag[e];
	}
	 
	for(uint e=0;e<twoToD;e++)
		for(uint ee=0;ee<numC;ee++)
			scores[ee+e*numC]=log(((double)counts[ee+e*numC])/((double)objInLeaf[e])*((double)numC));
	
	for(uint e=0;e<N;e++)
		for(uint ee=0;ee<numC;ee++)
			oobPrMatrix[e*numC+ee]=scores[idx[e]*numC+ee];
}

void predictFernAdd(PREDSET_,FERN_,double *ans,uint *idx,SIMP_){
	for(uint e=0;e<N;e++) idx[e]=0;
	//ans is a matrix of N columns of length numC
	for(uint e=0;e<D;e++){
		uint sA=splitAtts[e];
		if(X[sA].numCat==0){
			//Predict from continous split
			double *x=(double*)(X[sA].x);
			double threshold=thresholds[e].value;
			for(uint ee=0;ee<N;ee++)
				idx[ee]+=(1<<e)*(x[ee]<threshold);
		}else{
			//Predict from categorical split
			uint *x=(uint*)(X[sA].x);
			mask mask=thresholds[e].selection;
			for(uint ee=0;ee<N;ee++)
				idx[ee]+=(1<<e)*((mask&(1<<(x[ee])))>0);
		}
	}
	//Fill ans with actual predictions
	for(uint e=0;e<N;e++)
		for(uint ee=0;ee<numC;ee++)
			ans[e*numC+ee]+=scores[idx[e]*numC+ee];
}
