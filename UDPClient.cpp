#include <Winsock2.h>
#include <stdio.h>
#include <tchar.h>
#include <WS2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

#define BUFSIZE 4096 //test size, should work.

//Just a function to start Winsock up. 
void InitWinsock()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		fprintf(stderr, "Could not open Windows connection.\n");
		exit(-1);
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	SOCKET clientSocket;

	InitWinsock(); //start up Winsock.
	clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); //Make a socket.
	if (clientSocket == INVALID_SOCKET) //Make sure we made the socket successfully.
	{
		fprintf(stderr, "Could not create socket.\n");
		WSACleanup();
		exit(-1);
	}
	struct sockaddr_in local; //structure to store for ourselves as a client
	struct sockaddr_in to; //structure to store info for the server
	memset((void *)&local, '\0', sizeof(struct sockaddr_in));  //clear out memory for the struct
	memset((void *)&to, '\0', sizeof(struct sockaddr_in));

	//set family, port, and address for server in the next few lines.
	local.sin_family = AF_INET;
	local.sin_port = htons(10801); //port one higher than server 
	//manually assign the address. Each of b1, b2, ... corresponds to a section of the IP address in standard decimal format: xxx.xxx.xxx.xxx, four sections to match the b's. 
	// So since the client is the computer this code is running on, the IP address is the loopback address, "127.0.0.1".
	local.sin_addr.S_un.S_un_b.s_b1 = (unsigned char)127;
	local.sin_addr.S_un.S_un_b.s_b2 = (unsigned char)0;
	local.sin_addr.S_un.S_un_b.s_b3 = (unsigned char)0;
	local.sin_addr.S_un.S_un_b.s_b4 = (unsigned char)1;

	to.sin_family = AF_INET;
	to.sin_port = htons(10800);
	to.sin_addr.S_un.S_un_b.s_b1 = (unsigned char)127;
	to.sin_addr.S_un.S_un_b.s_b2 = (unsigned char)0;
	to.sin_addr.S_un.S_un_b.s_b3 = (unsigned char)0;
	to.sin_addr.S_un.S_un_b.s_b4 = (unsigned char)1;

	int tolen = sizeof(to); //calculate size, we'll use this later.

	if (bind(clientSocket, (sockaddr*)&local, sizeof(local)) == -1){ //one line check for bind attempt and success detection
		fprintf(stderr, "Could not bind name to socket. T_T\n");
		closesocket(clientSocket);
		WSACleanup();
		exit(-1);
	}

	int failcount = 0;
	while (1)
	{
		char buffer[BUFSIZE];
		ZeroMemory(buffer, sizeof(buffer));
		printf("Please input your message: ");
		scanf_s("%4096s", buffer, _countof(buffer)); //IMPORTANT:  MAKE THE NUMBER PREFIX FOR THE STRING MATCH BUFSIZE
		printf("You typed in a message: it was %s.\n", buffer);

		if (sendto(clientSocket, buffer, sizeof(buffer), 0, (sockaddr*)&to, tolen) != SOCKET_ERROR) //no send error
		{
			if (recvfrom(clientSocket, buffer, sizeof(buffer), 0, (sockaddr*)&to, &tolen) != SOCKET_ERROR)
			{
				printf("Received response from server: %s\n", buffer);
			}
			else
			{
				printf("recvfrom returned an error.\n");
				failcount++;
				if (failcount > 10){
					printf("Too many errors have occurred. Shutting down.");
					break;
				}
			}
		}
		
		else
		{
			printf("sendto returned an error.\n");
			failcount++;
			if (failcount > 10)
			{
				printf("Too many errors have occurred. Shutting down.");
				break;
			}
		}
	}

	closesocket(clientSocket);
	WSACleanup();
	return 0;
}