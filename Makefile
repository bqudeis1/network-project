CC = g++
CFLAGS = -std=c++11

all: main

main: main.cpp
	$(CC) $(CFLAGS) -o main main.cpp

clean:
	rm -f main
