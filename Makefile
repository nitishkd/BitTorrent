S : Server.cpp
	g++ -std=c++11 -pthread Server.cpp -o Server

C :	Client.cpp
	g++ -std=c++11 -pthread Client.cpp -o Client

clean:
	rm -f Client
	rm -f Server