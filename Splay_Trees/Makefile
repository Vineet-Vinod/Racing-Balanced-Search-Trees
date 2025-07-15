cpp: main.cpp splay_tree.h
	g++ -o main main.cpp -std=c++23 -O3
	./main

debug: main.cpp splay_tree.h
	g++ -o main main.cpp -std=c++23 -O3 -g
	gdb ./main

memory: main.cpp splay_tree.h
	g++ -o main main.cpp -std=c++23 -O3
	valgrind --leak-check=full ./main

clean:
	rm -rf main