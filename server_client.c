#include "server.h"

#define DEFAULT_ROOM "Lobby"

// USE THESE LOCKS AND COUNTER TO SYNCHRONIZE
extern int numReaders;
extern pthread_mutex_t rw_lock;
extern pthread_mutex_t mutex;

extern struct node *head;
extern struct room *rooms;
extern struct connection *connections;

extern char *server_MOTD;

// Reader-writer lock helper functions
void acquire_read_lock() {
   pthread_mutex_lock(&mutex);
   numReaders++;
   if(numReaders == 1) {
      pthread_mutex_lock(&rw_lock);
   }
   pthread_mutex_unlock(&mutex);
}

void release_read_lock() {
   pthread_mutex_lock(&mutex);
   numReaders--;
   if(numReaders == 0) {
      pthread_mutex_unlock(&rw_lock);
   }
   pthread_mutex_unlock(&mutex);
}

void acquire_write_lock() {
   pthread_mutex_lock(&rw_lock);
}

void release_write_lock() {
   pthread_mutex_unlock(&rw_lock);
}


/*
 *Main thread for each client.  Receives all messages
 *and passes the data off to the correct function.  Receives
 *a pointer to the file descriptor for the socket the thread
 *should listen on
 */

char *trimwhitespace(char *str)
{
  char *end;

  // Trim leading space
  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;

  // Write new null terminator character
  end[1] = '\0';

  return str;
}

