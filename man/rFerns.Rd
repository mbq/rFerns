\name{rFerns}
\alias{rFerns}
\alias{rFerns.formula}
\alias{rFerns.matrix}
\alias{rFerns.default}
\alias{print.rFerns}

\title{Classification with random ferns}
\description{
  This function builds a random ferns model on the given training data.
}
\usage{
\method{rFerns}{formula}(formula,data=.GlobalEnv,...)
\method{rFerns}{matrix}(x,y,...)
\method{rFerns}{default}(x,y,depth=5,ferns=1000,importance=FALSE,
  reportErrorEvery=0,saveErrorPropagation=FALSE,saveForest=TRUE,...)
\method{print}{rFerns}(x,...)
}
\arguments{
  \item{x}{Data frame containing attributes; must have unique names and contain only numeric, integer or (ordered) factor columns. Factors must have less than 31 levels. No \code{NA} values are permitted.}
  \item{y}{A decision vector. Must a factor of the same length as \code{nrow(X)} for ordinary many-label classification, or a logical matrix with each column corresponding to a class for multi-label classification.}
  \item{formula}{Formula representation of the model.}
  \item{data}{Frame containing formula elements.}
  \item{depth}{The depth of the ferns; must be in 1--16 range. Note that time and memory requirements scale with \code{2^depth}.}
  \item{ferns}{Number of ferns to be build.}
  \item{importance}{Should the attribute importance be calculated?}
  \item{reportErrorEvery}{If set to a number larger than 0, current OOB error approximation will be printed every time a chunk of \code{reportErrorEvery} ferns is finished.}
  \item{saveErrorPropagation}{Should the OOB error approximation be calculated after each ferns was created and saved? Setting to \code{FALSE} may improve performance}
  \item{saveForest}{Should the model be saved? It must be \code{TRUE} if you want to use the model for prediction; however, if you are interested in importance or OOB error only, setting it to \code{FALSE} significantly improves memory requirements, especially for large \code{depth} and \code{ferns}.}
  \item{...}{For formula and matrix methods, a place to state parameters to be passed to default method. For the print method, arguments to be passed to \code{print}.}
}

\value{
  An object of class \code{rFerns}, which is a list with the  following components:
  \item{model}{The built model; \code{NULL} if \code{saveForest} was \code{FALSE}.}
  \item{oobErr}{OOB approximation of accuracy. Ignores never-OOB-tested objects (see oobScores element).}
  \item{importance}{The importance scores or \code{NULL} if \code{importance} was set to \code{FALSE}. In a first case it is a \code{data.frame} with two columns: \code{MeanScoreLoss} which is the main importance score for each attribute and \code{SdScoreLoss} which is its standard deviation over ferns which used certain attribute. The \code{rownames} are set and equal to the \code{names(x)}.}
  \item{oobScores}{A matrix of OOB scores of each class for each object in training set. Rows correspond to classes in the same order as in \code{levels(Y)}. If the \code{ferns} is too small, some columns may contain \code{NA}s, what means that certain objects were never in test set.}
  \item{oobPreds}{A vector of OOB predictions of class for each object in training set. Never-OOB-tested objects (see above) have predictions equal to \code{NA}.}
  \item{oobConfusionMatrix}{Confusion matrix build from \code{oobPreds} and \code{y}.}
  \item{timeTaken}{Time used to train the model (smaller than wall time because data preparation and model final touches are excluded; however it includes the time needed to compute importance, if it applies). An object of \code{difftime} class.}
  \item{parameters}{Numerical vector of three elements: \code{classes}, \code{depth} and \code{ferns}, containing respectively the number of classes in decision and copies of \code{depth} and \code{ferns} parameters.}
  \item{classLabels}{Copy of \code{levels(Y)} after purging unused levels.}
  \item{isStruct}{Copy of the train set structure, required internally by predict method.}
}

\note{
  The unused levels of the decision will be removed; on the other hand unused levels of categorical attributes will be preserved, so that they could be present in the data later predicted with the model.

  The levels of ordered factors in training and predicted data must be identical.

  Do not use formula interface for a data with large number of attributes; the overhead from handling the formula may be significant.
}

\references{
Ozuysal M, Calonder M, Lepetit V & Fua P. (2010), \emph{Fast Keypoint Recognition using Random Ferns}, IEEE Transactions on Pattern Analysis and Machine Intelligence 32(3), 448-461.

Kursa MB (2012), \emph{Random ferns method implementation for the general-purpose machine learning}, arXiv:1202.1121 [cs.LG].
}

\author{Miron B. Kursa \email{M.Kursa@icm.edu.pl}}

\seealso{\code{\link{predict.rFerns}}}

\examples{
set.seed(77);

#Fetch Iris data
data(iris)

#Build model
rFerns(Species~.,data=iris)

##Importance
rFerns(Species~.,data=iris,importance=TRUE)->model
print(model$imp)
}


\keyword{classif}
