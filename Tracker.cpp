#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <signal.h>
#include <ctype.h>          
#include <arpa/inet.h>
#include <netdb.h>
#include <fstream>
#include <bits/stdc++.h>
#include <unistd.h>
using namespace std;
#define PORT 3000
#define BACKLOG 100
#define LENGTH 524288

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

vector<string >myIpPort;
vector<string >otherIpPort;
    

map< string,set<string> >TorrentList;

vector<string> split(std::string txt, char ch)
{
    size_t pos = txt.find( ch );
    size_t initialPos = 0;
    vector<string>strs;
    while( pos != std::string::npos ) {
        strs.push_back( txt.substr( initialPos, pos - initialPos ) );
        initialPos = pos + 1;

        pos = txt.find( ch, initialPos );
    }

    strs.push_back( txt.substr( initialPos, std::min( pos, txt.size() ) - initialPos + 1 ) );

    return strs;
}

void ShareTorrentFile(int nsockfd, string ipaddr, string TorrentData)
{
    int fr_block_sz;
    //string TorrentData;
    char buffer[LENGTH];
    cout<<"inside Share torrent method"<<endl;
    cout<<TorrentData<<endl;
    vector<string> V = split(TorrentData, '\n');
    TorrentList[V[4]].insert(ipaddr);
}

void SynchronizeTrackers(int nsockfd, string ipaddr)
{
    cout<<"Synchronizing Trackers"<<endl;
    string otherTracker = otherIpPort[0] + ":" + otherIpPort[1];
    cout<<otherTracker<<"    "<<ipaddr<<" "<<TorrentList.size()<<endl;
    //if(otherTracker != ipaddr)
    {
        string number = to_string( TorrentList.size());
        send(nsockfd, number.c_str(), number.length()+1, 0);
        string sendstr = "";
        for(auto it = TorrentList.begin(); it != TorrentList.end(); ++it)
        {
            sendstr += it->first;
            for(auto i = it->second.begin(); i != it->second.end(); ++i)
            {
                sendstr += "\n";
                sendstr += (*i);
            }
            sendstr += "\n";
        }
        cout<<sendstr<<endl;
        for(int i =0; i <= sendstr.length(); i += LENGTH)
        {
            string buff = sendstr.substr(i, min((unsigned long)LENGTH, sendstr.length() - i));
            send(nsockfd, buff.c_str(), buff.length(),0);
        }
    }
    close(nsockfd);
}

void Synchronize()
{
    int sockfd; 
    int nsockfd;
    char buffer[LENGTH]; 
    struct sockaddr_in remote_addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        fprintf(stderr, "ERROR: Failed to obtain Socket Descriptor! (errno = %d)\n",errno);
        exit(1);
    }

    remote_addr.sin_family = AF_INET; 
    remote_addr.sin_port = htons(stoi(otherIpPort[1])); 
    //remote_addr.sin_addr.s_addr = INADDR_ANY;
    inet_pton(AF_INET, otherIpPort[0].c_str(), &remote_addr.sin_addr); 
    bzero(&(remote_addr.sin_zero), 8);

    if (connect(sockfd, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr)) == -1)
    {
        fprintf(stderr, "ERROR: Failed to connect to the Other Tracker (errno = %d)\n",errno);
        return;
    }
    else 
        printf("[Client] Connected to Other Tracker...ok!\n");

    char* msg = "synchronize$1";

    send(sockfd, msg, strlen(msg), 0); 

    int valread = read( sockfd , buffer, LENGTH); 
    string num = buffer;
    int count = stoi(num);
    cout<<"Number of Torrents : "<<count<<endl;    
    bzero(buffer, LENGTH); 
    int fr_block_sz = 0;
    string strs = "";
    while((fr_block_sz = read(sockfd, buffer, LENGTH)) > 0)
    {
    	strs += buffer;
        bzero(buffer, LENGTH); 
    }
    vector<string> V = split(strs, '\n');
    string hash = "";
    for(int i =0; i < V.size()-1; ++i)
    {
        if(V[i].length() > 30)
            hash = V[i];
        else
            TorrentList[hash].insert(V[i]);
    }
    cout<<TorrentList.size()<<endl;
    close (sockfd);
}

