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
#include <fstream>


#define MYPORT "6666"	// the port users will be connecting to

#define MAXBUFLEN 100

using namespace std;
struct clientRequest {
    string command;
    string from_unit;
    string to_unit;
    double value;
} Trequest;
void parseInputString(const std::string& input, clientRequest& req);

// Response codes
enum RESPONSE_CODE {
    SUCCESS = 200,
    HELLO_RESPONSE = 200,
    HELP_RESPONSE = 200,
    MODE_RESPONSE = 250,
    SYNTAX_ERROR_WRONG_ORDER = 503,
    Syntax_Error_command_unrecognized = 500,
    Syntax_error_in_parameters = 501,
    Bad_conversion_request = 504
};

// Response messages
unordered_map<int, string> response_messages = {
    {SUCCESS, "Command Success."},
    {HELLO_RESPONSE, "HELO"},
    {HELP_RESPONSE, "<print menu>"},
    {MODE_RESPONSE, "Mode Ready"},
    {SYNTAX_ERROR_WRONG_ORDER, "Wrong Command Order"},
    {Syntax_Error_command_unrecognized,"Syntax Error, command unrecognized."},
    {Syntax_error_in_parameters,"Syntax error in parameters or arguments"},
    {Bad_conversion_request,"Bad conversion request"}

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
char msg[100];
string lastMode;
char ipAddr[INET6_ADDRSTRLEN];



double convertArea(double value, const string& from_unit, const string& to_unit) {
    const map<string, double> area_factors = {
        {"SQRMT", 1.0},
        {"SQRML", 0.000000386102},
        {"SQRIN", 0.00064516},
        {"SQRFT", 0.092903}
    };
    if ((from_unit == "SQRMT" || from_unit == "SQRML" || from_unit == "SQRIN" || from_unit == "SQRFT") &&
        (to_unit == "SQRMT" || to_unit == "SQRML" || to_unit == "SQRIN" || to_unit == "SQRFT")) {
        double from_factor = area_factors.at(from_unit);
        double to_factor = area_factors.at(to_unit);
        return value * from_factor / to_factor;

    }else if (from_unit == "LTR" || from_unit == "GALNU" || from_unit == "GALNI" || from_unit == "CUBM" ||
        from_unit == "KILO" || from_unit == "PND" || from_unit == "CART" || from_unit == "CELS" ||
        from_unit == "FAHR" || from_unit == "KELV" ||
        to_unit == "LTR" || to_unit == "GALNU" || to_unit == "GALNI" || to_unit == "CUBM" ||
        to_unit == "KILO" || to_unit == "PND" || to_unit == "CART" || to_unit == "CELS" ||
        to_unit == "FAHR" || to_unit == "KELV") {
        return -1;
    }
        return -2;
}

double convertVolume(double value, const string& from_unit, const string& to_unit) {
    const map<string, double> volume_factors = {
        {"LTR", 1.0},
        {"GALNU", 3.78541},
        {"GALNI", 4.54609},
        {"CUBM", 1000.0}
    };

    if ((from_unit == "LTR" || from_unit == "GALNU" || from_unit == "GALNI" || from_unit == "CUBM") &&
        (to_unit == "LTR" || to_unit == "GALNU" || to_unit == "GALNI" || to_unit == "CUBM")) {
        double from_factor = volume_factors.at(from_unit);
        double to_factor = volume_factors.at(to_unit);
        return value * from_factor / to_factor;
    }
    else if (from_unit == "SQRMT" || from_unit == "SQRML" || from_unit == "SQRIN" || from_unit == "SQRFT" ||
        from_unit == "KILO" || from_unit == "PND" || from_unit == "CART" || from_unit == "CELS" ||
        from_unit == "FAHR" || from_unit == "KELV" ||
        to_unit == "SQRMT" || to_unit == "SQRML" || to_unit == "SQRIN" || to_unit == "SQRFT" ||
        to_unit == "KILO" || to_unit == "PND" || to_unit == "CART" || to_unit == "CELS" ||
        to_unit == "FAHR" || to_unit == "KELV") {
        return -1;
    }

    return -2;
}


double convertWeight(double value, const string& from_unit, const string& to_unit) {
    const map<string, double> weight_factors = {
        {"KILO", 1.0},
        {"PND", 0.453592},
        {"CART", 0.0002}
    };

    if ((from_unit == "KILO" || from_unit == "PND" || from_unit == "CART") &&
        (to_unit == "KILO" || to_unit == "PND" || to_unit == "CART")) {
        double from_factor = weight_factors.at(from_unit);
        double to_factor = weight_factors.at(to_unit);
        return value * from_factor / to_factor;
    }
    else if (from_unit == "SQRMT" || from_unit == "SQRML" || from_unit == "SQRIN" || from_unit == "SQRFT" ||
        from_unit == "LTR" || from_unit == "GALNU" || from_unit == "GALNI" || from_unit == "CUBM" ||
        from_unit == "CELS" || from_unit == "FAHR" || from_unit == "KELV" ||
        to_unit == "SQRMT" || to_unit == "SQRML" || to_unit == "SQRIN" || to_unit == "SQRFT" ||
        to_unit == "LTR" || to_unit == "GALNU" || to_unit == "GALNI" || to_unit == "CUBM" ||
        to_unit == "CELS" || to_unit == "FAHR" || to_unit == "KELV") {
        return -1;
    }

    return -2;
}


double convertTemperature(double value, const string& from_unit, const string& to_unit) {
    if (from_unit == to_unit)
        return value;

    if ((from_unit == "SQRMT" || from_unit == "SQRML" || from_unit == "SQRIN" || from_unit == "SQRFT" ||
        from_unit == "LTR" || from_unit == "GALNU" || from_unit == "GALNI" || from_unit == "CUBM") ||
        (to_unit == "SQRMT" || to_unit == "SQRML" || to_unit == "SQRIN" || to_unit == "SQRFT" ||
            to_unit == "LTR" || to_unit == "GALNU" || to_unit == "GALNI" || to_unit == "CUBM")) {
        return -1;
    }


    if ((from_unit == "CELS" && (to_unit == "FAHR" || to_unit == "KELV")) ||
        (from_unit == "FAHR" && (to_unit == "CELS" || to_unit == "KELV")) ||
        (from_unit == "KELV" && (to_unit == "CELS" || to_unit == "FAHR"))) {
        if (from_unit == "CELS" && to_unit == "FAHR")
            return (value * 9 / 5) + 32;
        else if (from_unit == "CELS" && to_unit == "KELV")
            return value + 273.15;
        else if (from_unit == "FAHR" && to_unit == "CELS")
            return (value - 32) * 5 / 9;
        else if (from_unit == "FAHR" && to_unit == "KELV")
            return (value + 459.67) * 5 / 9;
        else if (from_unit == "KELV" && to_unit == "CELS")
            return value - 273.15;
        else if (from_unit == "KELV" && to_unit == "FAHR")
            return (value * 9 / 5) - 459.67;
    }

    return -2;
}

void setResponseCode(int code) {
    if (current_state != MODE)
        response_code = to_string(code) + " " + response_messages[code];

}
void setResponseCode2(int code) {
        response_code = to_string(code) + " " + response_messages[code];

}

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
        setResponseCode(Syntax_Error_command_unrecognized);
        return;
    }
}

