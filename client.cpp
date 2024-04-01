#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
using namespace std;
#define MAXDATASIZE 100 // max number of bytes we can get at once

typedef struct {
    char command[100];
    double value;
} request;

// Define struct serverresponse
typedef struct {
    int error_code;
    char err_msg[100];
    double result;
} response;

// Define the get_in_addr function
void* get_in_addr(struct sockaddr* sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[]) {
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    char server_ip[100];
    char server_port[10];

    // Read server IP and port from configuration file
    if (argc != 2) {
        cout << "usage: " << argv[0] << " client.conf" << endl;
        exit(1);
    }

    ifstream conf_file(argv[1]);
    if (!conf_file.is_open()) {
        perror("Error opening configuration file");
        exit(1);
    }

    string line;
    while (getline(conf_file, line)) {
        size_t pos = line.find('=');
        if (pos != string::npos) {
            string key = line.substr(0, pos);
            string value = line.substr(pos + 1);
            if (key == "SERVER_IP") {
                strcpy(server_ip, value.c_str());
            } else if (key == "SERVER_PORT") {
                strcpy(server_port, value.c_str());
            }
        }
    }

    conf_file.close();

    if (strlen(server_ip) == 0 || strlen(server_port) == 0) {
        cout << "Error: SERVER_IP or SERVER_PORT not found in configuration file" << endl;
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // set to AF_INET to use IPv4
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(server_ip, server_port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and make a socket
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("talker: socket");
            continue;
        }
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "talker: failed to create socket\n");
        return 2;
    }

    // Send commands and receive responses.
    char cmd[100];
    bool toExit = false;
    cout << "enter command: ";
    cin.getline(cmd, sizeof(cmd));
    request req;

    while (!toExit) {
        strcpy(req.command, cmd);
        req.value = 5;

        if ((numbytes = sendto(sockfd, &req, sizeof(request), 0,
                            p->ai_addr, p->ai_addrlen)) == -1) {
            perror("talker: sendto");
            exit(1);
        }

        response res;
        struct sockaddr their_addr;
        socklen_t addr_len = sizeof(their_addr);

        if ((numbytes = recvfrom(sockfd, &res, sizeof(response), 0,
                                &their_addr, &addr_len)) == -1) {
            perror("recvfrom");
            exit(1);
        }
        // check if msg contains "BYE"
        string responseMessage = res.err_msg;
        string byeMessage = "BYE";
        size_t found = responseMessage.find(byeMessage);
        if (found != string::npos) {
            toExit = true;
            break;
        }

        cout << "enter command: ";
        cin.getline(cmd, sizeof(cmd));
    }
    close(sockfd);
    return 0;
}
