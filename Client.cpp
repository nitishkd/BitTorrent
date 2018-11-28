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
#include <semaphore.h>

using namespace std;
string logfile;
#define debug(x) fprintf(stderr, "Reached Checkpoint: %d \n", x);
#define BACKLOG 100
#define PORT 2000
#define LENGTH 20480    //61440
int ServerFD;
map<string, string> HashFileMap;
map<string, bool> Download;
map<string, set<int> >FilePiecesAvailable;
vector<thread> TH;
int ThreadC;
sem_t mutexx;

void ShareTorrentWithTracker(string, string);

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
    // freopen(logfile.c_str(), "a+" , stderr);

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
        fprintf(stderr,"[Client] Connected to Tracker at port %d...ok!\n", portnum);
    
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
    fprintf(stderr,"[Client] Connection with Tracker closed.\n");
    return Torrent;
    
     
}            

void receivePackets(vector<int > packets,string IPport, string Filename, string hash)
{
    // freopen(logfile.c_str(), "a+" , stderr);

    int sockfd; 
    int nsockfd;
    char buffer[LENGTH+100]; 
    struct sockaddr_in remote_addr;
    sort(packets.begin(), packets.end());
    // debug(2000);
    vector<string> Vec = split(IPport, ':');
    int portnum = stoi(Vec[1]);
    string TrackerIp = Vec[0];
    
    // debug(2001);

    for(int i=0; i < packets.size(); ++i)
    {
        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        {
            fprintf(stderr, "ERROR: Failed to obtain Socket Descriptor! (errno = %d)\n",errno);
            exit(1);
        }    
        cout<<"Requesting for packet: "<<packets[i]<<endl;
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
            fprintf(stderr,"[Client] Connected to Client for Download at port %d...ok!\n", portnum);    
        
        string sendstr = "GetPackets$";
        sendstr += hash;
        sendstr += "$";
        sendstr += to_string(packets[i]);
        
        strcpy(buffer, sendstr.c_str());
        send(sockfd, buffer, strlen(buffer), 0);
        // debug(101);
        string fr_name = Filename;
            // debug(102);
            //shutdown(nsockfd, SHUT_WR);
            bzero(buffer, LENGTH+100); 
            int fr_block_sz = 0;
            string strs = "";
            
            while((fr_block_sz = recv(sockfd, buffer, LENGTH,0)) > 0)
            {
                // mtx.lock();
                // FILE *fr = fopen(fr_name.c_str(), "ab+");
                // fseek(fr, packets[i]*LENGTH, SEEK_SET);
                // printf("packet no: %d , starting pos: %d \n ",packets[i], packets[i]*LENGTH);
                // printf("%s", buffer);
                // int write_sz = fwrite(buffer, 1,fr_block_sz, fr);
                // FilePiecesAvailable[hash].insert(packets[i]);
                // bzero(buffer, LENGTH+100); 
                // fclose(fr);
                // mtx.unlock();
                
                sem_wait(&mutexx);
                fstream file;
                file.open(fr_name.c_str(),ios::out|ios::in|ios::binary);
                if(file.fail())
                {
                    file.clear();
                    file.open(fr_name.c_str(),ios::out|ios::binary);
                    file.close();    
                    file.open(fr_name.c_str(),ios::out|ios::in|ios::binary);
                
                }
                file.seekp(packets[i]*LENGTH, ios::beg);
                printf("packet no: %d , starting pos: %d \n ",packets[i], packets[i]*LENGTH);
                // printf("%s", buffer);
                file.write(buffer, fr_block_sz);
                FilePiecesAvailable[hash].insert(packets[i]);
                bzero(buffer, LENGTH+100); 
                file.close();
                sem_post(&mutexx);
            }
            
            // debug(104);
            fprintf(stderr,"Ok received from server!\n");
        close(sockfd);
    }
    
}

vector<int> GetPieceList(string IPport,string hash)
{
    // freopen(logfile.c_str(), "a+" , stderr);

    int sockfd; 
    int nsockfd;
    char buffer[LENGTH]; 
    struct sockaddr_in remote_addr;
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
    
    string sendstr = "PacketList$";
    sendstr += hash;
    bzero(buffer, LENGTH);
    strcpy(buffer, sendstr.c_str());
    send(sockfd, buffer, strlen(buffer), 0);
    bzero(buffer, LENGTH + 8);
    string packets = "";
    recv(sockfd, buffer, LENGTH, 0);
    //bzero(buffer, LENGTH);
    packets += buffer;
    vector<string> VT = split(packets,' ');
    vector<int> res;
    for(int i =0; i < VT.size(); ++i)
    {
        if(VT[i] != " ")
            res.push_back(stoi(VT[i]));
    }    
    cout<<"DONE"<<endl;
    return res;

}

