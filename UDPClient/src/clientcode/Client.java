package clientcode;

import java.io.*;
import java.net.*;
import java.nio.ByteBuffer;
import java.util.Arrays;
//import clientcode.FrameProcessorThread;
import java.util.concurrent.ConcurrentLinkedQueue;

public class Client {
	static final int FRAME_SIZE = 434176;
	static final int BUFSIZE = 65507;
	static final int CLIENT_PORT = 10801;
	static final int SERVER_PORT = 10800;
	static Object lock = new Object(); //just to have something to lock on
		
	
	public static void main(String args[]) throws Exception
	{
		ByteBuffer image = ByteBuffer.allocate(FRAME_SIZE); //place to store image.
		//ConcurrentLinkedQueue<Byte> imageBuffer = new ConcurrentLinkedQueue<Byte>();
		//Byte[] image;
		ConcurrentLinkedQueue<Byte> imageBuffer = new ConcurrentLinkedQueue<Byte>();
		FrameProcessorThread frameReader = new FrameProcessorThread(imageBuffer, CLIENT_PORT, SERVER_PORT);
		frameReader.run();
		int transferCount = 0;
		while(true)
		{
			while(!imageBuffer.isEmpty() && transferCount < FRAME_SIZE)
			{
				image.put(imageBuffer.poll());
				transferCount++;
				System.out.println(transferCount);
			}
			//For some reason, we're two bytes short of the limit.
			if(transferCount >= FRAME_SIZE){
				break;
			}
		}
		System.out.println("Broke out of loop!");
	}
}
	
	/* Working version of main if we need reversion outside of git backup or want to retest something
	 * 
	 *ConcurrentLinkedQueue<Byte> image = new ConcurrentLinkedQueue<Byte>();
		
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
	      byte fragNum;
	      byte sequenceNum;
	      //main loop goes below when this works  
	      //synchronized(lock){
	      int receiveCount = 0;
	      int dataLeft = FRAME_SIZE;
	      int recvlen;
	      while (receiveCount < 7){
	    	  clientSocket.receive(receivePacket); //receive frame fragment
	    	  recvlen = receiveData.length;
	    	  sequenceNum = receiveData[0];
		      fragNum = receiveData[1]; 
		      //image.put(Arrays.copyOfRange(receiveData, 2, Math.min(dataLeft, recvlen)), 0, Math.min(dataLeft, recvlen-2));
		      for(int i = 2; i < Math.min(dataLeft, recvlen); i++){
		    	  image.add(receiveData[i]);
		      }
		      
		      //System.out.printf("Got frame: %s with length %d", new String(receiveData), new String(receiveData).length());
		      System.out.printf("Got frame - Sequence Number : %d | Fragment Number : %d | last thing: %c\n", sequenceNum, fragNum, receiveData[Math.min(dataLeft, 65506)]);
		      dataLeft -= (recvlen-2);
		      receiveCount +=1;
	      }
	      System.out.println("Got all packets! yaaaay!");
	      clientSocket.close();
	      //}
	    	  
	      
	   } 
	 */
	  
