#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdexcept>
#include <thread>
#include <chrono>
#include <algorithm>
#include <mutex>


// server address
struct sockaddr_in serverAddress;
struct PlayerResults{
    std::string player1Results;
    std::string player2Results;
};
int player1Socket = -1;
int player2Socket = -1;
std::mutex choiceMutex;
std::string player1Choice;
std::string player2Choice;


PlayerResults determineWiner(std::string player1, std::string player2);
bool assignPlayerNumbers(int clientSocket);

void handleGame(int clientSocket);

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
        std::thread clientThread(handleGame, clientSocket);
        clientThread.detach();  // Detach the thread so it can run independently
    }

    
    close(serverSocket);
    return 0;   
}


bool assignPlayerNumbers(int clientSocket) {
    const char* player1Message = 
        "Welcome Player 1\n"
        "Enter your choice:\n"
        "- rock\n"
        "- paper\n"
        "- scissors\n\n";
    
    const char* player2Message = 
        "Welcome Player 2\n"
        "Enter your choice:\n"
        "- rock\n"
        "- paper\n"
        "- scissors\n\n";
    
    if (player1Socket == -1) {
        player1Socket = clientSocket;
        send(clientSocket, player1Message, strlen(player1Message), 0);
        return true;
    }
    
    player2Socket = clientSocket;
    send(clientSocket, player2Message, strlen(player2Message), 0);
    return false;
}


PlayerResults determineWiner(std::string player1, std::string player2){
    std::string resultPlayer1, resultPlayer2;
  if (player1 == player2){
    resultPlayer1 = resultPlayer2 = "Tie.\n";
  }
  else if (player1 == "rock" && player2 == "scissors" || 
           player1 == "paper" && player2 == "rock" || 
           player1 == "scissors" && player2 == "paper"){
            resultPlayer1 = "You win!\n"; 
            resultPlayer2 = "You lose!\n";
           }
    else {
        resultPlayer1 = "You lose!\n";
        resultPlayer2 = "You win!\n";
    }
    return {resultPlayer1, resultPlayer2};
}

void handleGame(int clientSocket) {
    char buffer[1024];

 
    std::string choice;
    PlayerResults results;
    // Send welcome message to clients
    const char* welcome = "Welcome to rock paper or scissors!\n";
    send(clientSocket, welcome, strlen(welcome), 0);

    bool isPlayer1 = assignPlayerNumbers(clientSocket);
    
    // Make sure both player are in game before starting
     if (player1Socket == -1 || player2Socket == -1){
        const char* waitMsg = "Waiting for opponent to connect...\n";
        send(clientSocket, waitMsg, strlen(waitMsg), 0);
    }

      // loop to give time for clients to connect
    while (player1Socket == -1 || player2Socket == -1){
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    const char* startMsg = "Opponent connected. Game starting\n";
    send(clientSocket, startMsg, strlen(startMsg), 0);

    
   


    

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
         
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            choice = buffer;

            // remove any trailing newline characters
            choice.erase(std::remove(choice.begin(), choice.end(), '\n'), choice.end());
            choice.erase(std::remove(choice.begin(), choice.end(), '\0'), choice.end());
            
            if (choice != "rock" && choice != "paper" && choice != "scissors") {
                const char* invalidChoice = "Invalid choice, you must pick rock, paper, or scissors\n"; 
                send(clientSocket, invalidChoice, strlen(invalidChoice), 0);
                continue; 
            }


            // lock the choice mutex to ensure that only one player can make a choice at a time
            {
                std::lock_guard<std::mutex> lock(choiceMutex);
                if (isPlayer1) {
                    player1Choice = choice;
                    const char* wait = "Move Picked. Waiting for player 2....\n";
                    std::cout << "Player 1: " << player1Choice << std::endl;
                    send(clientSocket, wait, strlen(wait), 0);
                }
                else {
                    player2Choice = choice;
                    const char* wait = "Move Picked. Waiting for results...\n";
                    std::cout << "Player 2: " << player2Choice << std::endl;
                    send(clientSocket, wait, strlen(wait), 0);
                }

                // Only calculate and send results when both players have made their choices
                if (!player1Choice.empty() && !player2Choice.empty()) {
                    // Small delay to ensure both players see their "Move Picked" messages
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                    
                    const char* calcMsg = "Calculating results...\n";
                    send(player1Socket, calcMsg, strlen(calcMsg), 0);
                    send(player2Socket, calcMsg, strlen(calcMsg), 0);
                    
                    results = determineWiner(player1Choice, player2Choice);
                    
                    // send results to both players and server
                    std::cout << "Player 1: " << results.player1Results;
                    std::cout << "Player 2: " << results.player2Results;
                    send(player1Socket, results.player1Results.c_str(), results.player1Results.length(), 0);
                    send(player2Socket, results.player2Results.c_str(), results.player2Results.length(), 0);

                    // reset for next round
                    player1Choice.clear(); 
                    player2Choice.clear();
                    
                    // Prompt for next round
                    const char* nextRound = "Enter rock paper or scissors to play again\n";
                    send(player1Socket, nextRound, strlen(nextRound), 0);
                    send(player2Socket, nextRound, strlen(nextRound), 0);

                    
                    
                }
            }
            
            continue;
        }

        if (bytesReceived == 0){
            std::cout << "Client disconnected." << std::endl;
            if (isPlayer1) {
                player1Socket = -1;
            }
            else {
                player2Socket = -1;
            }
            break;
        }
        else if (bytesReceived < 0){
            std::cout << "Error reading data" << std::endl;
            if (isPlayer1) {
                player1Socket = -1;
            }
            else {
                player2Socket = -1;
            }
            break;
        }
    }
}