context("Model merging");

test_that("Basic tests after simple merge",{
 rFerns(iris[,-5],iris[,5],importance='shadow',consistentSeed=c(23L,-1123L))->A;
 rFerns(iris[,-5],iris[,5],importance='shadow',consistentSeed=c(23L,-1123L))->B;
 merge(A,B,trueY=iris$Species)->AB;
 expect_output(print(AB),'Forest of');
 #OOB confusion matrix printed
 expect_output(print(AB),'versicolor');
 expect_lte(AB$oobErr,0.07);
 expect_identical(names(AB$importance),
  c("MeanScoreLoss","Shadow","Tries"));

 predict(AB,iris[,c(3,1,5,2,4)])->preds;
 expect_identical(length(preds),nrow(iris));
 expect_is(preds,"factor");
 mean(preds!=iris$Species)->predErr;
 expect_lte(predErr,0.07);
});

test_that("Consistent seed mismatch kills shadow importance",{
 rFerns(iris[,-5],iris[,5],importance='shadow',consistentSeed=c(23L,-1123L))->A;
 rFerns(iris[,-5],iris[,5],importance='shadow',consistentSeed=c(24L,-1123L))->B;
 merge(A,B,trueY=iris$Species)->AB;
 expect_identical(names(AB$importance),
  c("MeanScoreLoss","Tries"));
});

test_that("Merging small models with missing data in OOB scores",{
 rFerns(iris[,-5],iris[,5],ferns=10)->a;
 rFerns(iris[,-5],iris[,5],ferns=10)->b;
 ab<-merge(a,b);
});

test_that("Shadow and normal importance",{
 rFerns(iris[,-5],iris[,5],ferns=10,imp='simple')->a;
 rFerns(iris[,-5],iris[,5],ferns=10,imp='shadow')->b;
 ab<-merge(a,b);
 ba<-merge(b,a);
 expect_is(ab$importance,'data.frame');
 expect_is(ba$importance,'data.frame');
 expect_identical(names(ba$importance),c('MeanScoreLoss','Tries'));
 expect_identical(names(ab$importance),c('MeanScoreLoss','Tries'));
});

test_that("Hetero-merge",{
 rFerns(iris[c(T,F),-5],iris[c(T,F),5],ferns=4000,importance=TRUE)->a;
 rFerns(iris[c(F,T,F),-5],iris[c(F,T,F),5],ferns=3000,importance=TRUE)->b;
 expect_error(merge(a,b));
 ab<-merge(a,b,ignoreObjectConsistency=TRUE);
 expect_is(ab$importance,'data.frame');
 expect_is(predict(ab,iris),'factor');
 expect_lte(mean(predict(ab,iris)!=iris$Species),0.07);
});
