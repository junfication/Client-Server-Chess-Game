#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

// includes
#include <windows.h>
#include <winsock2.h> // winsock
#include <ws2tcpip.h> // winsock functions
#include <stdio.h> // printf
#include <iostream> // std::cin, std::getline, std::cout
#include <string> // std::string
#include <algorithm> // std::copy
#include <thread> // std::thread
#include <mutex> // std::mutex
#include <condition_variable> // std::unique_lock
#include <queue> // std::queue
#include <future> // std::future
#include <algorithm> // std::copy


#define LENGTH 512 // length of the arrays used, for I/O
#define HEADER 5 // size of the header, for the message
#define NUM_OF_WORKER_THREADS 10 // total number of threads

// Need to link with Ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

enum Room_Status
{
  // enum for room status
  NOT_USED = 0,
  USED = 1
};

enum Living
{
  // enum for chess pieces whether piece is dead or alive
  ALIVE = 0,
  DEAD = 1
};

enum Color
{
  // color of the pieces
  NIL = 0,
  WHITE = 1,
  BLACK = 2
};

enum Rank
{
  // enum for type of chess piece
  PAWN = 0,
  KNIGHT = 1,
  ROOK = 2,
  BISHOP = 3,
  QUEEN = 4,
  KING = 5
};

/*****************************************************************************/
/*!
  \class Vec2
  
  \brief
    This struct contains the x and y on the chess board
*/
/*****************************************************************************/

struct Vec2
{
  // x value of chess piece
  int x;
  // y value of chess piece
  int y;
  // default constructor
  Vec2(){}
  // non default constructor
  Vec2(int a, int b) : x(a), y(b) {}
  // operator overload to check if 2 vec is the same
  bool operator==(const Vec2& rhs)
  {
    return(rhs.x == x && rhs.y == y);
  }
};

/*****************************************************************************/
/*!
  \class Pieces
  
  \brief
    This struct contains the details of the pieces on the chess board, color,
    pos and etc
*/
/*****************************************************************************/

struct Pieces
{
  // the color of the chess piece
  Color base;
  // pos of the chess piece
  Vec2  pos;
  // rank of chess piece e.g rook, bishop, etc
  Rank  type;
  // whether the chess piece is living or death
  Living status;
  // default constructor
  Pieces() {}
  // non default constructor
  Pieces(Color clr, Vec2 m_pos, Rank m_type, Living m_status) : base(clr), pos(m_pos), type(m_type), status(m_status) {}
};

/*****************************************************************************/
/*!
  \class Message
  
  \brief
    This struct contains the message encapsulation to make it into the
    assignment given format
*/
/*****************************************************************************/

struct Message
{
  // message buf
  char* buf;
  // message size
  size_t sz;

  // non default constructor
  Message(std::string rhs) // allocate mem and set size
    : buf(new char[4 + rhs.size()]), sz(rhs.size() + 4)
  {
    buf[0] = 0; // print out message command
    buf[1] = 1; // 1 param
    // set size
    *((short*)(&buf[2])) = htons((short)rhs.size());
    // copy message over first param
    std::copy(rhs.c_str(), rhs.c_str() + rhs.size(), buf+4);
  }

  // get c string function
  char* c_str()
  {
    // return buf
    return buf;
  }

  // get size function
  size_t size()
  {
    // return size
    return sz;
  }

  // destructor
  ~Message() 
  {
    // delete the buf
    delete[] buf;
  }
};

/*****************************************************************************/
/*!
  \class Room
  
  \brief
    This struct contains the details of where 2 users will be playing the
    chess game, simulates a room 
*/
/*****************************************************************************/

struct Room
{
  // status of room
  Room_Status status;
  // white game id
  int white_id;
  // black game id
  int black_id;
  // whose turn is it
  Color turn;
  // black pieces
  Pieces black_chess[16];
  // white pieces
  Pieces white_chess[16];
  // reset the board
  void reset_board();
  // making move function
  std::string make_move(int id, Vec2 start, Vec2 End);
  // printing the board out
  std::string print_board();
  // finding the pieces with vec2
  char find_pieces(Vec2 pos);
  // getting the black piece with vec2
  Pieces* find_black_pieces(Vec2 pos);
  // getting white piece with vec 2
  Pieces* find_white_pieces(Vec2 pos);
  // constructor
  Room() { reset_board(); }
};

/*****************************************************************************/
/*!
  \brief
    This will reset the chess board to the original state 
*/
/*****************************************************************************/

