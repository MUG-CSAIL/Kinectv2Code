package clientcode;

public class FrameProcessorThread implements Runnable {

	final static int FRAME_SIZE = 434176;
	final static int BUFSIZE = 65507;
	static byte[] frame = new byte[BUFSIZE];
	static byte[] image = new byte[FRAME_SIZE];
	
	FrameProcessorThread(byte[] frame){
		this.frame = frame;
	}
	
	char[] imageBuffer = new char[FRAME_SIZE];
	@Override
	public void run() 
	{
		
	}

	public static void main(String args[]) 
	{
		(new Thread(new FrameProcessorThread(frame))).start();
	}
}
