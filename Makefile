CC = g++
CFLAGS = -std=c++11 -Wall

# File names
CLIENT_SRC = client.cpp
SERVER_SRC = server.cpp
CLIENT_EXE = client
SERVER_EXE = server

# Targets
all: $(CLIENT_EXE) $(SERVER_EXE)

$(CLIENT_EXE): $(CLIENT_SRC)
	$(CC) $(CFLAGS) -o $(CLIENT_EXE) $(CLIENT_SRC)

$(SERVER_EXE): $(SERVER_SRC)
	$(CC) $(CFLAGS) -o $(SERVER_EXE) $(SERVER_SRC)

# Clean the build directory
clean:
	rm -f $(CLIENT_EXE) $(SERVER_EXE)
