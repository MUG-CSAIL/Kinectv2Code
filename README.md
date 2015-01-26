Working towards having a working UDP server that streams Kinect images to a client. 
Server has bookkeeping in packets now. Has not yet been converted into a class for Kinect
integration. Java side client is receiving packets.

The Java part is contained in UDPClient. (additional Eclipse files are in .metadata and .recommenders,
though those are most likely unneeded.)

The C++ server is contained in UDPServerVideo.  