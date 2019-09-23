#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

// includes
#include <windows.h>
#include <winsock2.h> // winsock
#include <ws2tcpip.h> // winsock functions
#include <stdio.h> // printf
#include <iostream> // std::cin, std::getline
#include <string> // std::string
#include <algorithm> // std::copy
#include <thread> // std::thread

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

#define LENGTH 512 // length of the arrays used, for I/O
#define PLAY_HEADER 4 // size of play header
#define MOVE_HEADER 6 // size of move header
#define NO_PARAM 2 // size of no param header

/*****************************************************************************/
/*!
  \brief
    This function will be the blocking for the cin so the main thread will be a
    non blocking program

  \param buf
    The buffer to store the user input
*/
/*****************************************************************************/

void reading_input(std::string& buf)
{
  //infinite loop to get cin
  while (true)
  {
    // string to getline
    std::string tmp;
    // clear the cin so no rubblish cin
    std::cin.clear();
    // getline user input
    std::getline(std::cin, tmp);
    // set buf to tmp
    buf = tmp;
  }
}


int main(void)
{
  // struct to hold info of winsock
  WSADATA data;
  // bool to store whether we should quit or reconncet
  bool quit = false;
  u_long iMode = 0;
  std::string sendbuf;
  

  // zero the memory of the data
  SecureZeroMemory(&data, sizeof(data));
  // init winsock
  int result = WSAStartup(MAKEWORD(2, 2), &data);
  // if init fails
  if (result != NO_ERROR)
  {
    wprintf(L"WSAStartup function failed with error: %d\n", result);
    return 1;
  }

  // create thread to do cin
  auto th = std::thread(&reading_input, std::ref(sendbuf));

  do
  {
    // string to store the user input of the port number
    std::string port_input;
    // string to  store the user input of the ip
    std::string ip_input;
    // string to store username
    std::string username;
    
    
    // create a addrinfo to allocate the socket to, hints to store the info of the result_addr
    addrinfo * result_addr = nullptr;
    // ptr to loop throught the result_addr to find the address
    addrinfo * ptr = nullptr;
    addrinfo   hints;
    // zero the hints memory
    SecureZeroMemory(&hints, sizeof(hints));

    hints.ai_family = AF_INET; // ipv4
    hints.ai_socktype = SOCK_STREAM; // using reliable byte stream TCP
    hints.ai_protocol = IPPROTO_TCP; // using TCP

    // loop infinite
    while (true)
    {
      // prompt for the user to input server ip address
      printf("Please input server ip address...\n");

      // wait user user to input ip
      while (sendbuf.empty())
      {
        // clear cin to prevent rubbish values
        std::cin.clear();
      }
      // set ip input
      ip_input = sendbuf;
      // clear sendbuf
      sendbuf.clear();

      // prompt the user to input the server port number
      printf("Please input server port number...\n");

      // wait for user to input port
      while (sendbuf.empty())
      {
        // clear cin to prevent rubbish values
        std::cin.clear();
      }
      // store port
      port_input = sendbuf;
      // clear send buf
      sendbuf.clear();

      // get the address info of the server host and port, using hints as a reference to set the type of
      // connection and set the result at result_addr
      result = getaddrinfo(ip_input.c_str(), port_input.c_str(), &hints, &result_addr);

      // if the getaddr has error
      if (result != 0)
      {
        // print error and restart
        printf("Getaddrinfo failed with error: %ld\n", result);
        continue;
      }
      // if no error break from loop
      break;
    }
    // set ptr to point to the beginning of the result_addr
    ptr = result_addr;

    // create a socket to connect to the server
    SOCKET ConnectSock;

    // loop through the result_addr to search for a address to connect to
    for (ptr = result_addr; ptr != nullptr; ptr = ptr->ai_next)
    {
      // create a connect socket
      ConnectSock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
      // if creation fails
      if (ConnectSock == INVALID_SOCKET)
      {
        printf("socket function failed with error: %ld\n", WSAGetLastError());
      }

      // connect the connect socket to the ptr address
      result = connect(ConnectSock, ptr->ai_addr, static_cast<int>(ptr->ai_addrlen));

      // if cannot connect, close the connect socket and continue to next loop
      if (result == SOCKET_ERROR)
      {
        closesocket(ConnectSock);
        ConnectSock = INVALID_SOCKET;
        continue;
      }
      // if connected, break from for loop
      break;
    }

    // free the result_addr as we no need to use it anymore
    freeaddrinfo(result_addr);

    // print status of client
    printf("Connected to server.\n\n");

    // recvbuf to store the whole message
    char recvbuf[LENGTH];

    // ask client for username
    printf("Please input username...\n");
    // check if thread has pick up username
    while (sendbuf.empty())
    {
      // if not busy wait and clear cin to avoid rubbish input
      std::cin.clear();
    }
    // set username as client input
    username = sendbuf;
    // clear the input buff
    sendbuf.clear();

    // newline
    printf("\n");

    // create user buffer to send to server
    char * user = nullptr;
    // allocate mem
    user = new char[PLAY_HEADER + username.size() + 1];
    // set command line to 0
    user[0] = 0;
    // 1 param
    user[1] = 1;
    // set the size of first param
    *((short *)&user[2]) = htons((short)username.size());
    // copy the username over to user buf
    std::copy(username.c_str(), username.c_str() + username.size(), &user[PLAY_HEADER]);
    // calculate user buf size
    int size = PLAY_HEADER + (int)username.size() + 1;
    // set null char
    user[PLAY_HEADER + username.size()] = '\0';
    // send to server
    send(ConnectSock, user, size, 0);
    // delete the user buf
    delete[] user;

    // create message buf
    char* message = nullptr;

    // iMode for non blocking flag
    iMode = 1;

    // change the socket to non blocking
    ioctlsocket(ConnectSock, FIONBIO, &iMode);

    // prompt user for input
    printf("Please input...\n");

    do
    {
      // result to store error code
      result = 0;
      // size of send buf
      int size = 0;
      // command string to store the command user is making
      std::string command;
      // 0 the memory of the buffer
      SecureZeroMemory(recvbuf, LENGTH);

      // store the command of user is typing
      command = sendbuf.substr(0, 4);
      // set tmp as sendbuf
      std::string tmp = sendbuf;
      // if there is input
      if (!sendbuf.empty())
      {
        // exit
        if (sendbuf == std::string("exit")) //universal
        {
          // encapusulate the message
          message = new char[NO_PARAM + 1];
          message[0] = 0;
          message[1] = 0;
          size = NO_PARAM + 1;
          message[NO_PARAM] = '\0';
          // send to server
          result = send(ConnectSock, message, size, 0);
          // delete memory
          delete[] message;
          // quit to true to exit loop
          quit = true;
          // clear sendbuf
          sendbuf.clear();
        }
        // quit
        else if (sendbuf == std::string("quit")) // in game
        {
          //encapusulate the message
          message = new char[NO_PARAM + 1];
          message[0] = 1;
          message[1] = 0;
          size = NO_PARAM + 1;
          message[NO_PARAM] = '\0';
          // send to server
          result = send(ConnectSock, message, size, 0);
          // delete memory
          delete[] message;
          // clear sendbuf
          sendbuf.clear();
        }
        // accept
        else if (sendbuf == std::string("accept"))
        {
          // encapsulate the message
          message = new char[NO_PARAM + 1];
          message[0] = 5;
          message[1] = 0;
          size = NO_PARAM + 1;
          message[NO_PARAM] = '\0';
          // send to server
          result = send(ConnectSock, message, size, 0);
          // delete memory
          delete[] message;
          // clear sendbuf
          sendbuf.clear();
        }
        else if (sendbuf == std::string("list user")) // universal
        {
          // encapsulate the message
          message = new char[NO_PARAM + 1];
          message[0] = 3;
          message[1] = 0;
          size = NO_PARAM + 1;
          message[NO_PARAM] = '\0';
          // send to server
          result = send(ConnectSock, message, size, 0);
          // delete the memory
          delete[] message;
          // clear sendbuf
          sendbuf.clear();
        }
        else if (command == std::string("play")) // available
        {
          // erase the "play"
          tmp.erase(0, 5);
          //encapsulate the message
          message = new char[PLAY_HEADER + tmp.size() + 1];
          message[0] = 2;
          message[1] = 1;
          *((short *)&message[2]) = htons((short)tmp.size());
          // copy the name to the parameter portion
          std::copy(tmp.c_str(), tmp.c_str() + tmp.size(), &message[PLAY_HEADER]);
          // set size
          size = PLAY_HEADER + (int)tmp.size() + 1;
          // set last char to be null char
          message[PLAY_HEADER + tmp.size()] = '\0';
          // send to server
          result = send(ConnectSock, message, size, 0);
          // delete message
          delete[] message;
          // clear sendbuf
          sendbuf.clear();
        }
        else if (command == std::string("move")) // in game
        {
          // erase the "move"
          sendbuf.erase(0, 5);
          // encapsulate the message
          message = new char[MOVE_HEADER + sendbuf.size() + 1];
          size = MOVE_HEADER + (int)sendbuf.size() + 1;
          message[0] = 4;
          message[1] = 2;
          // find the first move
          std::string tmp = sendbuf.substr(0, sendbuf.find_first_of(' '));
          
          // find the length of first move and copy to first param
          short len = (short)tmp.size();
          *((short *)&message[2]) = htons((short)tmp.size());
          std::copy(tmp.c_str(), tmp.c_str() + len, &message[4]);

          // erase the first move
          sendbuf.erase(0, sendbuf.find_first_of(' ') + 1);
          // find the second move and its size
          tmp = sendbuf;
          *((short *)&message[4 + len]) = htons((short)tmp.size());
          // copy it to the second param
          std::copy(tmp.c_str(), tmp.c_str() + tmp.size(), &message[6 + len]);
          // set null char
          message[size - 1] = '\0';
          // send to server
          result = send(ConnectSock, message, size, 0);
          // delete message
          delete[] message;
          // clear sendbuf
          sendbuf.clear();
        }
        // print board
        else if (sendbuf == std::string("print board"))
        {
          // encapsulate the message
          message = new char[NO_PARAM + 1];
          message[0] = 6;
          message[1] = 0;
          size = NO_PARAM + 1;
          message[NO_PARAM] = '\0';
          result = send(ConnectSock, message, size, 0);
          delete[] message;
          sendbuf.clear();
        }
        else // invalid commands
        {
          // encapsulate the message
          message = new char[NO_PARAM + 1];
          message[0] = -1;
          message[1] = 0;
          size = NO_PARAM + 1;
          message[NO_PARAM] = '\0';
          result = send(ConnectSock, message, size, 0);
          delete[] message;
          sendbuf.clear();
        }
      }
      else // if buffer is empty
      {
        // need means to check if server is alive
        int error;
        // send empty packet
        error = send(ConnectSock, "", 0, 0);
        // if server is down disconnect and break from loop
        if (error == SOCKET_ERROR)
        {
          printf("Server Disconnected\n");
          break;
        }
      }
      // clear tmp and command
      tmp.clear();
      command.clear();
      // if sending got error
      if (result == SOCKET_ERROR)
      {
        // means server is disconnected
        printf("Server disconnected...\n");
        break;
      }

      // recieve the result from server, echo from server
      result = recv(ConnectSock, recvbuf, LENGTH, 0);

      // if the bytes received is more than 0
      if (result > 0)
      {
        // if is print command, only print command from server side
        if (recvbuf[0] == 0)
        {
          // clear cin
          std::cin.clear();
          // create a printbuf
          char* printbuf;
          // get the size of param
          short print_length = ntohs(*((short *)&recvbuf[2]));
          // allocate memory
          printbuf = new char[print_length + 1];
          // copy the first param
          std::copy(recvbuf + 4, recvbuf + 4 + print_length, printbuf);
          // add nullchar
          printbuf[print_length] = '\0';
          // print the param
          printf("%s", printbuf);
          // deallocate the param
          delete[] printbuf;
        }
        // prompt the user to input command
        printf("\nPlease input...\n");
      }
      
    } while (!quit); // loop send and recieve server has not dc and the client is not qutting

    // close the connect socket
    result = closesocket(ConnectSock);

    // if there is an error in closing the connect socket
    if (result == SOCKET_ERROR) 
    {
      printf("closesocket function failed with error: %ld\n", WSAGetLastError());
    }
  }
  while (!quit); // loop when the server dc, will reconnect to another server

  // detach the cin thread and destroy it
  th.detach();
  th.~thread();

  // clean up WSAdata
  WSACleanup();

  // return 0
  return 0;
}
