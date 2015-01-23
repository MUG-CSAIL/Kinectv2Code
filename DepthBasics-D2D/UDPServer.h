#include <Windows.h>
#include <winsock2.h>
#include <stdio.h>
#include <tchar.h>
#include <WS2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT  htons(10800);
#define WIDTH 512;
#define HEIGHT 424;
#define PACKET_SIZE 1500   
#define MAX_NUM_OF_PACKETS 290 //Math.Ceiling(WIDTH*HEIGHT*2 (2 is byte size) / 1500 (that's the MTU of ethernet))

class UDPServer{
public:
	int isDataReadyMutex;

	UDPServer(void);
	~UDPServer(void);
	int send(const char* frameData);
private:
	void InitWinsock();
	SOCKET setUpServerSocket();
};

