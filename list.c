#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "list.h"

//insert link at the first location
struct node* insertFirstU(struct node *head, int socket, char *username) {
    
   if(findUBySocket(head, socket) == NULL) {
           
       //create a link
       struct node *link = (struct node*) malloc(sizeof(struct node));

       link->socket = socket;
       strcpy(link->username,username);
       
       //point it to old first node
       link->next = head;

       //point first to new first node
       head = link;
 
   }
   else
       printf("Duplicate socket: %d\n", socket);
   return head;
}

//find a link with given username
struct node* findU(struct node *head, char* username) {

   //start from the first link
   struct node* current = head;

   //if list is empty
   if(head == NULL) {
      return NULL;
   }

   //navigate through list
    while(strcmp(current->username, username) != 0) {
	
      //if it is last node
      if(current->next == NULL) {
         return NULL;
      } else {
         //go to next link
         current = current->next;
      }
   }      
	
   //if username found, return the current Link
   return current;
}

//find a link with given socket
struct node* findUBySocket(struct node *head, int socket) {
   struct node* current = head;

   if(head == NULL) {
      return NULL;
   }

   while(current->socket != socket) {
      if(current->next == NULL) {
         return NULL;
      } else {
         current = current->next;
      }
   }      
	
   return current;
}

//delete a node with given socket
struct node* deleteU(struct node *head, int socket) {
   struct node* current = head;
   struct node* previous = NULL;

   if(head == NULL) {
      return NULL;
   }

   while(current->socket != socket) {
      if(current->next == NULL) {
         return head;  // not found
      } else {
         previous = current;
         current = current->next;
      }
   }

   if(current == head) {
      head = head->next;
   } else {
      previous->next = current->next;
   }

   free(current);
   return head;
}

//update username for a given socket
void updateUsername(struct node *head, int socket, char *new_username) {
   struct node* user = findUBySocket(head, socket);
   if(user != NULL) {
      strcpy(user->username, new_username);
   }
}

/////////////////// ROOMLIST //////////////////////////

//insert room at the first location
struct room* insertFirstR(struct room *head, char *roomname) {
   if(findR(head, roomname) == NULL) {
      struct room *link = (struct room*) malloc(sizeof(struct room));
      strcpy(link->roomname, roomname);
      link->users = NULL;
      link->next = head;
      head = link;
   }
   return head;
}

//find a room with given name
struct room* findR(struct room *head, char *roomname) {
   struct room* current = head;

   if(head == NULL) {
      return NULL;
   }

   while(strcmp(current->roomname, roomname) != 0) {
      if(current->next == NULL) {
         return NULL;
      } else {
         current = current->next;
      }
   }      
	
   return current;
}

//add user to a room
void addUserToRoom(struct room *room, int socket, char *username) {
   if(room != NULL && !isUserInRoom(room, socket)) {
      room->users = insertFirstU(room->users, socket, username);
   }
}

//remove user from a room
void removeUserFromRoom(struct room *room, int socket) {
   if(room != NULL) {
      room->users = deleteU(room->users, socket);
   }
}

//check if user is in room
int isUserInRoom(struct room *room, int socket) {
   if(room == NULL) return 0;
   return findUBySocket(room->users, socket) != NULL;
}

//delete a room
struct room* deleteR(struct room *head, char *roomname) {
   struct room* current = head;
   struct room* previous = NULL;

   if(head == NULL) {
      return NULL;
   }

   while(strcmp(current->roomname, roomname) != 0) {
      if(current->next == NULL) {
         return head;  // not found
      } else {
         previous = current;
         current = current->next;
      }
   }

   // Free all users in the room
   struct node* user = current->users;
   while(user != NULL) {
      struct node* temp = user;
      user = user->next;
      free(temp);
   }

   if(current == head) {
      head = head->next;
   } else {
      previous->next = current->next;
   }

   free(current);
   return head;
}

/////////////////// CONNECTIONLIST //////////////////////////

//insert connection at the first location
struct connection* insertFirstC(struct connection *head, int socket1, int socket2) {
   // Ensure socket1 < socket2 for consistency
   if(socket1 > socket2) {
      int temp = socket1;
      socket1 = socket2;
      socket2 = temp;
   }

   if(!areConnected(head, socket1, socket2)) {
      struct connection *link = (struct connection*) malloc(sizeof(struct connection));
      link->socket1 = socket1;
      link->socket2 = socket2;
      link->next = head;
      head = link;
   }
   return head;
}

//find if two users are connected
int areConnected(struct connection *head, int socket1, int socket2) {
   // Ensure socket1 < socket2 for consistency
   if(socket1 > socket2) {
      int temp = socket1;
      socket1 = socket2;
      socket2 = temp;
   }

   struct connection* current = head;

   if(head == NULL) {
      return 0;
   }

   while(current != NULL) {
      if(current->socket1 == socket1 && current->socket2 == socket2) {
         return 1;
      }
      current = current->next;
   }
	
   return 0;
}

//remove connection between two users
struct connection* removeConnection(struct connection *head, int socket1, int socket2) {
   // Ensure socket1 < socket2 for consistency
   if(socket1 > socket2) {
      int temp = socket1;
      socket1 = socket2;
      socket2 = temp;
   }

   struct connection* current = head;
   struct connection* previous = NULL;

   if(head == NULL) {
      return NULL;
   }

   while(current != NULL) {
      if(current->socket1 == socket1 && current->socket2 == socket2) {
         if(current == head) {
            head = head->next;
         } else {
            previous->next = current->next;
         }
         free(current);
         return head;
      }
      previous = current;
      current = current->next;
   }

   return head;
}

//remove all connections involving a socket
struct connection* removeAllConnections(struct connection *head, int socket) {
   struct connection* current = head;
   struct connection* previous = NULL;

   while(current != NULL) {
      if(current->socket1 == socket || current->socket2 == socket) {
         struct connection* temp = current;
         if(current == head) {
            head = head->next;
            current = head;
         } else {
            previous->next = current->next;
            current = current->next;
         }
         free(temp);
      } else {
         previous = current;
         current = current->next;
      }
   }

   return head;
}
