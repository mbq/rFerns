#    Naive wrapper for shadow VIM
#
#    Copyright 2011-2016 Miron B. Kursa
#
#    This file is part of rFerns R package.
#
#rFerns is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
#rFerns is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
#You should have received a copy of the GNU General Public License along with rFerns. If not, see http://www.gnu.org/licenses/.


#' Naive feature selection method utilising the rFerns shadow imporance
#'
#' Proof-of-concept ensemble of rFerns models, built to stabilise and improve selection based on shadow importance.
#' It employs a super-ensemble of \code{iterations} small rFerns forests, each built on a subspace of \code{size} attributes, which is selected randomly, but with a higher selection probability for attributes claimed important by previous sub-models.
#' Final selection is a group of attributes which hold a substantial weight at the end of the procedure.
#' @param x Data frame containing attributes; must have unique names and contain only numeric, integer or (ordered) factor columns.
#' Factors must have less than 31 levels. No \code{NA} values are permitted.
#' @param y A decision vector. Must a factor of the same length as \code{nrow(X)} for ordinary many-label classification, or a logical matrix with each column corresponding to a class for multi-label classification.
#' @param iterations Number of iterations i.e., the number of sub-models built.
#' @param depth The depth of the ferns; must be in 1--16 range. Note that time and memory requirements scale with \code{2^depth}.
#' @param ferns Number of ferns to be build in each sub-model. This should be a small number, around 3-5 times \code{size}.
#' @param size Number of attributes considered by each sub-model.
#' @param saveHistory Should weight history be stored.
#' @return An object of class \code{naiveWrapper}, which is a list with the following components:
#' \item{found}{Names of all selected attributes.}
#' \item{weights}{Vector of weights indicating the confidence that certain feature is relevant.}
#' \item{timeTaken}{Time of computation.}
#' \item{weightHistory}{History of weights over all iterations, present if \code{saveHistory} was \code{TRUE}}
#' \item{params}{Copies of algorithm parameters, \code{iterations}, \code{depth}, \code{ferns} and \code{size}, as a named vector.}
#' @author Miron B. Kursa
#' @references Kursa MB (2017). \emph{Efficient all relevant feature selection with random ferns}. In: Kryszkiewicz M., Appice A., Slezak D., Rybinski H., Skowron A., Ras Z. (eds) Foundations of Intelligent Systems. ISMIS 2017. Lecture Notes in Computer Science, vol 10352. Springer, Cham.
#' @examples
#' set.seed(77);
#' #Fetch Iris data
#' data(iris)
#' #Extend with random noise
#' noisyIris<-cbind(iris[,-5],apply(iris[,-5],2,sample));
#' names(noisyIris)[5:8]<-sprintf("Nonsense%d",1:4)
#' #Execute selection
#' naiveWrapper(noisyIris,iris$Species,iterations=50,ferns=20,size=8)
#' @export
naiveWrapper<-function(x,y,iterations=1000,depth=5,ferns=100,size=30,saveHistory=FALSE){

 if(size>ncol(x)){
  size<-ncol(x);
  warning(sprintf("size parameter limited to ncol(X)=%d",ncol(x)));
 }
 if(any(duplicated(names(x)))) stop("Cannot accept duplicated column names in x.");

 Sys.time()->b;

 wh<-NULL
 weight<-rep(1,ncol(x)); names(weight)<-names(x);
 for(iter in 1:iterations){
  weight[weight<.1]<-.1;
  if(saveHistory) cbind(wh,weight)->wh
  use<-sample(names(x),size,prob=weight/sum(weight));
  rFerns(x[,use,drop=FALSE],y,
   ferns=ferns,
   depth=depth,
   imp="sha")$importance->ii;
  found<-rownames(ii)[ii$MeanScoreLoss>max(ii$Shadow)];
  weight[found]<-weight[found]+5;
  weight[use[!(use%in%found)]]<-weight[use[!(use%in%found)]]-10;
 }
 weight[weight<.1]<-.1;

 found<-names(weight)[weight>max(c(weight,33))/2];

 Sys.time()->a;

 ans<-list(
  found=found,
  weight=weight,
  timeTaken=a-b,
  weightHistory=wh,
  parameters=c(
   iterations=iterations,
   depth=depth,
   ferns=ferns,
   size=size
  )
 )
 class(ans)<-"naiveWrapper"
 ans
}

#' @method print naiveWrapper
#' @export
print.naiveWrapper<-function(x,...){
 cat("Naive shadow importance feature selection\n");
 if(length(x$found)==0){
  cat(" No attributes selected.\n");
 }else{
  cat(strwrap(sprintf(" %d attributes selected: %s.\n",
   length(x$found),
   paste(x$found,collapse=", ")
  )));
 }
 cat("\n");
}
