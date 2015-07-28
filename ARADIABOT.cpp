//usr/bin/g++ ARADIABOT.cpp -o ARADIABOT -std=c++11 -pthread; exit

#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

#include <functional>
#include <iterator>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <sstream>
#include <string>
#include <cstdio>
#include <thread> 
#include <vector>
#include <chrono>
#include <map>
#include <ctime>

#define MAXLINE 4096

std::string server_hostname = "";
std::string server = "";
std::string channel = "";
std::string name = "";
unsigned int port = 0;
unsigned int line_numbers = 0;

int timeout = 550;

std::string last_person_to_talk = "";
std::string last_direct_sender = "";

std::string history_file = "history.log";
std::string users_file = "users.log";

std::map<std::string, int> registered_users;
std::map<std::string, bool> history_users;

int _send(int sock, std::string out);
int _read(int sock, char* buf);
void _write(std::string line);
void _writeusers();
void _ribbit(int sock);
void _asyncparse(int sock, std::string in);

std::string _getTime();
std::string _sendername(std::string & line);

std::map<std::string, std::function<void(int sock, std::string, std::string)>> privmsg_actions = {
    {":\001PING", [](int sock, std::string sender, std::string str){
                std::cout << "CTCP Ping request received from " << sender << "." << std::endl;
                _send( sock, "NOTICE " + sender + " :\001PING " + str + "\001\r\n");
            }},
    {":\001VERSION", [](int sock, std::string sender, std::string str){
                std::cout << "CTCP Version request received from " << sender << "." << std::endl;
                _send( sock, "NOTICE " + sender + " :\001VERSION ARADIABOT:0.1:UNIX\001\r\n");
            }},
    {":REGISTER", [](int sock, std::string sender, std::string str){
                std::cout << "Adding user " + sender + " to history service." << std::endl;
                _send( sock, "PRIVMSG " + sender + " :You've been added to the list of registered users.\r\n");
                registered_users[sender] = 1;
                _writeusers();
            }},
    {":UNREGISTER", [](int sock, std::string sender, std::string str){
                std::cout << "Request from " + sender + " to unregister from the history service." << std::endl;
                if (registered_users.find(sender) != registered_users.end()) {
                    registered_users.erase(sender);
                    std::cout << "Removing registered user " + sender + " from the history service." << std::endl;
                    _send( sock, "PRIVMSG " + sender + " :You've been removed from the list of registered users.\r\n");
                } else {
                    std::cout << "User " + sender + " is not registered in the history service." << std::endl;
                    _send( sock, "PRIVMSG " + sender + " :You're not on the list of registered users.\r\n");
                }
                _writeusers();
            }},
    {":LIST", [](int sock, std::string sender, std::string str){
                std::cout << "Request from " + sender + " to get the list of registered users:" << std::endl;
                if (registered_users.size()) {
                    _send( sock, "PRIVMSG " + sender + " :Here is the list of registered users:\r\n");
                    for(auto o : registered_users) {
                        _send( sock, "PRIVMSG " + sender + " : - " + o.first +"\r\n");
                        std::cout << " - " << o.first << std::endl;
                        std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
                    }
                    return;
                } 
                _send( sock, "PRIVMSG " + sender + " :No users registered yet.\r\n");
            }},
    {":STOP", [](int sock, std::string sender, std::string str){
                std::cout << "Request from " + sender + " to stop history loop." << std::endl;
                if(history_users[sender])
                    history_users[sender] = false;
                _send( sock, "PRIVMSG " + sender + " :y0u are an undecided kind of tr0ll.:\r\n");
            }},
    {":HISTORY", [](int sock, std::string sender, std::string str){
                std::cout << "Request from " + sender + " to get his relative history." << std::endl;
                if (registered_users.find(sender) != registered_users.end()) {
                    if (registered_users[sender] == 1) {
                        _send( sock, "PRIVMSG " + sender + " :You haven't left the channel since you registered to the history service.\r\n");
                        return;
                    }
                    if(history_users.find(sender) == history_users.end())
                        history_users.insert(std::par<std::string,bool>(sender,true));
                    if(!history_users[sender])
                        history_users[sender] = true;
                    _send( sock, "PRIVMSG " + sender + " :Here is what was said while you were away:\r\n");

                    std::ifstream file(history_file);
                    std::string line;
                    if (file.good()) {
                        int i = 0;
                        while (std::getline(file, line) && history_users[sender]) {
                            if (i > registered_users[sender]-2) {
                                _send( sock, "PRIVMSG " + sender + " :  || " + line + "\r\n");
                                std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
                            }
                            i++;
                        }
                    }
                    file.close();
                    history_users[sender] = false;
                } else {
                    _send( sock, "PRIVMSG " + sender + " :You're not registered to the history service.\r\n");
                }
            }},
    {":WRITE", [](int sock, std::string sender, std::string str) {
                std::cout << "Request from " + sender + " to write the list of users." << std::endl;
                _writeusers();
            }},
    {":HELP", [](int sock, std::string sender, std::string str) {
                std::cout << "Request from " + sender + " to get the list of all available commands." << std::endl;
                _send( sock, "PRIVMSG " + sender + " :Here is the list of available commands:\r\n");
                _send( sock, "PRIVMSG " + sender + " : REGISTER       Registers the user to the history service.\r\n");
                _send( sock, "PRIVMSG " + sender + " : UNREGISTER     Unregisters the user from the history service.\r\n");
                _send( sock, "PRIVMSG " + sender + " : LIST           Shows list of registered users.\r\n");
                _send( sock, "PRIVMSG " + sender + " : HISTORY        Sends you what you missed while you weren't here.\r\n");
                _send( sock, "PRIVMSG " + sender + " : WRITE          Request the list of registered users to be saved\r\n");
                _send( sock, "PRIVMSG " + sender + " : HELP           Shows this help message.\r\n");
                _send( sock, "PRIVMSG " + sender + " : R              ribbit\r\n");
            }},
    {":R", [](int sock, std::string sender, std::string str) {
                if (sender != last_direct_sender) {
                    last_direct_sender = sender;
                    _ribbit(sock);
                }
            }}
};

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

