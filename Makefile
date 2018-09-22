S : Server.cpp
	g++ -w -std=c++11 -pthread Server.cpp -o Server

C :	Client.cpp
	g++ -w -std=c++11 -pthread Client.cpp -o Client -L/usr/lib -lssl -lcrypto

MT : TorrentGen.cpp
	g++ -w -std=c++11 TorrentGen.cpp -o TorrentGen -L/usr/lib -lssl -lcrypto

TR: Tracker.cpp
	g++ -w -std=c++11 Tracker.cpp -o Tracker -pthread

clean:
	rm -f Client
	rm -f Server
	rm -f TorrentGen
	rm -f Tracker
	rm -f *.mtorrent
