#include <iostream>
#include <string>
#include <map>
#include <unordered_map>
#include <fstream>
#include <sys/socket.h>
#include <functional>
#include <netinet/in.h>
#include <arpa/inet.h> 

using namespace std;

string response_code;


enum STATE {HELO = 1, HELP, MODE, VALUES};



// Custom hash function for sockaddr_in
struct sockaddr_in_hash {
    size_t operator()(const struct sockaddr_in& addr) const {
        // Combine the hash values of IP address and port
        return std::hash<unsigned long>()(addr.sin_addr.s_addr) ^
            std::hash<unsigned short>()(addr.sin_port);
    }
};

// Custom equality predicate for sockaddr_in
struct sockaddr_in_equal {
    bool operator()(const struct sockaddr_in& lhs, const struct sockaddr_in& rhs) const {
        return lhs.sin_addr.s_addr == rhs.sin_addr.s_addr &&
            lhs.sin_port == rhs.sin_port;
    }
};

// Define your unordered_map with custom hash function and equality predicate
std::unordered_map<struct sockaddr_in, int, sockaddr_in_hash, sockaddr_in_equal> session_information;

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

int readUDP_Port(string file_name) {
    ifstream inputFile(file_name);
    string line;

    if (inputFile.is_open()) {
        while (getline(inputFile, line)) {
            if (line.find("UDP_PORT=") != string::npos) {
                size_t pos = line.find("=");
                 return stoi(line.substr(pos + 1));
                break;
            }
        }
        inputFile.close();
    }
    else {
        cout << "Unable to open file!" << endl;
    }
}

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
    switch (code) {
    case 200:
        response_code = "200";
        break;
    case 210:
        response_code = "210";
        break;
    case 220:
        response_code = "220";
        break;
    case 230:
        response_code = "230";
        break;
    case 240:
        response_code = "240";
        break;
    case 250:
        response_code = "250";
        break;
    case 500:
        response_code = "500 - Syntax Error, command unrecognized.";
        break;
    case 501:
        response_code = "501 - Syntax error in parameters or arguments.";
        break;
    case 503:
        response_code = "503 - Bad sequence of commands.";
        break;
    case 504:
        response_code = "504 - Bad conversion request.";
        break;
    default:
        response_code = "Unknown response code.";
    }
}


string enterMode() {
    string mode;
    cout << "Available modes: AREA, VOL, WGT, TEMP" << endl;
    cout << "Enter mode: ";
    cin >> mode;
    return mode;
}

double handleCommand(request *req, struct sockaddr_in* address) {
    /*string from_unit, to_unit;
    double value;

    //cout << "Enter conversion command (from_unit to_unit value): ";
    //cin >> from_unit >> to_unit >> value;

    double result = 0.0;
    if (mode == "AREA") {
        result = convertArea(value, from_unit, to_unit);
        setResponseCode(250);
    }
    else if (mode == "VOL") {
        result = convertVolume(value, from_unit, to_unit);
        setResponseCode(220);
    }
    else if (mode == "WGT") {
        result = convertWeight(value, from_unit, to_unit);
        setResponseCode(230);
    }
    else if (mode == "TEMP") {
        result = convertTemperature(value, from_unit, to_unit);
        setResponseCode(240);
    }

    cout << "Result: " << result << endl;
    */
    return result;
}

double ConversionMode() {
    string mode = enterMode();
    double result = 0.0;

    while (mode == "AREA" || mode == "VOL" || mode == "WGT" || mode == "TEMP") {
        result = handleCommand(mode);
        mode = enterMode();
    }

    setResponseCode(503);
    cout << "Invalid mode! Exiting the program." << endl;
    return result;
}

void getNextMessage(int sockfd, char* message, struct sockaddr_in* client_info) {
    struct sockaddr_storage their_addr;
    addr_len = sizeof their_addr;
    int numbytes;

    printf("Waiting for message...\n");
    if ((numbytes = recvfrom(sockfd, message, 512, 0,
        (struct sockaddr*)&their_addr, &addr_len)) == -1) {
        perror("recvfrom");
        exit(1);
    }

    printf("listener: got packet from %s\n",
        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr*)&their_addr),
            s, sizeof s));
    printf("listener: packet is %d bytes long\n", numbytes);
    // printf("listener: packet contains \"%s\"\n", buf);
    memcpy(client_info, &their_add, sizeof(their_addr));
}

int listen(int port, int *sock) {
    struct addrinfo hints, *servinfo, *p;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // set to AF_INET to use IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    int rv;
    int sockfd;
    if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
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

void logRequest(request* req) {
    printf("Request: command=%s, value=%f", req->command, req->value);
}

int main() {
    string file_name="server.gonfig.txt";
    int port = readUDP_Port(file_name);
    struct sockaddr_in client_info;
    int sockfd;
    // Handle errors in listen
    listen(port, &sockfd);

    while (true) {
        char message[512] = { 0 };
        request client_request;
        getNextMessage(sockfd, message, &client_info);
        if (!session_information.find(client_info)) {
            session_information.emplace(client_info, HELO);
        }
        
        memset(&client_request, message, sizeof(request)); // maybe use memcpy
        logRequest(&client_request);
        //handleCommand(&client_request, client_info);
    }
    ConversionMode();
    return 0;
}
// response code for results
// make sure of seq
//help
//helo
//state machine:
// 1. HELO state: Can only receive HELO command. Can go to 2, 3, 5.
// 2. HELP state: Can only show menu. Can go to  3, 5.
// 3. Mode selection state: Can select mode. Can go to 4, 5
// 4. Giving units and values: Can go to 3, 5. This can be used as state 3.
// 5. Exit.