
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

#include <iterator>
#include <iostream>
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

int main(int argc, const char *argv[])
{
    unsigned int port = 6667;
    char *server_hostname = (char*)"irc.rizon.net";
    char server[] = "";
    int sockfd, i;

    struct hostent *he;
    struct in_addr **addr_list;
         
    std::cout << "Getting IP list for the hostname " << server_hostname << 
        "..." << std::endl;
    if ((he = gethostbyname(server_hostname)) == NULL)
        return 1;
 
    addr_list = (struct in_addr **) he->h_addr_list;
     
    std::cout << "Selecting IP address..." << std::endl;
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

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return 1;

    if (inet_pton(AF_INET, server, &servaddr.sin_addr) <= 0)
        return 1;

    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0 )
        return 1;

    _send(sockfd, "NICK ARADIABOT\r\n");
    std::string userstring = "USER ARADIABOT " + 
            std::string(server_hostname) + " bla :Aradia Megido\r\n";
    _send(sockfd, userstring);
    std::cout << "Joining channel..." << std::endl;
    _send(sockfd, "JOIN #iridia\r\n");

    char recvline[MAXLINE];
    while(_read(sockfd, recvline)) {

        std::string s = recvline;
        std::stringstream ss(s);
        std::istream_iterator<std::string> begin(ss);
        std::istream_iterator<std::string> end;
        std::vector<std::string> vstrings(begin, end);

        std::string::size_type loc = s.find( "PING", 0 );
        if(loc != std::string::npos ){
            if (vstrings[2].compare("ARADIABOT")==0) {
                std::cout << "Ping request received." << std::endl;
                std::string pong = "PONG :" + vstrings[4] + "\r\n";
                _send(sockfd, pong);
                _send(sockfd, "PRIVMSG #iridia :ribbit\r\n");
            }
        }
    }
    return 0;
}
