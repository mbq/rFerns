context("Naive wrapper");

test_that("Naive wrapper works",{
 set.seed(77);
 data(iris)
 noisyIris<-cbind(iris[,-5],apply(iris[,-5],2,sample));
 names(noisyIris)[5:8]<-sprintf("Nonsense%d",1:4)
 naiveWrapper(noisyIris,iris$Species,
  iterations=50,ferns=20,size=8)->sel;
 expect_identical(sel$found,
  c("Sepal.Length","Sepal.Width","Petal.Length","Petal.Width"));
 expect_is(sel$timeTaken,"difftime");
 expect_is(sel,"naiveWrapper");
});

test_that("Printing of naiveWrapper objects",{
 set.seed(77);
 data(iris)
 noisyIris<-cbind(iris[,-5],apply(iris[,-5],2,sample));
 names(noisyIris)[5:8]<-sprintf("Nonsense%d",1:4)
 naiveWrapper(noisyIris,iris$Species,
  iterations=50,ferns=20,size=8)->sel;
 expect_output(print(sel),"ve shadow importance");
 expect_output(print(sel),sel$found[2]);
});

test_that("Errors and warnings",{
 set.seed(77);
 data(iris)
 noisyIris<-cbind(iris[,-5],apply(iris[,-5],2,sample));
 expect_error(
  naiveWrapper(noisyIris,iris$Species,iterations=50,ferns=20,size=4))
 names(noisyIris)[5:8]<-sprintf("Nonsense%d",1:4);
 expect_warning(
  naiveWrapper(noisyIris,iris$Species,iterations=50,ferns=20,size=800))
});
