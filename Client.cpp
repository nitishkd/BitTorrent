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
#define LENGTH 1025

vector<thread> TH;
int ThreadC;
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
        while((fr_block_sz = read(sockfd, buffer, LENGTH-1)) > 0)
            int write_sz = fwrite(buffer, sizeof(char), fr_block_sz, fr);
        printf("Ok received from server!\n");
        fclose(fr);
    }
    close (sockfd);
    printf("[Client] Connection lost.\n");
    // pthread_exit(EXIT_SUCCESS);

}

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{

    ThreadC = 40;
    for(int i =0; i< ThreadC; ++i)
    {
        TH.push_back(thread(filedownload, 2000,i));
        
    }
    for(auto &th : TH)
        if(th.joinable()) th.join();
        
    return (0);
}