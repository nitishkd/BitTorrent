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

#define PORT 20000
#define LENGTH 1024

void filedownload(int portnum)
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

    printf("[Client] Receiveing file from Server and saving it as final.txt...");
    string fr_name = "final" + to_string(portnum)+ ".pdf";
    FILE *fr = fopen(fr_name.c_str(), "a");
    if(fr == NULL)
        printf("File %s Cannot be opened.\n", fr_name);
    else
    {
        bzero(buffer, LENGTH); 
        int fr_block_sz = 0;
        while((fr_block_sz = recv(sockfd, buffer, LENGTH, 0)) > 0)
        {
            int write_sz = fwrite(buffer, sizeof(char), fr_block_sz, fr);
            if(write_sz < fr_block_sz)
            {
                printf("File write failed.\n");
            }
            bzero(buffer, LENGTH);
            if (fr_block_sz == 0 || fr_block_sz != LENGTH) 
            {
                break;
            }
        }
        if(fr_block_sz < 0)
        {
            if (errno == EAGAIN)
            {
                printf("recv() timed out.\n");
            }
            else
            {
                fprintf(stderr, "recv() failed due to errno = %d\n", errno);
            }
        }
        printf("Ok received from server!\n");
        fclose(fr);
    }
    close (sockfd);
    printf("[Client] Connection lost.\n");

}

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

vector<thread> TH;
int ThreadC;

int main(int argc, char *argv[])
{

    ThreadC = 2;
    for(int i =0; i< ThreadC; ++i)
    {
        TH.push_back(thread(filedownload, 20000+i));
    }
    for(auto &th : TH)
        if(th.joinable()) th.join();
        
    return (0);
}