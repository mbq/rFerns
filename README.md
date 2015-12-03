About
-----

rFerns is an extended [random ferns](http://cvlab.epfl.ch/alumni/oezuysal/ferns.html) implementation for [R](http://r-project.org); in comparison to original, it can handle standard information system containing both categorical and continous attributes. Moreover, it generates OOB error approximation and permutational importance measure similar to [randomForest](http://www.stat.berkeley.edu/~breiman/RandomForests/cc_home.htm).

How to use
---------

It should be soon on CRAN, so the best idea is to download it from there.

If you want to compile R package, fire `updoak.sh` and then execute `R CMD build rFerns`. 

If you want to use it / test it apart from R, it is quite possible -- consult `side_src/test.c` to see how this may work. 
Yet don't expect that this will become a standalone library.


