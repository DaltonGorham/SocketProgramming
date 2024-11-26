#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdexcept>
#include <thread>
#define PORT 12000

// server address
struct sockaddr_in serverAddress;


void handleClient(int clientSocket) {
    char buffer[1024];
    int bytesReceived;
    
    std::cout << "waiting for client data..." << std::endl;
    bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

    if (bytesReceived < 0){
        std::cout << "client disconnected or error receiving..." << std::endl;
    }
    else {
        buffer[bytesReceived] = '\0';
    }

    std::cout << "Message from client: " << buffer << std::endl;

    const char* response = "Message received!";
    send(clientSocket, response, strlen(response), 0);

    memset(buffer, 0, sizeof(buffer));

    // Close the connection after communication
    close(clientSocket);
}

int main() {
  
    int nRet = 0;
    // Create socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);   // AF_INET: IPv4, SOCK_STREAM: TCP, IPPROTO_TCP: TCP

    // Check if the socket was created successfully
    if (serverSocket < 0) {
        std::cerr << "The socket was not created" << std::endl;
    }
    else {
        std::cout << "The socket was created successfully" << std::endl;
    }

    // initialize server address
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = INADDR_ANY; // IP address of the server : the local machine

    // Bind the socket to the server address
    nRet = bind(serverSocket, (sockaddr*) &serverAddress, sizeof(serverAddress));
    
    if (nRet < 0) {
        std::cerr << "The socket was not binded to the server address" << std::endl;
        close(serverSocket);
        return -1;
    }
    else {
        std::cout << "The socket was binded to the server address" << std::endl;
    }

    // Listen for incoming connections (max 2 connections for 2 players)
    nRet = listen(serverSocket, 2);

    if (nRet < 0) {
        std::cout << "The socket was not listening for incoming connections" << std::endl;
        close(serverSocket);
        return -1;
        
    }
    else {
        std::cout << "The socket is listening for incoming connections" << std::endl;
    }

    // Accept incoming connections
    
    while (true) {
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket < 0) {
            std::cerr << "Failed to accept client connection" << std::endl;
            continue; // Wait for the next connection
        }
        std::cout << "Client connected" << std::endl;

        // Handle the client in a new thread
        std::thread clientThread(handleClient, clientSocket);
        clientThread.detach();  // Detach the thread so it can run independently
    }

    // Close the server socket (although this won't be reached in this loop)
    close(serverSocket);
    return 0;   
}