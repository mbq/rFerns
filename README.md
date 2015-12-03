About
-----

rFerns is an extended [random ferns](http://cvlab.epfl.ch/alumni/oezuysal/ferns.html) implementation for [R](http://r-project.org); in comparison to original, it can handle standard information system containing both categorical and continuous attributes. Moreover, it generates OOB error approximation and permutation-based attribute importance measure similar to [randomForest](http://www.stat.berkeley.edu/~breiman/RandomForests/cc_home.htm).
Here is a [paper with all the details](http://www.jstatsoft.org/article/view/v061i10).

rFerns is good for doing training fast and in predictable time; in general it is less accurate than Random Forest, yet not substantially, and obviously there are cases in which it is better.
It is also nice as a very fast variable importance source; in fact it was created to speed-up the [`Boruta` all relevant feature selector](https://m2.icm.edu.pl/boruta/), and [it did pretty well](http://www.biomedcentral.com/1471-2105/15/8/).
Finally, it is a very stochastic method, practically doing no optimisation at all; basically it is crazy that it works.
Hence, it is theoretically interesting (;

Since v0.3.2, it can do multi-label classification as well; here is [a conference paper about that](http://link.springer.com/chapter/10.1007/978-3-319-08326-1_22) ([arXiv version](http://arxiv.org/pdf/1403.7746)).

There is also a Spark version (not mine), [Sparkling Ferns](https://github.com/CeON/sparkling-ferns); also [this](https://spark-summit.org/eu-2015/events/sparkling-random-ferns-from-an-academic-paper-to-spark-packagesorg/).

How to use
---------

Quite fresh version should be on [CRAN](http://cran.r-project.org/web/packages/rFerns/index.html); see the R docs for more.

If you want to compile R package, fire `updpak.sh` and then execute `R CMD INSTALL rFerns_x.y.z.tar.gz`.
This will build `roxygen2` documentation and fire `testthat2` tests (both of those packages, as well as `devtools`, should be installed; note that `devtools::install_github` will not work).

If you want to use it / test it apart from R, it is quite possible -- consult `side_src/test.c` to see how this may work.
Yet don't expect that this will ever become a standalone library.
