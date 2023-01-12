#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
using namespace std;
#pragma comment(lib, "Ws2_32.lib")
#include <winsock2.h>
#include <string.h>
#include <time.h>
#include <vector>

#define BUFFER_SIZE  4096
#define MAX_SOCKETS  60
#define SOCKER_TIMEOUT 120

typedef struct Recv_headers
{
	string type;
	string URL;
	string Content_Type;
	string accept;
	string host;
	string connection;
	string language;
	string file_name;
	string content_len;
	string body;
	string raw_msg;
};

struct SocketState
{
	SOCKET id;			// Socket handle
	int	recv;			// Receiving?
	int	send;			// Sending?
	int sendSubType;	// Sending sub-type
	char buffer[BUFFER_SIZE];
	int len;
	Recv_headers headers;
	clock_t time_of_last_byte;
};

#include "requests_handler.h"
extern const int SERVER_PORT;
extern const string SERVER_PORT_STR;
extern const int EMPTY;
extern const int LISTEN;
extern const int RECEIVE;
extern const int IDLE;
extern const int SEND;
//TODO: delete this 2 lines if not used at the end. 
extern const int SEND_TIME;
extern const int SEND_SECONDS;
extern struct SocketState sockets[MAX_SOCKETS];
extern int socketsCount;;


bool addSocket(SOCKET id, int what);
void removeSocket(int index);
void acceptConnection(int index);
void receiveMessage(int index);
void sendMessage(int index);
void socketTimeoutHandler();