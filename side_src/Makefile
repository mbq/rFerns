all: test

test: test.c ../src/fern.h ../src/tools.h ../src/forest.h iris.h
	gcc -o $@ $< --std=c99 -lm -O3 -g

.PHONY: clean

clean:
	rm -f test