vector<vector<int> > Distribute(vector<vector<int> > AllPieces, int no_of_pieces, int Seeders)
{

    vector<vector<int> > R;
    R.resize(Seeders);
    vector<int> piece;
    for(int i =0; i < no_of_pieces; ++i)
    {
        piece.clear();
        for(int j = 0; j < AllPieces.size(); ++j)
        {
            if(binary_search(AllPieces[j].begin(), AllPieces[j].end(), i))
                piece.push_back(j);
        }
        int idx = rand()%((int)piece.size());
        R[piece[idx]].push_back(i);
    }

    return R;

}

void downloadManager(string PathTorrent, string result)
{
    // freopen(logfile.c_str(), "a+" , stderr);

    vector<string> TorrentInfo = GetSeederListFromTracker(PathTorrent);
    vector <string> SeederList;
    for(int i =6; i < TorrentInfo.size(); ++i)
        SeederList.push_back(TorrentInfo[i]);
    vector<thread> LocalThread;
    debug(-1);
    int fileSize = stoi(TorrentInfo[4]);

    int no_of_pieces = (fileSize + LENGTH - 1)/LENGTH;
    debug(0);
    string hash = TorrentInfo[5];
    string filepath;
    filepath = result;
    debug(1);
    if(filepath.back() != '/')
        filepath += "/";
    filepath += TorrentInfo[3];
    HashFileMap[hash] = filepath;
    debug(2);
    vector<vector<int> > AllListofPieces;
    debug(3);
    for(int i =0; i < SeederList.size(); ++i)
        AllListofPieces.push_back(GetPieceList(SeederList[i], hash));
    debug(3);
    vector<vector<int> > Pieces = Distribute(AllListofPieces, no_of_pieces, SeederList.size());
    debug(4);
    thread regT(ShareTorrentWithTracker,filepath, PathTorrent);
    regT.detach();
    
    debug(5000);   
    
    Download.insert({TorrentInfo[3], true});
    for(int i=0; i < Pieces.size(); ++i)   
        LocalThread.push_back(thread(receivePackets, Pieces[i], SeederList[i], filepath, hash));
    debug(5001);
    for(auto &it : LocalThread)
        if(it.joinable()) it.join();
    Download[TorrentInfo[3]] = false;    
    debug(7);    
}

void ShowDownloads()
{
    for(auto it = Download.begin(); it != Download.end(); ++it)
    {
        bool flag = it->second;
        if(flag)
            cout<<"[D] "<<it->first<<endl;
        else
            cout<<"[S] "<<it->first<<endl;
    }
}

//SERIAL DOWNLOAD

void filedownload(int portnum,int cnt)
{
    // freopen(logfile.c_str(), "a+" , stderr);

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
    FILE *fr = fopen(fr_name.c_str(), "w");
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
    // freopen(logfile.c_str(), "a+" , stderr);

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
    FILE *fs = fopen(FileName.c_str(), "r");
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
    // freopen(logfile.c_str(), "a+" , stderr);

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
    FILE *fs = fopen(FileName.c_str(), "r");
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
    // freopen(logfile.c_str(), "a+" , stderr);

    string fs_name = path;
    std::ifstream in(fs_name.c_str(), std::ifstream::ate | std::ifstream::binary);
    int filesize = in.tellg(); 
    in.close();
    vector<string> V = split(fs_name, '/');

    string Fname = V.back();
    char sdbuf[LENGTH];
    FILE *fs = fopen(fs_name.c_str(), "r");
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
    string hashnew = "";
    while((fs_block_sz = fread(sdbuf, sizeof(char), LENGTH, fs))>0)
    {
        string str = "";
        char newstr[20];
        SHA1((unsigned char *)sdbuf, strlen(sdbuf),hash);
        for(int i =0; i < 20; ++i){
            sprintf(newstr,"%02x", hash[i]);
            str.append(newstr);
        }
        hashnew += str;
        // fprintf(output,"%s",str.c_str());
        bzero(hash, SHA_DIGEST_LENGTH);
        bzero(sdbuf, LENGTH);
    }
    int idx = 0;
    string hashnewout = "";
    while(idx < hashnew.length())
    {
        string buff = hashnew.substr(idx, min(LENGTH, (int)hashnew.length()-idx));
        idx += LENGTH;
        string str = "";
        char newstr[20];
        SHA1((unsigned char *)(buff.c_str()), strlen(buff.c_str()),hash);
        for(int i =0; i < 20; ++i){
            sprintf(newstr,"%02x", hash[i]);
            str.append(newstr);
        }
        hashnewout += str;
        fprintf(output,"%s",str.c_str());
        bzero(hash, SHA_DIGEST_LENGTH);
        bzero(sdbuf, LENGTH);
    }
    
    int no_of_pieces = (filesize + LENGTH - 1)/LENGTH;
    for(int i = 0; i < no_of_pieces; ++i)
        FilePiecesAvailable[hashnewout].insert(i);
    fprintf(stderr, "Torrent File Generated \n");
    fclose(output);
    fclose(fs);
}

