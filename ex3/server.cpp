#include "server.h"

const int SERVER_PORT = 27015;
const string SERVER_PORT_STR = "27015";
const int EMPTY = 0;
const int LISTEN = 1;
const int RECEIVE = 2;
const int IDLE = 3;
const int SEND = 4;
//TODO: delete this 2 lines if not used at the end. 
const int SEND_TIME = 1;
const int SEND_SECONDS = 2;
struct SocketState sockets[MAX_SOCKETS] = { 0 };
int socketsCount = 0;


void main()
{
	// Initialize Winsock (Windows Sockets).

	// Create a WSADATA object called wsaData.
	// The WSADATA structure contains information about the Windows 
	// Sockets implementation.
	WSAData wsaData;

	// Call WSAStartup and return its value as an integer and check for errors.
	// The WSAStartup function initiates the use of WS2_32.DLL by a process.
	// First parameter is the version number 2.2.
	// The WSACleanup function destructs the use of WS2_32.DLL by a process.
	if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		cout << "Time Server: Error at WSAStartup()\n";
		return;
	}

	// Server side:
	// Create and bind a socket to an internet address.
	// Listen through the socket for incoming connections.

	// After initialization, a SOCKET object is ready to be instantiated.

	// Create a SOCKET object called listenSocket. 
	// For this application:	use the Internet address family (AF_INET), 
	//							streaming sockets (SOCK_STREAM), 
	//							and the TCP/IP protocol (IPPROTO_TCP).

	//TODO: shift the socket to non-blocking!
	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// Check for errors to ensure that the socket is a valid socket.
	// Error detection is a key part of successful networking code. 
	// If the socket call fails, it returns INVALID_SOCKET. 
	// The if statement in the previous code is used to catch any errors that
	// may have occurred while creating the socket. WSAGetLastError returns an 
	// error number associated with the last error that occurred.
	if (INVALID_SOCKET == listenSocket)
	{
		cout << "Time Server: Error at socket(): " << WSAGetLastError() << endl;
		WSACleanup();
		return;
	}

	// For a server to communicate on a network, it must bind the socket to 
	// a network address.

	// Need to assemble the required data for connection in sockaddr structure.

	// Create a sockaddr_in object called serverService. 
	sockaddr_in serverService;
	// Address family (must be AF_INET - Internet address family).
	serverService.sin_family = AF_INET;
	// IP address. The sin_addr is a union (s_addr is a unsigned long 
	// (4 bytes) data type).
	// inet_addr (Iternet address) is used to convert a string (char *) 
	// into unsigned long.
	// The IP address is INADDR_ANY to accept connections on all interfaces.
	serverService.sin_addr.s_addr = INADDR_ANY;
	// IP Port. The htons (host to network - short) function converts an
	// unsigned short from host to TCP/IP network byte order 
	// (which is big-endian).
	serverService.sin_port = htons(SERVER_PORT);

	// Bind the socket for client's requests.

	// The bind function establishes a connection to a specified socket.
	// The function uses the socket handler, the sockaddr structure (which
	// defines properties of the desired connection) and the length of the
	// sockaddr structure (in bytes).
	if (SOCKET_ERROR == bind(listenSocket, (SOCKADDR*)&serverService, sizeof(serverService)))
	{
		cout << "Time Server: Error at bind(): " << WSAGetLastError() << endl;
		closesocket(listenSocket);
		WSACleanup();
		return;
	}

	// Listen on the Socket for incoming connections.
	// This socket accepts only one connection (no pending connections 
	// from other clients). This sets the backlog parameter.
	if (SOCKET_ERROR == listen(listenSocket, 5))
	{
		cout << "Time Server: Error at listen(): " << WSAGetLastError() << endl;
		closesocket(listenSocket);
		WSACleanup();
		return;
	}
	addSocket(listenSocket, LISTEN);

	// Accept connections and handles them one by one.
	while (true)
	{
		// The select function determines the status of one or more sockets,
		// waiting if necessary, to perform asynchronous I/O. Use fd_sets for
		// sets of handles for reading, writing and exceptions. select gets "timeout" for waiting
		// and still performing other operations (Use NULL for blocking). Finally,
		// select returns the number of descriptors which are ready for use (use FD_ISSET
		// macro to check which descriptor in each set is ready to be used).
		fd_set waitRecv;
		FD_ZERO(&waitRecv);
		for (int i = 0; i < MAX_SOCKETS; i++)
		{
			if ((sockets[i].recv == LISTEN) || (sockets[i].recv == RECEIVE))
				FD_SET(sockets[i].id, &waitRecv);
		}

		fd_set waitSend;
		FD_ZERO(&waitSend);
		for (int i = 0; i < MAX_SOCKETS; i++)
		{
			if (sockets[i].send == SEND)
				FD_SET(sockets[i].id, &waitSend);
		}

		//
		// Wait for interesting event.
		// Note: First argument is ignored. The fourth is for exceptions.
		// And as written above the last is a timeout, hence we are blocked if nothing happens.
		int nfd;

		nfd = select(0, &waitRecv, &waitSend, NULL, NULL);

		if (nfd == SOCKET_ERROR)
		{
			cout << "Time Server: Error at select(): " << WSAGetLastError() << endl;
			WSACleanup();
			return;
		}

		for (int i = 0; i < MAX_SOCKETS && nfd > 0; i++)
		{
			if (FD_ISSET(sockets[i].id, &waitRecv))
			{
				nfd--;
				switch (sockets[i].recv)
				{
				case LISTEN:
					acceptConnection(i);
					break;

				case RECEIVE:
					receiveMessage(i);
					break;
				}
			}
		}

		for (int i = 0; i < MAX_SOCKETS && nfd > 0; i++)
		{
			if (FD_ISSET(sockets[i].id, &waitSend))
			{
				nfd--;
				sendMessage(i);
			}
		}
	}

	// Closing connections and Winsock.
	cout << "Time Server: Closing Connection.\n";
	closesocket(listenSocket);
	WSACleanup();
}

