package clientcode;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.util.concurrent.ConcurrentLinkedQueue;

public class FrameProcessorThread implements Runnable {

	final static int FRAME_SIZE = 434176;
	final static int BUFSIZE = 65507;
	byte[] frame = new byte[BUFSIZE];
	ConcurrentLinkedQueue<Byte> imageBuffer;
	final int CLIENT_PORT;
	final int SERVER_PORT;
	
	/**
	 * Constructor. 
	 * @param imageBuffer : The buffer into which we should put packet info for the main thread to read
	 * @param clientPort : Our port that we'll be receiving on
	 * @param serverPort : The port that the server's listening on
	 */
	FrameProcessorThread(ConcurrentLinkedQueue<Byte> imageBuffer, int clientPort, int serverPort){
		this.imageBuffer = imageBuffer;
		CLIENT_PORT = clientPort;
		SERVER_PORT = serverPort;
	}

	@Override
	public void run() 
	{
		BufferedReader inFromUser =new BufferedReader(new InputStreamReader(System.in)); //to get start message to send.
		DatagramSocket clientSocket = null;
		try 
		{
			clientSocket = new DatagramSocket(CLIENT_PORT); //make socket and bind to port.
		} 
		catch (SocketException e1) 
		{
			System.err.println("Failed to create socket.");
			e1.printStackTrace();
		} 
		String clientIP = "127.0.0.1"; //change later, after this works locally
		InetAddress serverAddress = null;
		try 
		{
			serverAddress = InetAddress.getByName(clientIP); //set IP address for socket.
		} 
		catch (UnknownHostException e1) 
		{
			System.err.println("Failed to get server address from the given information.");
			e1.printStackTrace();
		} 
		byte[] sendData = new byte[1024]; //short since we just need to send some small message
		byte[] receiveData = new byte[BUFSIZE]; //packet size from C++ server code
		DatagramPacket receivePacket = new DatagramPacket(receiveData,receiveData.length); //make packet for receiving frags
		
		//System.out.printf("Current local address: %s\n", clientSocket.getLocalAddress().toString()); //check to see that address is right
		//System.out.printf("Current local port: %d\n", clientSocket.getLocalPort()); //check to see that port is right
		
		System.out.println("Please type any short message to begin communication.");
		String sentence = null;
		try {
			sentence = inFromUser.readLine(); //read in whatever the start message is
		} 
		catch (IOException e1){
			System.err.println("For some strange reason, the I/O just failed.");
			e1.printStackTrace();
		} 
		sendData = sentence.getBytes();  //convert it to bytes
		DatagramPacket sendPacket =
				new DatagramPacket(sendData, sendData.length, 
						serverAddress, SERVER_PORT); //create packet to send to server

		try{
			clientSocket.send(sendPacket); //send start message
		}
		catch(Exception e){
			System.err.println("Send failed to complete.");
			e.printStackTrace();
		}
		byte fragNum;
		byte sequenceNum;
		//main loop goes below when this works  
		//synchronized(lock){
		int receiveCount = 0;
		int dataLeft = FRAME_SIZE;
		int recvlen;
		while (receiveCount < 7){ //need a better condition later but for testing, let it be
			try {
				clientSocket.receive(receivePacket); //receive frame fragment
			} catch (IOException e) {
				System.err.println("Receive failed and threw an exception.");
				e.printStackTrace();
			} 
			recvlen = receiveData.length;
			sequenceNum = receiveData[0];
			fragNum = receiveData[1]; 
			//image.put(Arrays.copyOfRange(receiveData, 2, Math.min(dataLeft, recvlen)), 0, Math.min(dataLeft, recvlen-2));
			for(int i = 2; i < Math.min(dataLeft, recvlen); i++){
				imageBuffer.add(receiveData[i]);
			}
			System.out.printf("Got frame - Sequence Number : %d | Fragment Number : %d | last thing: %c\n", sequenceNum, fragNum, receiveData[Math.min(dataLeft-1, 65506)]);
			dataLeft -= (recvlen-2);
			receiveCount +=1;
		}
		System.out.println("Got all packets! yaaaay!");
		clientSocket.close();
	}


}
