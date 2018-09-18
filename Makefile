S : Server.cpp
	g++ -std=c++11 -pthread Server.cpp -o Server

C :	Client.cpp
	g++ -std=c++11 -pthread Client.cpp -o Client

MT : TorrentGen.cpp
	g++ TorrentGen.cpp -o TorrentGen -L/usr/lib -lssl -lcrypto

clean:
	rm -f Client
	rm -f Server
	rm -f TorrentGen
	rm -f *.mtorrent