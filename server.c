#include "server.h"

int chat_serv_sock_fd; //server socket

/////////////////////////////////////////////
// USE THESE LOCKS AND COUNTER TO SYNCHRONIZE

int numReaders = 0; // keep count of the number of readers

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  // mutex lock
pthread_mutex_t rw_lock = PTHREAD_MUTEX_INITIALIZER;  // read/write lock

/////////////////////////////////////////////


char const *server_MOTD = "Thanks for connecting to the BisonChat Server.\n\nchat>";

struct node *head = NULL;  // List of all users
struct room *rooms = NULL;  // List of all rooms
struct connection *connections = NULL;  // List of all direct connections

int main(int argc, char **argv) {

   signal(SIGINT, sigintHandler);
    
   //////////////////////////////////////////////////////
   // create the default room for all clients to join when 
   // initially connecting
   rooms = insertFirstR(rooms, DEFAULT_ROOM);
   printf("Default room '%s' created.\n", DEFAULT_ROOM);
   //////////////////////////////////////////////////////

   // Open server socket
   chat_serv_sock_fd = get_server_socket();

   // step 3: get ready to accept connections
   if(start_server(chat_serv_sock_fd, BACKLOG) == -1) {
      printf("start server error\n");
      exit(1);
   }
   
   printf("Server Launched! Listening on PORT: %d\n", PORT);
    
   //Main execution loop
   while(1) {
      //Accept a connection, start a thread
      int new_client = accept_client(chat_serv_sock_fd);
      if(new_client != -1) {
         pthread_t new_client_thread;
         pthread_create(&new_client_thread, NULL, client_receive, (void *)&new_client);
         pthread_detach(new_client_thread);  // Detach thread to prevent resource leaks
      }
   }

   close(chat_serv_sock_fd);
}


int get_server_socket() {
    int opt = TRUE;   
    int master_socket;
    struct sockaddr_in address; 
    
    //create a master socket  
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)   
    {   
        perror("socket failed");   
        exit(EXIT_FAILURE);   
    }   
     
    //set master socket to allow multiple connections ,  
    //this is just a good habit, it will work without this  
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,  
          sizeof(opt)) < 0 )   
    {   
        perror("setsockopt");   
        exit(EXIT_FAILURE);   
    }   
     
    //type of socket created  
    address.sin_family = AF_INET;   
    address.sin_addr.s_addr = INADDR_ANY;   
    address.sin_port = htons( PORT );   
         
    //bind the socket to localhost port 8888  
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)   
    {   
        perror("bind failed");   
        exit(EXIT_FAILURE);   
    }   

   return master_socket;
}


int start_server(int serv_socket, int backlog) {
   int status = 0;
   if ((status = listen(serv_socket, backlog)) == -1) {
      printf("socket listen error\n");
   }
   return status;
}


int accept_client(int serv_sock) {
   int reply_sock_fd = -1;
   socklen_t sin_size = sizeof(struct sockaddr_storage);
   struct sockaddr_storage client_addr;
   //char client_printable_addr[INET6_ADDRSTRLEN];

   // accept a connection request from a client
   // the returned file descriptor from accept will be used
   // to communicate with this client.
   if ((reply_sock_fd = accept(serv_sock,(struct sockaddr *)&client_addr, &sin_size)) == -1) {
      printf("socket accept error\n");
   }
   return reply_sock_fd;
}


/* Handle SIGINT (CTRL+C) */
void sigintHandler(int sig_num) {
   printf("\nShutting down server gracefully...\n");

   //////////////////////////////////////////////////////////
   //Closing client sockets and freeing memory from user list
   //////////////////////////////////////////////////////////

   printf("--------CLOSING ACTIVE USERS--------\n");
   
   // Acquire write lock to ensure exclusive access
   pthread_mutex_lock(&rw_lock);
   
   // Close all client sockets
   struct node *current = head;
   while(current != NULL) {
      printf("Closing socket %d (user: %s)\n", current->socket, current->username);
      close(current->socket);
      current = current->next;
   }

   // Free all users
   current = head;
   while(current != NULL) {
      struct node *temp = current;
      current = current->next;
      free(temp);
   }
   head = NULL;

   // Free all rooms and their user lists
   struct room *room_current = rooms;
   while(room_current != NULL) {
      struct node *user = room_current->users;
      while(user != NULL) {
         struct node *temp = user;
         user = user->next;
         free(temp);
      }
      struct room *temp = room_current;
      room_current = room_current->next;
      free(temp);
   }
   rooms = NULL;

   // Free all connections
   struct connection *conn_current = connections;
   while(conn_current != NULL) {
      struct connection *temp = conn_current;
      conn_current = conn_current->next;
      free(temp);
   }
   connections = NULL;

   pthread_mutex_unlock(&rw_lock);

   printf("--------SERVER SHUTDOWN COMPLETE--------\n");

   close(chat_serv_sock_fd);
   exit(0);
}