bool addSocket(SOCKET id, int what)
{
	for (int i = 0; i < MAX_SOCKETS; i++)
	{
		if (sockets[i].recv == EMPTY)
		{
			sockets[i].id = id;
			sockets[i].recv = what;
			sockets[i].send = IDLE;
			if (sockets[i].buffer != NULL)
				strcpy(sockets[i].buffer, "\0");
			sockets[i].len = 0;
			socketsCount++;
			return (true);
		}
	}
	return (false);
}

void removeSocket(int index)
{
	sockets[index].recv = EMPTY;
	sockets[index].send = EMPTY;
	socketsCount--;
}

void acceptConnection(int index)
{
	SOCKET id = sockets[index].id;
	struct sockaddr_in from;		// Address of sending partner
	int fromLen = sizeof(from);

	SOCKET msgSocket = accept(id, (struct sockaddr*)&from, &fromLen);
	if (INVALID_SOCKET == msgSocket)
	{
		cout << "Time Server: Error at accept(): " << WSAGetLastError() << endl;
		return;
	}
	cout << "Server: Client " << inet_ntoa(from.sin_addr) << ":" << ntohs(from.sin_port) << " is connected." << endl;

	// Set the socket to be in non-blocking mode.
	unsigned long flag = 1;
	if (ioctlsocket(msgSocket, FIONBIO, &flag) != 0)
	{
		cout << "Time Server: Error at ioctlsocket(): " << WSAGetLastError() << endl;
	}

	if (addSocket(msgSocket, RECEIVE) == false)
	{
		cout << "\t\tToo many connections, dropped!\n";
		closesocket(id);
	}
	return;
}

int recvResHanler(int bytesRecv, int iter, bool& socket_not_empty, SOCKET& msgSocket, int ind)
{
	// if res = 1 -> return.
	int res = 0;

	if (!bytesRecv && !iter) // Socket was empty from beginning, should close socket or error
	{
		if (SOCKET_ERROR == bytesRecv) // error
		{
			cout << "Time Server: Error at recv(): " << WSAGetLastError() << endl;
		}
		closesocket(msgSocket);
		removeSocket(ind);
		res = 1;
	}
	if (bytesRecv == -1) // Finished reading from socket
	{
		socket_not_empty = false;
	}

	return res;
}


