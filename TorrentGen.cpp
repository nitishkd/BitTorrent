#include <bits/stdc++.h>
#include <openssl/sha.h>

using namespace std;
#define LENGTH 524288
int main()
{
    string TR1 = "167.233.221.22:4556";
    string TR2 = "167.233.221.23:4557";
    char* fs_name = "/home/nitish/Desktop/bla.cpp";
    std::ifstream in(fs_name, std::ifstream::ate | std::ifstream::binary);
    int filesize = in.tellg(); 
    in.close();
    string Fname = "image.png";
    char sdbuf[LENGTH];
    FILE *fs = fopen(fs_name, "rb");
    if(fs == NULL)
    {
        fprintf(stderr, "ERROR: File %s not found.(errno = %d)\n", fs_name, errno);
        exit(1);
    }
    bzero(sdbuf, LENGTH); 
    int fs_block_sz; 
    FILE* output = fopen("torrentFile.mtorrent", "w");
    unsigned char hash[LENGTH];
    fprintf(output,"%s\n", TR1.c_str());
    fprintf(output,"%s\n", TR2.c_str());
    fprintf(output,"%s\n", Fname.c_str());
    fprintf(output,"%d\n", filesize);
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
    fclose(stdout);
    fclose(fs);

    return 0;
}