void Room::reset_board()
{
  // set ids to negative
  white_id = -1;
  black_id = -1;
  // set turn to NIL
  turn = NIL;
  // set status to not used
  status = NOT_USED;

  // set black pieces
  black_chess[0] = Pieces(BLACK, Vec2(0, 0), ROOK, ALIVE);
  black_chess[1] = Pieces(BLACK, Vec2(0, 1), KNIGHT, ALIVE);
  black_chess[2] = Pieces(BLACK, Vec2(0, 2), BISHOP, ALIVE);
  black_chess[3] = Pieces(BLACK, Vec2(0, 3), QUEEN, ALIVE);
  black_chess[4] = Pieces(BLACK, Vec2(0, 4), KING, ALIVE);
  black_chess[5] = Pieces(BLACK, Vec2(0, 5), BISHOP, ALIVE);
  black_chess[6] = Pieces(BLACK, Vec2(0, 6), KNIGHT, ALIVE);
  black_chess[7] = Pieces(BLACK, Vec2(0, 7), ROOK, ALIVE);
  for(int i = 8; i < 16; ++i)
    black_chess[i] = Pieces(BLACK, Vec2(1, i - 8), PAWN, ALIVE);

  // set white pieces
  white_chess[0] = Pieces(WHITE, Vec2(7, 0), ROOK, ALIVE);
  white_chess[1] = Pieces(WHITE, Vec2(7, 1), KNIGHT, ALIVE);
  white_chess[2] = Pieces(WHITE, Vec2(7, 2), BISHOP, ALIVE);
  white_chess[3] = Pieces(WHITE, Vec2(7, 3), QUEEN, ALIVE);
  white_chess[4] = Pieces(WHITE, Vec2(7, 4), KING, ALIVE);
  white_chess[5] = Pieces(WHITE, Vec2(7, 5), BISHOP, ALIVE);
  white_chess[6] = Pieces(WHITE, Vec2(7, 6), KNIGHT, ALIVE);
  white_chess[7] = Pieces(WHITE, Vec2(7, 7), ROOK, ALIVE);
  for (int i = 8; i < 16; ++i)
    white_chess[i] = Pieces(WHITE, Vec2(6, i - 8), PAWN, ALIVE);
}

/*****************************************************************************/
/*!
  \brief
    This function will find the chess piece on the start pos and move it to
    the end pos
  
  \param id
    The current thread game id
    
  \param start
    The starting pos of the chess piece

  \param End
    The final pos u want to move the chess piece

  \return std::string
    The error code or the make move code
*/
/*****************************************************************************/

std::string Room::make_move(int id, Vec2 start, Vec2 End)
{
  // create the current player calling this function's color
  Color player_color = NIL;
  // check if white or black
  if (id == white_id) player_color = WHITE;
  else player_color = BLACK;
  // if currently is your turn
  if (player_color == turn)
  {
    // if you are black
    if (player_color == BLACK)
    {
      // find black pieces with start
      Pieces* my_piece = find_black_pieces(start);
      // if no pieces are found
      if (my_piece == nullptr)
      {
        // return error msg
        std::string error("Invalid Move\n\0");
        return error;
      }
      else // found piece
      {
        // find if any opponent piece is on the final pos
        Pieces* opponent_piece = find_white_pieces(End);
        // set it to dead if there is
        if (opponent_piece != nullptr) opponent_piece->status = DEAD;
        // set black piece finally pos to end
        my_piece->pos = End;
        // set turn to white
        turn = WHITE;
        // return turn white
        std::string error("Turn: WHITE\n\0");
        return error;
      }
    }
    else
    {
      // if u are white, find if there are any white pieces on start pos
      Pieces* my_piece = find_white_pieces(start);
      // no? return error message
      if (my_piece == nullptr)
      {
        std::string error("Invalid Move\n\0");
        return error;
      }
      else // there is white piece on start pos
      {
        // find if there are any opponent piece at end
        Pieces* opponent_piece = find_black_pieces(End);
        // if so set it to dead
        if (opponent_piece != nullptr) opponent_piece->status = DEAD;
        // set your piece pos to end
        my_piece->pos = End;
        // set turn to black
        turn = BLACK;
        // return turn black
        std::string error("Turn: BLACK\n\0");
        return error;
      }
    }
  }
  else// not your turn return error message
  {
    std::string error("Not your turn\n\0");
    return error;
  }
}

/*****************************************************************************/
/*!
  \brief
    This is a function which will return the chess board in the current
    room
  
  \return std::string
    The board of the chess board
*/
/*****************************************************************************/

std::string Room::print_board()
{
  // tmp as output string
  std::string tmp;
  // the y axis
  char alpha = 'A';
  // loop through the board
  for (int i = 0; i < 8; ++i) // x-axis
  {
    // header of the board
    tmp += " ----------------\n";
    // the y axis
    tmp += alpha;
    // separator
    tmp += '|';
    for (int j = 0; j < 8; ++j) // y-axis
    {
      // find if the pos contains any pieces 
      tmp += find_pieces(Vec2(i, j));
      // separator
      tmp += '|';
    }
    // newline
    tmp += '\n';
    // increment y axis
    ++alpha;
  }
  // footer
  tmp += " ----------------\n";
  // reference for y axis
  tmp += "  0 1 2 3 4 5 6 7 \n";
  // return the output string
  return tmp;
}

/*****************************************************************************/
/*!
  \brief
    This function will find the piece and return the correct char
    space if there is nothing
  
  \param pos
    The pos of the piece
    
  \return char
    The char code for the piece
*/
/*****************************************************************************/

