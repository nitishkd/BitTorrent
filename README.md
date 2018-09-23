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

` ./Client 127.0.0.1:2500 127.0.0.1:3000 127.0.0.1:3500 logC.txt