void SendSeederListofTorrent(int nsockfd, string ipaddr, string hash)
{
    int fr_block_sz;
    cout<<"Sending Seeders list"<<endl;
    string sendstr = "";
    cout<<hash<<endl;
    for(auto i = TorrentList[hash].begin(); i != TorrentList[hash].end(); ++i)
    {
        sendstr += (*i);
        sendstr += "\n";
    }
    cout<<sendstr<<endl;
    for(int i =0; i <= sendstr.length(); i += LENGTH)
    {
        string buff = sendstr.substr(i, min((unsigned long)LENGTH, sendstr.length() - i));
        send(nsockfd, buff.c_str(), buff.length(),0);
    }
    close(nsockfd);
}

void RequestHandler(int nsockfd, string ipaddr)
{
    char req[LENGTH];
    cout<<"Entered Handler"<<endl;
    bzero(req, LENGTH);
    int ReqLen = read(nsockfd, req, LENGTH);
    string msg = req;
    vector<string> message = split(msg,'$');
    string request = message[0];

    if(request == "share")
    {
        string hash = message[1];
        ShareTorrentFile(nsockfd, ipaddr, hash);
    }
    if(request == "seederlist")
    {
        string hash = message[1];
        SendSeederListofTorrent(nsockfd, ipaddr, hash);
    }
    if(request == "remove")
    {
        //TODO
    }
    if(request == "synchronize")
    {
        SynchronizeTrackers(nsockfd, ipaddr);
    }

}

vector<thread> TH;
int ThreadC;


int main (int argc, char const *argv[])
{
    int sockfd; 
    int nsockfd; 
    int num;
    socklen_t sin_size; 
    struct sockaddr_in addr_local; 
    struct sockaddr_in addr_remote; 
    char buffer[LENGTH], request[LENGTH];

    if(argc != 5)
    {
        cout<<"Command line arguments : <my_tracker_ip>:<my_tracker_port> <other_tracker_ip>:<other_tracker_port> <seederlist_file> <log_file>"<<endl;
        return 0;
    }

    string S1 = argv[1];
    string S2 = argv[2];
    string seederList = argv[3];
    string logfile = argv[4];

    myIpPort = split(S1, ':');
    otherIpPort = split(S2, ':');
    
    //SYNCHRONIZE TRACKERS
    Synchronize();
    
    
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1 )
    {
        fprintf(stderr, "ERROR: Failed to obtain Socket Descriptor. (errno = %d)\n", errno);
        exit(1);
    }
    else 
        printf("[Tracker] Obtaining socket descriptor successfully.\n");

    addr_local.sin_family = AF_INET;
    addr_local.sin_port = htons(stoi(myIpPort[1]));
    inet_pton(AF_INET,myIpPort[0].c_str() , &addr_local.sin_addr); 
    bzero(&(addr_local.sin_zero), 8);

    if( bind(sockfd, (struct sockaddr*)&addr_local, sizeof(struct sockaddr)) == -1 )
    {
        fprintf(stderr, "ERROR: Failed to bind Port. (errno = %d)\n", errno);
        exit(1);
    }
    else 
        printf("[Tracker] Binded tcp port %d in addr 127.0.0.1 sucessfully.\n",PORT);

    if(listen(sockfd,BACKLOG) == -1)
    {
        fprintf(stderr, "ERROR: Failed to listen Port. (errno = %d)\n", errno);
        exit(1);
    }
    else
        printf ("[Tracker] Listening the port %d successfully.\n", PORT);

    int success = 0;
    sin_size = sizeof(struct sockaddr_in);
    while((nsockfd = accept(sockfd, (struct sockaddr *)&addr_remote, &sin_size)) != -1)
    { 
        string ipaddr = inet_ntoa(addr_remote.sin_addr);
        printf("[TRACKER] Server has got connected from %s\n", ipaddr.c_str()); 
        //get the request header first
        ipaddr += ":";
        ipaddr += to_string(ntohs(addr_remote.sin_port));
        TH.push_back(thread(RequestHandler, nsockfd, ipaddr));
        sin_size = sizeof(struct sockaddr_in);
    }

    for(auto &th : TH)
        if(th.joinable()) th.join();

    return 0;
}