void handleCommand( clientRequest& request) {
    double result = 0.0;
    request.command = lastMode;
    if (request.command == "AREA") {
        result = convertArea(request.value, request.from_unit, request.to_unit);
        if (result == -1) {
            setResponseCode2(Bad_conversion_request);
        }
        else if (result == -2) {
            setResponseCode2(Syntax_error_in_parameters);
        }else
        response_code = "250 " + to_string(result);
    }
    else if (request.command == "VOL") {
        result = convertVolume(request.value, request.from_unit, request.to_unit);
        if (result == -1) {
            setResponseCode2(Bad_conversion_request);
        }
        else if (result == -2) {
            setResponseCode2(Syntax_error_in_parameters);
        }
        else
        response_code = "250 " + to_string(result);
    }
    else if (request.command == "WGT") {
        result = convertWeight(request.value, request.from_unit, request.to_unit);
        if (result == -1) {
            setResponseCode2(Bad_conversion_request);
        }
        else if (result == -2) {
            setResponseCode2(Syntax_error_in_parameters);
        }
        else
        response_code = "250 " + to_string(result);
    }
    else if (request.command == "TEMP") {
        result = convertTemperature(request.value, request.from_unit, request.to_unit);
        if (result == -1) {
            setResponseCode2(Bad_conversion_request);
        }
        else if (result == -2) {
            setResponseCode2(Syntax_error_in_parameters);
        }
        else
        response_code = "250 " + to_string(result);
    }
    else {
        //cout << "Invalid command!" << endl;
        setResponseCode(SYNTAX_ERROR_WRONG_ORDER);
        return;
    }
    cout << "res_coooooooooooooode: " <<result << " " << response_code << endl;
    //cout << response_messages[MODE_RESPONSE] << ": " << result << endl;
}

