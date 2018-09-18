#include <bits/stdc++.h>
#include <openssl/sha.h>

using namespace std;
#define LENGTH 1024
int main()
{
    string TR1 = "167.233.221.22:4556";
    string TR2 = "167.233.221.23:4557";
    char* fs_name = "/home/nitish/Desktop/image.png";
    char sdbuf[LENGTH];
    unsigned char hash[SHA_DIGEST_LENGTH];
    FILE *fs = fopen(fs_name, "r");
    if(fs == NULL)
    {
        fprintf(stderr, "ERROR: File %s not found.(errno = %d)\n", fs_name, errno);
        exit(1);
    }
    bzero(sdbuf, LENGTH); 
    int fs_block_sz; 
    freopen("torrentFile.mtorrent", "w", stdout);
    printf("%s\n", TR1.c_str());
    printf("%s\n", TR2.c_str());
    while((fs_block_sz = fread(sdbuf, sizeof(char), LENGTH, fs))>0)
    {
        //do stuff
        SHA1((const unsigned char *)sdbuf, fs_block_sz,hash);
        printf("%s", hash);
        bzero(hash, SHA_DIGEST_LENGTH);
        bzero(sdbuf, LENGTH);
    }
    fclose(stdout);
    fclose(fs);

    return 0;
}