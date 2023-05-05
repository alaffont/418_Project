CXX=g++ -m64
CXXFLAGS=-O3
LDFLAGS=-L/usr/local/cuda-11.7/lib64/ -lcudart
NVCC=nvcc
NVCCFLAGS=-O3 -m64 --gpu-architecture compute_61 -ccbin /usr/bin/g++

all: hsequential hparallel

.PHONY: all clean

clean:
	rm -f *.o hsequential hparallel

hsequential: huffman_sequential.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< 

hparallel: huffman_parallel.cpp hp_main.cpp
	$(NVCC) $(NVCCFLAGS) huffman_parallel.cpp -c -o huffman_parallel.o
	$(CXX) $(CXXFLAGS) hp_main.cpp -c -o hp_main.o
	$(CXX) $(CXXFLAGS) -o $@ hp_main.o huffman_parallel.o $(LDFLAGS)