// Function to process input
void processInput( clientRequest& request) {
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
            response_code += "\n modes:VOL, AREA, TEMP, WGT\n";

        }
        else if (request.command == "AREA" || request.command == "VOL" || request.command == "WGT" || request.command == "TEMP") {
            enterMode(request.command);
            cout << "reached entermode\n";
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
            response_code += "\n modes:VOL, AREA, TEMP, WGT\n";
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
        if (request.command == "HELO") {
            setResponseCode(SYNTAX_ERROR_WRONG_ORDER);
        }
        else if (request.command == "HELP") {
            cout << response_messages[HELP_RESPONSE] << endl;
            setResponseCode(HELP_RESPONSE);
            response_code += "\n modes:VOL, AREA, TEMP, WGT\n";
        }
        else {
            handleCommand(request);
            break;
        }
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
        if (current_state == MODE && (request.command != "AREA" && request.command != "VOL" && request.command != "WGT" && request.command != "TEMP")) {
            //cout << "Enter from_unit to_unit value: ";
            //cin >> request.from_unit >> request.to_unit >> request.value;
            //if (request.from_unit == "AREA" || request.from_unit == "VOL" || request.from_unit == "WGT" || request.from_unit == "TEMP") {
            //    current_state = HELLO;
            //    request.command = request.from_unit;
            //    processInput(request);
                //current_state = HELLO;
                //printResponse();
            //    printResponse();
            //    continue;

            //}
            //else
            //{
            //    cin >> request.to_unit >> request.value;
            //}
            cout << request.command << "1Handle " << request.from_unit << "2 " << request.to_unit << "3 " << request.value << endl;
            processInput(request);
            printResponse();
            //printResponse();
        }else 
            if (current_state == MODE && (request.command == "AREA" || request.command == "VOL" || request.command == "WGT" || request.command == "TEMP")) {
                current_state = HELLO;
                lastMode = request.command;
                processInput(request);
                printResponse();

            }else if (request.command == "HELO" || request.command == "HELP") {
            processInput(request);
            printResponse();
            //printResponse();
            cout << request.command << endl;
            //line22

        }
        else if (request.command == "AREA" || request.command == "VOL" || request.command == "WGT" || request.command == "TEMP") {
            //if (current_state == MODE) current_state = HELLO;
            processInput(request);
            //if (current_state = MODE) current_state = HELLO;
            //current_state = HELLO;
            cout << "request.command"<<request.command << endl;
            lastMode = request.command;
            printResponse();
            //printResponse();
        }
        else {
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
response resP = { 200, "OK", 10.0 };
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
    //from here
    clientRequest request3;
    cout << "message recieved" << message;
    //strcpy(request3.command, str.c_str());
    string str(message);
    cout << "message recieved in str" << str << endl;
    parseInputString(str, request3);
    //printf("reached message = % s\n", m);
    // We need to send the correct response in handleCommand function.
    //response res = {200, "OK", 10.0};
    mainCalco(request3);
    cout << "main calco done\n";
    response res = { 200, "msg", 10.0 };
    strcpy(msg, response_code.c_str());
    cout << "msg " << msg << endl;
    strcpy(res.err_msg, msg);        //res.err_msg = response_code; 
    //response resP = {200, "OK", 10.0};
    if((numbytes = sendto(sockfd, &res, sizeof(response), 0, (sockaddr*)&their_addr, addr_len)) == -1) {
      printf("Failed to send response, error=%s\n", strerror(errno));
      return;
    }
    //to here
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
    if ((rv = getaddrinfo(NULL, std::to_string(port).c_str(), &hints, &servinfo)) != 0) {
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
  //ipAddr= ipAddress;
  printf("Request: command=%s, port=%d\n", req->command, client_info->sin_port);
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
    cout << req.command << "1 "<< req.from_unit <<"2 "<< req.to_unit <<"3 "<< req.value << endl;
}
void readUDP_Port(string filename) {
    ifstream inputFile(filename);
    string line;

    if (inputFile.is_open()) {
        while (getline(inputFile, line)) {
            if (line.find("UDP_PORT=") != string::npos) {
                size_t pos = line.find("=");
                UDP_PORT = stoi(line.substr(pos + 1));
                //cout << "from udp" << UDP_PORT;
                break;
            }
        }
        inputFile.close();
    }
    else {
        cout << "Unable to open "<<filename<<"!" << endl;
    }
}
int main(int argc, char* argv[])
{
    if (argc != 2) {
        fprintf(stderr, "usage: server config filename \n");
        exit(1);
    }
    string filename = argv[1];
    readUDP_Port(filename);
   struct sockaddr_in client_info;
    int sockfd;
    int port = atoi(MYPORT);
    //cout << UDP_PORT<<endl;
    
    printf("Listining on port %d...\n", UDP_PORT);
    // Handle errors in listen
    listen(UDP_PORT, &sockfd);
    string cmd_init;

    clientRequest request3;
    while (true) {
        char message[512] = { 0 };
        request client_request;
        getNextMessage(sockfd, message, &client_info);
        //fromhere
        
        //printf("sent response: error_code=%d, message=%s, result=%f\n", res.error_code, res.err_msg, res.result);
        //sendResponse(sockfd, &res, &client_info);

        
        /*if (!session_information.find(client_info)) {
            session_information.emplace(client_info, HELO);
            }*/
        
        
        memcpy(&client_request, message, sizeof(request)); // maybe use memcpy
        logRequest(&client_request, &client_info);
        //handleCommand(&client_request, client_info);
    }
	return 0;
}
