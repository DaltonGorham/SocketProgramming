#include <iostream>
#include <winsock.h>
#include <cstring>
#include <string>

#define PORT 12000
struct sockaddr_in serverAddress;

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
  serverAddress.sin_port = htons(PORT);
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


  


  // sending data

  

  const char* message = "i hate you";

 nRet = send(clientSocket, message, strlen(message), 0);

 if (nRet < 0){
    std::cerr << "Failed to send message with error: " << WSAGetLastError() << std::endl;
    closesocket(clientSocket);
    WSACleanup();
    return -1;
 } 
 else {
  std::cout << "message sent successfully" << std::endl;
 }


 /* char buffer[1024];

  int bytesRecieved = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

  if(bytesRecieved < 0){
    std::cerr<< "Failed to received data. Error code: " << WSAGetLastError() << std::endl;
  }
  else if (bytesRecieved == 0){
    std::cout << "the server has closed it's connection." << std::endl;
  }
  else {
    buffer[bytesRecieved] = '\0';
  }

 
  std::cout << "server says: " << buffer << std::endl;

*/



 


  closesocket(clientSocket);
  WSACleanup();





  return 0;
}