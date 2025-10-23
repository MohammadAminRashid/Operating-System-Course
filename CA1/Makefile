abc: server.o system.o
	g++ -std=c++20 server.o system.o -o abc.out

server.o: server.cpp
	g++ -std=c++20 -c server.cpp -o server.o

system.o: system.cpp
	g++ -std=c++20 -c system.cpp -o system.o

clean:
	rm -f *.o abc.out