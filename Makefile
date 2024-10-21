
test1: assignment3.o
	./assignment3.o -l 12345  -p "happy"
	
assignment3.o: assignment3.cpp
	g++ assignment3.cpp -o assignment3.o -std=c++20