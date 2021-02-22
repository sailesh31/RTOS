//Headers: General libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <json-c/json.h>

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
#define MAX_MEMBERS 256
#define NUMBER_GROUPS 1


//Global Variables Required
int Socket_Descriptor=0;
int user_count=0;
int Collection_Usersockets[MAX_MEMBERS] = {0};
char UserNames[MAX_MEMBERS][20];
char Groups[NUMBER_GROUPS][NAME_BUF];
int Group_Size[NUMBER_GROUPS] = {0};
char Group_Members[NUMBER_GROUPS][MAX_MEMBERS][NAME_BUF];

//pthread variable
pthread_t client_threads[MAX_MEMBERS];
pthread_mutex_t registration_lock;

void parse_json()
{
    FILE *file_pointer;
    char buff[MAX_BUF_SIZE];

    struct json_object *parsed_json;
    struct json_object *groups;

    struct json_object *group_name;


    file_pointer = fopen("server_groups.json","r");
    fread(buff, MAX_BUF_SIZE, 1, file_pointer);
    fclose(file_pointer);

    parsed_json = json_tokener_parse(buff);

    json_object_object_get_ex(parsed_json, "groups", &groups);


    for(int i=0; i<NUMBER_GROUPS;i++)
    {
       char temp_group_name[NAME_BUF];
       sprintf(Groups[i],"group_%d",i+1);
       sprintf(temp_group_name,"group_%d",i+1);

       json_object_object_get_ex(groups, temp_group_name, &group_name);

       int temp_group_user_count = json_object_array_length(group_name);
       Group_Size[i] = temp_group_user_count;

       for(int j=0; j<temp_group_user_count;j++)
       {
         struct json_object *temp_users;
         temp_users = json_object_array_get_idx(group_name, j);
         strcpy(Group_Members[i][j], json_object_get_string(temp_users));
       }

    }


}


void *client_handler(void *socket_file_descriptor)
{
    int client_socket_file_descriptor = *(int *)socket_file_descriptor;
    char temp_client_name[NAME_BUF];
    recv(client_socket_file_descriptor, &temp_client_name, sizeof(temp_client_name),0);

    pthread_mutex_lock(&registration_lock);

    if(user_count >= MAX_MEMBERS)
    {
      printf("Maximum tenable user limit reached.\n");
      close(client_socket_file_descriptor);
      pthread_cancel(client_threads[user_count]);
    }

    //User Validation
    int user_exist_status = 0;
    for(int k=0; k<NUMBER_GROUPS; k++)
    {
      for(int l=0;l<Group_Size[k];l++)
      {
        if(strcmp(temp_client_name, Group_Members[k][l])==0)
        {
          user_exist_status = 1;
        }
      }
    }

    if(user_exist_status==0)
    {
      printf("Invalid user. Please check and login again\n");
      close(client_socket_file_descriptor);
      pthread_cancel(client_threads[user_count]);
    }


    strcpy(UserNames[user_count],temp_client_name);
    Collection_Usersockets[user_count]=client_socket_file_descriptor;
    user_count++;


    pthread_mutex_unlock(&registration_lock);

    struct Message received;

    while(recv(client_socket_file_descriptor,&received,sizeof(received),0))
    {
      printf("%d Request Type\n",received.msgtype);
      printf("%s Request Sender\n",received.name );
      printf("%s Request for\n",received.recipient_id);
      if(received.msgtype==1)
      {
        for(int k=0;k<user_count;k++)
        {
          if(strcmp(received.recipient_id,UserNames[k])==0)
          {
            int tosend_file_descriptor = 0;
            for(int l = 0; l < user_count; l++)
            {
                if(strcmp(UserNames[l], received.recipient_id) == 0)
                {
                  tosend_file_descriptor = Collection_Usersockets[l];
                }
            }
            send(tosend_file_descriptor,&received,sizeof(received),0);
            printf("Private Message Sent\n");
            break;
          }
        }
      }
      else if(received.msgtype==0)
      {
        printf("Gming\n");
        for(int k=0;k<NUMBER_GROUPS;k++)
        {
          printf("%s\n",Groups[k]);
          printf("%s\n",received.group_id);
          if(strcmp(received.group_id,Groups[k])==0)
          {
            printf("Got the group\n");
            for(int l=0; l<Group_Size[k];l++)
            {
              if(strcmp(received.name,Group_Members[k][l])!=0)
              {
                struct Message tosend;
                strcpy(tosend.recipient_id,Group_Members[k][l]);
                strcpy(tosend.name,received.name);
                tosend.msgtype = received.msgtype;
                strcpy(tosend.group_id,received.group_id);
                tosend.voice_or_text = received.voice_or_text;
                strcpy(tosend.msg,received.msg);
                int tosend_file_descriptor = 0;
                for(int m = 0; m < user_count; m++)
                {
                    if(strcmp(UserNames[m], Group_Members[k][l]) == 0)
                    {
                      tosend_file_descriptor = Collection_Usersockets[m];
                    }
                }(

                send(tosend_file_descriptor,&tosend,sizeof(tosend),0));
              }
            }
            printf("Group Message Sent\n");
            break;
          }

        }
      }
    }

 pthread_mutex_unlock(&registration_lock);



}


int main()
{
  parse_json();
  printf("Loaded Data Succesfully\n");
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

     printf("\nConnection succesful\n");
     pthread_create(&client_threads[user_count], NULL, client_handler, (void *)connection_file_descriptor);

  }




  return 0;
}
