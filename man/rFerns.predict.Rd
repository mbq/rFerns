\name{predict.rFerns}
\alias{predict.rFerns}

\title{Prediction with random ferns model}
\description{
  This function predicts classes of new objects with given \code{rFerns} object.
}
\usage{
\method{predict}{rFerns}(object,x,scores=FALSE,...)
}
\arguments{
  \item{object}{Object of a class \code{rFerns}; a model that will be used for prediction.}
  \item{x}{Data frame containing attributes; must have corresponding names to training set (although order is not important) and do not introduce new factor levels.
   If this argument is not given, OOB predictions on the training set will be returned.}
  \item{scores}{If \code{TRUE}, the result will contain score matrix instead of simple predictions.}
  \item{...}{Additional parameters.}
}

\value{
  Predictions. If \code{scores} is \code{TRUE}, a factor vector (for many-class classification) or a logical matrix (for multi-class classification) with predictions, else a matrix with class' scores.
}



\author{Miron B. Kursa \email{M.Kursa@icm.edu.pl}}



\examples{
set.seed(77)

#Fetch Iris data
data(iris)

#Split into tRain and tEst set
iris[c(TRUE,FALSE),]->irisR
iris[c(FALSE,TRUE),]->irisE

#Build model
rFerns(Species~.,data=irisR)->model
print(model)

#Test
predict(model,irisE)->p
print(table(
  Predictions=p,
  True=irisE[["Species"]]))

err<-mean(p!=irisE[["Species"]])
print(paste("Test error",err,sep=" "))

#Show first OOB scores
head(predict(model,scores=TRUE))
}


\keyword{classif}
