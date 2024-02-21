/*
** talker.c -- a datagram "client" demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <vector>
#include <iostream>
#include <string>
#include <cstdio>
#include <fstream>


using namespace std;

#define SERVERPORT "6666"	// the port users will be connecting to

typedef struct {
    char command[50];
    double value;
} request;
//global
string SERVER_IP;
int SERVER_PORT;
// Define struct serverresponse
typedef struct {
    int error_code;
    char err_msg[100];
    double result;
} response;

typedef struct {
  request req;
  response res;
} testCommand;

vector<vector<testCommand>> tests; 
  

void initTestCases() {
  // Each test case consists of a list of requests and their expected responses.
  tests.push_back(vector<testCommand>());
  tests[0].push_back({/*req=*/{"HELO", 5.0}, /*res=*/{200, "", 0.0}});
  tests[0].push_back({/*req=*/{"HELP", 10.0}, /*res=*/{200, "", 0.0}});
                     
  // Add more tests cases below.
}
// Define the get_in_addr function
void* get_in_addr(struct sockaddr* sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
void readServerConfig(string filename) {
	ifstream inputFile(filename);
	string line;

	if (inputFile.is_open()) {
		while (getline(inputFile, line)) {
			if (line.find("SERVER_IP=") != string::npos) {
				size_t pos = line.find("=");
				SERVER_IP = line.substr(pos + 1);
			}
			else if (line.find("SERVER_PORT=") != string::npos) {
				size_t pos = line.find("=");
				SERVER_PORT = atoi(line.substr(pos + 1).c_str());
			}
		}
		cout << "Port: " << SERVER_PORT << endl;
		cout << "IP: " << SERVER_IP << endl;
		inputFile.close();
	}
	else {
		cout << "Unable to open server_config.txt!" << endl;
	}
}




int main(int argc, char *argv[])
{
	readServerConfig(argv[1]);
	
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;

        // Each client should randomly pick a test case.
        initTestCases();
        int test_index = 0;
    
	if (argc != 2) {
		fprintf(stderr,"usage: talker hostname message\n");
		exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; // set to AF_INET to use IPv4
	hints.ai_socktype = SOCK_DGRAM;

	if ((rv = getaddrinfo("localhost", std::to_string(SERVER_PORT).c_str(), &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and make a socket
	for(p = servinfo; p != NULL; p = p->ai_next) {
	//while(true){
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
		string cmd;
		cout << "enter command: ";
		getline(cin, cmd);
		request req;
        //for(int i = 0; i < tests[test_index].size(); i++) {
          //testCommand command = tests[test_index][i];
		while(true){
			
			strcpy(req.command, cmd.c_str()); req.value = 5;
          if ((numbytes = sendto(sockfd, &req, sizeof(request), 0,
                                 p->ai_addr, p->ai_addrlen)) == -1) {
            perror("talker: sendto");
            exit(1);
          }

          //freeaddrinfo(servinfo);

	//printf("talker: sent %d bytes to %s\n", numbytes, argv[1]);

          // Now receive the response
          response res;
          sockaddr_in their_addr;
          socklen_t addr_len = sizeof(their_addr);
          if ((numbytes = recvfrom(sockfd, &res, sizeof(response) , 0,
		(struct sockaddr *)&their_addr, &addr_len)) == -1) {
		perror("recvfrom");
		exit(1);
          }
          //printf("Received response: error_code=%d, message=%s, result=%f\n", res.error_code, res.err_msg, res.result);
		  //line22
		  if (strcmp(res.err_msg, "200 Command Success.") == 0) {
			  // Declaration of ipAddress should be inside this block
			  char ipAddress[INET6_ADDRSTRLEN];
			  inet_ntop(their_addr.sin_family,
				  get_in_addr((struct sockaddr*)&their_addr),
				  ipAddress, sizeof ipAddress);
			  snprintf(res.err_msg, sizeof(res.err_msg), "200 HELO %s (UDP)", ipAddress);
		  }
		  //printf("Talker address: %s:%d\n", ipAddress, ntohs(their_addr.sin_port));
		  
		  //line22
		  //printf("Received response: error_code=%d, message=%s, result=%f\n", res.error_code, res.err_msg, res.result);
		  printf("%s\n", res.err_msg);
		  if (strcmp(res.err_msg, "BYE") == 0) break;
          // Below, add error message if response is not what you expect.

          // And lastly, if you want to test multiple clients then add a random delay below so the interactions are mixed in the server
		  cout << "enter command: ";
		  getline(cin, cmd);
        }

	close(sockfd);

	return 0;
}
