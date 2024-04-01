#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <thread>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

using namespace std;
#define MAXBUFLEN 100
#define BACKLOG 10  // Maximum pending connections

class Response {
public:
    int error_code;
    char err_msg[MAXBUFLEN];
    double result;

    Response(int code, const char* msg, double res) : error_code(code), result(res) {
        strncpy(err_msg, msg, MAXBUFLEN - 1);
        err_msg[MAXBUFLEN - 1] = '\0';
    }
};

class Request {
public:
    string command;
    string from_unit;
    string to_unit;
    double value;
    string user_name; // Added field to store user name
};

class State {
public:
    enum Type {
        NOT_IN_STATE = 1,
        HELLO,
        HELP,
        MODE,
        COMMAND_SUCCESS,
        SYNTAX_ERROR_COMMAND_UNRECOGNIZED,
        SYNTAX_ERROR_PARAMETER_ARGUMENTS,
        BAD_SEQUENCE_COMMANDS,
        BAD_CONVERSION_REQUEST
    };

    Type current_state;
    State() : current_state(NOT_IN_STATE) {}

    void setState(Type state) {
        current_state = state;
    }
    Type getState() const {
        return current_state;
    }
};

class CommandHandler {
    public:
        void processInput(const Request& request, State& current_state, string& response_code, const sockaddr_in& client_info) {
            switch (current_state.getState()) {
                case State::NOT_IN_STATE:
                    if (request.command == "HELO") {
                        string ip_address = string(inet_ntoa(client_info.sin_addr));
                        response_code = "200 HELO " + ip_address + " (UDP)"; 
                        current_state.setState(State::HELLO);
                    } else {
                        setResponseCode(response_code, State::SYNTAX_ERROR_COMMAND_UNRECOGNIZED);
                    }
                    break;

                case State::HELLO:
                case State::HELP:
                    if (request.command == "HELP") {
                        response_code = "200 THE FOLLOWING ARE COMMANDS AND PARAMETERS FOR THE CONVERT";
                    } else if (request.command == "AREA" || request.command == "VOL" || request.command == "WGT" || request.command == "TEMP") {
                        enterMode(request.command, current_state, response_code);
                    } else if (request.command == "BYE") {
                        string ip_address = string(inet_ntoa(client_info.sin_addr));
                        response_code = "200 BYE " + ip_address + " (UDP)"; 
                        current_state.setState(State::NOT_IN_STATE); 
                        return; 
                    } else {
                        setResponseCode(response_code, State::BAD_SEQUENCE_COMMANDS);
                    }
                    break;

                case State::MODE:
                    handleCommand(request, response_code);
                    break;
                    
                default:
                    setResponseCode(response_code, State::BAD_SEQUENCE_COMMANDS);
                    break;
            }
        }

    private:
        void enterMode(const string& command, State& current_state, string& response_code) {
            current_state.setState(State::MODE);
            if (command == "AREA") {
                response_code = "210 AREA Mode ready!";
            } else if (command == "VOL") {
                response_code = "220 VOL Mode ready!";
            } else if (command == "WGT") {
                response_code = "230 WGT Mode ready!";
            } else if (command == "TEMP") {
                response_code = "240 TEMP Mode ready!";
            } else {
                setResponseCode(response_code, State::BAD_SEQUENCE_COMMANDS);
            }
        }

        void handleCommand(const Request& request, string& response_code) {
            double result = 0.0;
            if (request.command == "AREA") {
                result = convertArea(request.value, request.from_unit, request.to_unit);
                response_code = "250 " + to_string(result);
            } else if (request.command == "VOL") {
                result = convertVolume(request.value, request.from_unit, request.to_unit);
                response_code = "250 " + to_string(result);
            } else if (request.command == "WGT") {
                result = convertWeight(request.value, request.from_unit, request.to_unit);
                response_code = "250 " + to_string(result);
            } else if (request.command == "TEMP") {
                result = convertTemperature(request.value, request.from_unit, request.to_unit);
                response_code = "250 " + to_string(result);
            } else {
                setResponseCode(response_code, State::BAD_SEQUENCE_COMMANDS);
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
            } else if (from_unit == "FAHR") {
                if (to_unit == "CELS")
                    return (value - 32) * 5 / 9;
                else if (to_unit == "KELV")
                    return (value + 459.67) * 5 / 9;
            } else if (from_unit == "KELV") {
                if (to_unit == "CELS")
                    return value - 273.15;
                else if (to_unit == "FAHR")
                    return (value * 9 / 5) - 459.67;
            }
            return value;
        }

