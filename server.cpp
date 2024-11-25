#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdexcept>
#define PORT 12000

// server address
struct sockaddr_in serverAddress;

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

    int clientSocket1 = accept(serverSocket, nullptr, nullptr);
    int clientSocket2 = accept(serverSocket, nullptr, nullptr);

    if (clientSocket1 < 0 || clientSocket2 < 0) {
        std::cout << "The socket was not accepted" << std::endl;
        close(serverSocket);
        return -1;
    }
    else {
        std::cout << "The socket was accepted" << std::endl;
    }

    // Receive the message from client 1

    char buffer[1024];
    int bytesReceived = recv(clientSocket1, buffer, sizeof(buffer), 0);

    if (bytesReceived < 0) {
        std::cout << "The message was not received." << std::endl;
        close(clientSocket1);
        close(clientSocket2);
        close(serverSocket);
        return -1;
    }
    else {
        std::cout << "The message was received from client 1" << std::endl;
    }

    // Send the message to client 2

    nRet = send(clientSocket2, buffer, bytesReceived, 0);

    if (nRet < 0) {
        std::cout << "The message was not sent to client 2" << std::endl;
        close(clientSocket1);
        close(clientSocket2);
        close(serverSocket);
        return -1;
    }
    else {
        std::cout << "The message was sent to client 2" << std::endl;
    }


    // Receive the message from client 2

    bytesReceived = recv(clientSocket2, buffer, sizeof(buffer), 0);

    if (bytesReceived < 0) {
        std::cout << "The message was not received." << std::endl;
        close(clientSocket1);
        close(clientSocket2);
        close(serverSocket);
        return -1;
    }
    else {
        std::cout << "The message was received from client 2" << std::endl;
    }

    // Send the message to client 1

    nRet = send(clientSocket1, buffer, bytesReceived, 0);

    if (nRet < 0) {
        std::cout << "The message was not sent to client 1" << std::endl;
        close(clientSocket1);
        close(clientSocket2);
        close(serverSocket);
        return -1;
    }
    else {
        std::cout << "The message was sent to client 1" << std::endl;
    }

    


    // close the sockets

    close(clientSocket1);
    close(clientSocket2);
    close(serverSocket);
    

    return 0;



}