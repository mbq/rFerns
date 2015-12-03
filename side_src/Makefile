all: testgprof test testslow testllvm testllvm3 testllvmm

testllvm3: test.c ../src/fern.h ../src/tools.h ../src/forest.h iris.h
	clang -o $@ $< -lm -O3 -g

testllvm: test.c ../src/fern.h ../src/tools.h ../src/forest.h iris.h
	clang -o $@ $< -lm -O0 -g

testllvmm: test.c ../src/fern.h ../src/tools.h ../src/forest.h iris.h
	clang -o $@ $< -lm -O0 -g -fsanitize=address -fno-omit-frame-pointer

testgprof: test.c ../src/fern.h ../src/tools.h ../src/forest.h iris.h
	gcc -o $@ $< --std=c99 -lm -O3 -g -pg

test: test.c ../src/fern.h ../src/tools.h ../src/forest.h iris.h
	gcc -o $@ $< --std=c99 -lm -O3 -g

testsse: test.c ../src/fern.h ../src/tools.h ../src/forest.h iris.h
	gcc -o $@ $< --std=c99 -lm -O3 -g -ftree-vectorize -msse2 -ftree-vectorizer-verbose=5

testslow: test.c ../src/fern.h ../src/tools.h ../src/forest.h iris.h
	gcc -o $@ $< --std=c99 -lm -g

.PHONY: clean

clean:
	rm -f test testslow testgprof testmulti testllvm testllvm3 testllvmm