char Room::find_pieces(Vec2 pos)
{
  // loop through all the chess pieces
  for (int i = 0; i < 16; ++i)
  {
    // check the black pieces and status
    if (black_chess[i].pos == pos && black_chess[i].status == ALIVE)
    {
      // check the type and return the char code
      if (black_chess[i].type == ROOK) return 'R';
      if (black_chess[i].type == KNIGHT) return 'N';
      if (black_chess[i].type == BISHOP) return 'B';
      if (black_chess[i].type == QUEEN) return 'Q';
      if (black_chess[i].type == KING) return 'K';
      if (black_chess[i].type == PAWN) return 'P';
    }
    // check the white pieces and status
    else if (white_chess[i].pos == pos && white_chess[i].status == ALIVE)
    {
      // check the type and return the char code
      if (white_chess[i].type == ROOK) return 'r';
      if (white_chess[i].type == KNIGHT) return 'n';
      if (white_chess[i].type == BISHOP) return 'b';
      if (white_chess[i].type == QUEEN) return 'q';
      if (white_chess[i].type == KING) return 'k';
      if (white_chess[i].type == PAWN) return 'p';
    }
  }
  return ' '; // if no pieces on the pos
}

/*****************************************************************************/
/*!
  \brief
    This function will find the black pieces if there is any at the pos
  
  \param pos
    The pos of the piece
    
  \return Pieces *
    The pointer to the piece, nullptr if empty
*/
/*****************************************************************************/

Pieces * Room::find_black_pieces(Vec2 pos)
{
  // loop through all the black pieces
  for (int i = 0; i < 16; ++i)
    // check if the piece is alive and the pos matches
    if (black_chess[i].pos == pos && black_chess[i].status == ALIVE)
      // return the pointer to the piece
      return &black_chess[i];
  // return nullptr if there is no pieces in the pos and is alive
  return nullptr;
}

/*****************************************************************************/
/*!
  \brief
    This function will find the white pieces if there is any at the pos
  
  \param pos
    The pos of the piece
    
  \return Pieces *
    The pointer to the piece, nullptr if empty
*/
/*****************************************************************************/

Pieces * Room::find_white_pieces(Vec2 pos)
{
  // loop through all the white pieces
  for (int i = 0; i < 16; ++i)
    // check if the pieces is alove and the pos matches
    if (white_chess[i].pos == pos && white_chess[i].status == ALIVE)
      // return the pointer to the piece
      return &white_chess[i];
  // return nullptr if thre is no pieces in the pos and is alive
  return nullptr;
}

// status of the thread
enum status
{
  NOT_CONNECTED = 0,
  AVAILABLE = 1,
  IN_GAME = 2,
  WAITING = 3,
  REQUESTING = 4
};


// the command id of the message 
enum Command
{
  EXIT = 0,
  QUIT = 1,
  PLAY = 2,
  LIST = 3,
  MOVE = 4,
  ACCEPT = 5,
  PRINT = 6
};

/*****************************************************************************/
/*!
  \class ThreadQueue
  
  \brief
    This class is like the master queue where all the threads can 
    communicate with others through here or get details of other through here
*/
/*****************************************************************************/

class ThreadQueue
{
  // socket buffer mutex
  std::mutex buf_mutex;
  //  socket item mutex
  std::mutex items_mutex;
  // condition variable for when buf is empty
  std::condition_variable buf_empty;
  // the number of thread
  int slots;
  // slot mutex
  std::mutex slots_mutex;
  // condition variable for when the buf is full
  std::condition_variable buf_full;
  // num of socket
  int items;
  // the buffer of socket
  std::queue<SOCKET> buffer;
  // the mutex for printing id
  std::mutex id_mutex;
  // conversion function to convert status to char*
  char* status_to_string(status stat);
public:
  // non default constructor
  ThreadQueue(int num) : slots(num), items{0} {};
  // produce to add socket into buffer
  void produce(SOCKET socket);
  // consume to create the socket for the thread
  SOCKET consume();
  // print the id of the threads
  void print_id(std::thread::id ID);
  // array to store each thread's status
  status Thread_status[NUM_OF_WORKER_THREADS];
  // array to store each thread's username
  std::string Thread_username[NUM_OF_WORKER_THREADS];
  // function to export the list of status of the threads
  std::string export_list();
  // function to find the user in the telnet server
  int find_user(std::string username);
  // array of std::futures for the 3 way handshake to go in a room
  std::future<int> requesting[NUM_OF_WORKER_THREADS];
  // Game Room for the users to play, we need num of threads/2 as each room will have 2 player
  Room Game_Room[NUM_OF_WORKER_THREADS/2];
  // function to search for free rooms
  Room* Search_Free_Rooms();
  // function to search room with game id
  Room* Search_Room_with_id(int id);
  // function to convert std string to x and y of the game board
  Vec2 Convert_String_to_Vec2(const char * rhs);
};

/*****************************************************************************/
/*!
  \brief
    This function will search through the game rooms and return a empty room
    
  \return Room *
    The pointer to the Room, nullptr if non is found
*/
/*****************************************************************************/

