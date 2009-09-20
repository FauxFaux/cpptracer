CC=g++
CFLAGS=-O3 -ffast-math -msse -msse2 -msse3 -msse4 -fexpensive-optimizations -fipa-struct-reorg -fipa-type-escape -ftree-loop-im -funswitch-loops -fivopts -ftree-loop-linear -fno-prefetch-loop-arrays -fipa-matrix-reorg -ftracer -fweb

all: cpptracer

out/combined.cpp: *.cpp
	mkdir -p out
	cat *.cpp > out/combined.cpp

cpptracer: out/combined.cpp Makefile
	$(CC) $(CFLAGS) -lboost_thread-mt out/combined.cpp -I${PWD} -o cpptracer
	strip --strip-all cpptracer

clean:
	rm -f cpptracer
	rm -rf out
