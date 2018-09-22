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
#include <bits/stdc++.h>
#include <unistd.h>
#include <openssl/sha.h>
#include <thread>

using namespace std;

#define BACKLOG 100
#define PORT 2000
#define LENGTH 524288
int ServerFD;
map<string, string> HashFileMap;

vector<thread> TH;
int ThreadC;

string TR1, TR2,SERVER;

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

std::vector<std::string> splitBig(std::string stringToBeSplitted, std::string delimeter)
{
     std::vector<std::string> splittedString;
     int startIndex = 0;
     int  endIndex = 0;
     while( (endIndex = stringToBeSplitted.find(delimeter, startIndex)) < stringToBeSplitted.size() )
    {
       std::string val = stringToBeSplitted.substr(startIndex, endIndex - startIndex);
       splittedString.push_back(val);
       startIndex = endIndex + delimeter.size();
     }
     if(startIndex < stringToBeSplitted.size())
     {
       std::string val = stringToBeSplitted.substr(startIndex);
       splittedString.push_back(val);
     }
     return splittedString;
}


vector<string> GetSeederListFromTracker(string Filename)
{
    int sockfd; 
    int nsockfd;
    char buffer[LENGTH]; 
    struct sockaddr_in remote_addr;

    string line;
    std::vector<string> Torrent;
    std::ifstream input(Filename.c_str());

    while( std::getline( input, line ) ) {
        Torrent.push_back(line);
    }
    
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        fprintf(stderr, "ERROR: Failed to obtain Socket Descriptor! (errno = %d)\n",errno);
        exit(1);
    }
    
    vector<string> Tracker1 = split(Torrent[0], ':');
    int portnum = stoi(Tracker1[1]);
    string TrackerIp = Tracker1[0];
    remote_addr.sin_family = AF_INET; 
    remote_addr.sin_port = htons(portnum); 
    //remote_addr.sin_addr.s_addr = INADDR_ANY;
    inet_pton(AF_INET, TrackerIp.c_str(), &remote_addr.sin_addr); 
    bzero(&(remote_addr.sin_zero), 8);
    if (connect(sockfd, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr)) == -1)
    {
        fprintf(stderr, "ERROR: Failed to connect to the Tracker! (errno = %d)\n",errno);
        exit(1);
    }
    else 
        printf("[Client] Connected to Tracker at port %d...ok!\n", portnum);
    
    string msg = "seederlist$";
    bzero(buffer, LENGTH); 
    
    string sendstr = Torrent.back();
    msg += sendstr;
    strcpy(buffer, msg.c_str());
    send(sockfd, buffer, strlen(buffer), 0);

    bzero(buffer, LENGTH); 
    int fr_block_sz = 0;
    string strs = "";
    
    fr_block_sz = read(sockfd, buffer, LENGTH);
        strs = buffer;
    vector<string> V = split(strs, '\n');
    string hash = "";
    for(int i =0; i < V.size(); ++i)
        Torrent.push_back(V[i]);
    close(sockfd);
    printf("[Client] Connection with Tracker closed.\n");
    return Torrent;
    
     
}


