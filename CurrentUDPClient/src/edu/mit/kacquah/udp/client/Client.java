package edu.mit.kacquah.udp.client;

import java.net.DatagramPacket;
import java.net.DatagramSocket;

import edu.mit.kacquah.udp.image.KinectImage;

public class Client {
  // Common globa constants
  public static int SERVER_PORT = 10800;
  public static int CLIENT_PORT = 10801;
  
  public static int MAX_UDP_BUFFER_SIZE = 65507; //65507/10;
  public static int NUMBER_OF_PACKETS_PER_IMAGE = 7;
  public static int PACKET_SIZE = MAX_UDP_BUFFER_SIZE;
  public static int PACKET_DATA_SIZE = MAX_UDP_BUFFER_SIZE - 2;
  // Number of packets needed per image. Note how we subtract 2 from the
  // max packet size since need two bytes in each packet to go to the image
  // number and part number.
	public static void main (String [] args) throws Exception  {
	  // Client Socket
	  UDPListener udpListener = new UDPListener();
	  // Start thread
	  udpListener.start();
	  System.out.println("Client thead running");
	  
	  // Create KinectImageConstructor for producing kinect images
	  KinectImageConstructor constructor = new KinectImageConstructor();
	  
	  // Recieve Messages
	  while (true) {
	    // Spin until we get a new packet.
	    while (!udpListener.hasPacket()) {
	      //System.out.println("No packet yet...");
	    }
	    
	    // Debug print
	    byte[] packetData = udpListener.getNextPacket();
	    //String received = new String(packetData, 0, packetData.length);
	    //System.out.println(received);
	    System.out.println("Got it.");
	    System.out.printf("received seq num %d, frag num %d\n", packetData[0] & 0xFF, packetData[1] & 0xFF);
	    
	    // Construct image
      constructor.processPacketData(packetData);
      if (constructor.hasNextKinectImage()) {
        KinectImage image = constructor.getNextKinectImage();
        // Check for image correctness then quit.
        if (!image.checkByteImage()) {
          System.out.println("Something went wrong");
          System.exit(-1);
        } else {
          System.out.println("Image is correct!!!");
          //System.exit(0);
        }
      }
	    
	  }
		
	}

}
