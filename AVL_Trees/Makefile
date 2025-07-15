cpp: main.cpp avl_tree.h
	g++ -o main main.cpp -std=c++23 -O3
	./main

debug: main.cpp avl_tree.h
	g++ -o main main.cpp -std=c++23 -O0 -g
	gdb ./main

memory: main.cpp avl_tree.h
	g++ -o main main.cpp -std=c++23 -O3
	valgrind --leak-check=full ./main

clean:
	rm -rf main