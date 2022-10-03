all: prog

prog: main.cpp
	g++ -g  -Wall -o myfind main.cpp

clean:
	rm myfind