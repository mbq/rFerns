#!/bin/bash

echo "Cleaning..."
rm -rf rFerns
rm rFerns_*.tar.gz
mkdir rFerns
cp -r inst rFerns/.
cp -r R rFerns/.
mkdir rFerns/src
cp -r src/*.c rFerns/src/.
cp -r src/*.h rFerns/src/.
cp DESCRIPTION rFerns/.

echo "Roxygenizing..."
R --vanilla --quiet <<< "library(roxygen2);roxygenise('rFerns')"

echo "Testing..."
R --vanilla --quiet <<< "library(devtools);library(testthat);load_all('rFerns/');test_dir('tests/testthat')"
#R -d valgrind --vanilla --quiet <<< "library(devtools);library(testthat);load_all('rFerns/');test_dir('tests/testthat')"

echo "Building..."
R CMD build rFerns/

echo "Cleaning again..."
rm -rf rFerns/
