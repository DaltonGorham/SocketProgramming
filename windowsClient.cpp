#include <iostream>
#include <winsock.h>
#include <cstring>
#include <string>
#include <thread>


struct sockaddr_in serverAddress;
void receiveMessagesFromServer(int clientSocket);



int main(){


  /*
  FOR WINDOWS
  WINSOCK INITIALIZATION
  */

  WSAData wsaData;

  int wsaResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

  if (wsaResult != 0){
    std::cerr << "WASStartup failed with an error" << wsaResult << std::endl;
    return -1;
  }



  int nRet = 0;

  // create socket
  int clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  // check if the socket was created succesfully
  if (clientSocket == INVALID_SOCKET) {
    std::cerr << "Socket failed to be created" << WSAGetLastError() << std::endl;
    WSACleanup();
    return -1;
  }


  // initialize server address
  memset(&serverAddress, 0, sizeof(serverAddress));
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(12000);
  serverAddress.sin_addr.s_addr = inet_addr("192.168.1.9");


  nRet = connect(clientSocket, (struct sockaddr*)& serverAddress, sizeof(serverAddress));


  // sending connection request
  if (nRet < 0 ){
    std::cerr << "Connection to the server failed with error: " << WSAGetLastError() << std::endl;
    closesocket(clientSocket);
    WSACleanup();
    return -1;
  }
  else {
    std::cout << "Succefully connected to server." << std::endl;
  }

  // start a thread to receive messages from the server
  std::thread receiveThread(receiveMessagesFromServer, clientSocket);
  receiveThread.detach();


  // choose your move (rock paper or scissors)
  std::string playerMove;

  while (true) {

  
  std::getline(std::cin, playerMove);

  // send move to the server
  nRet = (send(clientSocket, playerMove.c_str(), playerMove.length(), 0));


  if (nRet == SOCKET_ERROR){
      std::cerr << "Failed to send message with error: " << WSAGetLastError() << std::endl;
      break;
      
    } 
  }
  

  closesocket(clientSocket);
  WSACleanup();





  return 0;
}






void receiveMessagesFromServer(int clientSocket){
  char buffer[1024];
  while(true){
    memset(buffer, 0, sizeof(buffer));
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

    if (bytesReceived > 0){
      buffer[bytesReceived] = '\0';
      std::cout << "Server: " << buffer << std::endl;
    }
    else if (bytesReceived == 0){
      std::cout << "Server disconnected" << std::endl;
      break;
    }
    else {
      std::cout << "Error reading data from server" << std::endl;
      break;
    }
  }
}