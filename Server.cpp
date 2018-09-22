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
#define PORT 2000
#define BACKLOG 100
#define LENGTH 524288


void error(const char *msg)
{
    perror(msg);
    exit(1);
}

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

vector<thread> TH;
int ThreadC;

void sendPackets(int nsockfd)
{
    char sdbuf[LENGTH+8];
    bzero(sdbuf, LENGTH+8);
    read(nsockfd, sdbuf, LENGTH);
    string temp = sdbuf;
    bzero(sdbuf, LENGTH+8);
    vector<string> Pack = split(temp, ' ');
    vector<int> packet;
    for(int i =0; i < Pack.size(); ++i)
        packet.push_back(stoi(Pack[i]));
    
    char* fs_name = "/home/nitish/Desktop/input.txt";
    
    FILE *fs = fopen(fs_name, "r");
    if(fs == NULL)
    {
        fprintf(stderr, "ERROR: File %s not found on server. (errno = %d)\n", fs_name, errno);
        exit(1);
    }
    bzero(sdbuf, LENGTH+8); 
    int fs_block_sz;
    //sort(packet.begin(), packet.end());
    int k;
    string sendstr = "";
    for(int i =0; i < packet.size(); ++i)
    {
        
        fseek(fs, packet[i]*LENGTH, SEEK_SET);
        fs_block_sz = fread(sdbuf, sizeof(char), LENGTH, fs);
        if(fs_block_sz > 0)
        {
            cout<<strlen(sdbuf)<<endl;
            sendstr += sdbuf;
            if(i != (packet.size()-1) )
                sendstr += "##########";    
        }
        bzero(sdbuf, LENGTH+8);    
    }
    vector<string> VV = splitBig(sendstr, "##########");
    cout<<VV.size()<<endl;
    for(int i =0; i < VV.size(); ++i)
        cout<<i<<" "<<VV[i].length()<<endl;
    for(int i =0; i <= sendstr.length(); i += LENGTH)
    {
        string buff ="";
        buff =  sendstr.substr(i, min((unsigned long)LENGTH, strlen(sendstr.c_str()) - i));
        cout<<i<<" "<<strlen(buff.c_str())<<endl;
        send(nsockfd, buff.c_str(), strlen(buff.c_str()),0);
    }
    //send(nsockfd,sendstr.c_str(),strlen(sendstr.c_str()), 0 );
    fclose(fs);
    printf("Ok sent to client!\n");
    close(nsockfd);
    printf("[Server] Connection with Client closed. Server will wait now...\n");
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
    printf("Ok sent to client!\n");
    close(nsockfd);
    printf("[Server] Connection with Client closed. Server will wait now...\n");
    // pthread_exit(EXIT_SUCCESS);
}

int main ()
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

    addr_local.sin_family = AF_INET;
    addr_local.sin_port = htons(PORT);
    addr_local.sin_addr.s_addr = INADDR_ANY;
    bzero(&(addr_local.sin_zero), 8);

    if( bind(sockfd, (struct sockaddr*)&addr_local, sizeof(struct sockaddr)) == -1 )
    {
        fprintf(stderr, "ERROR: Failed to bind Port. (errno = %d)\n", errno);
        exit(1);
    }
    else 
        printf("[Server] Binded tcp port %d in addr 127.0.0.1 sucessfully.\n",PORT);

    if(listen(sockfd,BACKLOG) == -1)
    {
        fprintf(stderr, "ERROR: Failed to listen Port. (errno = %d)\n", errno);
        exit(1);
    }
    else
        printf ("[Server] Listening the port %d successfully.\n", PORT);

    int success = 0;
    sin_size = sizeof(struct sockaddr_in);
    while((nsockfd = accept(sockfd, (struct sockaddr *)&addr_remote, &sin_size)) != -1)
    {
        printf("[Server] Server has got connected from %s.\n", inet_ntoa(addr_remote.sin_addr));    
        TH.push_back(thread(sendPackets, nsockfd));
        sin_size = sizeof(struct sockaddr_in);
    }

    for(auto &th : TH)
        if(th.joinable()) th.join();

    return 0;
}