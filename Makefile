CC=g++
CFLAGS=-O2

all: cpptracer

cpptracer: *.cpp
	$(CC) $(CFLAGS) -lboost_thread-mt *.cpp -o cpptracer
