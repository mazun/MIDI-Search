all: main sound
	g++ -std=gnu++0x -O2 -o midisearch main.o sound.o

main: main.cpp sound.hpp
	g++ -std=gnu++0x -O2 -o main.o -c main.cpp

sound: sound.cpp sound.hpp reverse.hpp
	g++ -std=gnu++0x -O2 -o sound.o -c sound.cpp

clean:
	rm -f main.o sound.o midisearch midisearch.exe