Room* ThreadQueue::Search_Free_Rooms()
{
  // search though the rooms
  for (int i = 0; i < NUM_OF_WORKER_THREADS / 2; ++i)
  {
    // if there is a room that is not in use, return the pointer of it
    if (Game_Room[i].status == NOT_USED)
      return &Game_Room[i];
  }
  // if no room found return nullptr
  return nullptr;
}

/*****************************************************************************/
/*!
  \brief
    This function will search the room with the id
  
  \param id
    The game id of the thread
    
  \return Room *
    The pointer to the Room, nullptr if non is found
*/
/*****************************************************************************/

Room * ThreadQueue::Search_Room_with_id(int id)
{
  // search through the rooms
  for (int i = 0; i < NUM_OF_WORKER_THREADS / 2; ++i)
  {
    // if the game room is used and black id is same return pointer to room
    if (Game_Room[i].status == USED && Game_Room[i].black_id == id)
      return &Game_Room[i];
  }
  // return nullptr if no room found
  return nullptr;
}

/*****************************************************************************/
/*!
  \brief
    This is a simple function to convert a string to a vec2
  
  \param rhs
    The string to be converted
    
  \return Vec2
    The converted vec2
*/
/*****************************************************************************/

Vec2 ThreadQueue::Convert_String_to_Vec2(const char * rhs)
{
  // the simple conversion to vec2
  int x = (int)rhs[0] - 65;
  int y = rhs[1] - '0';
  return Vec2(x, y);
}

/*****************************************************************************/
/*!
  \brief
    This function will find if the user exists in the telnet server
  
  \param username
    The username which we want to find
    
  \return int
    -1 if not found else the game id
*/
/*****************************************************************************/

int ThreadQueue::find_user(std::string username)
{
  // search the array of username
  for (int i = 0; i < NUM_OF_WORKER_THREADS; ++i)
    // if matches return the game id
    if (Thread_username[i] == username)
      return i;
  // else return -1
  return -1;
}

/*****************************************************************************/
/*!
  \brief
    This function will convert a status to char*
  
  \param stat
    The status that you want to convert
    
  \return char *
    The char* after we converted
*/
/*****************************************************************************/

char* ThreadQueue::status_to_string(status stat)
{
  // convert the stat to char*
  if (stat == NOT_CONNECTED) return "Not Connected";
  else if (stat == AVAILABLE) return "Available";
  else if (stat == WAITING) return "Waiting";
  else if (stat == REQUESTING) return "Requesting";
  else return "In Game";
}

/*****************************************************************************/
/*!
  \brief
    This function will export the list of users
    
  \return std::string
    The string of the list of users
*/
/*****************************************************************************/

std::string ThreadQueue::export_list()
{
  // the list we will return
  std::string list;
  // the numbering
  int index = 1;
  // search the list of threads
  for (int i = 0; i < NUM_OF_WORKER_THREADS; ++i)
  {
    // if thread is not not connected
    if (Thread_status[i] != NOT_CONNECTED)
    {
      // convert the index to string
      list += std::to_string(index).c_str();
      // add the .
      list += ". ";
      // add the username
      list += Thread_username[i];
      // add the open bracket
      list += " (";
      // add the status of the thread
      list += status_to_string(Thread_status[i]);
      // add the close bracket
      list += ")";
      // add the newline
      list += "\n";
      // increment the index
      ++index;
    }
  }
  // add the null char
  list += "\0";
  // return the list
  return list;
}

/*****************************************************************************/
/*!
  \brief
    This function will print the id of the thread
  
  \param id
    The id of the thread
*/
/*****************************************************************************/

void ThreadQueue::print_id(std::thread::id ID)
{
  // prevent race condition
  std::lock_guard<std::mutex> id_lock(id_mutex);
  // print the id
  std::cout << "My Thread ID is " << ID << std::endl;
}

/*****************************************************************************/
/*!
  \brief
    This will produce a socket into the bufer
  
  \param socket
    The socket that will be put into the buffer
*/
/*****************************************************************************/

void ThreadQueue::produce(SOCKET socket)
{
  {
    // prevent multiple entries
    std::unique_lock<std::mutex> lock_slots(slots_mutex);
    // if no more slots wait
    while (slots <= 0) buf_empty.wait(lock_slots);
    // decrement the slots
    --slots;
  }
  {
    // prevent multiple entries
    std::lock_guard<std::mutex> lock_buf(buf_mutex);
    // push socket into buffer
    buffer.push(socket);
  }
  // prevent multiple entries 
  std::lock_guard<std::mutex> lock_items(items_mutex);
  // increment items
  ++items;
  // notify the buf_wait
  buf_empty.notify_one();
}

/*****************************************************************************/
/*!
  \brief
    This function will allow the thread to consume one socket from the buffer
    
  \return SOCKET
    The socket which the thread will communicate with the client
*/
/*****************************************************************************/

