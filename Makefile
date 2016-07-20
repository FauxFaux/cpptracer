CC=g++
CFLAGS=-Ofast -march=ivybridge -fexpensive-optimizations -ftree-loop-im -fivopts -ftree-loop-linear -fipa-matrix-reorg -ftracer -fweb $(XFLAGS)

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
