CC=g++
CFLAGS=-O3 -ffast-math

all: cpptracer

cpptracer: *.cpp
	$(CC) $(CFLAGS) -lboost_thread-mt *.cpp -o cpptracer

clean:
	rm -f cpptracer
