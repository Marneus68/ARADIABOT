//usr/bin/g++ ARADIABOT.cpp -o ARADIABOT -std=c++11 -pthread; exit

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
#include <thread> 
#include <vector>

#define MAXLINE 4096

std::string server_hostname = "";
std::string server = "";
std::string channel = "";
std::string name = "";
unsigned int port = 0;

std::string last_person_to_talk = "";

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

std::string _sendername(std::string & line) {
    int a = line.find(":");
    if (a == std::string::npos)
        return "";
    int b = line.find("!"); 
    return line.substr(a+1, b-1);
}

void _ribbit(int sock) {
    if (last_person_to_talk != name) {
        last_person_to_talk = name;
        _send(sock, "PRIVMSG " + channel + " :ribbit\r\n");
    }
}

void _asyncparse(int sock, std::string in) {
    std::stringstream ss(in);
    std::istream_iterator<std::string> begin(ss);
    std::istream_iterator<std::string> end;
    std::vector<std::string> vstrings(begin, end);

    if (vstrings.size() > 1) {
        if (vstrings[0].find("PING") != std::string::npos) {
            // IRC Ping Request
            std::cout << "IRC Ping request received the server." << std::endl;
            _send(sock, "PING " + vstrings[1]);
            _ribbit(sock);
            return;
        }
    } 
    
    if (vstrings.size() > 2) {
        std::string sendername = _sendername(in);
        if (vstrings[1].find("PRIVMSG") != std::string::npos) {
            if (vstrings[2].find(name) != std::string::npos) {
                if (vstrings[3].find(":\001PING") != std::string::npos) {
                    // CTCP Ping Request
                    std::cout << "CTCP Ping request received from " << sendername << "." << std::endl;
                    _send( sock, "NOTICE " + sendername + " :\001PING " + vstrings[4] + "\001\r\n");
                    _ribbit(sock);
                } else if (vstrings[3].find(":\001VERSION") != std::string::npos) {
                    // CTCP Version Request
                    std::cout << "CTCP Version request received from " << sendername << "." << std::endl;
                    _send( sock, "NOTICE " + sendername + " :\001VERSION ARADIABOT:0.1:UNIX\001\r\n");
                } else if (vstrings[3].find(":REGISTER") != std::string::npos) {
                    // REGISTER
                    std::cout << "Adding user " + sendername + " to history service." << std::endl;
                    _send( sock, "PRIVMSG " + sendername + ":" + sendername + " registered.\r\n");
                } else if (vstrings[3].find(":UNREGISTER") != std::string::npos) {
                    // UNREGISTER
                    std::cout << "Removing user " + sendername + " from the history service." << std::endl;
                    _send( sock, "PRIVMSG " + sendername + ":" + sendername + " unregistered.\r\n");
                }
            } else if (vstrings[2].find(channel) != std::string::npos) {
                // Public message on the channel
                std::cout << "Public message on the channel from " << sendername << "." << std::endl;
                last_person_to_talk = sendername;
            }
        }
    }
}

int main(int argc, const char *argv[]) {
    if (argc < 5) {
        std::cout << "Not enough parameters provided." << std::endl << 
                "Usage: ARADIABOT <server name> <port> <channel> <bot name>" << std::endl;
        exit(0);
    }

    server_hostname = argv[1];
    server = "";
    channel = argv[3];
    name = argv[4];
    port = std::stoul(argv[2]);

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
        std::thread(_asyncparse, sockfd, s).detach();
    }
    return 0;
}
