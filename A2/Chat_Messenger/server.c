//Headers: General libraries
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
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

//Max users tenable
#define MAX_MEMBERS 10

//Message structs
struct Message
{
  char sender_name[MAX_BUF_SIZE];
  char receiver_name[MAX_BUF_SIZE];
  char message[MAX_BUF_SIZE];
  char phone_number[10];
  int message_type;
};

//Creating Message Variables for received and transmitted messsages
char Received_Message[MAX_BUF_SIZE];
char Server_Message[MAX_BUF_SIZE];

//Global Variables Required
int Socket_Descriptor=0;

int Collection_Usersockets[MAX_MEMBERS] = {0};

int user_count = 0;
int group_count = 0;

char DataBase_Name[MAX_MEMBERS][1024];
char DataBase_Number[MAX_MEMBERS][10];

char Group_DB[MAX_MEMBERS][1024];

char *group_code = "123456789";


pthread_t client_threads[MAX_MEMBERS];
pthread_mutex_t registration_lock;
pthread_t read_thread, write_thread;


void *client_handler(void *socket_file_descriptor){
    int client_file_descriptor = *(int *)socket_file_descriptor;
    struct Message temp_message;
    recv(client_file_descriptor, &temp_message, sizeof(temp_message), 0);

    pthread_mutex_lock(&registration_lock);

    if(user_count >= MAX_MEMBERS){
        printf("At Max tenable limit. Cannot acceot anymore people\n");
        struct Message temp_reply;
        temp_reply.message_type = 77;
        send(client_file_descriptor, &temp_reply, sizeof(temp_reply), 0);
        close(client_file_descriptor);
        pthread_cancel(client_threads[user_count]);
    }

    else if(temp_message.message_type==01)
     {
       struct Message temp_reply;
       for(int i=0;i<user_count;i++)
       {
         if(strcmp(DataBase_Name[user_count],temp_message.sender_name) == 0 )
         {
           if(strcmp(DataBase_Number[user_count],temp_message.phone_number) == 0 )
           {
             temp_reply.message_type = 99;
           }
         }
       }
       send(client_file_descriptor, &temp_reply, sizeof(temp_reply), 0);
     }

     else if(temp_message.message_type==00)
     {
       struct Message temp_reply;
       strcpy(DataBase_Name[user_count],temp_message.sender_name);
       strcpy(DataBase_Number[user_count],temp_message.phone_number);
       temp_reply.message_type = 99;
       send(client_file_descriptor, &temp_reply, sizeof(temp_reply), 0);
       Collection_Usersockets[user_count] = client_file_descriptor;
       user_count+=1;
     }

     else if(temp_message.message_type==20)
     {
       struct Message temp_reply;
       if(strcmp(temp_message.message,group_code)==0)
       {
         temp_reply.message_type = 99;
         send(client_file_descriptor, &temp_reply, sizeof(temp_reply), 0);
       }
       strcpy(Group_DB[group_count],temp_message.sender_name);
       group_count+=1;
     }

     pthread_mutex_unlock(&registration_lock);

     struct Message communication;

     while(recv(client_file_descriptor, &communication, sizeof(communication), 0))
     {
        if(communication.message_type==10)
        {
          for(int i = 0; i < user_count; i++)
          {
                if(strcmp(DataBase_Name[i], communication.receiver_name) == 0)
                {
                    send(Collection_Usersockets[i], &communication, sizeof(communication), 0);
                }
          }
        }

        else if(communication.message_type==11)
        {
          for(int i = 0; i < group_count; i++)
          {
                if(strcmp(Group_DB[i], communication.sender_name) != 0)
                {
                    send(Collection_Usersockets[i], &communication, sizeof(communication), 0);
                }
          }
        }

     }

       // printf("%s Left the group\n",temp_message.sender_name);
       pthread_mutex_lock(&registration_lock);
       int exit_member_id;
       for(int i=0;i<MAX_MEMBERS;i++){
           if(strcmp(temp_message.sender_name, DataBase_Name[i]) == 0){
               exit_member_id = 1;
               break;
           }
       }
       for(exit_member_id;exit_member_id<user_count;exit_member_id++){
           Collection_Usersockets[exit_member_id] = Collection_Usersockets[exit_member_id + 1];
           strcpy(DataBase_Name[exit_member_id],DataBase_Number[exit_member_id + 1]);
           strcpy(DataBase_Name[exit_member_id],DataBase_Number[exit_member_id + 1]);
       }
       user_count--;
       pthread_mutex_unlock(&registration_lock);



}


int main()
{
  Socket_Descriptor = socket(AF_INET, SOCK_STREAM, 0);

  //Checking for the proper creation of socket
  if(Socket_Descriptor==-1)
  {
    printf("\nSocket Creation Error. Unable to create desired socket.\n");
    return -1;
  }
  printf("\nSocket creation successful\n");

  //
  int opt = 1;
  int sockopt_success = setsockopt(Socket_Descriptor, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
  if(sockopt_success==-1)
  {
    printf("\n Unable to setsockopt\n" );
  }

  //Defining the properties of the connection socket
  struct sockaddr_in Address;
  Address.sin_family = AF_INET;
  Address.sin_port = htons(PORT_NO);
  Address.sin_addr.s_addr = INADDR_ANY;
  int Addresslen = sizeof(Address);

  //Binding the socket
  int bind_success = bind(Socket_Descriptor, (struct sockaddr *)&Address, sizeof(Address));
  if(bind_success == -1)
  {
         printf("\n Unable to bind the port to the socket\n");
         return -1;
  }
  printf("\nSocket binding complete\n");

  //Listening on the socket for connections
  int listen_success = listen(Socket_Descriptor, MAX_MEMBERS);
  if(listen_success == -1)
  {
         printf("\nListen operation not succesful on the socket\n");
         return -1;
  }
  printf("\nListening\n");

  int temp = 0;
  struct sockaddr_in temp_serv;
  for(;;)
  {
    int *connection_file_descriptor = malloc(sizeof(int));
   *connection_file_descriptor = accept(Socket_Descriptor, (struct sockaddr*)&temp_serv, &temp);

   if(*connection_file_descriptor < 0){
       printf("Client connection failed\n");
       exit(0);
   }

   printf("connection succesful\n")
   pthread_create(&client_threads[user_count], NULL, client_handler, (void *)connection_file_descriptor);

  }




  return 0;
}
