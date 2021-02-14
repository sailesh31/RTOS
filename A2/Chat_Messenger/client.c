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

//Server port the client will be connecting to
#define PORT_NO 9000

//Max size of the bytes which can be streamed at one go
#define MAX_BUF_SIZE 1024

//Creating Message Variables for received and transmitted messsages
char Client_Message[MAX_BUF_SIZE];
char Received_Message[MAX_BUF_SIZE];

//Message structs
struct Message
{
  char sender_name[MAX_BUF_SIZE];
  char receiver_name[MAX_BUF_SIZE];
  char message[MAX_BUF_SIZE];
  char phone_number[10];
  int message_type;
};

int Socket_Descriptor=0;

//Thread based function to receive data from central server
pthread_t read_thread;
void *Receive_Message()
{
    for (;;)
    {
        int message_success;
        struct Message m;
        message_success = recv(Socket_Descriptor, &m, sizeof(m), 0);
        fprintf(stdout,"\33[2K\r%s : %s Sent: ", m.sender_name ,m.message);
        fflush(stdout);
    }
}

//Additional function definations
int Register_user(char user_name[],char phone_number[],int Socket_Descriptor);
int Authenticate_user(char user_name[],char phone_number[],int Socket_Descriptor);
int Send_Private_Message(int a,char Receiver_Name[],char Client_Message[],int Socket_Descriptor,char user_name[]);
int Send_Group_Message(int a,char group_code[],char Client_Message[],int Socket_Descriptor,char user_name[]);
int Join_Group(char group_code[],int Socket_Descriptor, char user_name[]);



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
  char serverdn[] = "ec2-13-127-241-9.ap-south-1.compute.amazonaws.com";
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

  // 1) Existing/New User
  int user_type = 0;
  printf("\nIf existing user enter 1 else enter 0\n");
  scanf("%d",&user_type);

  // 2) Username
  char user_name[MAX_BUF_SIZE];
  char phone_number[10];
  if(user_type==0)
  {
    printf("Please enter a username to continue\n");
    scanf("%s",&user_name);
    printf("Please enter your mobile number\n");
    scanf("%s",&phone_number);
    int reg_success = Register_user(user_name,phone_number,Socket_Descriptor);

    if(reg_success==1)
    {
      printf("Succesfully Registered. You can continue to the Messenger\n");
    }
    else if(reg_success==0)
    {
      printf("Username/Mobile number in use. Unable to continue\n");
      exit(0);
    }
    else if(reg_success==-1)
    {
      printf("Error registering due to Max tenable limit. Please try again\n");
      exit(0);
    }
  }

  else if(user_type==1)
  {
    printf("Please enter your username\n");
    scanf("%s",&user_name);
    printf("Please enter your mobile number\n");
    scanf("%s",&phone_number);
    int auth_success = Authenticate_user(user_name,phone_number,Socket_Descriptor);
    if(auth_success==1)
    {
      printf("Welcome back user. You can continue to the Messenger\n");
    }
    else
    {
      printf("Unable to authenticate. Please try again\n");
      exit(0);
    }
  }



  //For receiving messsages
  pthread_create(&read_thread, NULL, Receive_Message, NULL);


  //User messenging action
  for(;;)
  {
    int user_action;
    printf("Choose Action : 1)Private Message 2)Group Message 3)Join Group \n");
    scanf("%d",&user_action);
    if(user_action==1)
    {
      char Receiver_Name[MAX_BUF_SIZE];
      printf("Enter username to message:\n");
      scanf("%s", &Receiver_Name);
      printf("Enter Message to send:\n");
      scanf("%s",&Client_Message);
      Send_Private_Message(1,Receiver_Name,Client_Message,Socket_Descriptor,user_name);
    }
    else if(user_action==2)
    {
      char group_code[MAX_BUF_SIZE];
      printf("Enter group code\n");
      scanf("%s",&group_code);
      printf("Enter Message to send:\n");
      scanf("%s",&Client_Message);
      Send_Group_Message(2,group_code,Client_Message,Socket_Descriptor,user_name);
    }
    else if(user_action==3)
    {
      char group_code[MAX_BUF_SIZE];
      printf("Enter group code\n");
      scanf("%s",&group_code);
      int group_join_success = Join_Group(group_code,Socket_Descriptor,user_name);
      if(group_join_success==1)
      {
        printf("Succesfully Joined the group\n");
      }
      else
      {
        printf("Wrong code. Unable to join the group\n");
      }
    }
  }

  return 0;
}



//Defining Custom Functions for the messenger application
int Register_user(char user_name[],char phone_number[],int Socket_Descriptor)
{
  struct Message A1;
  strcpy(A1.sender_name,user_name);
  strcpy(A1.phone_number,phone_number);
  A1.message_type = 00;
  send(Socket_Descriptor, &A1, sizeof(A1), 0);

  struct Message B1;
  read(Socket_Descriptor, &B1, sizeof(B1));

  if(B1.message_type==99)
  {
    return 1;
  }

  else if(B1.message_type==88)
  {
    return 0;
  }

  else if(B1.message_type==77)
  {
    return -1;
  }
}


int Authenticate_user(char user_name[],char phone_number[],int Socket_Descriptor)
{
  struct Message A2;
  strcpy(A2.sender_name,user_name);
  strcpy(A2.phone_number,phone_number);
  A2.message_type = 01;
  send(Socket_Descriptor, &A2, sizeof(A2), 0);

  struct Message B2;
  read(Socket_Descriptor, &B2, sizeof(B2));
  if(B2.message_type==99)
  {
    return 1;
  }

  else
  {
    return 0;
  }

}

int Send_Private_Message(int a,char Receiver_Name[],char Client_Message[],int Socket_Descriptor,char user_name[])
{
  struct Message A3;
  strcpy(A3.receiver_name,Receiver_Name);
  strcpy(A3.sender_name,user_name);
  A3.message_type = 10;
  strcpy(A3.message,Client_Message);
  send(Socket_Descriptor, &A3, sizeof(A3), 0);
}

int Send_Group_Message(int a,char group_code[],char Client_Message[],int Socket_Descriptor,char user_name[])
{
  struct Message A4;
  strcpy(A4.receiver_name,group_code);
  strcpy(A4.sender_name,user_name);
  A4.message_type = 11;
  strcpy(A4.message,Client_Message);
  send(Socket_Descriptor, &A4, sizeof(A4), 0);

}

int Join_Group(char group_code[],int Socket_Descriptor,char user_name[])
{
  struct Message A5;
  strcpy(A5.sender_name,user_name);
  A5.message_type = 20;
  strcpy(A5.message,group_code);
  send(Socket_Descriptor, &A5, sizeof(A5), 0);

  struct Message B5;
  read(Socket_Descriptor, &B5, sizeof(B5));
  if(B5.message_type==99)
  {
    return 1;
  }

  else
  {
    return 0;
  }
}
