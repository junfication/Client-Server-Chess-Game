# Client-Server-Chess-Game
A multithreaded Server where different can play amongst each other  
Note that legal moves are not implemented and players can 'cheat' as there is no checking
of legal moves

Dependencies : Visual Studio 2015

# Usage

## Server

Upon compiling and running the visual studio project, the server program  
will ask the user for a port number. And after entering the port number,  
the server will connect to the port and print all the details which the  
server is connected for the client to connect to.

## Client

Upon compiling and running the visual studio project, the client program  
will ask the user for the server's ip address and port number. This can  
be gotten from the server side, after enter the port number into the  
server will print out the details. You just have to input them into the  
client.

Upon connected to the server, the program will prompt the user to input  
the message to be sent to the server.

The commands for the client are  
__Universal commands can be used in any state__  
_list user_	 (to see the list of user)  
_exit_ 	         (to exit the server)  

__Available state commands__  
_play user_ 	(to play with other users replace user with the clients username)  

__Waiting state commands__  
_accept_	        (to accept the game)  
any other input will be treated as decline  

__Ingame state commands__  
_move_          (start end move a piece at start pos to end pos, start and end pos alphabet needs to be CAPS)  
_quit_	       	(quit the game)  
_print board_ 	(print the current game board)  

In any case of the server were to disconnect, the client will prompt the  
user to reenter a ip address and port number to reconnect again.
