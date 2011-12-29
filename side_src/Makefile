all: testgprof test testslow 

testgprof: test.c ../src/fern.h ../src/tools.h ../src/forest.h 
	gcc -o $@ $< --std=c99 -lm -O3 -g -pg

test: test.c ../src/fern.h ../src/tools.h ../src/forest.h
	gcc -o $@ $< --std=c99 -lm -O3 -g

testsse: test.c ../src/fern.h ../src/tools.h ../src/forest.h 
	gcc -o $@ $< --std=c99 -lm -O3 -g -ftree-vectorize -msse2 -ftree-vectorizer-verbose=5
	
testslow: test.c ../src/fern.h ../src/tools.h ../src/forest.h 
	gcc -o $@ $< --std=c99 -lm -g

.PHONY: clean

clean:
	rm -f test testslow testgprof
