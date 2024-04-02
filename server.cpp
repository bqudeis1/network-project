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
        string from_unit="";
        string to_unit="";
        double value=0;
        string user_name; 
        string mode;

        void clearCalc(){
            from_unit=""; 
            to_unit=""; 
            value=0;
        };
};

class State {
    public:
        bool AREA = false;
        bool VOL = false;
        bool TEMP = false;
        bool WGT = false;
        bool badConversion = false;

        enum Type {
            NOT_IN_STATE = 1, HELLO, HELP, MODE,
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
        void setMode(string Mode) {
            if (Mode == "AREA") {
                AREA = 1;
            } if (Mode == "VOL") {
                VOL = 1;
            } if (Mode == "WGT") {
                WGT = 1;
            } if (Mode == "TEMP") {
                TEMP = 1;
            }           
        }
        string getMode() const {
            if (AREA) {
                return "AREA";
            } if (VOL) {
                return "VOL";
            } if (WGT) {
                return "WGT";
            } if (TEMP) {
                return "TEMP";
            } else {
                return 0;
            }
        }
        void setAllFalse() {
            AREA = 0; 
            VOL = 0; 
            WGT = 0; 
            TEMP = 0;
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
                        setResponseCode(response_code, State::BAD_SEQUENCE_COMMANDS);
                    }
                    break;

                case State::HELLO:
                case State::HELP:
                    if (request.command == "HELP") {
                        response_code = "200 <menu>";
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
                    if (request.command == "AREA" || request.command == "VOL" || request.command == "WGT" || request.command == "TEMP") {
                        current_state.setAllFalse();
                        enterMode(request.command, current_state, response_code);
                    } 
                    if ((request.command == "SQRMT" || request.command == "SQRML" || request.command == "SQRIN" || request.command == "SQRFT") && current_state.getMode() == "AREA" && current_state.badConversion == false) {
                        if (current_state.badConversion == false) {
                            response_code = "250 " + convertArea(request.value, request.from_unit, request.to_unit);
                        }
                    } 
                    if ((request.command == "LTR" || request.command == "GALNU" || request.command == "GALNI" || request.command == "CUBM") && current_state.getMode() == "VOL" && current_state.badConversion == false) {
                        if (current_state.badConversion == false) {
                            response_code = "250 " + convertVolume(request.value, request.from_unit, request.to_unit);
                        }
                    } 
                    if ((request.command == "KILO" || request.command == "PND" || request.command == "CART") && current_state.getMode() == "WGT" && current_state.badConversion == false) {
                        if (current_state.badConversion == false) {
                            response_code = "250 " + convertWeight(request.value, request.from_unit, request.to_unit);
                        }                       
                    } 
                    if ((request.command == "CELS" || request.command == "FAHR" || request.command == "KELV") && current_state.getMode() == "TEMP" && current_state.badConversion == false) {
                        if (current_state.badConversion == false) {
                            response_code = "250 " + convertTemperature(request.value, request.from_unit, request.to_unit);
                        }  
                    }
                    
                    if (request.command == "BYE") {
                        string ip_address = string(inet_ntoa(client_info.sin_addr));
                        response_code = "200 BYE " + ip_address + " (UDP)"; 
                        current_state.setState(State::NOT_IN_STATE); 
                        return; 
                    }
                    break;
                    
                default:
                    setResponseCode(response_code, State::SYNTAX_ERROR_COMMAND_UNRECOGNIZED);
                    break;
            }
        }

        void enterMode(const string& command, State& current_state, string& response_code) {
            current_state.setState(State::MODE);
            if (command == "AREA") {
                response_code = "210 AREA Mode ready!";
                current_state.setMode("AREA");
            } else if (command == "VOL") {
                response_code = "220 VOL Mode ready!";
                current_state.setMode("VOL");
            } else if (command == "WGT") {
                response_code = "230 WGT Mode ready!";
                current_state.setMode("WGT");
            } else if (command == "TEMP") {
                response_code = "240 TEMP Mode ready!";
                current_state.setMode("TEMP");
            } else {
                setResponseCode(response_code, State::SYNTAX_ERROR_COMMAND_UNRECOGNIZED); 
            }
        }

