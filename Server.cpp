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
#define PORT 20000 
#define BACKLOG 5
#define LENGTH 1024


void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main ()
{
    int sockfd; 
    int nsockfd; 
    int num;
    socklen_t sin_size; 
    struct sockaddr_in addr_local; /* client addr */
    struct sockaddr_in addr_remote; /* server addr */
    char buffer[LENGTH]; // Receiver buffer

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
    while(success == 0)
    {
        sin_size = sizeof(struct sockaddr_in);

        if ((nsockfd = accept(sockfd, (struct sockaddr *)&addr_remote, &sin_size)) == -1) 
        {
            fprintf(stderr, "ERROR: Obtaining new Socket Despcritor. (errno = %d)\n", errno);
            exit(1);
        }
        else 
            printf("[Server] Server has got connected from %s.\n", inet_ntoa(addr_remote.sin_addr));


            char* fs_name = "/home/nitish/Desktop/Torrent/OS.pdf";
            char sdbuf[LENGTH];
            printf("[Server] Sending %s to the Client...", fs_name);
            FILE *fs = fopen(fs_name, "r");
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
                    fprintf(stderr, "ERROR: Failed to send file %s. (errno = %d)\n", fs_name, errno);
                    exit(1);
                }
                bzero(sdbuf, LENGTH);
            }
            printf("Ok sent to client!\n");
            success = 1;
            close(nsockfd);
            printf("[Server] Connection with Client closed. Server will wait now...\n");
            while(waitpid(-1, NULL, WNOHANG) > 0);
        //}
    }
}