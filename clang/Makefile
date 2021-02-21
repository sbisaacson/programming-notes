gcd.svg: gcd.dot
	dot -Tsvg -o $@ $^

gcd.dot: cfg.ll
	opt -analyze --dot-cfg $^
	mv ._Z3gcdmm.dot $@

cfg.ll:	cfg.cc
	clang++ -O3 -Wall -Wextra -emit-llvm -S -o $@ $^

clean:
	rm -f cfg.ll gcd.dot gcd.svg
.PHONY: clean
