all:
	g++  yieldt.cpp  -g -c
	g++  main.cpp -g -o main yieldt.o
clean:
	rm -f yieldt.o main