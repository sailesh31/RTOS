//Headers: General libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

//Headers: Socket related functions
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

//Server port the client will be connecting to
#define PORT_NO 4000

//Max size of the bytes which can be streamed at one go
#define MAX_BUF_SIZE 1024

int main()
{
  //Creating a stream socket; Adressfamily_INET domain; Protocol '0' to choose the proper protocol for the given socket.
  int Socket_Descriptor = socket(AF_INET, SOCK_STREAM, 0);

  //Checking for the proper creation of socket
  if(Socket_Descriptor==-1)
  {
    printf("\nSocket Creation Error. Unable to create desired socket.\n");
    return -1;
  }
  printf("\nSocket Created\n");

  //Defining the structure for server socket

  struct hostent *serverIP;
  char serverdn[] = "ec2-52-66-204-32.ap-south-1.compute.amazonaws.com";
  serverIP = gethostbyname(serverdn);

  struct sockaddr_in Server_Address;
  Server_Address.sin_family = AF_INET;
  Server_Address.sin_port = htons(PORT_NO);

  bcopy((char *) serverIP->h_addr, (char *) &Server_Address.sin_addr.s_addr, serverIP->h_length);
  printf("\nServer Socket Defined\n");

  //Connecting to server socket
  int connect_success = connect(Socket_Descriptor,(struct sockaddr *)  &Server_Address, sizeof(Server_Address));
  if(connect_success==-1)
  {
  		printf("\nUnable to make a socket connection\n");
  		return -1;
  }
  printf("\nConnected to server socket\n");

  //Fullduplex data transmission to server on cloud from a client on local system.
  char Client_Message[MAX_BUF_SIZE];
  char Received_Message[MAX_BUF_SIZE];

  if(fork()==0)
  {
    while(1){
    memset(Client_Message, 0, sizeof(Client_Message));
    printf("\nEnter message to transmit to server: \n");
    scanf("%[^\n]%*c", Client_Message);
    int msg_success = send(Socket_Descriptor, Client_Message, sizeof(Client_Message), 0);
    if(msg_success!=-1)
    {
      printf("Message Sent\n");
    }
    else
    {
      printf("\nError sending message on the socket\n");
    }}
 }
 else
 {
   while(1)
   {
    memset(Received_Message, 0, sizeof(Received_Message));
    int read_success = recv(Socket_Descriptor, Received_Message,MAX_BUF_SIZE,0);
    if(read_success>0)
    {
    printf("Message from Server: %s\n", Received_Message);
    printf("\n");
    }
  }
 }

  return 0;
}
