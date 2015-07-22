//usr/bin/g++ ARADIABOT.cpp -o ARADIABOT -std=c++11; exit

#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

#include <iterator>
#include <iostream>
#include <cstdlib>
#include <sstream>
#include <string>
#include <vector>

#define MAXLINE 4096

int _send(int sock, std::string out) {
    return send(sock, out.c_str(), out.size(), 0);
}

int _read(int sock, char* buf) {
    int len;
    for(len=0;recv(sock, buf, 1, 0);len++, buf++)
        if (*buf=='\n')
            if (*(buf-1)=='\r') {
                *(buf+1)='\0';
                return len;
            }
    return 0;
}

int main(int argc, const char *argv[]) {
    if (argc < 5) {
        std::cout << "Not enough parameters provided." << std::endl << 
                "Usage: ARADIABOT <server name> <port> <channel> <bot name>" << std::endl;
        exit(0);
    }

    std::string server_hostname = argv[1];
    std::string server = "";
    std::string channel = argv[3];
    std::string name = argv[4];
    unsigned int port = std::stoul(argv[2]);
    int sockfd, i;

    struct hostent *he;
    struct in_addr **addr_list;

    if (channel.c_str()[0] != '#')
        channel = "#" + channel;
         
    std::cout << "Getting IP list for the hostname " << server_hostname << 
        "..." << std::endl;
    if ((he = gethostbyname((char*)server_hostname.c_str())) == NULL)
        return 1;
 
    addr_list = (struct in_addr **) he->h_addr_list;
     
    std::cout << "Selecting IP address..." << std::endl;
    for(i = 0; addr_list[i] != NULL; i++)  {
        server.assign(inet_ntoa(*addr_list[i]));
        break;
    }

    std::cout << "Connecting to " << server_hostname << " " << " (" << 
            server << ":" << port << ")" << std::endl;

    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return 1;

    if (inet_pton(AF_INET, server.c_str(), &servaddr.sin_addr) <= 0)
        return 1;

    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0 )
        return 1;

    _send(sockfd, "NICK " + name + "\r\n");
    _send(sockfd, "USER  " + name + " "  + server_hostname + " bla :Aradia Megido\r\n");
    std::cout << "Joining channel..." << std::endl;
    _send(sockfd, "JOIN " + channel + "\r\n");

    char recvline[MAXLINE];
    while(_read(sockfd, recvline)) {

        std::string s = recvline;
        std::stringstream ss(s);
        std::istream_iterator<std::string> begin(ss);
        std::istream_iterator<std::string> end;
        std::vector<std::string> vstrings(begin, end);

        if (vstrings.size() > 2) {
            std::cout << vstrings[3] << std::endl;
            if (vstrings[3].find("PING") != std::string::npos) {
                std::cout << "Ping request received." << std::endl;
            } else if (vstrings[3].find("VERSION") != std::string::npos) {
                std::cout << "Version request received." << std::endl;
            }
        }
    }
    return 0;
}
