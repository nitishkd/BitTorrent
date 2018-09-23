# BitTorrent
Bit Torrent File Sharing P2P System

## Features:

* Multithreaded Client and Tracker

* Supports Download from all the Cilents seeding that file.

* Seeder List getting updated in real time.

* Clients starts seeding file as soon as it downloads packets.

* Can download multiple files simultaneously

* Enables a client to share torrent.

### Command to Build Tracker:

`make TR`

### Command to Build Client:

`make C`

### Command to Run Tracker:

`./Tracker 127.0.0.1:3500 127.0.0.1:3000 seeder.txt log2.txt`

### Command to Run Client: 

`./Client 127.0.0.1:2500 127.0.0.1:3000 127.0.0.1:3500 logC.txt`

### Commands for Client:

* Make Torrent : Takes absoulte file path to generate torrent file.

`gen /home/nitish/Public/input.txt`

* Share Torrent : Takes absolute path of file to be shared and its torrent file.

`share /home/nitish/Documents/input.txt input.mtorrent`

* Download Torrent : Destination Path to save file and torrent file.

`get /home/nitish/Downloads/ input.mtorrent`

* Show Downloads:

`show downloads`

* Remove Torrent from Tracker:

`remove input.mtorrent`

NOTE: If file is a text file, then multi-threaded download from different clients works perfectly, but some bytes gets missing in case of other files. For that, there is also functionality to download it sequently from any one single client like regular download.