        void setResponseCode(string& response_code, State::Type code) {
            switch (code) {
                case State::HELLO:
                    response_code = "200 HELO";
                    break;
                case State::HELP:
                    response_code = "200 <menu>";
                    break;
                case State::MODE:
                    response_code = "210 MODE ready!";
                    break;
                case State::COMMAND_SUCCESS:
                    response_code = "250 Command Successful";
                    break;
                case State::SYNTAX_ERROR_COMMAND_UNRECOGNIZED:
                    response_code = "500 Syntax Error, command unrecognized.";
                    break;
                case State::SYNTAX_ERROR_PARAMETER_ARGUMENTS:
                    response_code = "501 Syntax error in parameters or arguments.";
                    break;
                case State::BAD_SEQUENCE_COMMANDS:
                    response_code = "503 Bad sequence of commands.";
                    break;
                case State::BAD_CONVERSION_REQUEST:
                    response_code = "504 Bad conversion request.";
                    break;
                default:
                    response_code = "Unknown Error"; // Empty response code for unknown states
                    break;
            }
        }
};

void *get_in_addr(struct sockaddr *sa) {
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
    printf("Waiting for message...\n");
    if ((numbytes = recvfrom(sockfd, message, 512, 0,
        (struct sockaddr*)&their_addr, &addr_len)) == -1) {
        perror("recvfrom");
        exit(1);
    }
    client_info->sin_family = their_addr.sin_family;
    client_info->sin_port = their_addr.sin_port;
    client_info->sin_addr = their_addr.sin_addr;
    memset(client_info->sin_zero, 0, 8);
}

void sendResponse(int sockfd, Response *res, struct sockaddr_in *client_info) {
    int num_bytes;
    printf("%s\n", res->err_msg); // Print the response before sending
    if ((num_bytes = sendto(sockfd, res, sizeof(Response), 0, (struct sockaddr*)client_info, sizeof(*client_info))) == -1) {
        printf("Failed to send response.\n");
        return;
    }
}


void logRequest(const Request* req, struct sockaddr_in *client_info) {
    char ipAddress[INET6_ADDRSTRLEN];
    inet_ntop(client_info->sin_family,
        get_in_addr((struct sockaddr*)client_info),
        ipAddress, sizeof ipAddress);
    //printf("\nRequest: command=%s, value=%f, source IP=%s, port=%d\n", req->command.c_str(), req->value, ipAddress, ntohs(client_info->sin_port));
}

int readUDPPortFromConfig(const char *configFilePath) {
    ifstream configFile(configFilePath);
    if (!configFile.is_open()) {
        cerr << "Error: Unable to open configuration file." << endl;
        return 0;
    }
    string line;
    while (getline(configFile, line)) {
        size_t found = line.find("UDP_PORT=");
        if (found != string::npos) {
            string udpPortStr = line.substr(found + 9); // Length of "UDP_PORT=" is 8
            configFile.close();
            return atoi(udpPortStr.c_str());
        }
    }
    configFile.close();
    cerr << "Error: UDP_PORT not found in configuration file." << endl;
    return 0; // Return 0 as error code
}

int listen(int port, int *sock) {
    struct addrinfo hints, *servinfo, *p;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // set to AF_INET to use IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    int rv, sockfd;
    if ((rv = getaddrinfo(NULL, to_string(port).c_str(), &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
            p->ai_protocol)) == -1) {
            continue;
        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            continue;
        }
        break;
    }
    if (p == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
        return 2;
    }
    freeaddrinfo(servinfo);
    *sock = sockfd;
    return 0;
}

void mainCalculation(Request request, State& current_state, string& response_code, const sockaddr_in& client_info) {
    CommandHandler handler;
    handler.processInput(request, current_state, response_code, client_info);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        cerr << "Format: " << argv[0] << " <config_file_path>" << endl;
        return 1;
    }

    // Read UDP port from configuration file
    int port = readUDPPortFromConfig(argv[1]);
    if (port == 0) {
        cerr << "Invalid UDP port specified in the configuration file." << endl;
        return 1;
    }

    struct sockaddr_in client_info;
    int sockfd;
    listen(port, &sockfd);
    string cmd_init;

    Request request;
    State current_state;
    string response_code;

    while (true) {
        char message[512] = { 0 };
        getNextMessage(sockfd, message, &client_info);
        string str(message); // char array -> string
        istringstream iss(str);
        iss >> request.command;

        // Process HELO command differently to extract user name
        if (request.command == "HELO") {
            iss >> request.user_name; // Extract user name
        } else if (request.command == "SQRMT" || request.command == "SQRML" || request.command == "SQRIN" || request.command == "SQRFT" ||
            request.command == "LTR" || request.command == "GALNU" || request.command == "GALNI" || request.command == "CUBM" ||
            request.command == "KILO" || request.command == "PND" || request.command == "CART" || request.command == "CELS" ||
            request.command == "FAHR" || request.command == "KELV") {
            request.from_unit = request.command;
            request.command = "";
            iss >> request.to_unit >> request.value;
        } else {
            request.to_unit = "";
            request.value = 0.0;
        }
        mainCalculation(request, current_state, response_code, client_info);
        Response res(200, response_code.c_str(), 10.0);
        sendResponse(sockfd, &res, &client_info);
        logRequest(&request, &client_info);
    }

    return 0;
}
