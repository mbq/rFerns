#    R part of rFerns
#
#    Copyright 2011,2012 Miron B. Kursa
#
#    This file is part of rFerns R package.
#
#rFerns is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
#rFerns is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
#You should have received a copy of the GNU General Public License along with rFerns. If not, see http://www.gnu.org/licenses/.

rFerns<-function(x,...)
	UseMethod("rFerns")

rFerns.formula<-function(formula,data=.GlobalEnv,...){
	#! Warning: this is not an efficient way
	#!          of doing things. Don't use this
	#!          method when not prototyping.
	
	#Convert formula into a data frame
	terms.formula(formula,data=data)->t
	rx<-eval(attr(t,"variables"),data)
	apply(attr(t,"factors"),1,sum)>0->sel
	nam<-rownames(attr(t,"factors"))[sel]
	data.frame(rx[sel])->x; names(x)<-nam
	rx[[attr(t,"response")]]->y
	#Pass to the default method
	rFerns.default(x,y,...)
}

rFerns.matrix<-function(x,y,...){
	#If the input is matrix, data.frame it first
	rFerns.default(data.frame(x),y,...)
}

rFerns.default<-function(x,y,depth=5,ferns=1000,importance=FALSE,reportErrorEvery=0,saveErrorPropagation=FALSE,saveForest=TRUE,...){
	#Stop on bad input
	stopifnot(depth>0 && depth<=16)
	stopifnot(ferns>0)
	if(!is.data.frame(x)) stop("x must be a data frame.")
	if(is.na(names(x)) || any(duplicated(names(x)))) stop("Attribute names must be unique.")
	if(!is.factor(y) || !is.null(dim(y))) stop("y must be a factor vector.")
	if(!all(sapply(x,function(j) any(class(j)%in%c("numeric","integer","factor","ordered"))))) stop("All attributes must be either numeric or factor.")
	if(length(y)!=nrow(x)) stop("Attributes' and decision's sizes must match.")
	if(any((sapply(x,function(a) ((length(levels(a))>30)&&(!is.ordered(a)))))->bad)){
	 stop(sprintf("Attribute(s) %s is/are unordered factor(s) with above 30 levels. Split or convert to ordered.",paste(names(x)[bad],collapse=", ")))
	}
	y<-factor(y)
	
	if(reportErrorEvery<1 || reportErrorEvery>ferns) reportErrorEvery<-ferns+1
	saveOobErr<-ifelse(saveErrorPropagation,1,-1)*reportErrorEvery
	
	Sys.time()->before
	.Call(random_ferns,x,y,
		as.integer(depth[1]),
		as.integer(ferns[1]),
		as.integer(importance[1]),
		as.integer(saveOobErr),
		as.integer(saveForest))->ans
	after<-Sys.time()
	
	#Adjust C output with proper factor levels
	ans$oobPreds<-factor(ans$oobPreds,
					levels=0:(length(levels(y))-1),
					labels=levels(y))
		
	ans$classLabels<-levels(y)
	if(saveForest){
		ans$isStruct<-list()
		lapply(x,levels)->ans$isStruct$predictorLevels
		sapply(x,is.integer)->ans$isStruct$integerPredictors
		sapply(x,is.ordered)->ans$isStruct$orderedFactorPredictors
	}
		
	table(Predicted=ans$oobPreds,True=y)->ans$oobConfusionMatrix
	if(is.null(ans$oobErr)) ans$oobErr<-mean(ans$oobPreds!=y,na.rm=TRUE)
	ans$parameters<-c(classes=length(levels(y)),depth=depth,ferns=ferns)
	
	if(!is.null(ans$importance)){
		ans$importance<-data.frame(matrix(ans$importance,ncol=2))
		names(ans$importance)<-c("MeanScoreLoss","SdScoreLoss")
		if(!is.null(names(x))) rownames(ans$importance)<-names(x)
	}
	
	#Calculate time taken by the calculation
	ans$timeTaken<-after-before
	
	class(ans)<-"rFerns"
	
	return(ans)
}

predict.rFerns<-function(object,x,scores=FALSE,...){
	#Validate input
	if(!("rFerns"%in%class(object))) stop("object must be of a rFerns class")
	if(is.null(object$model)) stop("This fern forest object does not contain the model.")

	iss<-object$isStruct
	if(is.null(iss)){
		#object is a v0.1 rFerns
		object$isStruct$predictorLevels<-object$predictorLevels
		object$isStruct$integerPredictors<-
			object$isStruct$orderedFactorPredictors<-
				rep(FALSE,length(iss$predictorLevels));
	}
	iss$predictorLevels->pL
	pN<-names(pL)
	
	if(!identical(names(x),pN)){
		#Restore x state from training based on x's names
		x[,pN]->x
	}
	
	#Fail for NAs in input
	if(any(is.na(x))) stop("NAs in predictors.")
	
	for(e in 1:ncol(x))
		if(is.null(pL[[e]])){
			if(iss$integerPredictors[e]){
				if(!("integer"%in%class(x[,e]))) stop(sprintf("Attribute %s should be integer.",pN[e]))
			}else{
				if(!("numeric"%in%class(x[,e]))) stop(sprintf("Attribute %s should be numeric.",pN[e]))
			}
		}else{
			if(iss$orderedFactorPredictors[e]){
				#Check if given attribute is also ordered
				if(!is.ordered(x[,e])) stop(sprintf("Attribute %s should be an ordered factor.",pN[e]))
				#Convert levels
				if(!identical(levels(x[,e]),pL[[e]]))
					stop(sprintf("Levels of %s does not match those from training (%s).",pN[e],paste(pL[[e]],collapse=", ")))
			}else{
				#Convert factor levels to be compatible with training
				if(!identical(levels(x[,e]),pL[[e]]))
					x[,e]<-factor(x[,e],levels=pL[[e]])
				#In case of mismatch, NAs will appear -- catch 'em and fail
				if(any(is.na(x[,e]))) stop(sprintf("Levels of %s does not match those from training (%s).",pN[e],paste(pL[[e]],collapse=", ")))
			}
		}
	
	#Prediction itself
	Sys.time()->before
	.Call(random_ferns_predict,x,
		object$model,
		as.integer(object$parameters["depth"]),
		as.integer(object$parameters["ferns"]),
		as.integer(length(object$classLabels)),as.integer(scores)[1])->ans
	after<-Sys.time()
	
	if(scores){
		ans<-data.frame(matrix(ans,ncol=length(object$classLabels),byrow=TRUE)/object$parameters["ferns"])
		object$classLabels->names(ans)
	}else{
		ans<-factor(ans,levels=0:(length(object$classLabels)-1),
			labels=object$classLabels)
	}
	
	#Store the timing
	attr(ans,"timeTaken")<-after-before
	
	return(ans)
}

print.rFerns<-function(x,...){
	#Pretty-print rFerns output
	cat(sprintf("\n Forest of %d ferns of a depth %d.\n\n",
		x$parameters["ferns"],x$parameters["depth"]))
	if(!is.null(x$oobErr)) cat(sprintf(" OOB error %0.2f%%;",tail(x$oobErr,1)*100))
	cat(" OOB confusion matrix:\n"); print(x$oobConfusionMatrix)
	if(any(is.na(x$oobScores))) cat(" Note: forest too small to provide good OOB approx.\n")
}
