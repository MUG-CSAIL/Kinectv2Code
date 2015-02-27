#include "UDPServerVideo.h"

int main(){
	UDPServer server = UDPServer();
	//server.sendTestImageStream();
	UINT16 pBuffer[PBUFFER_SIZE];
	for (int i = 1; i < PBUFFER_SIZE-1; i++){
		pBuffer[i] = (UINT16)300;
	}
	pBuffer[0] = 100;
	pBuffer[PBUFFER_SIZE - 1] = 500;
	server.sendKinectImage(pBuffer);
}