void receivePackets(vector<int > packets,string IPport, string Filename, string hash)
{
    int sockfd; 
    int nsockfd;
    char buffer[LENGTH+8]; 
    struct sockaddr_in remote_addr;
    sort(packets.begin(), packets.end());
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        fprintf(stderr, "ERROR: Failed to obtain Socket Descriptor! (errno = %d)\n",errno);
        exit(1);
    }
    vector<string> Vec = split(IPport, ':');
    int portnum = stoi(Vec[1]);
    string TrackerIp = Vec[0];
    remote_addr.sin_family = AF_INET; 
    remote_addr.sin_port = htons(portnum); 
    //remote_addr.sin_addr.s_addr = INADDR_ANY;
    inet_pton(AF_INET, TrackerIp.c_str(), &remote_addr.sin_addr); 
    bzero(&(remote_addr.sin_zero), 8);
    if (connect(sockfd, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr)) == -1)
    {
        fprintf(stderr, "ERROR: Failed to connect to the Server! (errno = %d)\n",errno);
        exit(1);
    }
    else 
        fprintf(stderr,"[Client] Connected to Server at port %d...ok!\n", portnum);
    
    sockfd;
    string sendstr = hash;
    sendstr += " ";
    for(int i =0; i < packets.size(); ++i)
    {
        sendstr += to_string(packets[i]);
        if(i != (packets.size()-1))
            sendstr += " ";
    }
    
    strcpy(buffer, sendstr.c_str());
    send(sockfd, buffer, strlen(buffer), 0);
    
    string fr_name = Filename;
    FILE *fr = fopen(fr_name.c_str(), "w");
    if(fr == NULL)
        fprintf(stderr,"File %s Cannot be opened.\n", fr_name);
    else
    {
        //shutdown(nsockfd, SHUT_WR);
        bzero(buffer, LENGTH+8); 
        int fr_block_sz = 0;
        string strs = "";
        while((fr_block_sz = recv(sockfd, buffer, LENGTH,0)) > 0)
        {
            strs += buffer;
            bzero(buffer, LENGTH+8); 
        }
        vector<string> pieces = splitBig(strs, "##########");
        for(int i =0; i < packets.size(); ++i)
        {
            fseek(fr, packets[i]*LENGTH, SEEK_SET);
            int write_sz = fwrite(pieces[i].c_str(), sizeof(char), strlen(pieces[i].c_str()), fr);
        }
        fprintf(stderr,"Ok received from server!\n");
        fclose(fr);
    }
    close(sockfd);
}

void downloadManager(string PathTorrent, string result)
{
    vector<string> TorrentInfo = GetSeederListFromTracker(PathTorrent);
    vector <string> SeederList;
    for(int i =6; i < TorrentInfo.size(); ++i)
        SeederList.push_back(TorrentInfo[i]);
    vector<thread> LocalThread;
    int fileSize = stoi(TorrentInfo[4]);
    int no_of_pieces = (fileSize + LENGTH - 1)/LENGTH;
    vector<vector<int> > Pieces;
    Pieces.resize(SeederList.size());

    for(int i = 0; i < no_of_pieces; ++i)
        Pieces[i%Pieces.size()].push_back(i);
    string hash = TorrentInfo[5];
    for(int i=0; i < Pieces.size(); ++i)   
        LocalThread.push_back(thread(receivePackets, Pieces[i], SeederList[i], TorrentInfo[3], hash));

    for(auto &it : LocalThread)
        if(it.joinable()) it.join();
        
    //receivePackets(v,2000);  
}

void filedownload(int portnum,int cnt)
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
    remote_addr.sin_port = htons(portnum); 
    //remote_addr.sin_addr.s_addr = INADDR_ANY;
    inet_pton(AF_INET, "127.0.0.1", &remote_addr.sin_addr); 
    bzero(&(remote_addr.sin_zero), 8);

    if (connect(sockfd, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr)) == -1)
    {
        fprintf(stderr, "ERROR: Failed to connect to the host! (errno = %d)\n",errno);
        exit(1);
    }
    else 
        fprintf(stderr,"[Client] Connected to server at port %d...ok!\n", portnum);

    string fr_name = "recieve" + to_string(cnt)+ ".png";
    FILE *fr = fopen(fr_name.c_str(), "wb");
    if(fr == NULL)
        fprintf(stderr,"File %s Cannot be opened.\n", fr_name);
    else
    {
        bzero(buffer, LENGTH); 
        int fr_block_sz = 0;
        while((fr_block_sz = read(sockfd, buffer, LENGTH)) > 0)
            int write_sz = fwrite(buffer, sizeof(char), fr_block_sz, fr);
        fprintf(stderr,"Ok received from server!\n");
        fclose(fr);
    }
    close (sockfd);
    fprintf(stderr,"[Client] Connection lost.\n");
    // pthread_exit(EXIT_SUCCESS);

}

