/*
** listener.c -- a datagram sockets "server" demo
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
#include <unordered_map>
#include <string>
#include <errno.h>

#define MYPORT "4950"	// the port users will be connecting to

#define MAXBUFLEN 100

using namespace std;
enum STATE {HELO = 1, HELP, MODE, VALUES};

//std::unordered_map<struct sockaddr_in, int> session_information;
// Define struct clientcommand
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

// Not working. Please fix.
/* int readUDP_Port(string file_name) { */
/*     ifstream inputFile(file_name); */
/*     string line; */

/*     if (inputFile.is_open()) { */
/*         while (getline(inputFile, line)) { */
/*             if (line.find("UDP_PORT=") != string::npos) { */
/*                 size_t pos = line.find("="); */
/*                  return stoi(line.substr(pos + 1)); */
/*                 break; */
/*             } */
/*         } */
/*         inputFile.close(); */
/*     } */
/*     else { */
/*         cout << "Unable to open file!" << endl; */
/*     } */
/* } */

double convertArea(double value, const string& from_unit, const string& to_unit) {
  unordered_map<string, double> area_factors;
  area_factors = {
        {"SQRMT", 1.0},
        {"SQRML", 0.000000386102},
        {"SQRIN", 0.00064516},
        {"SQRFT", 0.092903}
    };

    double from_factor = area_factors.at(from_unit);
    double to_factor = area_factors.at(to_unit);

    return value * from_factor / to_factor;
}

double convertVolume(double value, const string& from_unit, const string& to_unit) {
  unordered_map<string, double> volume_factors;
  volume_factors = {
        {"LTR", 1.0},
        {"GALNU", 3.78541},
        {"GALNI", 4.54609},
        {"CUBM", 1000.0}
    };

    double from_factor = volume_factors.at(from_unit);
    double to_factor = volume_factors.at(to_unit);

    return value * from_factor / to_factor;
}

double convertWeight(double value, const string& from_unit, const string& to_unit) {
  unordered_map<string, double> weight_factors;
    weight_factors = {
        {"KILO", 1.0},
        {"PND", 0.453592},
        {"CART", 0.0002}
    };

    double from_factor = weight_factors.at(from_unit);
    double to_factor = weight_factors.at(to_unit);

    return value * from_factor / to_factor;
}

double convertTemperature(double value, const string& from_unit, const string& to_unit) {
    if (from_unit == to_unit)
        return value;

    if (from_unit == "CELS") {
        if (to_unit == "FAHR")
            return (value * 9 / 5) + 32;
        else if (to_unit == "KELV")
            return value + 273.15;
    }
    else if (from_unit == "FAHR") {
        if (to_unit == "CELS")
            return (value - 32) * 5 / 9;
        else if (to_unit == "KELV")
            return (value + 459.67) * 5 / 9;
    }
    else if (from_unit == "KELV") {
        if (to_unit == "CELS")
            return value - 273.15;
        else if (to_unit == "FAHR")
            return (value * 9 / 5) - 459.67;
    }

    return value;
}

string getResponseMessage(int code) {
    switch (code) {
    case 200:
    case 210:
    case 220:
    case 230:
    case 240:
    case 250:
      return "" + code;
      break;
    case 500:
        return "500 - Syntax Error, command unrecognized.";
        break;
    case 501:
        return "501 - Syntax error in parameters or arguments.";
        break;
    case 503:
        return "503 - Bad sequence of commands.";
        break;
    case 504:
        return "504 - Bad conversion request.";
        break;
    default:
        return "Unknown response code.";
    }
}

/* double handleCommand(request *req, struct sockaddr_in* address) { */
/*     string from_unit, to_unit; */
/*     double value; */
/*     string response_code; */

/*     //cout << "Enter conversion command (from_unit to_unit value): "; */
/*     //cin >> from_unit >> to_unit >> value; */

/*     double result = 0.0; */
/*     if (mode == "AREA") { */
/*         result = convertArea(value, from_unit, to_unit); */
/*         setResponseCode(250); */
/*     } */
/*     else if (mode == "VOL") { */
/*         result = convertVolume(value, from_unit, to_unit); */
/*         setResponseCode(220); */
/*     } */
/*     else if (mode == "WGT") { */
/*         result = convertWeight(value, from_unit, to_unit); */
/*         setResponseCode(230); */
/*     } */
/*     else if (mode == "TEMP") { */
/*         result = convertTemperature(value, from_unit, to_unit); */
/*         setResponseCode(240); */
/*     } */