void _write(std::string line) {
    try {
        std::ofstream file;
        file.open(history_file, std::ios_base::app);
        file << line<< std::endl;
        line_numbers++;
        file.close();
    } catch (...) {
        std::cerr << "Error while writing in the history file (" << history_file << ")." << std::endl;
    }
}

void _writeusers() {
    remove(users_file.c_str());
    try {
        std::ofstream file;
        file.open(users_file, std::ios_base::app);
        for (auto o : registered_users) {
            file << o.first << " " << o.second << std::endl;
        }
        file.close();
    } catch (...) {
        std::cerr << "Error while writing in the users file (" << users_file << ")." << std::endl;
    }
}

void _ribbit(int sock) {
    if (last_person_to_talk != name) {
        last_person_to_talk = name;
        _send(sock, "PRIVMSG " + channel + " :ribbit\r\n");
        _write(name + "\t :ribbit");
    }
}

std::string _getTime(){
    time_t t = time(0);
    struct tm * now = localtime( & t );
    char* buffer;
    strftime(buffer,sizeof(buffer),"[%H:%M:%S]",now);
    return buffer;

}

std::string _upperCase(std::string word){ 
    std::transform(word.begin(), word.end(),word.begin(), ::toupper);
    return word;
}

void _asyncparse(int sock, std::string in) {
    std::stringstream ss(in);
    std::istream_iterator<std::string> begin(ss);
    std::istream_iterator<std::string> end;
    std::vector<std::string> vstrings(begin, end);

    if (vstrings.size() > 1) {
        if (vstrings[0].find("PING") != std::string::npos) {
            // IRC Ping Request
            std::cout << "IRC Ping request received from the server." << std::endl;
            _send(sock, "PING " + vstrings[1] + "\r\n");
            return;
        }
    } 
    
    if (vstrings.size() > 2) {
        std::string sendername = _sendername(in);
        if (vstrings[1].find("PRIVMSG") != std::string::npos) {
            if (vstrings[2].find(name) != std::string::npos) {
                std::string remains = "";
                for (int i = 4; i < vstrings.size(); i++)
                    remains += " " + vstrings[i];
                for (auto act : privmsg_actions) {
                    if (_upperCase(vstrings[3]).compare(act.first) == 0) {
                        act.second(sock, sendername, remains);
                        break;
                    }
                }
            } else if (vstrings[2].find(channel) != std::string::npos) {
                // Public message on the channel
                std::cout << "Public message on the channel from " << sendername << "." << std::endl;
                std::string remains = "";
                last_person_to_talk = sendername;

                for (int i = 3; i < vstrings.size(); i++)
                    remains += " " + vstrings[i];

                _write(_getTime()+"\t"+sendername + "\t " + remains);

            }
        } else if (vstrings[1].find("QUIT") != std::string::npos) {
            // Someone left the channel
            std::string sendername = _sendername(in);
            if (sendername.compare(name)) {
                std::cout << _getTime() << " " << sendername << " left " << channel << "." << std::endl;
                _write(_getTime() + " " + sendername + " left " + channel);

                if (registered_users.find(sendername) != registered_users.end())
                    registered_users[sendername] = line_numbers;
            }
            return;
        } else if (vstrings[1].find("JOIN") != std::string::npos) {
            // Someone joined the channel
            std::string sendername = _sendername(in);
            if (sendername.compare(name)) {
                std::cout << _getTime() << " " << sendername << " joined " << channel << "." << std::endl;
                _write(_getTime() + " " + sendername + " joined " + channel);

                if (registered_users.find(sendername) != registered_users.end()) {
                    _send(sock, "PRIVMSG " + sendername + " :It appears as if you're registered to the history service.\r\n");
                    _send(sock, "PRIVMSG " + sendername + " :Type in \"/MSG " + name + " HISTORY\" to see what you've missed.\r\n");
                } else {
                    _send(sock, "PRIVMSG " + sendername + " :Welcome to " + channel + ".\r\n");
                    _send(sock, "PRIVMSG " + sendername + " :I'm " + name + " and if you don't already know, you can type in \"/MSG " + name + " HELP\" to see what I can do for you.\r\n");
                }
            }
            return;
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

    std::string line;

    // Loading the number of lines in the local history file
    std::cout << "Checking local history file." << std::endl;
    std::ifstream histfile(history_file);
    if (histfile.good()) {
        while (std::getline(histfile, line))
            line_numbers++;
        std::cout << line_numbers << " lines in recorded history." << std::endl;
    } else {
        std::cout << "No prior history file." << std::endl;
    }

    // Grabs the registered users from the local file
    std::cout << "Checking list of registered users." << std::endl;
    std::ifstream usefile(users_file);
    if (usefile.good()) {
        int numusers = 0;

        while (std::getline(usefile, line)) {
            std::stringstream ss(line);
            std::string item;
            std::vector<std::string> splits;
            while (std::getline(ss, item, ' ')) {
                splits.push_back(item);
            } 
            std::cout << splits[0] << " " << splits[1] << std::endl;
            registered_users[splits[0]] = std::stoi(splits[1]);
            numusers++;
        }
        std::cout << numusers << " registered users." << std::endl;
    } else {
        std::cout << "No previously registered users." << std::endl;
    }

    _write(name + " " + " joined " + channel + ".");

    char recvline[MAXLINE];
    while(_read(sockfd, recvline)) {
        std::string s = recvline;
        std::thread(_asyncparse, sockfd, s).detach();
    }
    
    _write(_getTime() + " " + name + " " + " left " + channel + ".");
    _writeusers();

    return 0;
}
