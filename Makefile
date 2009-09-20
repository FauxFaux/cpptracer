CC=g++
CFLAGS=-O3 -ffast-math -msse -msse2 -msse3 -msse4 -fexpensive-optimizations -fipa-struct-reorg -fipa-type-escape

all: cpptracer

out/combined.cpp: *.cpp
	rm -rf out
	mkdir out
	cat *.cpp > out/combined.cpp

cpptracer: out/combined.cpp Makefile
	$(CC) $(CFLAGS) -lboost_thread-mt out/combined.cpp -I${PWD} -o cpptracer

clean:
	rm -f cpptracer
	rm -rf out