void *client_receive(void *ptr) {
   int client = *(int *) ptr;  // socket
  
   int received, i;
   char buffer[MAXBUFF], sbuffer[MAXBUFF];  //data buffer of 2K  
   char tmpbuf[MAXBUFF];  //data temp buffer of 1K  
   char cmd[MAXBUFF], username[20];
   char *arguments[80];
    
   send(client  , server_MOTD , strlen(server_MOTD) , 0 ); // Send Welcome Message of the Day.

   // Creating the guest user name
   sprintf(username,"guest%d", client);
   
   // Add user to global list (write operation)
   acquire_write_lock();
   head = insertFirstU(head, client, username);
   // Add the GUEST to the DEFAULT ROOM (i.e. Lobby)
   struct room *lobby = findR(rooms, DEFAULT_ROOM);
   if(lobby != NULL) {
      addUserToRoom(lobby, client, username);
   }
   release_write_lock();

   while (1) {
       
      if ((received = read(client , buffer, MAXBUFF)) > 0) {
      
            // Ensure null termination
            if(received >= MAXBUFF) {
               received = MAXBUFF - 1;
            }
            buffer[received] = '\0'; 
            
            // Remove trailing newline/carriage return if present
            while(received > 0 && (buffer[received-1] == '\n' || buffer[received-1] == '\r')) {
               buffer[received-1] = '\0';
               received--;
            }
            
            strcpy(cmd, buffer);  
            strcpy(sbuffer, buffer);
         
            /////////////////////////////////////////////////////
            // we got some data from a client

            // 1. Tokenize the input in buf (split it on whitespace)
            // Initialize arguments array
            for(i = 0; i < 80; i++) {
               arguments[i] = NULL;
            }

            // get the first token 
            arguments[0] = strtok(cmd, delimiters);
            if(arguments[0] != NULL) {
               // Trim the first argument in place (trimwhitespace modifies the string and may return a shifted pointer)
               arguments[0] = trimwhitespace(arguments[0]);
            }

            // walk through other tokens 
            i = 0;
            while(arguments[i] != NULL && i < 79) {
               arguments[++i] = strtok(NULL, delimiters);
               if(arguments[i] != NULL) {
                  // Trim each argument in place (trimwhitespace modifies the string and may return a shifted pointer)
                  arguments[i] = trimwhitespace(arguments[i]);
               }
            }
            
            // Validate argument count (prevent buffer overflow)
            if(i >= 80) {
               sprintf(buffer, "Error: Too many arguments.\nchat>");
               send(client, buffer, strlen(buffer), 0);
               continue;
            } 

             // Arg[0] = command
             // Arg[1] = user or room
             
             /////////////////////////////////////////////////////
             // 2. Execute command: TODO


            if(strcmp(arguments[0], "create") == 0)
            {
               if(arguments[1] == NULL) {
                  sprintf(buffer, "Error: Room name required.\nchat>");
                  send(client, buffer, strlen(buffer), 0);
               } else if(strlen(arguments[1]) >= 50) {
                  sprintf(buffer, "Error: Room name too long (max 49 characters).\nchat>");
                  send(client, buffer, strlen(buffer), 0);
               } else {
                  printf("create room: %s\n", arguments[1]);
                  
                  acquire_write_lock();
                  struct room *existing = findR(rooms, arguments[1]);
                  if(existing == NULL) {
                     rooms = insertFirstR(rooms, arguments[1]);
                     sprintf(buffer, "Room '%s' created successfully.\nchat>", arguments[1]);
                  } else {
                     sprintf(buffer, "Error: Room '%s' already exists.\nchat>", arguments[1]);
                  }
                  release_write_lock();
                  
                  send(client, buffer, strlen(buffer), 0);
               }
            }
            else if (strcmp(arguments[0], "join") == 0)
            {
               if(arguments[1] == NULL) {
                  sprintf(buffer, "Error: Room name required.\nchat>");
                  send(client, buffer, strlen(buffer), 0);
               } else if(strlen(arguments[1]) >= 50) {
                  sprintf(buffer, "Error: Room name too long (max 49 characters).\nchat>");
                  send(client, buffer, strlen(buffer), 0);
               } else {
                  printf("join room: %s\n", arguments[1]);
                  
                  acquire_write_lock();
                  struct room *room = findR(rooms, arguments[1]);
                  struct node *user = findUBySocket(head, client);
                  
                  if(room == NULL) {
                     sprintf(buffer, "Error: Room '%s' does not exist.\nchat>", arguments[1]);
                  } else if(user == NULL) {
                     sprintf(buffer, "Error: User not found.\nchat>");
                  } else {
                     addUserToRoom(room, client, user->username);
                     sprintf(buffer, "Joined room '%s'.\nchat>", arguments[1]);
                  }
                  release_write_lock();
                  
                  send(client, buffer, strlen(buffer), 0);
               }
            }
            else if (strcmp(arguments[0], "leave") == 0)
            {
               if(arguments[1] == NULL) {
                  sprintf(buffer, "Error: Room name required.\nchat>");
                  send(client, buffer, strlen(buffer), 0);
               } else if(strlen(arguments[1]) >= 50) {
                  sprintf(buffer, "Error: Room name too long (max 49 characters).\nchat>");
                  send(client, buffer, strlen(buffer), 0);
               } else {
                  printf("leave room: %s\n", arguments[1]);
                  
                  acquire_write_lock();
                  struct room *room = findR(rooms, arguments[1]);
                  
                  if(room == NULL) {
                     sprintf(buffer, "Error: Room '%s' does not exist.\nchat>", arguments[1]);
                  } else if(strcmp(room->roomname, DEFAULT_ROOM) == 0) {
                     sprintf(buffer, "Error: Cannot leave the default room '%s'.\nchat>", DEFAULT_ROOM);
                  } else {
                     removeUserFromRoom(room, client);
                     sprintf(buffer, "Left room '%s'.\nchat>", arguments[1]);
                  }
                  release_write_lock();
                  
                  send(client, buffer, strlen(buffer), 0);
               }
            } 
            else if (strcmp(arguments[0], "connect") == 0)
            {
               if(arguments[1] == NULL) {
                  sprintf(buffer, "Error: Username required.\nchat>");
                  send(client, buffer, strlen(buffer), 0);
               } else if(strlen(arguments[1]) >= 30) {
                  sprintf(buffer, "Error: Username too long (max 29 characters).\nchat>");
                  send(client, buffer, strlen(buffer), 0);
               } else {
                  printf("connect to user: %s\n", arguments[1]);
                  
                  acquire_write_lock();
                  struct node *target_user = findU(head, arguments[1]);
                  struct node *current_user = findUBySocket(head, client);
                  
                  if(target_user == NULL) {
                     sprintf(buffer, "Error: User '%s' not found.\nchat>", arguments[1]);
                  } else if(current_user == NULL) {
                     sprintf(buffer, "Error: Current user not found.\nchat>");
                  } else if(client == target_user->socket) {
                     sprintf(buffer, "Error: Cannot connect to yourself.\nchat>");
                  } else if(areConnected(connections, client, target_user->socket)) {
                     sprintf(buffer, "Already connected to '%s'.\nchat>", arguments[1]);
                  } else {
                     connections = insertFirstC(connections, client, target_user->socket);
                     sprintf(buffer, "Connected to '%s'.\nchat>", arguments[1]);
                  }
                  release_write_lock();
                  
                  send(client, buffer, strlen(buffer), 0);
               }
            }
            else if (strcmp(arguments[0], "disconnect") == 0)
            {
               if(arguments[1] == NULL) {
                  sprintf(buffer, "Error: Username required.\nchat>");
                  send(client, buffer, strlen(buffer), 0);
               } else if(strlen(arguments[1]) >= 30) {
                  sprintf(buffer, "Error: Username too long (max 29 characters).\nchat>");
                  send(client, buffer, strlen(buffer), 0);
               } else {
                  printf("disconnect from user: %s\n", arguments[1]);
                  
                  acquire_write_lock();
                  struct node *target_user = findU(head, arguments[1]);
                  
                  if(target_user == NULL) {
                     sprintf(buffer, "Error: User '%s' not found.\nchat>", arguments[1]);
                  } else if(!areConnected(connections, client, target_user->socket)) {
                     sprintf(buffer, "Error: Not connected to '%s'.\nchat>", arguments[1]);
                  } else {
                     connections = removeConnection(connections, client, target_user->socket);
                     sprintf(buffer, "Disconnected from '%s'.\nchat>", arguments[1]);
                  }
                  release_write_lock();
                  
                  send(client, buffer, strlen(buffer), 0);
               }
            }                  
            else if (strcmp(arguments[0], "rooms") == 0)
            {
               printf("List all the rooms\n");
               
               acquire_read_lock();
               strcpy(buffer, "Rooms:\n");
               struct room *current = rooms;
               int count = 0;
               while(current != NULL) {
                  char room_info[100];
                  sprintf(room_info, "  - %s\n", current->roomname);
                  strcat(buffer, room_info);
                  current = current->next;
                  count++;
               }
               if(count == 0) {
                  strcat(buffer, "  (no rooms)\n");
               }
               release_read_lock();
               
               strcat(buffer, "chat>");
               send(client, buffer, strlen(buffer), 0);
            }   
            else if (strcmp(arguments[0], "users") == 0)
            {
               printf("List all the users\n");
               
               acquire_read_lock();
               strcpy(buffer, "Users:\n");
               struct node *current = head;
               int count = 0;
               while(current != NULL) {
                  char user_info[100];
                  sprintf(user_info, "  - %s\n", current->username);
                  strcat(buffer, user_info);
                  current = current->next;
                  count++;
               }
               if(count == 0) {
                  strcat(buffer, "  (no users)\n");
               }
               release_read_lock();
               
               strcat(buffer, "chat>");
               send(client, buffer, strlen(buffer), 0);
            }                           
            else if (strcmp(arguments[0], "login") == 0)
            {
               if(arguments[1] == NULL) {
                  sprintf(buffer, "Error: Username required.\nchat>");
                  send(client, buffer, strlen(buffer), 0);
               } else if(strlen(arguments[1]) >= 30) {
                  sprintf(buffer, "Error: Username too long (max 29 characters).\nchat>");
                  send(client, buffer, strlen(buffer), 0);
               } else {
                  printf("login: %s\n", arguments[1]);
                  
                  acquire_write_lock();
                  struct node *existing_user = findU(head, arguments[1]);
                  struct node *current_user = findUBySocket(head, client);
                  
                  if(existing_user != NULL && existing_user->socket != client) {
                     sprintf(buffer, "Error: Username '%s' is already taken.\nchat>", arguments[1]);
                  } else if(current_user == NULL) {
                     sprintf(buffer, "Error: Current user not found.\nchat>");
                  } else {
                     char old_username[30];
                     strcpy(old_username, current_user->username);
                     
                     // Update username in user list
                     updateUsername(head, client, arguments[1]);
                     
                     // Update username in all rooms
                     struct room *room = rooms;
                     while(room != NULL) {
                        struct node *room_user = findUBySocket(room->users, client);
                        if(room_user != NULL) {
                           strcpy(room_user->username, arguments[1]);
                        }
                        room = room->next;
                     }
                     
                     sprintf(buffer, "Logged in as '%s'.\nchat>", arguments[1]);
                  }
                  release_write_lock();
                  
                  send(client, buffer, strlen(buffer), 0);
               }
            } 
            else if (strcmp(arguments[0], "help") == 0 )
            {
                sprintf(buffer, "login <username> - \"login with username\" \ncreate <room> - \"create a room\" \njoin <room> - \"join a room\" \nleave <room> - \"leave a room\" \nusers - \"list all users\" \nrooms -  \"list all rooms\" \nconnect <user> - \"connect to user (DM)\" \ndisconnect <user> - \"disconnect from user\" \nexit - \"exit chat\" \nlogout - \"exit chat\" \n");
                send(client , buffer , strlen(buffer) , 0 ); // send back to client 
            }
            else if (strcmp(arguments[0], "exit") == 0 || strcmp(arguments[0], "logout") == 0)
            {
               printf("User exiting\n");
               
               acquire_write_lock();
               
               // Remove user from all rooms
               struct room *room = rooms;
               while(room != NULL) {
                  removeUserFromRoom(room, client);
                  room = room->next;
               }
               
               // Remove all direct connections
               connections = removeAllConnections(connections, client);
               
               // Remove user from global list
               head = deleteU(head, client);
               
               release_write_lock();
               
               close(client);
               return NULL;
            }                         
            else { 
                 /////////////////////////////////////////////////////////////
                 // 3. sending a message
           
                 // send a message in the following format followed by the promt chat> to the appropriate receipients based on rooms, DMs
                 // ::[userfrom]> <message>
              
                 acquire_read_lock();
                 
                 struct node *sender = findUBySocket(head, client);
                 if(sender == NULL) {
                    release_read_lock();
                    continue;
                 }
                 
                 sprintf(tmpbuf,"\n::%s> %s\nchat>", sender->username, sbuffer);
                 strcpy(sbuffer, tmpbuf);
                 
                 // Create a set of recipient sockets (using a simple array)
                 int recipients[100];
                 int recipient_count = 0;
                 
                 // Find all users in the same rooms as the sender
                 struct room *room = rooms;
                 while(room != NULL) {
                    if(isUserInRoom(room, client)) {
                       struct node *room_user = room->users;
                       while(room_user != NULL) {
                          if(room_user->socket != client) {  // Don't send to yourself
                             // Check if already added
                             int already_added = 0;
                             for(int i = 0; i < recipient_count; i++) {
                                if(recipients[i] == room_user->socket) {
                                   already_added = 1;
                                   break;
                                }
                             }
                             if(!already_added) {
                                recipients[recipient_count++] = room_user->socket;
                             }
                          }
                          room_user = room_user->next;
                       }
                    }
                    room = room->next;
                 }
                 
                 // Find all users directly connected (DM) to the sender
                 struct connection *conn = connections;
                 while(conn != NULL) {
                    int target_socket = -1;
                    if(conn->socket1 == client) {
                       target_socket = conn->socket2;
                    } else if(conn->socket2 == client) {
                       target_socket = conn->socket1;
                    }
                    
                    if(target_socket != -1) {
                       // Check if already added
                       int already_added = 0;
                       for(int i = 0; i < recipient_count; i++) {
                          if(recipients[i] == target_socket) {
                             already_added = 1;
                             break;
                          }
                       }
                       if(!already_added) {
                          recipients[recipient_count++] = target_socket;
                       }
                    }
                    conn = conn->next;
                 }
                 
                 release_read_lock();
                 
                 // Send message to all recipients
                 for(int i = 0; i < recipient_count; i++) {
                    send(recipients[i], sbuffer, strlen(sbuffer), 0);
                 }
          
            }
 
         memset(buffer, 0, sizeof(1024));
      } else if (received == 0) {
         // Client disconnected unexpectedly
         printf("Client disconnected unexpectedly (socket: %d)\n", client);
         
         acquire_write_lock();
         
         // Remove user from all rooms
         struct room *room = rooms;
         while(room != NULL) {
            removeUserFromRoom(room, client);
            room = room->next;
         }
         
         // Remove all direct connections
         connections = removeAllConnections(connections, client);
         
         // Remove user from global list
         head = deleteU(head, client);
         
         release_write_lock();
         
         close(client);
         return NULL;
      } else {
         // Error reading from socket
         perror("read error");
         break;
      }
   }
   
   // Cleanup on exit
   acquire_write_lock();
   struct room *room = rooms;
   while(room != NULL) {
      removeUserFromRoom(room, client);
      room = room->next;
   }
   connections = removeAllConnections(connections, client);
   head = deleteU(head, client);
   release_write_lock();
   
   close(client);
   return NULL;
}