void ShareTorrentWithTracker(string filepath, string FileName)
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
    vector<string> V = split(TR1, ':');
    int portnum = stoi(V[1]);
    string TrackerIp = V[0];
    remote_addr.sin_family = AF_INET; 
    remote_addr.sin_port = htons(portnum); 
    //remote_addr.sin_addr.s_addr = INADDR_ANY;
    inet_pton(AF_INET, TrackerIp.c_str(), &remote_addr.sin_addr); 
    bzero(&(remote_addr.sin_zero), 8);
    if (connect(sockfd, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr)) == -1)
    {
        fprintf(stderr, "ERROR: Failed to connect to the Tracker! (errno = %d)\n",errno);
        exit(1);
    }
    else 
        fprintf(stderr,"[Client] Connected to Tracker at port %d...ok!\n", portnum);
    string msg = "share$";
    
    //send(sockfd, msg, strlen(msg), 0);    

    char sdbuf[LENGTH];
    FILE *fs = fopen(FileName.c_str(), "rb");
    if(fs == NULL)
    {
        fprintf(stderr, "ERROR: File not found on Client. (errno = %d)\n", errno);
        exit(1);
    }
    bzero(sdbuf, LENGTH); 
    int fs_block_sz;
    msg += SERVER;
    msg += "\n";
    while((fs_block_sz = fread(sdbuf, sizeof(char), LENGTH, fs))>0)
    {
        msg += sdbuf;
        bzero(sdbuf, LENGTH);
    }
    send(sockfd, msg.c_str(), strlen(msg.c_str()), 0);    
    fclose(fs);
    string hash = "";
    ifstream infile (FileName.c_str());
    while(getline(infile, hash));
    HashFileMap.insert({hash, filepath});
    fprintf(stderr,"Ok sent to Tracker!\n");
    close(sockfd);
    fprintf(stderr,"[Client] Connection with Tracker closed.\n");
 
}

void RemoveTorrentFromTracker(string FileName)
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
    vector<string> V = split(TR1, ':');
    int portnum = stoi(V[1]);
    string TrackerIp = V[0];
    remote_addr.sin_family = AF_INET; 
    remote_addr.sin_port = htons(portnum); 
    //remote_addr.sin_addr.s_addr = INADDR_ANY;
    inet_pton(AF_INET, TrackerIp.c_str(), &remote_addr.sin_addr); 
    bzero(&(remote_addr.sin_zero), 8);
    if (connect(sockfd, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr)) == -1)
    {
        fprintf(stderr, "ERROR: Failed to connect to the Tracker! (errno = %d)\n",errno);
        exit(1);
    }
    else 
        fprintf(stderr,"[Client] Connected to Tracker at port %d...ok!\n", portnum);
    string msg = "remove$";
    
    //send(sockfd, msg, strlen(msg), 0);    

    char sdbuf[LENGTH];
    FILE *fs = fopen(FileName.c_str(), "rb");
    if(fs == NULL)
    {
        fprintf(stderr, "ERROR: File not found on Client. (errno = %d)\n", errno);
        exit(1);
    }
    bzero(sdbuf, LENGTH); 
    int fs_block_sz;
    msg += SERVER;
    msg += "\n";
    while((fs_block_sz = fread(sdbuf, sizeof(char), LENGTH, fs))>0)
    {
        msg += sdbuf;
        bzero(sdbuf, LENGTH);
    }
    send(sockfd, msg.c_str(), strlen(msg.c_str()), 0);    
    fclose(fs);
    string hash = "";
    ifstream infile (FileName.c_str());
    
    while(getline(infile, hash));
    
    HashFileMap.erase(hash);
    
    fprintf(stderr,"Ok sent to Tracker!\n");
    close(sockfd);
    fprintf(stderr,"[Client] Connection with Tracker closed.\n");
 

}

