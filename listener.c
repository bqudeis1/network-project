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
#include <map>
#include <iostream>
#include <sstream>
#include <cstring>


#define MYPORT "6666"	// the port users will be connecting to

#define MAXBUFLEN 100

using namespace std;
// Response codes
enum RESPONSE_CODE {
    SUCCESS = 200,
    HELLO_RESPONSE = 200,
    HELP_RESPONSE = 200,
    MODE_RESPONSE = 250,
    SYNTAX_ERROR_WRONG_ORDER = 503
};

// Response messages
unordered_map<int, string> response_messages = {
    {SUCCESS, "Command Success."},
    {HELLO_RESPONSE, "HELO"},
    {HELP_RESPONSE, "<print menu>"},
    {MODE_RESPONSE, "Mode Ready"},
    {SYNTAX_ERROR_WRONG_ORDER, "Wrong Command Order: Mode Not Selected"}
};

// Enum for states
enum STATE { NOT_IN_STATE = 1, HELLO, HELP, MODE };

// Global variables
STATE current_state = NOT_IN_STATE;
string response_code;
int UDP_PORT;
string SERVER_IP;
int SERVER_PORT;
char m[512];

struct clientRequest {
    string command;
    string from_unit;
    string to_unit;
    double value;
} Trequest;

double convertArea(double value, const string& from_unit, const string& to_unit) {
    const map<string, double> area_factors = {
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
    const map<string, double> volume_factors = {
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
    const map<string, double> weight_factors = {
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

void setResponseCode(int code) {
    if (current_state != MODE)
        response_code = to_string(code) + " " + response_messages[code];

}


//string getResponseMessage(int code) {
//    switch (code) {
//    case 200:
//    case 210:
//    case 220:
//    case 230:
//    case 240:
//    case 250:
//        return "" + code;
//        break;
//    case 500:
//        return "500 - Syntax Error, command unrecognized.";
//        break;
//    case 501:
//        return "501 - Syntax error in parameters or arguments.";
//        break;
//    case 503:
//        return "503 - Bad sequence of commands.";
//        break;
//    case 504:
//        return "504 - Bad conversion request.";
//        break;
//    default:
//        return "Unknown response code.";
//    }
//}

void enterMode(string str) {
    //cout << response_messages[MODE_RESPONSE] << endl;
    //setResponseCode(MODE_RESPONSE);
    current_state = MODE;
    if (str == "AREA") {
        response_code = "210 AREA MODE READY!";
    }
    else if (str == "VOL") {
        response_code = "220 VOL MODE READY!";
    }
    else if (str == "WGT") {
        response_code = "230 WGT MODE READY!";
    }
    else if (str == "TEMP") {
        response_code = "240 TEMP MODE READY!";
    }
    else {
        //cout << "Invalid command!" << endl;
        setResponseCode(SYNTAX_ERROR_WRONG_ORDER);
        return;
    }
}

void handleCommand(const clientRequest& request) {
    double result = 0.0;

    if (request.command == "AREA") {
        result = convertArea(request.value, request.from_unit, request.to_unit);
        response_code = "250" + to_string(result);
    }
    else if (request.command == "VOL") {
        result = convertVolume(request.value, request.from_unit, request.to_unit);
        response_code = "250" + to_string(result);
    }
    else if (request.command == "WGT") {
        result = convertWeight(request.value, request.from_unit, request.to_unit);
        response_code = "250" + to_string(result);
    }
    else if (request.command == "TEMP") {
        result = convertTemperature(request.value, request.from_unit, request.to_unit);
        response_code = "250" + to_string(result);
    }
    else {
        cout << "Invalid command!" << endl;
        setResponseCode(SYNTAX_ERROR_WRONG_ORDER);
        return;
    }

    //cout << response_messages[MODE_RESPONSE] << ": " << result << endl;
}

// Function to process input
void processInput(const clientRequest& request) {
    switch (current_state) {
    case NOT_IN_STATE:
        if (request.command == "HELO") {
            cout << response_messages[HELLO_RESPONSE] << endl;
            setResponseCode(HELLO_RESPONSE);
            current_state = HELLO;
        }
        else {
            cout << response_messages[SYNTAX_ERROR_WRONG_ORDER] << endl;
            setResponseCode(SYNTAX_ERROR_WRONG_ORDER);
        }
        break;
    case HELLO:
        if (request.command == "HELP") {
            cout << response_messages[HELP_RESPONSE] << endl;
            setResponseCode(HELP_RESPONSE);
        }
        else if (request.command == "AREA" || request.command == "VOL" || request.command == "WGT" || request.command == "TEMP") {
            enterMode(request.command);
        }
        else {
            cout << response_messages[SYNTAX_ERROR_WRONG_ORDER] << endl;
            setResponseCode(SYNTAX_ERROR_WRONG_ORDER);
        }
        break;

    case HELP:
        if (request.command == "HELP") {
            cout << response_messages[HELP_RESPONSE] << endl;
            setResponseCode(HELP_RESPONSE);
        }
        else if (request.command == "AREA" || request.command == "VOL" || request.command == "WGT" || request.command == "TEMP") {
            enterMode(request.command);
        }
        else {
            cout << response_messages[SYNTAX_ERROR_WRONG_ORDER] << endl;
            setResponseCode(SYNTAX_ERROR_WRONG_ORDER);
        }
        break;

    case MODE:
        handleCommand(request);
        break;
    }
}

// Function to print response
void printResponse() {
    cout << response_code << endl;
}
string mainCalco(clientRequest& request) {
    //cout << "Enter input command: ";
    //cin >> request.command;

    if (request.command != "BYE") {
        if (request.command == "HELO" || request.command == "HELP") {
            processInput(request);
            //printResponse();
            //printResponse();
        }
        else if (request.command == "AREA" || request.command == "VOL" || request.command == "WGT" || request.command == "TEMP") {
            processInput(request);
            //printResponse();
            //printResponse();
        }
        else {

        }
        if (current_state == MODE) {
            //cout << "Enter from_unit to_unit value: ";
            //cin >> request.from_unit >> request.to_unit >> request.value;
            //if (request.from_unit == "AREA" || request.from_unit == "VOL" || request.from_unit == "WGT" || request.from_unit == "TEMP") {
            //    current_state = HELLO;
            //    request.command = request.from_unit;
            //    processInput(request);
            //    printResponse();
            //    printResponse();
            //    continue;

            //}
            //else
            //{
            //    cin >> request.to_unit >> request.value;
            //}
            processInput(request);
            //printResponse();
            //printResponse();
        }
        //printResponse();

        //cout << "Enter input command: ";
        //cin >> request.command;
    }
    else {

        setResponseCode(SUCCESS);
        response_code = "BYE";
        printResponse();
    }
    return response_code;
    //cout << "BYE" << endl;
}

//std::unordered_map<struct sockaddr_in, int> session_information;526
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
    strcpy(m, message);

    //m = message;
}

int listen(int port, int *sock) {
    struct addrinfo hints, *servinfo, *p;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // set to AF_INET to use IPv4
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
void parseInputString(const std::string& input, clientRequest& req) {
    std::istringstream iss(input);
    iss >> req.command;

    if (req.command == "SQRMT" || req.command == "SQRML" || req.command == "SQRIN" || req.command == "SQRFT" ||
        req.command == "LTR" || req.command == "GALNU" || req.command == "GALNI" || req.command == "CUBM" ||
        req.command == "KILO" || req.command == "PND" || req.command == "CART" || req.command == "CELS" ||
        req.command == "FAHR" || req.command == "KELV") {
        // Parse three strings and a double value
        req.from_unit = req.command;
        req.command = "";
        iss >> req.to_unit >> req.value;
    }
    else {
        // If command is not one of the specified units, then parse it as a single string
        //req.from_unit = req.command;
        req.to_unit = ""; // or any default value
        req.value = 0.0;  // or any default value
    }
}
int main(void)
{
   struct sockaddr_in client_info;
    int sockfd;
    int port = atoi(MYPORT);
    printf("Listining on port %d...\n", port);
    // Handle errors in listen
    listen(port, &sockfd);
    string cmd_init;

    clientRequest request3;
    while (true) {
        char message[512] = { 0 };
        request client_request;
        getNextMessage(sockfd, message, &client_info);
        //strcpy(request3.command, str.c_str());
        string str(m);
        parseInputString(str, request3);
        printf("reached message = % s/n", m);
        // We need to send the correct response in handleCommand function.
        //response res = {200, "OK", 10.0};
        mainCalco(request3);
        response res = { 200, "OK", 10.0 };
        //res.err_msg = response_code;
        strcpy(res.err_msg, response_code.c_str());
        printf("sent response: error_code=%d, message=%s, result=%f\n", res.error_code, res.err_msg, res.result);
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
