CXXFLAGS=-Ofast -std=c++11 -march=ivybridge -fexpensive-optimizations -ftree-loop-im -fivopts -ftree-loop-linear -fipa-matrix-reorg -ftracer -fweb $(XFLAGS)

all: cpptracer

objects = main.o tracer.o bitmap.o

main.o: bitmap.h tracer.h

tracer.o: objects.h tracer.h

bitmap.o: bitmap.h

cpptracer: $(objects)
	$(CXX) $(CXXFLAGS) $(objects) -lpthread -I${PWD} -o cpptracer
	strip --strip-all cpptracer

clean:
	rm -f cpptracer
	rm -rf out
