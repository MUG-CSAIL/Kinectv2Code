package clientcode;

import java.io.*;
import java.net.*;
import java.nio.ByteBuffer;
import java.util.Arrays;
//import clientcode.FrameProcessorThread;

public class Client {
	static final int FRAME_SIZE = 434176;
	static final int BUFSIZE = 65507;
	static final int CLIENT_PORT = 10801;
	static final int SERVER_PORT = 10800;
	static Object lock = new Object(); //just to have something to lock on
	
	public static void main(String args[]) throws Exception
	   {
		ByteBuffer image = ByteBuffer.allocate(FRAME_SIZE); //place to store image.
	      BufferedReader inFromUser =
	         new BufferedReader(new InputStreamReader
	                     (System.in)); //to get start message to send.
	      DatagramSocket clientSocket = new DatagramSocket(CLIENT_PORT); //make socket and bind to port.
	      String clientIP = "127.0.0.1"; //change later, after this works locally
	      InetAddress serverAddress = InetAddress.getByName(clientIP); //set IP address for socket.
	      byte[] sendData = new byte[1024]; //short since we just need to send some small message
	      byte[] receiveData = new byte[BUFSIZE]; //packet size from C++ server code
	      DatagramPacket receivePacket = 
	    		  new DatagramPacket(receiveData,receiveData.length); //make packet for receiving frags
	      System.out.printf("Current local address: %s\n", clientSocket.getLocalAddress().toString()); //check to see that address is right
	      System.out.printf("Current local port: %d\n", clientSocket.getLocalPort()); //check to see that port is right
	      System.out.println("Please type any short message to begin communication.");
	      String sentence = inFromUser.readLine(); //read in whatever the start message is
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
	      int fragNum;
	      int sequenceNum;
	      //main loop goes below when this works  
	      //synchronized(lock){
	      clientSocket.receive(receivePacket); //receive frame fragment
	      //fragNum = receiveData[1]; 
	      //sequenceNum = receiveData[0];
	      //receiveData = Arrays.copyOfRange(receiveData, 2, receiveData.length);
	      //image.put(receiveData, BUFSIZE*fragNum, receiveData.length);
	      //System.out.printf("Got frame: %s with length %d", new String(receiveData), new String(receiveData).length());
	      System.out.printf("Got frame - Sequence Number : %d | Fragment Number : %d | last thing: %c\n", receiveData[0], receiveData[1], receiveData[65506]);
	      clientSocket.close();
	      //}
	    	  
	      
	   }
}

