context("Importance");

test_that("Model without importance",{
 expect_null(rFerns(iris[,-5],iris[,5],importance=FALSE)$importance);
 expect_null(rFerns(iris[,-5],iris[,5],importance="none")$importance);
 #Like, default
 expect_null(rFerns(iris[,-5],iris[,5])$importance);
});

test_that("Error on bad importance",{
 expect_error(rFerns(iris[,-5],iris[,5],importance="blargh"),"Wrong importance value");
});

test_that("Model with importance=TRUE",{
 rFerns(iris[,-5],iris[,5],importance=TRUE)$importance->imp;
 atts<-names(iris[,-5]);
 expect_is(imp,"data.frame");
 expect_identical(dim(imp),c(length(atts),2L));
 expect_identical(names(imp),c("MeanScoreLoss","Tries"));
 expect_identical(rownames(imp),atts);
 expect_true(all(is.finite(imp$MeanScoreLoss)));
 expect_true(all(is.finite(imp$Tries)));
});

test_that("Model with importance=simple",{
 rFerns(iris[,-5],iris[,5],importance="simpl")$importance->imp;
 atts<-names(iris[,-5]);
 expect_is(imp,"data.frame");
 expect_identical(dim(imp),c(length(atts),2L));
 expect_identical(names(imp),c("MeanScoreLoss","Tries"));
 expect_identical(rownames(imp),atts);
 expect_true(all(is.finite(imp$MeanScoreLoss)));
 expect_true(all(is.finite(imp$Tries)));
});

test_that("Model with importance=simple & saveForest=FALSE",{
 rFerns(iris[,-5],iris[,5],importance="simpl",saveForest=FALSE)$importance->imp;
 atts<-names(iris[,-5]);
 expect_is(imp,"data.frame");
 expect_identical(dim(imp),c(length(atts),2L));
 expect_identical(names(imp),c("MeanScoreLoss","Tries"));
 expect_identical(rownames(imp),atts);
 expect_true(all(is.finite(imp$MeanScoreLoss)));
 expect_true(all(is.finite(imp$Tries)));
});

test_that("Model with importance=shadow",{
 (mo<-rFerns(iris[,-5],iris[,5],importance="shad"))$importance->imp;
 atts<-names(iris[,-5]);
 expect_is(imp,"data.frame");
 expect_identical(dim(imp),c(length(atts),3L));
 expect_identical(names(imp),c("MeanScoreLoss","Shadow","Tries"));
 expect_identical(rownames(imp),atts);
 expect_true(all(is.finite(imp$MeanScoreLoss)));
 expect_true(all(is.finite(imp$Tries)));
 expect_true(all(is.finite(imp$Shadow)));
 expect_is(mo$consistentSeed,'integer');
 expect_identical(length(mo$consistentSeed),2L);
});

test_that("Model with importance=shadow & saveForest=FALSE",{
 rFerns(iris[,-5],iris[,5],importance="shad",saveForest=FALSE)$importance->imp;
 atts<-names(iris[,-5]);
 expect_is(imp,"data.frame");
 expect_identical(dim(imp),c(length(atts),3L));
 expect_identical(names(imp),c("MeanScoreLoss","Shadow","Tries"));
 expect_identical(rownames(imp),atts);
 expect_true(all(is.finite(imp$MeanScoreLoss)));
 expect_true(all(is.finite(imp$Tries)));
 expect_true(all(is.finite(imp$Shadow)));
});

test_that("consistentSeed handling",{
 rFerns(iris[,-5],iris[,5],importance="shad",consistentSeed=c(1L,2L))->mo;
 expect_identical(mo$consistentSeed,c(1L,2L));
 expect_error(rFerns(iris[,-5],iris[,5],imp="shad",cons=c(7L)));
 expect_error(rFerns(iris[,-5],iris[,5],imp="shad",cons=c(1L,7L,9L)));
 expect_error(rFerns(iris[,-5],iris[,5],imp="shad",cons=c(3.5,2.1)));
 expect_warning(rFerns(iris[,-5],iris[,5],imp="simp",cons=c(1L,2L))->mo2);
 expect_identical(mo2$consistentSeed,NULL);
});