SOCKET ThreadQueue::consume()
{
  {
    // prevent multiple entries
    std::unique_lock<std::mutex> lock_items(items_mutex);
    // if no more items wait
    while (items <= 0) buf_empty.wait(lock_items);
    // decrement item
    --items;
  }
  SOCKET socket; // socket to return
  {
    // prevent multiple entries
    std::lock_guard<std::mutex> lock_buf(buf_mutex);
    // store buffer socket to socket
    socket = buffer.front();
    // pop buffer
    buffer.pop();
  }
  // prevent multiple entries
  std::lock_guard<std::mutex> lock_slots(slots_mutex);
  // increment slots
  ++slots;
  // notify buffer full
  buf_full.notify_one();
  // return the socket
  return socket;
}

// creating the master queue
ThreadQueue Queue_of_Threads(NUM_OF_WORKER_THREADS);

/*****************************************************************************/
/*!
  \brief
    This function will simulate the finite state machine of the thread
  
  \param master_queue
    The master queue where all the information is at
    
  \return num
    The game id of the thread
*/
/*****************************************************************************/

void threads_function(ThreadQueue & master_queue, int num)
{
  // set game id as num
  int game_id = num;
  // set my room as nullptr
  Room* myRoom = nullptr;

  // get thread id
  std::thread::id ID_ = std::this_thread::get_id();
  // print thread id
  master_queue.print_id(ID_);

  // create buffer
  char buffer[LENGTH];
  // create username buffer
  char username[LENGTH];
  // zero the buffers
  SecureZeroMemory(buffer, LENGTH);
  SecureZeroMemory(username, LENGTH);
  // result to store error code
  int result = 0;
  // iMode to turn socket to non blocking
  u_long iMode = 1;

  // loop
  while (true)
  {
    // create and set oppponet id as -1
    int opponent_id = -1;
    // set running to true
    bool running = true;
    // set thread status to not connected
    master_queue.Thread_status[game_id] = NOT_CONNECTED;
    // set username to unused thread
    master_queue.Thread_username[game_id] = std::string("UNUSED THREAD");

    // consume wait for client to connect
    SOCKET client = master_queue.consume();
    // print out after thread has connected with client
    std::cout << "Thread " << ID_ << " accepted new connection" << std::endl;

    // recv user name
    result = recv(client, buffer, LENGTH, 0);
    // if client dc during username input set running to false
    if (result == SOCKET_ERROR) running = false;
    // if running, client no dc
    if (running)
    {
      // get the username for the message
      char* user_name;
      short user_length = ntohs(*((short *)&buffer[2]));
      user_name = new char[user_length];
      std::copy(buffer + 4, buffer + 4 + user_length, user_name);
      // store username to string
      std::string user(user_name);
      // deallocate memory
      delete[] user_name;
      // set thread status to available
      master_queue.Thread_status[game_id] = AVAILABLE;
      // set username
      master_queue.Thread_username[game_id] = user;
      // set socket to non blocking
      ioctlsocket(client, FIONBIO, &iMode);
    }
    else // if client dc
    {
      // set room if not nullptr to not used
      if (master_queue.Thread_status[game_id] == IN_GAME) myRoom->status = NOT_USED;
      // clear the username 
      master_queue.Thread_username[game_id].clear();
      // print client dc 
      printf("Client Disconnected\n");
      // shutdown socket
      shutdown(client, SD_BOTH);
      // close socket
      closesocket(client);
    }
    // finite state machine loop
    while (running)
    {
      // zero buffer
      SecureZeroMemory(buffer, LENGTH);
      // set reuslt to 0
      result = 0;

      // handshaking portion, when a client request a game to another client
      if (master_queue.requesting[game_id].valid())
      {
        // if thread is on the receiving side
        if (master_queue.Thread_status[game_id] == AVAILABLE)
        {
          // set thread to waiting
          master_queue.Thread_status[game_id] = WAITING;
          // set opponent id
          opponent_id = master_queue.requesting[game_id].get();
          // create error msg
          std::string error("Request to play with you, type accept to accept and any other message to decline\n\0");
          // encapsulate error msg
          Message error_msg(error);
          //send error msg
          send(client, error_msg.c_str(), (int)error_msg.size(), 0);
        }
        // if thread is on the sending side
        else if (master_queue.Thread_status[game_id] == REQUESTING)
        {
          // get the decision of the other client
          int decision = master_queue.requesting[game_id].get();
          // if is 1 means accepted
          if (decision == 1)
          {
            // search for used room created by other client for the game
            myRoom = master_queue.Search_Room_with_id(game_id);
            // change status to in game
            master_queue.Thread_status[game_id] = IN_GAME;
            // encapsualte and print error msg
            std::string error("Opponent has accepted, You are playing as Black\n\0");
            Message error_msg(error);
            send(client, error_msg.c_str(), (int)error_msg.size(), 0);
            //encapsulate and print board
            std::string board(myRoom->print_board());
            Message board_msg(board);
            send(client, board_msg.c_str(), (int)board_msg.size(), 0);

          }
          else // other client declined
          {
            // set status to available
            master_queue.Thread_status[game_id] = AVAILABLE;
            // set oppponent id to -1
            opponent_id = -1;
            // encapsulate and send error msg
            std::string error("Opponent has declined\n\0");
            Message error_msg(error);
            send(client, error_msg.c_str(), (int)error_msg.size(), 0);
          }
        }
      }

      // status checking
      if (master_queue.Thread_status[game_id] == WAITING)
      {
        // if client dc
        if (master_queue.Thread_status[opponent_id] == NOT_CONNECTED)
        {
          // set thread status to avilable
          master_queue.Thread_status[game_id] = AVAILABLE;
          // encapsulate and send error msg
          std::string error("Opponent has exited\n\0");
          Message error_msg(error);
          send(client, error_msg.c_str(), (int)error_msg.size(), 0);
        }
      }
      // else if thread is in game
      else if (master_queue.Thread_status[game_id] == IN_GAME)
      {
        // if room is no longer used
        if (myRoom->status != USED)
        {
          // set status to available
          master_queue.Thread_status[game_id] = AVAILABLE;
          // encapsulate and send error msg
          std::string error("Opponent has exited or quitted or disconnected\n\0");
          Message error_msg(error);
          send(client, error_msg.c_str(), (int)error_msg.size(), 0);
        }
      }
      // receive msg form client
      result = recv(client, buffer, LENGTH, 0);
      if (result > 0) // if client sent some message
      {
        // waiting state
        if (master_queue.Thread_status[game_id] == WAITING)
        {
          // if command id is accept
          if (buffer[0] == ACCEPT)
          {
            // search free room
            myRoom = master_queue.Search_Free_Rooms();
            // reset the board
            myRoom->reset_board();
            // set own thread as white
            myRoom->white_id = game_id;
            // set opponent as black
            myRoom->black_id = opponent_id;
            // set turn as white
            myRoom->turn = WHITE;
            // set status as used
            myRoom->status = USED;
            // set thread status as in game
            master_queue.Thread_status[game_id] = IN_GAME;
            // create promise and set the future of promise to opponent requesting
            std::promise<int> tmp;
            master_queue.requesting[opponent_id] = tmp.get_future();
            // set promise to be 1
            tmp.set_value(1);

            // encapsualte and send error msg
            std::string error("You have accepted, You are playing as White\n\0");
            Message error_msg(error);
            send(client, error_msg.c_str(), (int)error_msg.size(), 0);

            // encapsulate and send board
            std::string board(myRoom->print_board());
            Message board_msg(board);
            send(client, board_msg.c_str(), (int)board_msg.size(), 0);
            
          }
          // if command id is exit
          else if(buffer[0] == EXIT)
          {
            // create promise and set the future of promise to opponent requesting
            std::promise<int> tmp;
            master_queue.requesting[opponent_id] = tmp.get_future();
            //set value as 0
            tmp.set_value(0);
            // clear username
            master_queue.Thread_username[game_id].clear();
            // print client exited
            printf("Client Exited\n");
            // send back same buffer
            send(client, buffer, result, 0);
            // shutdown client
            shutdown(client, SD_BOTH);
            // closesocket
            closesocket(client);
            // break from loop
            break;
          }
          // if command id is list
          else if (buffer[0] == LIST)
          {
            // encapsulate and send list
            std::string error = master_queue.export_list();
            Message error_msg(error);
            send(client, error_msg.c_str(), (int)error_msg.size(), 0);
          }
          else // anything else will be treated as decline msg
          {
            // set thread status to available
            master_queue.Thread_status[game_id] = AVAILABLE;
            // create promise and set the future of promise to opponent requesting
            std::promise<int> tmp;
            master_queue.requesting[opponent_id] = tmp.get_future();
            // set to 0
            tmp.set_value(0);
            // encapsulate and print out error msg
            std::string error("You have declined\n\0");
            Message error_msg(error);
            send(client, error_msg.c_str(), (int)error_msg.size(), 0);
          }
        }
        // requesting state
        else if (master_queue.Thread_status[game_id] == REQUESTING)
        {
          // if command id is list user
          if (buffer[0] == LIST)
          {
            // encapsulate and send out list
            std::string error = master_queue.export_list();
            Message error_msg(error);
            send(client, error_msg.c_str(), (int)error_msg.size(), 0);
          }
          // if command is is exit
          else if (buffer[0] == EXIT)
          {
            // clear id
            master_queue.Thread_username[game_id].clear();
            //print out client exited
            printf("Client Exited\n");
            // send buffer back to client
            send(client, buffer, result, 0);
            // shutdown client
            shutdown(client, SD_BOTH);
            // close socket
            closesocket(client);
            // break from loop
            break;
          }
          else // anything else will be treated as invalid command
          {
            // encapsulate error msg and send
            std::string error("Please until opponent responses to your request\n\0");
            Message error_msg(error);
            send(client, error_msg.c_str(), (int)error_msg.size(), 0);
          }
        }
        // available state
        else if (master_queue.Thread_status[game_id] == AVAILABLE)
        {
          if (buffer[0] == LIST) // if command id is list user
          {
            // encapsulate and send list
            std::string error = master_queue.export_list();
            Message error_msg(error);
            send(client, error_msg.c_str(), (int)error_msg.size(), 0);
          }
          else if (buffer[0] == EXIT) // if command id is exit
          {
            // clear username
            master_queue.Thread_username[game_id].clear();
            // print client exited
            printf("Client Exited\n");
            // send back buffer
            send(client, buffer, result, 0);
            // shutdown client
            shutdown(client, SD_BOTH);
            // close socket
            closesocket(client);
            // break from loop
            break;
          }
          else if (buffer[0] == PLAY) // if command is play user
          {
            // get the size of user
            int size = ntohs(*((short *)&buffer[2]));
            // copy the user
            std::copy(buffer + 4, buffer + size + 4, username);
            // find the user with the list of usernames
            int index = master_queue.find_user(username);
            // if the user does exists and is not available
            if (index != -1 && master_queue.Thread_status[index] != AVAILABLE)
            {
              // encapsulate and send error msg
              std::string error("Invalid Command\n\0");
              Message error_msg(error);
              send(client, error_msg.c_str(), (int)error_msg.size(), 0);
            }
            // if the user does not exists or the index is own thread
            else if (index == -1 || index == game_id)
            {
              // encapsulate and send error msg
              std::string error("No users matches name\n\0");
              Message error_msg(error);
              send(client, error_msg.c_str(), (int)error_msg.size(), 0);
            }
            else // user exists and is available
            {
              // set status to requesting
              master_queue.Thread_status[game_id] = REQUESTING;
              // set opponent id to index
              opponent_id = index;
              // create promise and set to opponent requesting
              std::promise<int> tmp;
              master_queue.requesting[index] = tmp.get_future();
              // set the promise to threads id
              tmp.set_value(game_id);
              // encapsulate and send error msg
              std::string error("Waiting for opponent to response\n\0");
              Message error_msg(error);
              send(client, error_msg.c_str(), (int)error_msg.size(), 0);
            }
          }
          else // anything else will be treated as invalid command
          {
            // encapsulate and send error msg
            std::string error("Invalid Command\n\0");
            Message error_msg(error);
            send(client, error_msg.c_str(), (int)error_msg.size(), 0);
          }
        }
        else // In_game state
        {
          // if command id is print board
          if (buffer[0] == PRINT)
          {
            // encapsulate and send board
            std::string error = myRoom->print_board();
            Message error_msg(error);
            send(client, error_msg.c_str(), (int)error_msg.size(), 0);
          }
          else if (buffer[0] == LIST) // if command is list user
          {
            // encapsulate and send list
            std::string error = master_queue.export_list();
            Message error_msg(error);
            send(client, error_msg.c_str(), (int)error_msg.size(), 0);
          }
          else if (buffer[0] == EXIT) // command is exit
          {
            // set room status to not used
            myRoom->status = NOT_USED;
            // clear username
            master_queue.Thread_username[game_id].clear();
            // print client exited
            printf("Client Exited\n");
            // send buffer back
            send(client, buffer, result, 0);
            // shutdown client
            shutdown(client, SD_BOTH);
            // close client socket
            closesocket(client);
            // break from loop
            break;
          }
          else if (buffer[0] == QUIT) // command is quit
          {
            // set room status to not used
            myRoom->status = NOT_USED;
            // set thread status to available
            master_queue.Thread_status[game_id] = AVAILABLE;
            // set opponent id to -1
            opponent_id = -1;
            // encapsulate and send error msg
            std::string error("You have Quit from the game\n\0");
            Message error_msg(error);
            send(client, error_msg.c_str(), (int)error_msg.size(), 0);
          }
          else if (buffer[0] == MOVE) // command is move
          {
            // create start buffer
            char start[LENGTH];
            // zero start buffer
            SecureZeroMemory(start, LENGTH);
            // set end buffer
            char end[LENGTH];
            // zero end buffer
            SecureZeroMemory(end, LENGTH);

            // get start move length
            short start_length = ntohs(*((short *)&buffer[2]));
            // copy start move to start buffer
            std::copy(buffer + 4, buffer + 4 + start_length, start);

            // get end move length
            int len = start_length + 4;
            short end_length = ntohs(*((short *)&buffer[len]));

            // copy end move to end buffer
            len += 2;
            std::copy(buffer + len, buffer + len + end_length, end);

            // convert start and end to vec2
            Vec2 startpos = master_queue.Convert_String_to_Vec2(start);
            Vec2 endpos = master_queue.Convert_String_to_Vec2(end);

            // get move message from make move
            std::string move = myRoom->make_move(game_id, startpos, endpos);
            // get board message
            std::string error = myRoom->print_board();
            // concate them together
            error += move;
            // encapsulate and send message
            Message error_msg(error);
            send(client, error_msg.c_str(), (int)error_msg.size(), 0);
          }
          else // anything else will be invalid command
          {
            // encapsulate and send message
            std::string error("Invalid Command\n\0");
            Message error_msg(error);
            send(client, error_msg.c_str(), (int)error_msg.size(), 0);
          }
        }
      }
      else // no commands
      {
        // need some form of checking with the client
        int error;
        // send empty packet
        error = send(client, "", 0, 0);
        // if socket error, client has dc
        if (error == SOCKET_ERROR)
        {
          // set room to not used
          if (master_queue.Thread_status[game_id] == IN_GAME) myRoom->status = NOT_USED;
          // clear username
          master_queue.Thread_username[game_id].clear();
          // print client disconnected
          printf("Client Disconnected\n");
          // shutdown client and close client socket
          shutdown(client, SD_BOTH);
          closesocket(client);
          // break from loop
          break;
        }
      }
    }
  }

}


