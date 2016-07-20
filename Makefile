CC=g++
CFLAGS=-O3 -ffast-math -msse -msse2 -msse3 -msse4 -fexpensive-optimizations -ftree-loop-im -fivopts -ftree-loop-linear -fipa-matrix-reorg -ftracer -fweb

all: cpptracer

out/combined.cpp: *.cpp
	mkdir -p out
	cat *.cpp > out/combined.cpp

cpptracer: out/combined.cpp Makefile
	$(CC) $(CFLAGS) out/combined.cpp -lboost_thread -lboost_system -I${PWD} -o cpptracer
	strip --strip-all cpptracer

clean:
	rm -f cpptracer
	rm -rf out
