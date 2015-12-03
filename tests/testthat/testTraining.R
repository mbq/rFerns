context("Many-class model building");

test_that("Model builds",{
 rFerns(iris[,-5],iris[,5])->model;
 expect_output(print(model),'Forest of');
 #OOB confusion matrix printed
 expect_output(print(model),'versicolor');
 expect_lte(model$oobErr,0.07);
});

test_that("Model builds via formula",{
 rFerns(Species~.,data=iris)->model;
 expect_lte(model$oobErr,0.07);
});

test_that("Prints note about too little trees",{
 expect_output(print(rFerns(Species~.,data=iris,ferns=1)),"forest too small");
});

test_that("Model builds without saving",{
 rFerns(iris[,-5],iris[,5],saveForest=FALSE)->model;
 expect_null(model$model);
 expect_null(model$isStruct);
 expect_lte(model$oobErr,0.07);
});

test_that("Naked predict return oobPreds",{
 rFerns(iris[,-5],iris[,5],saveForest=FALSE)->model;
 expect_identical(predict(model),model$oobPreds);
});

test_that("Naked predict with scores returns oobScores as a data.frame",{
 rFerns(iris[,-5],iris[,5],saveForest=FALSE)->model;
 expect_is(predict(model,scores=TRUE),'data.frame');
});

test_that("Predict works",{
 rFerns(iris[,-5],iris[,5])->model;
 predict(model,iris[,c(3,1,5,2,4)])->preds;
 expect_identical(length(preds),nrow(iris));
 expect_is(preds,"factor");
 mean(preds!=iris$Species)->predErr;
 expect_lte(predErr,0.07);
});

test_that("Predict score works",{
 rFerns(iris[,-5],iris[,5])->model;
 predict(model,iris[,c(3,1,5,2,4)],scores=TRUE)->preds;
 expect_is(preds,"data.frame");
 expect_identical(nrow(preds),nrow(iris));
 expect_identical(ncol(preds),length(unique(iris$Species)));
 expect_identical(names(preds),model$classLabels);
 mean(names(preds)[apply(preds,1,which.max)]!=iris$Species)->predErr;
 expect_lte(predErr,0.07);
});

test_that("Predict fails without model",{
 rFerns(iris[,-5],iris[,5],saveForest=FALSE)->model;
 expect_error(predict(model,iris),"not contain the model");
});

test_that("Predict fails on incomplete input",{
 rFerns(iris[,-5],iris[,5])->model;
 expect_error(predict(model,iris[,-2]),"training attributes missing");
});

test_that("Depth/ferns parameters are validated",{
 expect_error(rFerns(iris[,-5],iris[,5],ferns=-7));
 expect_error(rFerns(iris[,-5],iris[,5],depth=0));
 expect_error(rFerns(iris[,-5],iris[,5],depth=-12));
 expect_error(rFerns(iris[,-5],iris[,5],depth=792));
 rFerns(iris[,-5],iris[,5],depth=7.1,ferns=21.3)->model;
 expect_output(print(model),"21 ferns of a depth 7");
});

test_that("Printing survives side cases",{
 rFerns(iris[,-5],iris[,5])->A;
 A$parameters["ferns"]<-30.5;
 A$parameters["depth"]<-2.5;
 A$oobScores<-NULL;
 expect_output(print(A),"30.5 ferns of a depth 2.5");
});
