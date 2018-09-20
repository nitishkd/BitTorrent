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
        
    return (0);
}