#include "UDPServer.h"

#pragma comment(lib, "Ws2_32.lib")

int isDataReadyMutex = 1; //to be used for thread safety.

struct sockaddr_in local;
struct sockaddr_in from;
SOCKET serverSocket;

UDPServer::UDPServer(void){
	InitWinsock();
	serverSocket = setUpServerSocket();
}


//Just a function to start Winsock up. 
void UDPServer::InitWinsock()
{
	WSADATA wsaData;
	if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		fprintf(stderr, "Could not open Windows connection.\n");
		exit(-1);
	}
}

SOCKET setUpServerSocket()
{
	SOCKET serverSocket;
	serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); //Make a socket.
	if (serverSocket == INVALID_SOCKET) //Make sure we made the socket successfully.
	{
		fprintf(stderr, "Could not create socket.\n");
		WSACleanup();
		exit(-1);
	}
	
	memset((void *)&local, '\0', sizeof(struct sockaddr_in));  //clear out memory for the global structs
	memset((void *)&from, '\0', sizeof(struct sockaddr_in));


	//set family, port, and address for server in the next few lines.
	local.sin_family = AF_INET;
	local.sin_port = htons(10800);
	//manually assign the address. Each of b1, b2, ... corresponds to a section of the IP address in standard decimal format: xxx.xxx.xxx.xxx, four sections to match the b's. 
	// So since the server is clearly the computer this code is running on, the IP address is the loopback address, "127.0.0.1".
	local.sin_addr.S_un.S_un_b.s_b1 = (unsigned char)127;
	local.sin_addr.S_un.S_un_b.s_b2 = (unsigned char)0;
	local.sin_addr.S_un.S_un_b.s_b3 = (unsigned char)0;
	local.sin_addr.S_un.S_un_b.s_b4 = (unsigned char)1;

	from.sin_family = AF_INET;
	from.sin_port = htons(10801);

	//Change the below once we move the client to another computer
	from.sin_addr.S_un.S_un_b.s_b1 = (unsigned char)127;
	from.sin_addr.S_un.S_un_b.s_b2 = (unsigned char)0;
	from.sin_addr.S_un.S_un_b.s_b3 = (unsigned char)0;
	from.sin_addr.S_un.S_un_b.s_b4 = (unsigned char)1;
	

	//At this point we should have a valid socket. Let's try binding it.
	if (bind(serverSocket, (sockaddr*)&local, sizeof(local)) == -1){ //one line check for bind attempt and success detection
		fprintf(stderr, "Could not bind name to socket. T_T\n");
		closesocket(serverSocket);
		WSACleanup();
		exit(-1);
	}
	return serverSocket;
}

int send(const char * frameData)
{
	char buffer[PACKET_SIZE];
	int fromlen = sizeof(from); //calculate size, we'll use this later.
	printf("Waiting...\n"); //print to console to show server is up and waiting for a connection
	int failCount = 0; //number of failed attempts before we give up and disconnect. 
	char buffer[PACKET_SIZE]; //buffer where we'll put messages.
	ZeroMemory(buffer, sizeof(buffer));  //zeroing the buffer
	{
		int result = sendto(serverSocket, frameData, sizeof(frameData), 0, (sockaddr*)&from, fromlen); //send the concatenated result
		if (result == SOCKET_ERROR) //If we get an error report, increase error count.
		{
			fprintf(stderr, "Error sending datagram.\n");
			failCount++;
			if (failCount > 3)
			{
				closesocket(serverSocket);
				WSACleanup();
				exit(-1);
			}
		}
	}
	closesocket(serverSocket); //This is unreachable code but may become reachable eventually, so just to be safe.
	WSACleanup();
	return 0;
}










//struct fd_set * sockets;


/*
//set up timeout value for receiving so we don't block forever. seconds and microseconds are the two parameters.
struct timeval timeout;
timeout.tv_sec = 30;
timeout.tv_usec = 0;
const timeval *timer = &timeout;

//set up FD_SET we'll need later for timeout_recvfrom
setUpFDSet(serverSocket);

*/




/*
void setUpFDSet(SOCKET serverSocket){
fd_set socketData;
FD_ZERO(&socketData);
FD_SET(serverSocket, &socketData);
sockets =  &socketData;
}

*/


/*
int timeout_recvfrom(SOCKET serverSocket, char *buffer, int len, int flags, struct sockaddr *from, int fromlen, const struct timeval *timeout){
int iResult = select(0, sockets, sockets, NULL, timeout);
if (iResult == 0) //timed out
{
printf("The socket timed out. Now closing.");
closesocket(serverSocket);
WSACleanup();
return 0;
}
else
{
return recvfrom(serverSocket, buffer, sizeof(buffer), 0, (sockaddr*)&from, &fromlen);
}
}
*/
