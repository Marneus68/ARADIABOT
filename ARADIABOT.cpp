
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

#include <iostream>
#include <string>

#define MAXLINE 4096


int _send(int sockfd, std::string out) {
    return send(sockfd, out.c_str(), out.size(), 0);
}

int main(int argc, const char *argv[])
{
    unsigned int port = 6667;
    char *server_hostname = "irc.rizon.net";
    char server[] = "";
    int sockfd, i;

    struct hostent *he;
    struct in_addr **addr_list;
         
    if ((he = gethostbyname(server_hostname)) == NULL) {
        return 0;
    }
 
    addr_list = (struct in_addr **) he->h_addr_list;
     
    for(i = 0; addr_list[i] != NULL; i++)  {
        strcpy(server , inet_ntoa(*addr_list[i]));
        break;
    }

    std::cout << "Connecting to " << server_hostname << " " << " (" << 
            server << ":" << port << ")" << std::endl;

    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return 1;
    }

    if (inet_pton(AF_INET, server, &servaddr.sin_addr) <= 0) {
        return 1;
    }

    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0 ) {
        return 1;
    }    

    _send(sockfd, "NICK ARADIABOT\r\n");
    std::string userstring = "USER ARADIABOT " + std::string(server_hostname) + " bla :Aradia Megido\r\n";
    _send(sockfd, userstring);
    _send(sockfd, "JOIN #iridia\r\n");

    while(1) {
        char recvline[MAXLINE+1];
        if (read(sockfd, recvline, MAXLINE) > 0) {
            std::string line = std::string(recvline);
            if (line.find(":PING") != std::string::npos) {
                std::cout << "Received ping request..." << std::endl;
            } else {
                std::cout << line << std::endl;
            } 
        }
    }

    return 0;
}