int main(void)
{
  // array to hold the host name
  char hostname[512];
  // socket for listening
  SOCKET ListenSock;
  // socket to transfer data
  SOCKET DataSock;
  // struct to hold info of winsock
  WSADATA data;
  // zero the memory of the data
  SecureZeroMemory(&data, sizeof(data));
  // init winsock
  int result = WSAStartup(MAKEWORD(2, 2), &data);
  // if init fails
  if (result)
  {
    printf("WSAStartup failed: %d\n", result);
    return 1;
  }
  // store the version of the winsock
  int major = LOBYTE(data.wVersion);
  int minor = HIBYTE(data.wVersion);
  // print the version of winsock
  printf("WinSock version : %d.%d\n", major, minor);
  // string to store the user input of port
  std::string port_input;

  // create a socket for listening
  ListenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  // if the listening socket creation has an error
  if (ListenSock == INVALID_SOCKET)
  {
    printf("socket failed with error: %ld\n", WSAGetLastError());
    WSACleanup();
    return 2;
  }
  // get the hostname and store it in the hostname array
  result = gethostname(hostname, LENGTH);
  // if there is an error in gethostname
  if (result == SOCKET_ERROR)
  {
    printf("Gethostname failed with error: %ld\n", WSAGetLastError());
    WSACleanup();
    return 9;
  }
  // create a addrinfo to allocate the socket to, hints to store the info of the result_addr
  addrinfo * result_addr, hints;
  // zero the memory space of hints
  SecureZeroMemory(&hints, sizeof(hints));

  hints.ai_family = AF_INET; // ipv4
  hints.ai_socktype = SOCK_STREAM; // using reliable byte stream TCP
  hints.ai_protocol = IPPROTO_TCP; // using TCP
  hints.ai_flags = AI_PASSIVE; // telling the socket that it will be use for binding later

  // prompt for user to input port num
  printf("Please input port number...\n");

  // store port num in port_input string
  std::getline(std::cin, port_input);

  printf("\n");

  // get the address info of the host and port, using hints as a reference to set the type of
  // connection and set the result at result_addr
  result = getaddrinfo(hostname, port_input.c_str(), &hints, &result_addr);

  // if there is an error getting the address info
  if (result != 0)
  {
    printf("Getaddrinfo failed with error: %ld\n", result);
    WSACleanup();
    return 10;
  }

  // printf the host name
  printf("Hostname: %s\n", hostname);

  // create a sockaddr to store the address where the socket will connect
  sockaddr_in* addr;

  // store the binary address to the sockaddr_
  addr = (sockaddr_in*)result_addr->ai_addr;
  // make a pointer to point to the address
  void* ip = &addr->sin_addr;
  // create a char array to store the ip address of the host
  char ip_addr[LENGTH];
  // convert the host form binary to text and store it in the ip_addr array
  inet_ntop(AF_INET, ip, ip_addr, INET_ADDRSTRLEN);

  // print out the ip address and port
  printf("IP Address: %s\n", ip_addr);
  printf("Port Number: %s\n\n", port_input.c_str());

  // bind the listening socket, if got error print out error
  if (bind(ListenSock, (SOCKADDR *)addr, sizeof(*addr)) == SOCKET_ERROR)
  {
    printf("bind failed with error: %ld\n", WSAGetLastError());
    closesocket(ListenSock);
    WSACleanup();
    return 3;
  }

  // after binding the listening socket, we wont use the addr anymore so we can free it
  freeaddrinfo(result_addr);

  // listening on the listen socket
  if (listen(ListenSock, 1) == SOCKET_ERROR)
  {
    printf("listen failed with error: %ld\n", WSAGetLastError());
    closesocket(ListenSock);
    WSACleanup();
    return 4;
  }

  // std::vector of threads
  std::vector<std::thread> workers;

  // creating thread as sockets
  for (int i = 0; i < NUM_OF_WORKER_THREADS; ++i)
  {
    auto th = std::thread(threads_function, std::ref(Queue_of_Threads), std::ref(i));
    workers.push_back(std::move(th));
  }

  // loop
  while (true)
  {
    // listen on the main server and if got client request make it accept with the thread socket
    DataSock = accept(ListenSock, NULL, NULL);
    // if socket is error printf error msg else produce the socket to master queue
    if (DataSock == INVALID_SOCKET)
      printf("accept failed with error: %ld\n", WSAGetLastError());
    else
      Queue_of_Threads.produce(DataSock);
  }

  // shutdown the listening socket
  shutdown(ListenSock, SD_BOTH);
  // close listening socket
  closesocket(ListenSock);
  // clean up WSA
  WSACleanup();

  // return 0
  return 0;
}