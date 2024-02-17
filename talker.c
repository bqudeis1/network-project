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

using namespace std;

#define SERVERPORT "6666"	// the port users will be connecting to

typedef struct {
    char command[50];
    double value;
} request;

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

int main(int argc, char *argv[])
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;

        // Each client should randomly pick a test case.
        initTestCases();
        int test_index = 0;
        
	if (argc != 3) {
		fprintf(stderr,"usage: talker hostname message\n");
		exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; // set to AF_INET to use IPv4
	hints.ai_socktype = SOCK_DGRAM;

	if ((rv = getaddrinfo(argv[1], SERVERPORT, &hints, &servinfo)) != 0) {
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
        for(int i = 0; i < tests[test_index].size(); i++) {
          testCommand command = tests[test_index][i];
          if ((numbytes = sendto(sockfd, &command.req, sizeof(request), 0,
                                 p->ai_addr, p->ai_addrlen)) == -1) {
            perror("talker: sendto");
            exit(1);
          }

          freeaddrinfo(servinfo);

	printf("talker: sent %d bytes to %s\n", numbytes, argv[1]);

          // Now receive the response
          response res;
          sockaddr_in their_addr;
          socklen_t addr_len = sizeof(their_addr);
          if ((numbytes = recvfrom(sockfd, &res, sizeof(response) , 0,
		(struct sockaddr *)&their_addr, &addr_len)) == -1) {
		perror("recvfrom");
		exit(1);
          }
          printf("Received response: error_code=%d, message=%s, result=%f\n", res.error_code, res.err_msg, res.result);

          // Below, add error message if response is not what you expect.

          // And lastly, if you want to test multiple clients then add a random delay below so the interactions are mixed in the server
        }

	close(sockfd);

	return 0;
}
