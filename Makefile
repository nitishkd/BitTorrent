C : Client.cpp
	g++ -std=c++11 -pthread Client.cpp -o Client	
S : Server.cpp
	g++ -std=c++11 -pthread Server.cpp -o Server

clean: 
		rm -f Client
		rm -f Server