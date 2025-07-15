cpp: main.cpp
	clear
	rm -f main
	g++ -o main main.cpp -std=c++23 -O3
	./main

debug: main.cpp
	clear
	rm -f main
	g++ -o main main.cpp -std=c++23 -O0 -g
	gdb ./main

time: main.cpp
	clear
	rm -f main
	g++ -o main main.cpp -std=c++23 -O3 -DTIME

ccpp: cmain.cpp
	clear
	rm -f cmain
	g++ -o cmain cmain.cpp -std=c++23 -O3
	./cmain