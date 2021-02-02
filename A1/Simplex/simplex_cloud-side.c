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
  int Socket_Descriptor = socket(AF_INET, SOCK_STREAM, 0);

  //Checking for the proper creation of socket
  if(Socket_Descriptor==-1)
  {
    printf("\nSocket Creation Error. Unable to create desired socket.\n");
    return -1;
  }
  printf("\nSocket creation successful\n");
  int opt = 1;
  int sockopt_success = setsockopt(Socket_Descriptor, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

  if(sockopt_success==-1)
  {
    printf("\n Unable to setsockopt\n" );
  }

  struct sockaddr_in Address;
  Address.sin_family = AF_INET;
  Address.sin_port = htons(PORT_NO);
  Address.sin_addr.s_addr = INADDR_ANY;

  int Addresslen = sizeof(Address);

  int bind_success = bind(Socket_Descriptor, (struct sockaddr *)&Address, sizeof(Address));

  if(bind_success == -1)
  {
         printf("\n Unable to bind the port to the socket\n");
         return -1;
  }
  printf("\nSocket binding complete\n");
  int listen_success = listen(Socket_Descriptor, 3);

  if(listen_success == -1)
  {
         printf("\nListen operation not succesful on the socket\n");
         return -1;
  }
  printf("\nListening\n");
  int New_socket_descriptor = accept(Socket_Descriptor, (struct sockaddr *) &Address, (socklen_t *) &Addresslen);

  if(New_socket_descriptor==-1)
  {
         printf("\n Unable to accept request from client\n");
  }

  printf("\nSuccessful Connection established with client\n");
  char Received_Message[MAX_BUF_SIZE];
  while(1)
  {
      int read_success = read(Socket_Descriptor, Received_Message,MAX_BUF_SIZE);
      printf("Message from Client: %s\n", Received_Message);
  }




  return 0;
}
