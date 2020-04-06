# Client-Server-Programming

  In this repository, there are three client-server programs which are Basic Client-Server Calculator,
  Chat Room, and Client-Server Using OpenSSL. These are my homework from my Network Programming course.
  
 # Definitions
 
 # Basic Client-Server Calculator

  In the homework definition, this program server waits for basic mathematical operation from the client to calculate it. 
  When the client requests a mathematical operations result from the server, the server returns the result of the operation to the client.
  
  Compile :
  
  gcc server.c -o server -lpthread
  <br>
  gcc client.c -o client -lpthread
  
 # Chat Room
 
  In this homework definition, the instructor wants us to build a chatroom. In the client, you need to define a user name and a password. 
  Username is your nickname in the chat room and a password is for a private chat room. If you don't enter a password,  you will be in the public chat room.
  
  Compile :
  
  gcc server.c -o server -lpthread
  <br>
  gcc client.c -o client -lpthread
  
 # Client-Server Using OpenSSL
 
 In this homework definition, the instructor wants us to create a simple TCP server and client that can talk to each other using a secure 
 channel using the OpenSSL library. The server should echo back what the client writes indefinitely until the client sends the QUIT command.
 
 Compile :

 gcc server.c -o server -lssl -lcrypto
 <br>
 gcc client.c -o client -lssl -lcrypto