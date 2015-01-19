#include <winsock2.h>
#include <stdio.h>
#include <tchar.h>
#include <WS2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

#define BUFSIZE 4096 //test size, should work.

//Just a function to start Winsock up. 
void InitWinsock()
{
	WSADATA wsaData;
	if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		fprintf(stderr, "Could not open Windows connection.\n");
		exit(-1);
	}
}

struct fd_set * sockets;

void setUpFDSet(SOCKET serverSocket){
	fd_set socketData;
	FD_ZERO(&socketData);
	FD_SET(serverSocket, &socketData);
	sockets =  &socketData;
}

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

int _tmain(int argc, _TCHAR* argv[])
{
	SOCKET serverSocket;

	InitWinsock(); //start up Winsock.
	serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); //Make a socket.
	if (serverSocket == INVALID_SOCKET) //Make sure we made the socket successfully.
	{
		fprintf(stderr, "Could not create socket.\n");
		WSACleanup();
		exit(-1);
	}

	struct sockaddr_in local; //structure to store our information as a server
	struct sockaddr_in from; //structure to store info from our one client
	memset((void *)&local, '\0', sizeof(struct sockaddr_in));  //clear out memory for the struct
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
	from.sin_addr.S_un.S_un_b.s_b1 = (unsigned char)127;
	from.sin_addr.S_un.S_un_b.s_b2 = (unsigned char)0;
	from.sin_addr.S_un.S_un_b.s_b3 = (unsigned char)0;
	from.sin_addr.S_un.S_un_b.s_b4 = (unsigned char)1;
	int fromlen = sizeof(from); //calculate size, we'll use this later.

	//At this point we should have a valid socket. Let's try binding it.
	if (bind(serverSocket, (sockaddr*)&local, sizeof(local)) == -1){ //one line check for bind attempt and success detection
		fprintf(stderr, "Could not bind name to socket. T_T\n");
		closesocket(serverSocket);
		WSACleanup();
		exit(-1);
	}

	char responsePrefix[BUFSIZE] = "Got your message. It was: "; //Making this a loop-invariant constant because we'll add onto it in a loop.
	
	//set up timeout value for receiving so we don't block forever. seconds and microseconds are the two parameters.
	struct timeval timeout;
	timeout.tv_sec = 30;
	timeout.tv_usec = 0;
	const timeval *timer = &timeout;

	//set up FD_SET we'll need later for timeout_recvfrom
	setUpFDSet(serverSocket);
	
	//Main loop
	while (1)
	{
		int failCount = 0; //number of failed attempts before we give up and disconnect. 
		char buffer[BUFSIZE]; //buffer where we'll put messages.
		ZeroMemory(buffer, sizeof(buffer));  //zeroing the buffer
		char address[INET_ADDRSTRLEN]; //buffer to hold address we will receive from
		ZeroMemory(address, sizeof(address));
		printf("Waiting...\n"); //print to console to show server is up and waiting for a connection
		if (recvfrom(serverSocket, buffer, sizeof(buffer), 0, (sockaddr*)&from, &fromlen) != SOCKET_ERROR) //if recvfrom succeeds,
		{
			printf("Received message from %s: %s\n", inet_ntop(AF_INET, &from.sin_addr, address, sizeof(address)), buffer);
			char response[BUFSIZE] = "";
			strcat_s(response, sizeof(buffer), responsePrefix); //copy the prefix into response
			strcat_s(response, sizeof(buffer), buffer); //concatenate what we received onto the end of the prefix
			int result = sendto(serverSocket, response, sizeof(response), 0, (sockaddr*)&from, fromlen); //send the concatenated result
			if (result != sizeof(response)) //If we get a report that we sent less bytes than the message, increase error count.
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

		else{ //if recvfrom failed with some sort of error, increment failcount and potentially leave
			fprintf(stderr, "Could not receive datagram.\n");
			failCount++;
			if (failCount > 3){
				fprintf(stderr, "Could not receive datagram 3 times. Shutting down.\n");
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