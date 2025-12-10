#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

struct node {
   char username[30];
   int socket;
   struct node *next;
};

// Room structure: contains room name and list of users in the room
struct room {
   char roomname[50];
   struct node *users;  // List of users in this room
   struct room *next;
};

// Direct connection structure: tracks DM connections between users
struct connection {
   int socket1;  // Socket descriptor of first user
   int socket2;  // Socket descriptor of second user
   struct connection *next;
};

/////////////////// USERLIST //////////////////////////

//insert node at the first location
struct node* insertFirstU(struct node *head, int socket, char *username);

//find a node with given username
struct node* findU(struct node *head, char* username);

//find a node with given socket
struct node* findUBySocket(struct node *head, int socket);

//delete a node with given socket
struct node* deleteU(struct node *head, int socket);

//update username for a given socket
void updateUsername(struct node *head, int socket, char *new_username);

/////////////////// ROOMLIST //////////////////////////

//insert room at the first location
struct room* insertFirstR(struct room *head, char *roomname);

//find a room with given name
struct room* findR(struct room *head, char *roomname);

//add user to a room
void addUserToRoom(struct room *room, int socket, char *username);

//remove user from a room
void removeUserFromRoom(struct room *room, int socket);

//check if user is in room
int isUserInRoom(struct room *room, int socket);

//delete a room
struct room* deleteR(struct room *head, char *roomname);

/////////////////// CONNECTIONLIST //////////////////////////

//insert connection at the first location
struct connection* insertFirstC(struct connection *head, int socket1, int socket2);

//find if two users are connected
int areConnected(struct connection *head, int socket1, int socket2);

//remove connection between two users
struct connection* removeConnection(struct connection *head, int socket1, int socket2);

//remove all connections involving a socket
struct connection* removeAllConnections(struct connection *head, int socket);
