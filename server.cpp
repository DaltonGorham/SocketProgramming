#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdexcept>
#include <thread>

// server address
struct sockaddr_in serverAddress;


void handleClient(int clientSocket) {
    char buffer[1024];
    
    // Send welcome message to client
    const char* welcome = "Welcome to the server!\n";
    send(clientSocket, welcome, strlen(welcome), 0);
    
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
         
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            std::cout << "Client message: " << buffer << std::endl;
            
            // Echo the message back to client (or send any other response)
            std::string response = "Server received: ";
            response += buffer;
            send(clientSocket, response.c_str(), response.length(), 0);
        }
        else if (bytesReceived == 0) {
            std::cout << "Client disconnected." << std::endl;
            break;
        }
        else {
            std::cerr << "Error reading data: " << strerror(errno) << std::endl;
            break;
        }
    }
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
    serverAddress.sin_port = htons(12000);
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