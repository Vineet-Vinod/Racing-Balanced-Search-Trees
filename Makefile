build: main.cpp
	g++ -std=c++23 -O3 -o benchmark main.cpp

bench: build
	./benchmark

debug: main.cpp
	g++ -std=c++23 -O0 -o benchmark main.cpp
	gdb ./benchmark

memory: benchmark
	valgrind --leak-check=full ./benchmark

clean:
	rm -rf benchmark *.o