/*     cout << "Result: " << result << endl; */
    
/*     return result; */
/* } */

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void getNextMessage(int sockfd, char* message, struct sockaddr_in* client_info) {
    struct sockaddr_in their_addr;
    unsigned int addr_len;
    addr_len = sizeof their_addr;
    int numbytes;
    char s[INET6_ADDRSTRLEN];

    printf("Waiting for message...\n");
    if ((numbytes = recvfrom(sockfd, message, /*message_size=*/512, 0,
        (struct sockaddr*)&their_addr, &addr_len)) == -1) {
        perror("recvfrom");
        exit(1);
    }

    printf("listener: got packet with size %d bytes from %s\n", numbytes,
        inet_ntop(their_addr.sin_family,
            get_in_addr((struct sockaddr*)&their_addr),
            s, sizeof s));

    response res = {200, "OK", 10.0};
    if((numbytes = sendto(sockfd, &res, sizeof(response), 0, (sockaddr*)&their_addr, addr_len)) == -1) {
      printf("Failed to send response, error=%s\n", strerror(errno));
      return;
    }

    printf("Sent response of %d bytes.\n", numbytes);
    //printf("listener: packet is %d bytes long\n", numbytes);
    //printf("listener: packet contains \"%s\"\n", buf);
    //printf("their: %s\n", s);
    //memcpy(client_info, &their_addr, sizeof(their_addr));
    client_info->sin_family = their_addr.sin_family;
    client_info->sin_port = their_addr.sin_port;
    client_info->sin_addr = their_addr.sin_addr;
    memset(client_info->sin_zero, 0, 8);
}

int listen(int port, int *sock) {
    struct addrinfo hints, *servinfo, *p;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET6; // set to AF_INET to use IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    int rv;
    int sockfd;
    if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    printf("getaddrinfo\n");
    // loop through all the results and bind to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
            p->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }
        printf("created socket\n");

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("listener: bind");
            continue;
        }

        printf("socket is bound\n");
        // Successful socket creation and binding.
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
        return 2;
    }

    printf("listen is successful\n");

    freeaddrinfo(servinfo);
    *sock = sockfd;
    return 0;
}

void logRequest(request* req, struct sockaddr_in *client_info) {
  char ipAddress[INET6_ADDRSTRLEN];
  inet_ntop(client_info->sin_family,
            get_in_addr((struct sockaddr*)client_info),
            ipAddress, sizeof ipAddress);
//inet_ntop(AF_INET6, &client_info->sin_addr, ipAddress, INET6_ADDRSTRLEN);
  printf("Request: command=%s, value=%f, source IP= %s, port=%d\n", req->command, req->value, ipAddress, client_info->sin_port);
}

void sendResponse(int sockfd, response *res, struct sockaddr_in *client_info) {
  int num_bytes;
  if((num_bytes = sendto(sockfd, &res, sizeof(response), 0, (sockaddr*)client_info, sizeof(*client_info))) == -1) {
    printf("Failed to send response.\n");
    return;
  }

  printf("Sent response of %d bytes.\n", num_bytes);
}

int main(void)
{
   struct sockaddr_in client_info;
    int sockfd;
    int port = atoi(MYPORT);
    printf("Listining on port %d...\n", port);
    // Handle errors in listen
    listen(port, &sockfd);

    while (true) {
        char message[512] = { 0 };
        request client_request;
        getNextMessage(sockfd, message, &client_info);

        // We need to send the correct response in handleCommand function.
        response res = {200, "OK", 10.0};
        sendResponse(sockfd, &res, &client_info);

        
        /*if (!session_information.find(client_info)) {
            session_information.emplace(client_info, HELO);
            }*/
        
        memcpy(&client_request, message, sizeof(request)); // maybe use memcpy
        logRequest(&client_request, &client_info);
        //handleCommand(&client_request, client_info);
    }
	return 0;
}
