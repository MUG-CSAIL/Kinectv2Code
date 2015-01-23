#include <Winsock2.h>
#include <stdio.h>
#include <tchar.h>
#include <WS2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

#define BUFSIZE 65507 //discovered by using getsockopt; this is the max packet size we can send.
#define FRAME_SIZE 434176 //(512*424*2 is how that was calculated, size of a Kinect 2.0 depth image is 512x424, *2 for byte.)
#define NEEDED_PACKETS 7 //Math.Ceiling(FRAME_SIZE / BUFSIZE)

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

void main(int argc, _TCHAR* argv[])
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
	char image[FRAME_SIZE];
	ZeroMemory(image, sizeof(image));
	char packet[BUFSIZE];
	char * imagePointer = &image[0];
	int receivedBytes;
	int receivedByteTotal = 0;
	int dataLeft = FRAME_SIZE;

	ZeroMemory(packet, sizeof(packet));
	printf("Please input any message to begin transmission: ");
	scanf_s("%4096s", packet, _countof(packet)); //IMPORTANT:  MAKE THE NUMBER PREFIX FOR THE STRING <= BUFSIZE
	if (sendto(clientSocket, packet, sizeof(packet), 0, (sockaddr*)&to, tolen) != SOCKET_ERROR) //no send error
	{
		while (1)
		{
			while (receivedByteTotal < FRAME_SIZE)
			{
				ZeroMemory(packet, sizeof(packet));
				int receivedBytes = recvfrom(clientSocket, packet, sizeof(packet), 0, (sockaddr*)&to, &tolen);
				if (receivedBytes == SOCKET_ERROR)
				{
					printf("recvfrom returned an error.\n");
					failcount++;
					if (failcount > 10){
						printf("Too many errors have occurred. Shutting down.");
						break;
					}
					continue;
				}
				receivedByteTotal += receivedBytes; //received correctly.
				printf("Received transmission.\n", receivedBytes);
				dataLeft -= receivedBytes;
				memcpy(imagePointer, packet, min(sizeof(packet), dataLeft));
				imagePointer += min(dataLeft, sizeof(packet));
			}
			imagePointer = &image[0];
			printf("Received Byte Total is equal to or exceeds frame size!");
			printf("Received image.\n First part of image starts with sequence number %u and fragment number %u\n", image[0], image[1]);
			printf("Second part of image starts with sequence number %u and fragment number %u\n", image[BUFSIZE], image[BUFSIZE+1]);
			printf("Third part of image starts with sequence number %u and fragment number %u\n", image[BUFSIZE * 2], image[BUFSIZE * 2 +1]);
			printf("Fourth part of image starts with sequence number %u and fragment number %u\n", image[BUFSIZE * 3], image[BUFSIZE * 3 +1]);
			printf("Fifth part of image starts with sequence number %u and fragment number %u\n", image[BUFSIZE * 4], image[BUFSIZE * 4 +1]);
			printf("Sixth part of image starts with sequence number %u and fragment number %u\n", image[BUFSIZE * 5], image[BUFSIZE * 5 +1]);
			printf("Last part of image starts with sequence number %u and fragment number %u\n", packet[0], packet[1]);
			receivedByteTotal = 0;
			ZeroMemory(image, sizeof(image));
			dataLeft = sizeof(image);
		}
	}
	else
	{
		printf("sendto returned an error.\n");
		failcount++;
		if (failcount > 10)
		{
			printf("Too many errors have occurred. Shutting down.");
			closesocket(clientSocket);
			WSACleanup();
			exit(-1);
		}
	}

	closesocket(clientSocket);
	WSACleanup();
	exit(0);
}