package edu.mit.kacquah.udp.image;

public class KinectImage {

  public static final int IMAGE_WIDTH = 512;
  public static final int IMAGE_HEIGHT = 424;
  public static final int IMAGE_LENGTH = IMAGE_WIDTH * IMAGE_HEIGHT;
  public static final int IMAGE_BYTES_LENGTH = IMAGE_LENGTH * 2;

  public static final byte FIRST_PIXEL = '1';
  public static final byte MIDDLE_PIXEL = '2';
  public static final byte LAST_PIXEL = '3';

  public short pixels[];

  public KinectImage() {
    pixels = new short[IMAGE_BYTES_LENGTH];
  }

  /**
   * Fills an arbitrary image.
   * @param image
   */
  public void fillImage() {
    for (int i = 1; i < IMAGE_LENGTH - 1; ++i) {
      pixels[i] = MIDDLE_PIXEL;
    }
    // First pixel is a 1, last pixel is a 3
    pixels[0] = FIRST_PIXEL;
    pixels[IMAGE_LENGTH - 1] = LAST_PIXEL;
  }
  
  /**
   * Checks an arbitrary image. Returns true if image is correct.
   * @param image
   * @return
   */
  public boolean checkImage() {
    for (int i = 1; i < IMAGE_LENGTH - 1; ++i) {
      if (pixels[i] != MIDDLE_PIXEL) {
        System.out.printf("Middle pixel at %d wrong: %c\n", i, pixels[i]);
        return false;
      }
    }
    if (pixels[0] != FIRST_PIXEL) {
      System.out.println("First pixel wrong.");
      return false;
    }
    if (pixels[IMAGE_LENGTH - 1] != LAST_PIXEL) {
      System.out.println("Last pixel wrong.");
      return false;
    }
    return true;
  }
  
  public boolean checkByteImage(){
	  for (int i = 1; i < IMAGE_BYTES_LENGTH - 1; ++i) {
		  if (pixels[i] != MIDDLE_PIXEL) {
			  System.out.printf("Middle pixel at %d wrong: %c\n", i, pixels[i]);
			  return false;
		  }
	  }
	  if (pixels[0] != FIRST_PIXEL) {
		  System.out.println("First pixel wrong.");
		  return false;
	  }
	  if (pixels[IMAGE_BYTES_LENGTH - 1] != LAST_PIXEL) {
		  System.out.println("Last pixel wrong.");
		  return false;
	  }
	  return true;
  }

  /**
   * Returns a byte buffer representation of this image's data.
   * 
   * @return
   */
  public byte[] imageToBytes() {
    byte [] result = new byte[IMAGE_BYTES_LENGTH];
    for (int i = 0; i < IMAGE_LENGTH; ++i) {
      short pixel = pixels[i];
      byte lowByte = (byte) pixel;
      byte highByte = (byte) ((pixel >> 8) & 0xff);
      int byteIndex = i * 2;
      result[byteIndex] = highByte;
      result[byteIndex + 1] = lowByte;
    }
    return result;
  }

  /**
   * Takes in a byte representation of this image's data and sets the image's
   * pixels.
   * 
   * @param bytes
   */
  public void bytesToImage(byte[] bytes) {
    pixels = new short[IMAGE_LENGTH];
    for (int i = 0; i < IMAGE_LENGTH; ++i) {
      int byteIndex = i * 2;
      byte highByte = bytes[byteIndex];
      byte lowByte = bytes[byteIndex + 1];
      short pixel = (short) ((highByte << 8) + lowByte);
      pixels[i] = pixel;
    }
  }

  public void copyBytes(byte[] bytes){
	  for(int i = 0; i < bytes.length; i++){
		  //System.out.printf("Byte %d is a %c.\n", i, bytes[i]);
		  pixels[i] = bytes[i];
	  }
  }
}
