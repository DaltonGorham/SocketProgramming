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

struct Score{
    int player1Score;
    int player2Score;  
};


// Struct to hold the results of the game
struct PlayerResults{
    std::string player1Results;
    std::string player2Results;
};

Score globalScore = {0, 0};

// Set socket variables to negative to indicate that no player is connected
int player1Socket = -1;
int player2Socket = -1;

// Mutex to ensure that only one player can make a choice at a time
std::mutex choiceMutex;

// Player choices
std::string player1Choice;
std::string player2Choice;


// Function to determine the winner of the game
PlayerResults determineWiner(std::string player1, std::string player2);

// Function to assign player numbers
bool assignPlayerNumbers(int clientSocket);

// Function to handle the game
void handleGame(int clientSocket);

int main() {
    // Return value for socket functions to determine success or failure
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

    // Initialize server address
    serverAddress.sin_family = AF_INET;          // IPv4
    serverAddress.sin_port = htons(12000);       // Port number
    serverAddress.sin_addr.s_addr = INADDR_ANY;  // IP address of the server : the local machine

    // Bind the socket to the server address if successful return 0, otherwise return -1
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

    // Infinite loop to accept incoming connections and handle each client in a new thread
    
    while (true) {
        int clientSocket = accept(serverSocket, nullptr, nullptr); // Accept incoming connection
        if (clientSocket < 0) { // If the connection fails, print an error message and wait for the next connection
            std::cerr << "Failed to accept client connection" << std::endl;
            continue; // Wait for the next connection
        }
        std::cout << "Client connected" << std::endl;
        
        

        // Handle the client in a new thread so that the server can handle multiple clients simultaneously
        std::thread clientThread(handleGame, clientSocket);
        clientThread.detach();  // Detach the thread so it can run independently without blocking the main thread
    }

    // Close the server socket
    close(serverSocket);
    return 0;   
}


/*

Function to assign player numbers
The first client to connect is assigned player 1, the second client to connect is assigned player 2 
After assigning the player numbers, the funtion sends a welcome message to the client and returns true if the client is player 1, otherwise it returns false
*/
bool assignPlayerNumbers(int clientSocket) {
    const char* player1Message = 
        "Welcome Player 1\n";
    
    const char* player2Message = 
        "Welcome Player 2\n";

    // Assign player 1 if no player 1 is connected
    if (player1Socket == -1) {
        player1Socket = clientSocket;
        send(clientSocket, player1Message, strlen(player1Message), 0);
        return true;
    }
    // Assign player 2 if player 1 is already connected
    player2Socket = clientSocket;
    send(clientSocket, player2Message, strlen(player2Message), 0);
    return false;
}

void startCountdown(int clientSocket){
    const char* SHOOT = "SHOOT!\n";
    const char* THREE = "rock\n";
    const char* TWO = "paper\n";
    const char* ONE = "scissors\n";

    send(clientSocket, THREE, strlen(THREE), 0);
    std::this_thread::sleep_for(std::chrono::seconds(1));   
    send(clientSocket, TWO, strlen(TWO), 0);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    send(clientSocket, ONE, strlen(ONE), 0);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    send(clientSocket, SHOOT, strlen(SHOOT), 0);
}

/*
Function to send the choice message to the client
*/
void getPlayerChoice(int clientSocket){
    const char* choiceMsg = 
     "Enter your choice:\n"
        "- rock\n"
        "- paper\n"
        "- scissors\n\n";
        send(clientSocket, choiceMsg, strlen(choiceMsg), 0);
}


/*
Function to determine the winner of the game
The function takes the choices of the two players and returns a struct containing the results for each player 
*/
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

/*
Function to keep track of the score
*/

Score keepScore(std::string player1Result, std::string player2Result){
    
    if (player1Result == "You win!\n"){
        globalScore.player1Score++;
    }
    else if (player2Result == "You win!\n"){
        globalScore.player2Score++;
    }    
  
    return globalScore;

}

void displayScore(Score score, int clientSocket){
    std::string scoreMsg = "Score:\nPlayer 1: " + std::to_string(score.player1Score) + "\nPlayer 2: " + std::to_string(score.player2Score) + "\n";
    send(clientSocket, scoreMsg.c_str(), scoreMsg.length(), 0);
}

/*
Function to handle the game
The function takes the client socket as a parameter and handles the game logic
*/
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

    // Get the player's choice
    getPlayerChoice(clientSocket);
    


    // infinite loop to handle the game
    while (true) {
        memset(buffer, 0, sizeof(buffer)); // Clear the buffer
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0); // Receive data from the client
         
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0'; // Null-terminate the received data
            choice = buffer; // Convert the received data to a string

            // remove any trailing newline characters 
            choice.erase(std::remove(choice.begin(), choice.end(), '\n'), choice.end());
            choice.erase(std::remove(choice.begin(), choice.end(), '\0'), choice.end());
            
            // Check if the choice is valid
            if (choice != "rock" && choice != "paper" && choice != "scissors") {
                const char* invalidChoice = "Invalid choice, you must pick rock, paper, or scissors\n"; 
                send(clientSocket, invalidChoice, strlen(invalidChoice), 0);
                continue;   // restart the loop to get a valid choice
            }


            // lock the choice mutex to ensure both players make their choices before calculating the results
            {
                std::lock_guard<std::mutex> lock(choiceMutex);
                if (isPlayer1) {
                    player1Choice = choice;
                    const char* wait = "Move Picked. Waiting for results....\n";
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
                    startCountdown(player1Socket);
                    startCountdown(player2Socket);
         
                    results = determineWiner(player1Choice, player2Choice);
                    globalScore = keepScore(results.player1Results, results.player2Results);
                    
                    // send results to both players and server
                    std::cout << "Player 1: " << results.player1Results;
                    std::cout << "Player 2: " << results.player2Results;
                    send(player1Socket, results.player1Results.c_str(), results.player1Results.length(), 0);
                    send(player2Socket, results.player2Results.c_str(), results.player2Results.length(), 0);
                    displayScore(globalScore, player1Socket);
                    displayScore(globalScore, player2Socket);

                    // reset for next round
                    player1Choice.clear(); 
                    player2Choice.clear();
                    
                    // Prompt for next round
                    const char* nextRound = "Enter rock paper or scissors to play again\n";
                    send(player1Socket, nextRound, strlen(nextRound), 0);
                    send(player2Socket, nextRound, strlen(nextRound), 0);

                    
                    
                }
            }
            
            
        }

        // If the client disconnects, reset the player socket variables and break the loop
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
        // If there is an error reading data, reset the player socket variables and break the loop
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