package edu.mit.kacquah.udp.server;

import java.net.*;
import java.util.LinkedList;

import edu.mit.kacquah.udp.image.KinectImage;

public class Server {
  public static int SERVER_PORT = 4445;
  public static int CLIENT_PORT = 4446;
  
  public static int MAX_UDP_BUFFER_SIZE = 65507; //65507/10;
  // Size of each packet and packet data.
  public static int PACKET_SIZE = MAX_UDP_BUFFER_SIZE;
  public static int PACKET_DATA_SIZE = MAX_UDP_BUFFER_SIZE - 2;
  // Number of packets needed per image. Note how we subtract 2 from the
  // max packet size since need two bytes in each packet to go to the image
  // number and part number.
  public static int NUMBER_OF_PACKETS_PER_IMAGE = (int) Math
      .ceil((double) KinectImage.IMAGE_BYTES_LENGTH
          / ((double) PACKET_DATA_SIZE));

  // Current sequence number for our image.
  public static int imageNumber = 0;
  
  public static void checkImageSerialization() {
    KinectImage image = new KinectImage();
    image.fillImage();
    byte[] bytes = image.imageToBytes();
    image.bytesToImage(bytes);
    if (!image.checkImage()) {
      System.out.println("Something went wrong");
      System.exit(-1);
    }
  }
  
  public static LinkedList<byte[]>  imageToPackets(KinectImage image) {
    // Get image bytes
    byte[] imageBytes = image.imageToBytes();
    System.out.println("The number of packets: " + NUMBER_OF_PACKETS_PER_IMAGE);
    // Store packets for output image
    LinkedList<byte[]> imagePackets = new LinkedList<byte[]>();
    // Iterate over each packet
    for( int i = 0; i < NUMBER_OF_PACKETS_PER_IMAGE; ++i) {
      byte[] packetData = new byte[PACKET_SIZE];
      // First byte is image number
      packetData[0] = (byte)imageNumber;
      // Second byte is image part number
      packetData[1] = (byte)i;
      // Remaining bytes are imageBytes
      int imageBytesOffsetIndex = i * PACKET_DATA_SIZE;
      for (int j = 0; j < PACKET_DATA_SIZE; ++j) {
        // The last packet won't be full all the way.
        if (imageBytesOffsetIndex + j >= imageBytes.length) {
          break;
        }
        packetData[j + 2] = imageBytes[imageBytesOffsetIndex + j];
      }
      // Add packet to output
      imagePackets.add(packetData);
    } // end for i numberOfPackets
    // Update image number
    imageNumber = (imageNumber + 1) % 256;
    // Return output
    return imagePackets;
  }
  
  public void sendImagePackets(LinkedList<byte[]> imagePacketData) {
    
  }
  
  public static void main(String[] args) throws Exception {
    // Check the image class
    checkImageSerialization(); 
    
    // We're gonna send a string to the client
    byte[] sendData = new byte[MAX_UDP_BUFFER_SIZE];
    for (int i = 0; i < sendData.length; ++i) {
      sendData[i] = 'a';
    }
    
    // Packet for client.
    InetAddress clientAddress = InetAddress.getLocalHost();
    DatagramPacket sendPacket = new DatagramPacket(sendData, sendData.length,
        clientAddress, CLIENT_PORT);
    // Server socket
    DatagramSocket datagramSocket = new DatagramSocket(SERVER_PORT);
    System.out.println(datagramSocket.getSendBufferSize());
    
//    while (true) {
//      datagramSocket.send(sendPacket);
//    }
    
    // Create new image
    KinectImage image = new KinectImage();
    image.fillImage();
    // Convert to packets
    LinkedList<byte[]> imagePacketData =  imageToPackets(image);
    // Send each packet
    for( byte[] packetData : imagePacketData) {
      sendPacket = new DatagramPacket(packetData, packetData.length,
          clientAddress, CLIENT_PORT);
      datagramSocket.send(sendPacket);
    }
    System.out.println("Done sending packets");

    
  }
}