        string convertArea(double value, const string& from_unit, const string& to_unit) {
            if (from_unit == "SQRMT") {
                if (to_unit == "SQRML") return to_string(value / 2589988.11);
                if (to_unit == "SQRIN") return to_string(value * 1550);
                if (to_unit == "SQRFT") return to_string(value * 10.764);
            } else if (from_unit == "SQRML") {
                if (to_unit == "SQRMT") return to_string(value * 2589988.11);
                if (to_unit == "SQRIN") return to_string(value * 4014489600);
                if (to_unit == "SQRFT") return to_string(value * 27878555.87);
            } else if (from_unit == "SQRIN") {
                if (to_unit == "SQRMT") return to_string(value / 1550);
                if (to_unit == "SQRML") return to_string(value / 4014489600);
                if (to_unit == "SQRFT") return to_string(value / 144);
            } else if (from_unit == "SQRFT") {
                if (to_unit == "SQRMT") return to_string(value / 10.764);
                if (to_unit == "SQRML") return to_string(value / 27878555.87);
                if (to_unit == "SQRIN") return to_string(value * 144);
            } else if (from_unit == to_unit) {value = value;}
            return ""; // Handle other cases
        }

        string convertVolume(double value, const string& from_unit, const string& to_unit) {
            if (from_unit == "LTR") {
                if (to_unit == "GALNU") return to_string(value / 3.785);
                if (to_unit == "GALNI") return to_string(value / 4.546);
                if (to_unit == "CUBM") return to_string(value / 1000);
            } else if (from_unit == "GALNU") {
                if (to_unit == "LTR") return to_string(value * 3.785);
                if (to_unit == "GALNI") return to_string(value * 0.83267384);
                if (to_unit == "CUBM") return to_string(value / 264.2);
            } else if (from_unit == "GALNI") {
                if (to_unit == "LTR") return to_string(value * 4.546);
                if (to_unit == "GALNU") return to_string(value / 0.83267384);
                if (to_unit == "CUBM") return to_string(value / 220);
            } else if (from_unit == "CUBM") {
                if (to_unit == "LTR") return to_string(value * 1000);
                if (to_unit == "GALNU") return to_string(value * 264.2);
                if (to_unit == "GALNI") return to_string(value * 220);
            } else if (from_unit == to_unit) {value = value;}
            return ""; // Handle other cases
        }

        string convertWeight(double value, const string& from_unit, const string& to_unit) {
            if (from_unit == "KILO") {
                if (to_unit == "PND") return to_string(value * 2.205);
                if (to_unit == "CART") return to_string(value * 5000);
            } else if (from_unit == "PND") {
                if (to_unit == "KILO") return to_string(value / 2.205);
                if (to_unit == "CART") return to_string(value * 2268);
            } else if (from_unit == "CART") {
                if (to_unit == "KILO") return to_string(value / 5000);
                if (to_unit == "PND") return to_string(value / 2268);
            } else if (from_unit == to_unit) {value = value;}
            return ""; // Handle other cases
        }

        string convertTemperature(double value, const string& from_unit, const string& to_unit) {
            if (from_unit == "CELS") {
                if (to_unit == "FAHR") return to_string((value * 9 / 5) + 32);
                if (to_unit == "KELV") return to_string(value + 273.15);
            } else if (from_unit == "FAHR") {
                if (to_unit == "CELS") return to_string((value - 32) * 5 / 9);
                if (to_unit == "KELV") return to_string((value + 459.67) * 5 / 9);
            } else if (from_unit == "KELV") {
                if (to_unit == "CELS") return to_string(value - 273.15);
                if (to_unit == "FAHR") return to_string((value * 9 / 5) - 459.67);
            } else if (from_unit == to_unit) {value = value;}
            return ""; // Handle other cases
        }


        void setResponseCode(string& response_code, State::Type code) {
            switch (code) {
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
    inet_ntop(client_info->sin_family, get_in_addr((struct sockaddr*)client_info), ipAddress, sizeof ipAddress);
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
            string udpPortStr = line.substr(found + 9); // len of "UDP_PORT=" is 8
            configFile.close();
            return atoi(udpPortStr.c_str());
        }
    }
    configFile.close();
    cerr << "Error: UDP_PORT not found in configuration file." << endl;
    return 0;
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

