#include <winsock2.h>
#include <stdio.h>
#include <tchar.h>
#include <WS2tcpip.h>
#include <string>

#pragma comment(lib, "Ws2_32.lib")

#define BUFSIZE 65507 //discovered by using getsockopt; this is the max packet size we can send.
#define FRAME_SIZE 434176 //(512*424*2 is how that was calculated, size of a Kinect 2.0 depth image is 512x424, *2 for byte.)
#define NEEDED_PACKETS 7 //Math.Ceiling(FRAME_SIZE / BUFSIZE)

//Code to discover optimal buffer size to use for BUFSIZE
/*
int optlen = sizeof(int);
int optval;
getsockopt(serverSocket, SOL_SOCKET, SO_MAX_MSG_SIZE, (char *)&optval, &optlen);
printf("Max size of buffer to send is %d.", optval);
*/

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
/*
struct fd_set * sockets;

void setUpFDSet(SOCKET serverSocket){
fd_set socketData;
FD_ZERO(&socketData);
FD_SET(serverSocket, &socketData);
sockets = &socketData;
}

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

void main(int argc, _TCHAR* argv[])
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
	struct sockaddr_in to; //structure to store info from our one client
	memset((void *)&local, '\0', sizeof(struct sockaddr_in));  //clear out memory for the struct
	memset((void *)&to, '\0', sizeof(struct sockaddr_in));


	//set family, port, and address for server in the next few lines.
	local.sin_family = AF_INET;
	local.sin_port = htons(10800);
	//manually assign the address. Each of b1, b2, ... corresponds to a section of the IP address in standard decimal format: xxx.xxx.xxx.xxx, four sections to match the b's. 
	// So since the server is clearly the computer this code is running on, the IP address is the loopback address, "127.0.0.1".
	local.sin_addr.S_un.S_un_b.s_b1 = (unsigned char)127;
	local.sin_addr.S_un.S_un_b.s_b2 = (unsigned char)0;
	local.sin_addr.S_un.S_un_b.s_b3 = (unsigned char)0;
	local.sin_addr.S_un.S_un_b.s_b4 = (unsigned char)1;

	to.sin_family = AF_INET;
	to.sin_port = htons(10801);
	to.sin_addr.S_un.S_un_b.s_b1 = (unsigned char)127;
	to.sin_addr.S_un.S_un_b.s_b2 = (unsigned char)0;
	to.sin_addr.S_un.S_un_b.s_b3 = (unsigned char)0;
	to.sin_addr.S_un.S_un_b.s_b4 = (unsigned char)1;
	int tolen = sizeof(to); //calculate size, we'll use this later.

	//At this point we should have a valid socket. Let's try binding it.
	if (bind(serverSocket, (sockaddr*)&local, sizeof(local)) == -1){ //one line check for bind attempt and success detection
		fprintf(stderr, "Could not bind name to socket. T_T\n");
		closesocket(serverSocket);
		WSACleanup();
		exit(-1);
	}

	int failCount = 0;

	char packet[BUFSIZE]; //buffer where we'll put messages.
	ZeroMemory(packet, sizeof(packet));
	char image[FRAME_SIZE];
	ZeroMemory(image, sizeof(image));
	char * imagePointer = &image[0];

	//for loop to create image, set of 7 numbers in sequence; packet 1 is 11111111..., packet 2 is 222222..., etc up to NEEDED_PACKETS.
	for (int i = 0; i < NEEDED_PACKETS; i++){
		char number = '0' + i;
		memset(imagePointer, number, BUFSIZE);
		imagePointer += BUFSIZE;
	}
	imagePointer = &image[0]; //reset image pointer
	printf("Waiting...\n"); //print to console to show server is up and waiting for a connection, just a test before beginning
	if (recvfrom(serverSocket, packet, sizeof(packet), 0, (sockaddr*)&to, &tolen) != SOCKET_ERROR) //if recvfrom succeeds,
	{
		printf("Received message from client: %s\n", packet);
		int sent; //how many bytes we just send with our latest attempt at transmission
		int sentTotal = 0; //how many bytes we have sent in total through transmission
		int dataLeft = sizeof(image); //how much data is left before the current image has been completely transmitted
		int failCount = 0; //number of failed attempts before we give up and disconnect. 
		BYTE sequenceNum = 0; //for bookkeeping, the frame number in our infinite sequence, ranges from 0-255 then loops.
		BYTE fragmentNum = 0; //for bookkeeping, what fragment of the current frame is this packet?
		char * packetPointer = &packet[0];
		/* This will be the main loop for our transmission function. What we want to do is as follows.
		 * We have an image buffer that contains the entire image. This is too big to send in one packet.
		 * So we have a packet buffer into which we can copy parts of the image. We also need to modify the packet
		 * with a very small header to contain the sequence number and fragment number of our data. 
		 * This protocol is as follows:
		 *
		 * For each iteration, we zero the packet buffer to clear out any old information that may be there.
		 * Next, we use a pointer into the image buffer in combination with memcpy to copy a packet's worth of data
		 * from the image buffer to the packet buffer.
		 * We then update the image buffer pointer to point a packet size farther into the image.
		 * After that, send the packet buffer's contents over the network as one packet.
		 * (If transmission as a unit fails, then keep trying to send, updating a pointer into the given packet to show how
		 *  much of it was successfully transmitted. Don't worry about this yet though.)
		 *
		 * The client will be doing recv from and copying the contents of the packet into their version of the image buffer.
		 * We just keep streaming whatever is passed in.
		 */
		while (1)
		{
			ZeroMemory(packet, sizeof(packet));  //zero the packet buffer.
			//memset(packetPointer, sequenceNum, sizeof(sequenceNum));
			//packetPointer += sizeof(sequenceNum);
			//memset(packetPointer, fragmentNum, sizeof(fragmentNum));
			memcpy(packet, imagePointer, sizeof(packet)); //copy a packet's worth of data starting from pointer location
			sent = sendto(serverSocket, packet, sizeof(packet), 0, (sockaddr*)&to, tolen); //try to send the packet
			if (sent == SOCKET_ERROR){
				failCount += 1;
				if (failCount > 10){
					printf("Failed 10 times. Shutting down.");
					closesocket(serverSocket);
					WSACleanup();
					exit(-1);
				}
				continue; //don't do any further adjustments, just zero the packet and try again.
			}
			imagePointer += sizeof(packet); //increment image pointer, since we sent successfully
			dataLeft -= sent;
			sentTotal += sent;
			while (sentTotal < FRAME_SIZE) //until we finish sending the image, repeat the above.
			{
				ZeroMemory(packet, sizeof(packet));
				memcpy(packet, imagePointer, min(sizeof(packet), dataLeft));
				sent = sendto(serverSocket, packet, sizeof(packet), 0, (sockaddr*)&to, tolen); //try to send the rest of the image
				if (sent == SOCKET_ERROR)
				{
					failCount += 1;
					if (failCount > 10)
					{
						printf("Failed 10 times. Shutting down.");
						closesocket(serverSocket);
						WSACleanup();
						exit(-1);
					}
					continue;
				}
				imagePointer += min(sizeof(packet), dataLeft); //increment image pointer.
				dataLeft -= sent;
				sentTotal += sent;
				Sleep(1);
			}
			printf("Full image has been sent. Check the client.\n");
			imagePointer = &image[0]; //we've sent the whole image at this point, so reset the image pointer
			failCount = 0; //reset fail counter too
			sentTotal = 0; //reset total bytes sent counter
			dataLeft = sizeof(image); //reset how much data is left to send.
			//start over at start of loop.
		}
	}
	else //if recvfrom failed with some sort of error, increment failcount and potentially leave
	{
		fprintf(stderr, "Could not receive start message.\n");
		failCount++;
		if (failCount > 3)
		{
			fprintf(stderr, "Could not receive start message 3 times. Shutting down.\n");
			closesocket(serverSocket);
			WSACleanup();
			exit(-1);
		}

	}
	closesocket(serverSocket);
	WSACleanup();
	exit(0);
}