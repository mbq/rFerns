#!/bin/bash

echo "Copying stuff to form valid package in rFerns/"
rm -rf rFerns
mkdir rFerns
cp -r inst rFerns/.
cp -r man rFerns/.
cp -r R rFerns/.
mkdir rFerns/src
cp -r src/*.c rFerns/src/.
cp -r src/*.h rFerns/src/.
cp DESCRIPTION rFerns/.
cp NAMESPACE rFerns/.
echo "Copying done."

rm rFerns_*.tar.gz
R CMD build rFerns/