    // Vector to hold thread objects
    vector<thread> threads;

    while (true) {
        char message[512] = { 0 };
        getNextMessage(sockfd, message, &client_info);
        string str(message); // char array -> string
        istringstream iss(str);
        iss >> request.command;

        if (request.command == "HELO") {
            iss >> request.user_name;
        } 
        if ((request.command == "SQRMT" || request.command == "SQRML" || request.command == "SQRIN" || request.command == "SQRFT") && current_state.getMode() == "AREA"){
            request.from_unit = request.command;
            iss >> request.to_unit;
            if (request.to_unit == "SQRMT" || request.to_unit == "SQRML" || request.to_unit == "SQRIN" || request.to_unit == "SQRFT") {
                iss >> request.value;
                current_state.setMode("AREA");
                current_state.badConversion = false;
            } else {
                cout << "504 Bad conversion request." << endl;
                current_state.badConversion = true;
            }
        } 
        if ((request.command == "LTR" || request.command == "GALNU" || request.command == "GALNI" || request.command == "CUBM") && current_state.getMode() == "VOL"){
            request.from_unit = request.command;
            iss >> request.to_unit;
            if (request.to_unit == "LTR" || request.to_unit == "GALNU" || request.to_unit == "GALNI" || request.to_unit == "CUBM") {
                iss >> request.value;
                current_state.setMode("VOL");
                current_state.badConversion = false;
            } else {
                cout << "504 Bad conversion request." << endl;
                current_state.badConversion = true;
            }
        } 
        if ((request.command == "KILO" || request.command == "PND" || request.command == "CART") && current_state.getMode() == "WGT"){
            request.from_unit = request.command;
            iss >> request.to_unit;
            if (request.to_unit == "KILO" || request.to_unit == "PND" || request.to_unit == "CART") {
                iss >> request.value;
                current_state.setMode("WGT");
                current_state.badConversion = false;
            } else {
                cout << "504 Bad conversion request." << endl;
                current_state.badConversion = true;
            }
        } 
        if ((request.command == "CELS" || request.command == "FAHR" || request.command == "KELV") && current_state.getMode() == "TEMP"){
            request.from_unit = request.command;
            iss >> request.to_unit;
            if (request.to_unit == "CELS" || request.to_unit == "FAHR" || request.to_unit == "KELV") {
                iss >> request.value;
                current_state.setMode("TEMP");
                current_state.badConversion = false;
            } else {
                cout << "504 Bad conversion request." << endl;
                current_state.badConversion = true;
            }
        }
        threads.emplace_back([&request, &current_state, &response_code, &client_info]() { // new thread for each req
            mainCalculation(request, current_state, response_code, client_info);
        });
        for (auto& thread : threads) {// save resources
            if (thread.joinable()) {
                thread.join();
            }
        }
        threads.clear();

        // Send response to the client
        Response res(200, response_code.c_str(), 10.0);
        sendResponse(sockfd, &res, &client_info);

        if(request.command == "HELP") {
            cout << "The following commands are available with their parameters:\n";
            cout << "-HELO 'initalizes' the client and is entered like: HELO <name>\n";
            cout << "-HELP shows the commands and entered like: HELP\n";
            cout << "The following are different 'mode' commands and respective units\n";
            cout << "*AREA: SQRMT, SQRML, SQRIN, and SQRFT\t";
            cout << "*VOL: LTR, GALNU, GALNI, and CUBM\n";
            cout << "*WGT: KILO, PND, and CART\t\t";
            cout << "*TEMP: CELS, FAHR, and KELV\n";
            cout << "Enter over two inputs as such: \n-<mode>\n-<fromUnit> <toUnit> <value>\n";
            cout << "BYE closes the connection and entered as such: BYE\n\n";
        }
        logRequest(&request, &client_info);
        current_state.badConversion = false;
        request.clearCalc();
    }

    return 0;
}
