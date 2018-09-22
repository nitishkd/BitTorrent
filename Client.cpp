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
#include <thread>
using namespace std;

#define PORT 2000
#define LENGTH 524288

vector<thread> TH;
int ThreadC;

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

void receivePackets(vector<int > packets,int port)
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
    
    int portnum = port;
    string TrackerIp = "127.0.0.1";
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
        printf("[Client] Connected to Server at port %d...ok!\n", portnum);
    
    sockfd;
    string sendstr = "";
    for(int i =0; i < packets.size(); ++i)
    {
        sendstr += to_string(packets[i]);
        if(i != (packets.size()-1))
            sendstr += " ";
    }
    
    strcpy(buffer, sendstr.c_str());
    send(sockfd, buffer, strlen(buffer), 0);
    
    string fr_name = "recieve.txt";
    FILE *fr = fopen(fr_name.c_str(), "w");
    if(fr == NULL)
        printf("File %s Cannot be opened.\n", fr_name);
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
        cout<<"No of pieces : "<<pieces.size()<<endl;
        for(int i =0; i < packets.size(); ++i)
        {
            cout<<packets[i]<<" "<<pieces[i].length()<<endl;
            fseek(fr, packets[i]*LENGTH, SEEK_SET);
            // if(pieces[i].length() > LENGTH)
            //     pieces[i].substr(0, LENGTH);
            int write_sz = fwrite(pieces[i].c_str(), sizeof(char), strlen(pieces[i].c_str()), fr);
        }
        printf("Ok received from server!\n");
        fclose(fr);
    }
    close(sockfd);
}

void downloadManager()
{
    vector<int> v,u;
    v.push_back(2);
    v.push_back(1);
v.push_back(0);
v.push_back(3);
v.push_back(5);
v.push_back(4);
v.push_back(6);
v.push_back(9);
v.push_back(7);
v.push_back(8);
v.push_back(12);
v.push_back(11);
v.push_back(13);
v.push_back(10);

    receivePackets(v,2000);
    //receivePackets(u,3000);  
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
        printf("[Client] Connected to server at port %d...ok!\n", portnum);

    string fr_name = "recieve" + to_string(cnt)+ ".png";
    FILE *fr = fopen(fr_name.c_str(), "wb");
    if(fr == NULL)
        printf("File %s Cannot be opened.\n", fr_name);
    else
    {
        bzero(buffer, LENGTH); 
        int fr_block_sz = 0;
        while((fr_block_sz = read(sockfd, buffer, LENGTH)) > 0)
            int write_sz = fwrite(buffer, sizeof(char), fr_block_sz, fr);
        printf("Ok received from server!\n");
        fclose(fr);
    }
    close (sockfd);
    printf("[Client] Connection lost.\n");
    // pthread_exit(EXIT_SUCCESS);

}

void ShareTorrentWithTracker(string FileName)
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
    int portnum = 3000;
    string TrackerIp = "127.0.0.1";
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
    string msg = "share$1";
    
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
    while((fs_block_sz = fread(sdbuf, sizeof(char), LENGTH, fs))>0)
    {
        msg += sdbuf;
        // if(send(sockfd, sdbuf, fs_block_sz, 0) < 0)
        // {
        //     fprintf(stderr, "ERROR: Failed to send file. (errno = %d)\n", errno);
        //     exit(1);
        // }
        bzero(sdbuf, LENGTH);
    }
    send(sockfd, msg.c_str(), strlen(msg.c_str()), 0);    

    fclose(fs);
    printf("Ok sent to Tracker!\n");
    close(sockfd);
    printf("[Client] Connection with Tracker closed.\n");
 
}

void GetSeederListFromTracker()
{
    int sockfd; 
    int nsockfd;
    char buffer[LENGTH]; 
    struct sockaddr_in remote_addr;

    string line;
    std::vector<string> Torrent;
    std::ifstream input("torrentFile.mtorrent");

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
    //send(sockfd, msg, strlen(msg), 0);    

    string sendstr = Torrent.back();
    msg += sendstr;
    strcpy(buffer, msg.c_str());
    send(sockfd, buffer, strlen(buffer), 0);

    // for(int i =0; i <= sendstr.length(); i += LENGTH)
    // {
    //     string buff = sendstr.substr(i, min((unsigned long)LENGTH, sendstr.length() - i));
    //     send(sockfd, buff.c_str(), buff.length(),0);
    // }
    printf("Ok sent to Tracker!\n");

    bzero(buffer, LENGTH); 
    int fr_block_sz = 0;
    string strs = "";
    
    fr_block_sz = read(sockfd, buffer, LENGTH);
        strs = buffer;
    vector<string> V = split(strs, '\n');
    string hash = "";
    for(int i =0; i < V.size(); ++i)
        Torrent.push_back(V[i]);
    
    cout<<Torrent.size()<<endl;
    for(int i =0; i < Torrent.size(); ++i)
        cout<<Torrent[i]<<endl;

    close(sockfd);
    printf("[Client] Connection with Tracker closed.\n");
     
}

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    //ShareTorrentWithTracker("torrentFile.mtorrent");
    //GetSeederListFromTracker();
    downloadManager();
    return (0);
}