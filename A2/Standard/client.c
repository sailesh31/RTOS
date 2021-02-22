//Headers: General libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

//Headers: Socket related functions
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

//Message structure header
#include "message.h"

//Server port the client will be connecting to
#define PORT_NO 9000

//Max size of the bytes which can be streamed at one go
#define MAX_BUF_SIZE 1024
#define NAME_BUF 20

//Creating Message Variables for received and transmitted messsages
struct Message ClientSide;
struct Message ReceiverSide;

//Message structs
int Socket_Descriptor=0;

//Thread based function to receive data from central server
pthread_t read_thread;

void *Receive_Message(){
    for(;;){

        int message_receive_success;
        message_receive_success = recv(Socket_Descriptor, &ReceiverSide, sizeof(ReceiverSide), 0);

        if(ReceiverSide.voice_or_text==1)
        {
          if(ReceiverSide.msgtype==0)
          {
            printf("\nGroup Message Received in :%s; Message sent by :%s; Message :%s\n", ReceiverSide.group_id, ReceiverSide.name, ReceiverSide.msg);
            fflush(stdout);
          }
          else if(ReceiverSide.msgtype==1)
          {
            printf("\nPrivate Message Sent by :%s; Message:%s \n",ReceiverSide.name, ReceiverSide.msg);
            fflush(stdout);
          }

        }

        else if(ReceiverSide.voice_or_text==0)
        {
          fflush(stdout);
        }

    }
}

//Username
char Username[NAME_BUF];

//Additional function descriptions
void Copy_Message_Private(char Username[], int msgtype, char id[], char client_message[], int voice_or_text, struct Message *ClientSide);
void Copy_Message_Group(char Username[], int msgtype, char id[], char client_message[], int voice_or_text, struct Message *ClientSide);

int main()
{
  //Creating a stream socket; Adressfamily_INET domain; Protocol '0' to choose the proper protocol for the given socket.
  Socket_Descriptor = socket(AF_INET, SOCK_STREAM, 0);

  //Checking for the proper creation of socket
  if(Socket_Descriptor==-1)
  {
    printf("\nSocket Creation Error. Unable to create desired socket.\n");
    return -1;
  }
  printf("\nSocket Created\n");

  //Defining the structure for server socket
  struct hostent *serverIP;
  // char serverdn[] = "ec2-13-127-241-9.ap-south-1.compute.amazonaws.com";
  char serverdn[] = "127.0.0.1";

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


  //Getting User Info
  printf("\nWelcome to My Chat Messenger\n");

  // Username
  printf("\nEnter your username:");
  scanf("%s",&Username);
  send(Socket_Descriptor,&Username,sizeof(Username),0);

  //For receiving messsages
  pthread_create(&read_thread, NULL, Receive_Message, NULL);

  //User messenging action
  for(;;)
  {
    int voice_or_text;
    printf("Choose communication type : 1)Text Mesage 0)Voice Message\n");
    scanf("%d",&voice_or_text);

    if(voice_or_text==1)
    {
        int msgtype;
        printf("Choose Action : 1)Private Message 0)Group Message\n");
        scanf("%d",&msgtype);
        if(msgtype==1)
        {
          char recipient_id[NAME_BUF];
          printf("Enter the username of recepient to message:\n");
          scanf("%s", &recipient_id);


          printf("Enter Message to send: ");
          char client_message[MAX_BUF_SIZE];
          scanf(" %[^\n]%*c",client_message);
                // scanf("%[^\n]s",client_message);

          printf("%s\n",client_message );

          Copy_Message_Private(Username, msgtype, recipient_id,client_message,voice_or_text,&ClientSide);
          // printf("Sending message %s\n",ClientSide.name);
          int message_send_success = send(Socket_Descriptor, &ClientSide, sizeof(ClientSide), 0);
        }

        else if(msgtype==0)
        {
          char group_id[NAME_BUF];
          printf("Enter group code\n");
          scanf("%s",&group_id);


          printf("Enter Message to send: ");
          char client_message[MAX_BUF_SIZE];
          scanf(" %[^\n]%*c",client_message);
          printf("%s\n",client_message);
          // scanf("%[^\n]s",client_message);

          Copy_Message_Group(Username, msgtype, group_id,client_message,voice_or_text,&ClientSide);
          int message_send_success = send(Socket_Descriptor, &ClientSide, sizeof(ClientSide), 0);
        }
    }

    else if(voice_or_text==0)
    {
      printf("Coming soon\n");
      // int msgtype;
      // printf("Choose Action : 1)Private Message 0)Group Message\n");
      // scanf("%d",&msgtype);
    }

  }

  return 0;
}


void Copy_Message_Private(char Username[], int msgtype, char id[], char client_message[], int voice_or_text, struct Message *ClientSide)
{
   ClientSide->msgtype = msgtype;
   ClientSide->voice_or_text = voice_or_text;
   strcpy(ClientSide->name,Username);
   strcpy(ClientSide->recipient_id,id);
   strcpy(ClientSide->msg, client_message);
}

void Copy_Message_Group(char Username[], int msgtype, char id[], char client_message[], int voice_or_text, struct Message *ClientSide)
{
   ClientSide->msgtype = msgtype;
   ClientSide->voice_or_text = voice_or_text;
   strcpy(ClientSide->name,Username);
   strcpy(ClientSide->group_id,id);
   strcpy(ClientSide->msg, client_message);
}