void error(const char *msg)
{
    freopen(logfile.c_str(), "a+" , stderr);

    perror(msg);
    exit(1);
}

void sendPackets(int nsockfd, string hash, string temp)
{
    char sdbuf[LENGTH];
    bzero(sdbuf, LENGTH);
    //read(nsockfd, sdbuf, LENGTH);
    //string temp = sdbuf;
    //bzero(sdbuf, LENGTH);
    vector<string> Pack = split(temp, ' ');
    vector<int> packet;
    //string hash = Pack[0];
    for(int i =0; i < Pack.size(); ++i)
        packet.push_back(stoi(Pack[i]));
    
    string fs_name = HashFileMap[hash];
    
    FILE *fs = fopen(fs_name.c_str(), "rb");
    if(fs == NULL)
    {
        fprintf(stderr, "[Server] : ERROR: File %s not found on server. (errno = %d)\n", fs_name.c_str(), errno);
        exit(1);
    }
    bzero(sdbuf, LENGTH); 
    int fs_block_sz;
    int k;
    string sendstr = "";
    for(int i =0; i < packet.size(); ++i)
    {
        fseek(fs, packet[i]*LENGTH, SEEK_SET);
        fs_block_sz = fread(sdbuf, sizeof(char), LENGTH, fs);
        cout<<"block_size: "<<fs_block_sz<<endl;    
        if(fs_block_sz > 0)
            send(nsockfd, sdbuf, fs_block_sz,0);
        bzero(sdbuf, LENGTH);    
    }
    debug(101);
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

void ServerHandler(int nsockfd)
{
    char sdbuf[LENGTH];
    bzero(sdbuf, LENGTH);
    string action = "";
    string hash;
    recv(nsockfd, sdbuf, LENGTH,0);
    action += sdbuf;
    bzero(sdbuf, LENGTH);
    
    vector<string> V = split(action, '$');
    if(V[0] == "PacketList")
    {
        string sendavailPacks = "";
        hash = V[1];
        for(auto it = FilePiecesAvailable[hash].begin(); it != FilePiecesAvailable[hash].end(); ++it)
        {
            if(it != FilePiecesAvailable[hash].begin())
                sendavailPacks += " ";
            sendavailPacks += to_string(*it);
        }
        send(nsockfd, sendavailPacks.c_str(), strlen(sendavailPacks.c_str()), 0);
        close(nsockfd);
    }
    else if(V[0] == "GetPackets")
    {
        hash = V[1];
        sendPackets(nsockfd, hash, V[2]);
    }
    
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
        fprintf(stderr,"[Server] Obtaining socket descriptor successfully.\n");

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
        TH.push_back(thread(ServerHandler, nsockfd));
        sin_size = sizeof(struct sockaddr_in);
    }

    for(auto &th : TH)
        if(th.joinable()) th.join();

}

int main(int argc, char *argv[])
{
    sem_init(&mutexx, 0, 1);
    if(argc != 5 )
    {
        cout<<"Invalid Format : <CLIENT_IP>:<UPLOAD_PORT> <TRACKER_IP_1>:<TRACKER_PORT_1> <TRACKER_IP_2>:<TRACKER_PORT_2> <log_file>"<<endl;
        return 0;
    }

    SERVER = argv[1];
    TR1 = argv[2];
    TR2 = argv[3];
    logfile = argv[4];
    // freopen(logfile.c_str(), "a" , stderr);
    
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
                printf( "Invalid argument : <filepathname> \n");
            else
            {
                args = argument[1];
                TH.push_back(thread(makeTorrent, args));
            }
        }
        else if(action == "share")
        {
            if(argument.size() != 3)
                printf("Invalid argument : <absolute_local_file_path> <filename.mtorrent>\n");
            else
                TH.push_back(thread( ShareTorrentWithTracker, argument[1],argument[2]));
        }
        else if(action == "get")
        {
            if(argument.size() != 3)
                printf( "Invalid argument: <local file path> <filename>.mtorrent \n");
            else
                TH.push_back(thread(downloadManager, argument[2], argument[1]));
        }
        else if(action == "remove")
        {
            
            if(argument.size() != 2)
                printf( "Invalid argument: <filename>.mtorrent \n");
            else
                TH.push_back(thread(RemoveTorrentFromTracker, argument[1]));
        }
        else if(args == "show downloads")
        {
            TH.push_back(thread(ShowDownloads));
        }

    }
    return (0);
}