void makeTorrent(string path)
{
    string fs_name = path;
    std::ifstream in(fs_name.c_str(), std::ifstream::ate | std::ifstream::binary);
    int filesize = in.tellg(); 
    in.close();
    vector<string> V = split(fs_name, '/');

    string Fname = V.back();
    char sdbuf[LENGTH];
    FILE *fs = fopen(fs_name.c_str(), "rb");
    if(fs == NULL)
    {
        fprintf(stderr, "ERROR: File %s not found.(errno = %d)\n", Fname.c_str(), errno);
        exit(1);
    }
    vector<string>T = split(Fname, '.');
    string initname = T[0];
    initname += ".mtorrent";
    bzero(sdbuf, LENGTH); 
    int fs_block_sz; 
    FILE* output = fopen(initname.c_str(), "w");
    unsigned char hash[LENGTH];
    fprintf(output,"%s\n", TR1.c_str());
    fprintf(output,"%s\n", TR2.c_str());
    fprintf(output,"%s\n", fs_name.c_str());
    fprintf(output,"%s\n", Fname.c_str());
    fprintf(output,"%d\n", filesize);
    int i = 0;
    while((fs_block_sz = fread(sdbuf, sizeof(char), LENGTH, fs))>0)
    {
        string str = "";
        char newstr[20];
        SHA1((unsigned char *)sdbuf, strlen(sdbuf),hash);
        for(int i =0; i < 20; ++i){
            sprintf(newstr,"%02x", hash[i]);
            str.append(newstr);
        }
        fprintf(output,"%s",str.c_str());
        bzero(hash, SHA_DIGEST_LENGTH);
        bzero(sdbuf, LENGTH);
    }
    fprintf(stderr, "Torrent File Generated \n");
    fclose(output);
    fclose(fs);
}

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void sendPackets(int nsockfd)
{
    char sdbuf[LENGTH+8];
    bzero(sdbuf, LENGTH+8);
    read(nsockfd, sdbuf, LENGTH);
    string temp = sdbuf;
    bzero(sdbuf, LENGTH+8);
    vector<string> Pack = split(temp, ' ');
    vector<int> packet;
    string hash = Pack[0];
    for(int i =1; i < Pack.size(); ++i)
        packet.push_back(stoi(Pack[i]));
    
    string fs_name = HashFileMap[hash];
    
    FILE *fs = fopen(fs_name.c_str(), "r");
    if(fs == NULL)
    {
        fprintf(stderr, "[Server] : ERROR: File %s not found on server. (errno = %d)\n", fs_name.c_str(), errno);
        exit(1);
    }
    bzero(sdbuf, LENGTH+8); 
    int fs_block_sz;
    int k;
    string sendstr = "";
    for(int i =0; i < packet.size(); ++i)
    {
        
        fseek(fs, packet[i]*LENGTH, SEEK_SET);
        fs_block_sz = fread(sdbuf, sizeof(char), LENGTH, fs);
        if(fs_block_sz > 0)
        {
            sendstr += sdbuf;
            if(i != (packet.size()-1) )
                sendstr += "##########";    
        }
        bzero(sdbuf, LENGTH+8);    
    }
    for(int i =0; i <= sendstr.length(); i += LENGTH)
    {
        string buff ="";
        buff =  sendstr.substr(i, min((unsigned long)LENGTH, strlen(sendstr.c_str()) - i));
        send(nsockfd, buff.c_str(), strlen(buff.c_str()),0);
    }
    fclose(fs);
    fprintf(stderr,"[Server] Ok sent to client!\n");
    close(nsockfd);
    fprintf(stderr,"[Server] Connection with Client closed. Server will wait now...\n");
}

void fileServe(int nsockfd)
{
    char* fs_name = "/home/nitish/Desktop/image.png";
    char sdbuf[LENGTH];
    int pid = pthread_self();
    printf("%d: [Server] Sending %s to the Client...\n", pid, fs_name);
    FILE *fs = fopen(fs_name, "rb");
    if(fs == NULL)
    {
        fprintf(stderr, "ERROR: File %s not found on server. (errno = %d)\n", fs_name, errno);
        exit(1);
    }
    bzero(sdbuf, LENGTH); 
    int fs_block_sz; 
    while((fs_block_sz = fread(sdbuf, sizeof(char), LENGTH, fs))>0)
    {
        if(send(nsockfd, sdbuf, fs_block_sz, 0) < 0)
        {
            fprintf(stderr, "%d ERROR: Failed to send file %s. (errno = %d)\n", pid, fs_name, errno);
            exit(1);
        }
        bzero(sdbuf, LENGTH);
    }
    fclose(fs);
    fprintf(stderr,"Ok sent to client!\n");
    close(nsockfd);
    fprintf(stderr,"[Server] Connection with Client closed. Server will wait now...\n");
}

