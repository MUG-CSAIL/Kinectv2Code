#include <winsock2.h>
#include <stdio.h>
#include <tchar.h>
#include <WS2tcpip.h>
#include <string>

#pragma comment(lib, "Ws2_32.lib")

#define PACKET_SIZE 65507 //discovered by using getsockopt; this is the max packet size we can send. I've tried 65508, critical failure. 
#define PACKET_DATA_SIZE PACKET_SIZE-2 //this is how much data will be in each packet, ignoring header bytes. 
#define FRAME_SIZE 65505 //434176 //(512*424*2 is how that was calculated, size of a Kinect 2.0 depth image is 512x424, *2 for byte.)
#define NEEDED_PACKETS 7 //Math.Ceiling(FRAME_SIZE / PACKET_DATA_SIZE)

//Code to discover optimal buffer size to use for PACKET_SIZE
/*
int optlen = sizeof(int);
int optval;
getsockopt(serverSocket, SOL_SOCKET, SO_MAX_MSG_SIZE, (char *)&optval, &optlen);
printf("Max size of buffer to send is %d.", optval);
*/

struct sockaddr_in local; //structure to store our information as a server
struct sockaddr_in to; //structure to store info from our one client

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

int setUpSockAddrs(SOCKET serverSocket){
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
	

	//At this point we should have a valid socket. Let's try binding it.
	if (bind(serverSocket, (sockaddr*)&local, sizeof(local)) == -1){ //one line check for bind attempt and success detection
		fprintf(stderr, "Could not bind name to socket. T_T\n");
		closesocket(serverSocket);
		WSACleanup();
		exit(-1);
	}
	return 0;
}

int createFakeImage(char * imagePointer){
	
	BYTE startPixel = '1';
	BYTE middlePixel = '2';
	BYTE lastPixel = '3';
	memset(imagePointer, startPixel, sizeof(startPixel));
	imagePointer += sizeof(startPixel);

	memset(imagePointer, middlePixel, FRAME_SIZE-2);
	imagePointer += (FRAME_SIZE - 2);
	
	memset(imagePointer, lastPixel, sizeof(lastPixel));
	return 0;
}

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

	setUpSockAddrs(serverSocket); //set up all the address information needed to do the transmissions
	
	int failCount = 0;
	int tolen = sizeof(to); //calculate size, we'll use this later.
	char packet[PACKET_SIZE]; //buffer where we'll put messages.
	ZeroMemory(packet, sizeof(packet));
	char image[FRAME_SIZE];
	ZeroMemory(image, sizeof(image));
	char * imagePointer = &image[0];

	BYTE startPixel = '1';
	BYTE middlePixel = '2';
	BYTE lastPixel = '3';

	for (int i = 1; i < FRAME_SIZE - 1; i++){
		image[i] = middlePixel;
	}
	image[0] = startPixel;
	image[FRAME_SIZE - 1] = lastPixel;

	//createFakeImage(imagePointer); //simple method to make a tailored image so I know if it works. DOES NOT RESET IMAGE POINTER WHEN DONE.
	imagePointer = &image[0]; //reset image pointer, since it wasn't done in the method. much easier to do it safely here.
	
	//printf("Waiting...\n"); //print to console to show server is up and waiting for a connection, just a test before beginning
	//if (recvfrom(serverSocket, packet, sizeof(packet), 0, (sockaddr*)&to, &tolen) != SOCKET_ERROR) //if recvfrom succeeds,
	if (true) // no message exchange, just start throwing
	{
		//printf("Received message from client: %s\n", packet);
		int sent; //how many bytes we just send with our latest attempt at transmission
		int sentTotal = 0; //how many bytes we have sent in total through transmission
		int dataLeft = sizeof(image); //how much data is left before the current image has been completely transmitted
		int failCount = 0; //number of failed attempts before we give up and disconnect. 
		BYTE sequenceNum = 0; //for bookkeeping, the frame number in our infinite sequence, ranges from 0-255 then loops.
		BYTE fragmentNum = 0; //for bookkeeping, what fragment of the current frame is this packet? from 0-6 for 7 total frags
		char * packetPointer = &packet[0];
		int remainingSize = sizeof(packet) - sizeof(sequenceNum) - sizeof(fragmentNum); //sizeof packet - sizeof sequenceNum - sizeof fragmentNum.
		//Since sizeof(char) is defined to be 1, this is actually sizeof(packet) - 2.

		/* This will be the main loop for our transmission function. What we want to do is as follows.
		 * We have an image buffer that contains the entire image. This is too big to send in one packet.
		 * So we have a packet buffer into which we can copy parts of the image. We also need to modify the packet
		 * with a very small header to contain the sequence number and fragment number of our data.
		 * We want to make sure that the packet's received correctly, so we also want a prefix and suffix that are easy to check. 
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
			memset(packetPointer, sequenceNum, sizeof(sequenceNum)); //set sequence number as first byte
			packetPointer += sizeof(sequenceNum); //move past the newly set sequence number 
			memset(packetPointer, fragmentNum, sizeof(fragmentNum)); //set the fragment number as second byte
			packetPointer += sizeof(fragmentNum); //move past the fragment number
			memcpy(packetPointer, imagePointer, remainingSize); //copy the packet's remaining capacity of data starting from pointer location
			sent = sendto(serverSocket, packet, sizeof(packet), 0, (sockaddr*)&to, tolen); //try to send the packet
			if (sent == SOCKET_ERROR){
				failCount += 1;
				if (failCount > 10){
					printf("Failed 10 times. Shutting down.");
					closesocket(serverSocket);
					WSACleanup();
					exit(-1);
				}
				packetPointer = &packet[0]; //reset packet pointer
				continue; //don't do any further adjustments, just zero the packet and try again.
			}
			imagePointer += (sizeof(packet)-2); //increment image pointer, since we sent successfully. -2 because of header.
			dataLeft -= (sent-2); //we have two extra bytes left to transmit because of the space taken by the header
			sentTotal += (sent-2);
			fragmentNum += 1;
			//packet pointer now points to beyond fragment number, which needs to be fixed. Will do so in the loop below.
			while (sentTotal < FRAME_SIZE) //until we finish sending the image, repeat the above with one slight change
			{
				ZeroMemory(packet, sizeof(packet)); //clear out packet
				packetPointer = &packet[0]; //reset packet pointer
				memset(packetPointer, sequenceNum, sizeof(sequenceNum)); //set sequence num again
				packetPointer += sizeof(sequenceNum); //increment pointer
				memset(packetPointer, fragmentNum, sizeof(fragmentNum)); //set frag number
				packetPointer += sizeof(fragmentNum); //increment pointer
				memcpy(packetPointer, imagePointer, min(remainingSize, dataLeft)); //put image data in rest of packet
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
				imagePointer += min(remainingSize, dataLeft); //increment image pointer.
				fragmentNum += 1; //increase fragment number
				dataLeft -= (sent-2); //again, 2 less than was actually sent because of header.
				sentTotal += (sent-2); //increase how much we've transmitted
				Sleep(100); //# of clock cycle delays so we don't overload receiving buffer. Try to take this out when client is multithreaded. 
			}
			printf("Full image has been sent. Resetting data.\n");
			break;
			imagePointer = &image[0]; //we've sent the whole image at this point, so reset the image pointer
			failCount = 0; //reset fail counter too
			sentTotal = 0; //reset total bytes sent counter
			sequenceNum = (sequenceNum + 1) % 256; //increment sequence number
			fragmentNum = 0; //reset fragment number 
			dataLeft = sizeof(image); //reset how much data is left to send.
			packetPointer = &packet[0]; //lastly, reset packet pointer
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