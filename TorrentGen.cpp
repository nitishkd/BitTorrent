#include <bits/stdc++.h>
#include <openssl/sha.h>

using namespace std;
#define LENGTH 524288


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

int main(int argc, char const *argv[])
{

    if(argc != 4 )
    {
        cerr<<"Invalid Argument : <trackerip1:port> <trackerip2:port> <filenamewithpath>"<<endl;
        return 0;
    }
    string TR1 = argv[1];
    string TR2 = argv[2];
    string fs_name = argv[3];
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
    //fprintf(output,"\n"); 
    fclose(output);
    fclose(fs);

    return 0;
}