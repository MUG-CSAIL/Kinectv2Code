#ifndef UDPSERVER_H
#define UDPSERVER_H
#include <winsock2.h>
#include <stdio.h>
#include <tchar.h>
#include <WS2tcpip.h>
#include <string>

#pragma comment(lib, "Ws2_32.lib")

#define PACKET_SIZE 65507 //discovered by using getsockopt; this is the max packet size we can send. I've tried 65508, critical failure. 
#define PACKET_DATA_SIZE PACKET_SIZE-2 //this is how much data will be in each packet, ignoring header bytes. 
#define FRAME_SIZE 434176 //(512*424*2 is how that was calculated, size of a Kinect 2.0 depth image is 512x424, *2 for byte.)
#define PBUFFER_SIZE 217088 //512*424
#define NEEDED_PACKETS 7 //Math.Ceiling(FRAME_SIZE / PACKET_DATA_SIZE)

class UDPServer{

	//From inside the Kinect code, we want to do 
	// UDPServer server = new UDPServer(); //will automatically set up winsock and data vals.
private:
	SOCKET serverSocket;
	BYTE byteImage[FRAME_SIZE];

	int createFakeImage(char * imagePointer);
	BYTE * convertBufferToImage(UINT16 * pBuffer);
	void executeMainLoop(UINT16* pBuffer);
	void InitWinsock();
	int setUpSockAddrs();
public:
	
	UDPServer();
	void sendTestImageStream();
	void sendKinectImage(UINT16 *pBuffer);
};

#endif