void serverInit()
{
    int sockfd; 
    int nsockfd; 
    int num;
    socklen_t sin_size; 
    struct sockaddr_in addr_local; 
    struct sockaddr_in addr_remote; 
    char buffer[LENGTH];

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1 )
    {
        fprintf(stderr, "ERROR: Failed to obtain Socket Descriptor. (errno = %d)\n", errno);
        exit(1);
    }
    else 
        printf("[Server] Obtaining socket descriptor successfully.\n");

    vector<string> ServCred = split(SERVER, ':');
    addr_local.sin_family = AF_INET;
    addr_local.sin_port = htons(stoi(ServCred[1]));
    inet_aton(ServCred[0].c_str(), &addr_local.sin_addr);
    bzero(&(addr_local.sin_zero), 8);

    if( bind(sockfd, (struct sockaddr*)&addr_local, sizeof(struct sockaddr)) == -1 )
    {
        fprintf(stderr, "ERROR: Failed to bind Port. (errno = %d)\n", errno);
        exit(1);
    }
    else 
        fprintf(stderr,"[Server] Binded tcp port %s in addr %s sucessfully.\n",ServCred[1].c_str(), ServCred[0].c_str());

    if(listen(sockfd,BACKLOG) == -1)
    {
        fprintf(stderr, "ERROR: Failed to listen Port. (errno = %d)\n", errno);
        exit(1);
    }
    else
        fprintf (stderr,"[Server] Listening the port %d successfully.\n", PORT);

    int success = 0;
    sin_size = sizeof(struct sockaddr_in);
    while((nsockfd = accept(sockfd, (struct sockaddr *)&addr_remote, &sin_size)) != -1)
    {
        fprintf(stderr,"[Server] Server has got connected from %s.\n", inet_ntoa(addr_remote.sin_addr));    
        TH.push_back(thread(sendPackets, nsockfd));
        sin_size = sizeof(struct sockaddr_in);
    }

    for(auto &th : TH)
        if(th.joinable()) th.join();

}

int main(int argc, char *argv[])
{
    if(argc != 5 )
    {
        cerr<<"Invalid Format : <CLIENT_IP>:<UPLOAD_PORT> <TRACKER_IP_1>:<TRACKER_PORT_1> <TRACKER_IP_2>:<TRACKER_PORT_2> <log_file>"<<endl;
        return 0;
    }

    SERVER = argv[1];
    TR1 = argv[2];
    TR2 = argv[3];
    string action,args;
    thread server(serverInit);
    server.detach();
    while(true)
    {
        getline(cin,args);
        vector<string> argument = split(args,' ');
        action = argument[0];    
        if(action == "gen")
        {
            if(argument.size() != 2)
                fprintf(stderr, "Invalid argument : <filepathname> \n");
            else
            {
                args = argument[1];
                TH.push_back(thread(makeTorrent, args));
            }
        }
        else if(action == "share")
        {
            if(argument.size() != 3)
                fprintf(stderr, "Invalid argument : <absolute_local_file_path> <filename.mtorrent>\n");
            else
                TH.push_back(thread( ShareTorrentWithTracker, argument[1],argument[2]));
        }
        else if(action == "get")
        {
            if(argument.size() != 3)
                fprintf(stderr, "Invalid argument: <local file path> <filename>.mtorrent \n");
            else
                TH.push_back(thread(downloadManager, argument[2], argument[1]));
        }
        else if(action == "remove")
        {
            
            if(argument.size() != 2)
                fprintf(stderr, "Invalid argument: <filename>.mtorrent \n");
            else
                TH.push_back(thread(RemoveTorrentFromTracker, argument[1]));
        }

    }

    return (0);
}