int recvExpandedHelper(int ind)
{
	SocketState* socketToReadFrom = sockets + ind;
	SOCKET msgSocket = socketToReadFrom->id;

	int bytesRecv = 0, bytes_sum = 0, iter = 0;;
	unsigned int cur_buf_size = BUFFER_SIZE;

	char* buffer = socketToReadFrom->buffer;
	char* temp = nullptr;

	bool socket_not_empty = true;

	while (socket_not_empty)
	{
		bytesRecv = recv(msgSocket, buffer + bytes_sum, cur_buf_size - bytes_sum, 0);

		// if res = 1 -> return ( = error or empty socket)
		if (recvResHanler(bytesRecv, iter, socket_not_empty, msgSocket, ind)) return 0;

		else
		{
			bytes_sum += bytesRecv;

			// reallocate buffer 
			if (cur_buf_size <= bytes_sum)
			{
				cur_buf_size *= 2;
				temp = new char[cur_buf_size];
				memcpy(temp, buffer, bytes_sum);
				buffer = temp;
			}
		}
		++iter;
	}
	delete(temp);
	temp = new char[bytes_sum + 1];
	memcpy(temp, buffer, bytes_sum);
	temp[bytes_sum] = '\0';
	buffer = temp;
	socketToReadFrom->len = bytes_sum;
	return 1;
}



void getHeadersHelper(Recv_headers& headers, char* msg_received)
{
	string buffer(msg_received);

	size_t accept = buffer.find("Accept:") + 7; //saves location after the 'accept:'
	headers.accept = strtok(&msg_received[accept], " \r\n");

	size_t host = buffer.find("Host:") + 5;
	headers.host = strtok(&msg_received[host], " \r\n");

	size_t connection = buffer.find("Connection:") + 11;
	headers.connection = strtok(&msg_received[connection], " \r\n");

	size_t content_Type = buffer.find("Content-Type:");
	if (content_Type != string::npos) { headers.Content_Type = strtok(&msg_received[content_Type + 13], " \r\n"); }
	else { headers.Content_Type = "text"; }

	//TODO:test res for content_len with real req.
	size_t content_len = buffer.find("Content-Length:") + 15;
	headers.content_len = strtok(&msg_received[content_len], " \r\n");

	size_t body = buffer.find(string("\r\n\r\n")) + 4;
	headers.body.assign(&msg_received[body]);

	size_t language = buffer.find("?lang=");
	if (language != string::npos) { headers.language = strtok(&msg_received[language + 6], " \r\n"); }
	else { headers.language.clear(); }

	size_t file_id = buffer.find(SERVER_PORT_STR + "/ ");
	if (file_id != string::npos) { headers.file_name.clear(); }
	else {
		size_t file_id = buffer.find(SERVER_PORT_STR + "/");
		headers.file_name = strtok(&msg_received[file_id + SERVER_PORT_STR.length() + 1], " ?\r\n");
	}
}

Recv_headers getHeaders(char* msg_received)
{
	Recv_headers headers;

	char msg_copy[BUFFER_SIZE];
	strcpy(msg_copy, msg_received);

	char* token = strtok(msg_copy, "- ");
	headers.type = token;
	token = strtok(NULL, "- ");
	//TODO:test res for URL with real req.
	headers.URL = token;
	headers.raw_msg = msg_received;

	printf("\nMessage Received: \n%s \n-----------\n", string(msg_received).data());

	getHeadersHelper(headers, msg_received);

	return headers;
}

void receiveMessage(int ind)
{
	SOCKET msgSocket = sockets[ind].id;

	// get data to buffer
	if (recvExpandedHelper(ind)) {
		sockets[ind].headers = getHeaders(sockets[ind].buffer);
		sockets[ind].send = SEND;
	}
	return;
}




void sendMessage(int index)
{
	int bytesSent = 0;
	char sendBuff[255];
	string response;

	SOCKET msgSocket = sockets[index].id;

	// Answer client's request according to request method.
	response = requestHandler(sockets[index].headers);
	copy(response.begin(), response.end(), sendBuff);
	

	//send message
	bytesSent = send(msgSocket, sendBuff, (int)strlen(sendBuff), 0);
	if (SOCKET_ERROR == bytesSent)
	{
		cout << "Server: Error at send(): " << WSAGetLastError() << endl;
		return;
	}

	cout << "Server: Sent: " << bytesSent << "\\" << strlen(sendBuff) << " bytes of \"" << sendBuff << "\" message.\n";

	sockets[index].send = IDLE;
}
