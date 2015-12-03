context("Multi-class model building");

test_that("Multi-class model builds",{
 cbind(
  setosa=rep(c(T,F,F),each=50),
  versicolor=rep(c(F,T,F),each=50),
  virginica=rep(c(F,F,T),each=50))->iris_multi_decision;
 rFerns(iris[,-5],iris_multi_decision)->model;
 expect_output(print(model),'multi-class');
 #OOB Hamming
 expect_output(print(model),'Hamming');
 expect_output(print(model),'virginica');
 expect_lte(model$oobErr,0.3);
});

test_that("Multi-class model builds without saving",{
 cbind(
  setosa=rep(c(T,F,F),each=50),
  versicolor=rep(c(F,T,F),each=50),
  virginica=rep(c(F,F,T),each=50))->iris_multi_decision;
 rFerns(iris[,-5],iris_multi_decision,saveForest=FALSE)->model;
 expect_output(print(model),'multi-class');
 #OOB Hamming
 expect_output(print(model),'Hamming');
 expect_output(print(model),'virginica');
 expect_lte(model$oobErr,0.3);
});

test_that("Naked predict return oobPreds as data.frame",{
 cbind(
  setosa=rep(c(T,F,F),each=50),
  versicolor=rep(c(F,T,F),each=50),
  virginica=rep(c(F,F,T),each=50))->iris_multi_decision;
 rFerns(iris[,-5],iris_multi_decision)->model;
 predict(model)->mp;
 expect_identical(mp,data.frame(model$oobPreds));
 expect_identical(dim(mp),c(nrow(iris),3L));
 expect_is(mp,'data.frame');
 expect_identical(names(mp),model$classLabels);
});

test_that("Predict works",{
 cbind(
  setosa=rep(c(T,F,F),each=50),
  versicolor=rep(c(F,T,F),each=50),
  virginica=rep(c(F,F,T),each=50))->iris_multi_decision;
 rFerns(iris[,-5],iris_multi_decision)->model;
 predict(model,iris[,c(3,1,5,2,4)])->preds;
 expect_identical(dim(preds),c(nrow(iris),3L));
 expect_is(preds,'data.frame');
 expect_identical(names(preds),model$classLabels);
});

test_that("Predict score works",{
 cbind(
  setosa=rep(c(T,F,F),each=50),
  versicolor=rep(c(F,T,F),each=50),
  virginica=rep(c(F,F,T),each=50))->iris_multi_decision;
 rFerns(iris[,-5],iris_multi_decision)->model;
 predict(model,iris[,c(3,1,5,2,4)],scores=TRUE)->preds;
 expect_is(preds,"data.frame");
 expect_identical(nrow(preds),nrow(iris));
 expect_identical(ncol(preds),length(unique(iris$Species)));
 expect_identical(names(preds),model$classLabels);
 predict(model,iris[,c(3,1,5,2,4)],scores=FALSE)->predsStrict;
 expect_true(all((preds>0)==predsStrict));
});

test_that("Predict fails without model",{
 cbind(
  setosa=rep(c(T,F,F),each=50),
  versicolor=rep(c(F,T,F),each=50),
  virginica=rep(c(F,F,T),each=50))->iris_multi_decision;
 rFerns(iris[,-5],iris_multi_decision,saveForest=FALSE)->model;
 expect_error(predict(model,iris),"not contain the model");
});

test_that("Predict fails on incomplete input",{
 cbind(
  setosa=rep(c(T,F,F),each=50),
  versicolor=rep(c(F,T,F),each=50),
  virginica=rep(c(F,F,T),each=50))->iris_multi_decision;
 rFerns(iris[,-5],iris_multi_decision)->model;
 expect_error(predict(model,iris[,-2]),"training attributes missing");
});

test_that("Importance is refused for multilabel",{
 cbind(
  setosa=rep(c(T,F,F),each=50),
  versicolor=rep(c(F,T,F),each=50),
  virginica=rep(c(F,F,T),each=50))->iris_multi_decision;
 expect_error(rFerns(iris[,-5],iris_multi_decision,importance=TRUE),
  "Importance is not yet");
 expect_error(rFerns(iris[,-5],iris_multi_decision,importance='simple'),
   "Importance is not yet");
 expect_error(rFerns(iris[,-5],iris_multi_decision,importance='shadow'),
   "Importance is not yet");
 });
