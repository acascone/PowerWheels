/***********************************************************************************************************
* The server expects a data packets based on the first byte sent
* 1) 1 unsigned byte for the function
*    value 0 = send colour
*    At a later time I will setup functions to return data from the server
*    that will tell you info about the board and its configuration such as number of lights on x or y 
* 2) 1 unsigned byte x cord valid numbers 1-255
* 3) 1 unsigned byte y cord valid numbers 1-255
* 4) 1 unsigned byte red valid numbers 1-15
* 5) 1 unsigned byte green valid numbers 1-15
* 6) 1 unsigned byte blue valid numbers 1-15
* 7) 1 unsigned byte brightness valid numbers 1-255
* Packed BIGENDIAN as all network data should be packed 
***********************************************************************************************************/
// Import 
import processing.net.*;
import java.util.concurrent.*;

// This class defines a thread and a message passing system so we can talk to the light board
// without blocking the main thread
class G35LightBoard extends Thread
{
  /**************** Constructors ****************/
  public G35LightBoard(PApplet parent, String address)
  {
    parrentApplet = parent;
    addressForNetworkConnection = address;
  }
  
  /**************** Destructor... darn java ****************/
  // Call This to close the connection
  public void Close()
  {
    if( connectionToLightBoard != null )
    {
      connectionToLightBoard.stop();
    }
  }
  
  /**************** Methods ****************/
  public void UpdateData(float Current, float Voltage, float Throttle, float ControllerTemp )
  {
    dataToProcessFromApp.add( new DataFromApplication( Current, Voltage, Throttle, ControllerTemp ) );
  }

  /**************** Private Methods ****************/
  // General Lightboard methods
  private void ClearScreen()
  {
    for( int x = 0; x < LIGHT_BOARD_WIDTH; x++ )
    {
      for( int y = 0; y < LIGHT_BOARD_HEIGHT; y++ )
      {
        SendLightPacket( (byte)x, (byte)y, (byte)0, (byte)0, (byte)0, (byte)0 );
      }
    }
  }
  private void ClearRow(int rowToClear)
  {
    for( int x = 0; x < LIGHT_BOARD_WIDTH; x++ )
    {
      SendLightPacket( (byte)x, (byte)rowToClear, (byte)0, (byte)0, (byte)0, (byte)0 );
    }
  }
  
  // Send Data to the lightboard
  private void SendLightPacket(byte x, byte y, byte red, byte green, byte blue, byte brightness)
  {
    final byte LIGHT_PACKET_ID = 0;
    
    this.SendPacket(LIGHT_PACKET_ID, x, y, red, green, blue, brightness);
  }
  private void SendPacket(byte messageId, byte arg0, byte arg1, byte arg2, byte arg3, byte arg4, byte arg5 )
  {
    // We always send 7 bytes to the server
    final int PACKET_SIZE = 7;
    
    // Pack the data into a byte array
    byte[] packet = new byte[PACKET_SIZE];
    packet[0] = messageId;
    packet[1] = arg0;
    packet[2] = arg1;
    packet[3] = arg2;
    packet[4] = arg3;
    packet[5] = arg4;
    packet[6] = arg5;
    
    // If the network connection is dead jump out
    if( connectionToLightBoard.active() )
    {
      // Send the bytes off to the network connection
      connectionToLightBoard.write(packet);
    }
  }
  
  /**************** Threadding Stuff ****************/
  public void start()
  {
    // Setup connection to client
    connectionToLightBoard = new Client( parrentApplet, addressForNetworkConnection, PORT_NUMBER);
    
    // Blank the Lightboard
    ClearScreen();
    
    // Run the default start info 
    super.start();
  }
  public void run() 
  {
    // Keep the thread going while we wait for data to process
    while( !KillThread )
    {
      // Check our Queue to see if anything needs processed
      DataFromApplication info = dataToProcessFromApp.poll();
      if( info != null )
      {
        // If a change was made since the previously sent value output the new changes
        if( info.m_Current != LastSentDataSet.m_Current )               DisplayCurrent( info.m_Current );
        if( info.m_Voltage != LastSentDataSet.m_Voltage )               DisplayVoltage( info.m_Voltage );
        if( info.m_Throttle != LastSentDataSet.m_Throttle )             DisplayThrottle( info.m_Throttle );
        if( info.m_ControllerTemp != LastSentDataSet.m_ControllerTemp ) DisplayControllerTemp( info.m_ControllerTemp );
        
        LastSentDataSet = info;
      }
    }
  }
  // Send packets to the light board for each 
  private void DisplayCurrent(float current)
  {
    DrawColoredLineLeftToRightGreenToRedBasedOnPersentage(4, current);
  }
  private void DisplayVoltage(float voltage)
  {
    DrawColoredLineLeftToRightGreenToRedBasedOnPersentage(2, voltage);
  }
  private void DisplayThrottle(float throttle)
  {
    DrawColoredLineLeftToRightGreenToRedBasedOnPersentage(6, throttle);
  }
  private void DisplayControllerTemp(float controllerTemp)
  {
    DrawColoredLineLeftToRightGreenToRedBasedOnPersentage(0, controllerTemp);
  }
  
  // Draw methods specific to Display functions
  // yLineNumber is the line you wish to render to
  // persentage is a number between 1.0 and 0.0 that tells us how much of the line to draw
  private void DrawColoredLineLeftToRightGreenToRedBasedOnPersentage( int yLineNumber, float percentage )
  {
    // quick and dirty lookup tables for getting the colors to render
    final byte[] red   = { (byte)0, (byte)0, (byte)0, (byte)2, (byte)4, (byte)6, (byte)8, (byte)10, (byte)12, (byte)15  };
    final byte[] blue  = { (byte)0, (byte)0, (byte)0, (byte)0, (byte)0, (byte)0, (byte)0, (byte)0, (byte)0, (byte)0  };
    final byte[] green = { (byte)15, (byte)12, (byte)9, (byte)6, (byte)4, (byte)2, (byte)0, (byte)0, (byte)0, (byte)0  };
    
    for( int x = 0; x < LIGHT_BOARD_WIDTH ; x++)
    {  
      if( x < ( percentage * LIGHT_BOARD_WIDTH ) )
      {
        SendLightPacket( (byte)x, (byte)yLineNumber, red[x], green[x], blue[x], (byte)128 );
      }
      else
      {
        SendLightPacket( (byte)x, (byte)yLineNumber, (byte)0, (byte)0, (byte)0, (byte)0 );
      }
    }
  }
  private void DrawColoredLineLeftToRightRedToGreenBasedOnPersentage( int yLineNumber, float percentage )
  {
    // quick and dirty lookup tables for getting the colors to render
    final byte[] red     = { (byte)15, (byte)12, (byte)9, (byte)6, (byte)4, (byte)2, (byte)0,  (byte)0, (byte)0,  (byte)0  };
    final byte[] blue    = { (byte)0,  (byte)0,  (byte)0, (byte)0, (byte)0, (byte)0, (byte)0,  (byte)0, (byte)0,  (byte)0  };
    final byte[] green   = { (byte)0,  (byte)0,  (byte)0, (byte)2, (byte)4, (byte)6, (byte)8, (byte)10, (byte)12, (byte)15  };
    
    for( int x = 0; x < LIGHT_BOARD_WIDTH ; x++)
    {  
      if( x < ( percentage * LIGHT_BOARD_WIDTH ) )
      {
        SendLightPacket( (byte)x, (byte)yLineNumber, red[x], green[x], blue[x], (byte)128 );
      }
      else
      {
        SendLightPacket( (byte)x, (byte)yLineNumber, (byte)0, (byte)0, (byte)0, (byte)0 );
      }
    }
  }
  
  /**************** Data ****************/
  // Network Connection Data
  private Client connectionToLightBoard;
  private ConcurrentLinkedQueue<DataFromApplication> dataToProcessFromApp = new ConcurrentLinkedQueue<DataFromApplication>();
  private DataFromApplication LastSentDataSet = new DataFromApplication( 0.0f, 0.0f, 0.0f, 0.0f );
  private String addressForNetworkConnection;
  private PApplet parrentApplet;
  
  // Data for threadding
  boolean KillThread = false;
  
  /**************** Constants ****************/
  // Lightboard constants that we will replace with calls later
  private final int NUMBER_OF_LIGHTS   = 70;
  private final int LIGHT_BOARD_HEIGHT = 7;
  private final int LIGHT_BOARD_WIDTH  = 10;
  // Network Connection Constants
  private final int PORT_NUMBER = 6234;
  
  /**************** Inner Classes ****************/
  private class DataFromApplication
  {
    // Constructor
    public DataFromApplication(float Current, float Voltage, float Throttle, float ControllerTemp)
    {
      m_Current = Current;
      m_Voltage = Voltage;
      m_Throttle = Throttle;
      m_ControllerTemp = ControllerTemp;
    }

    // Data
    float m_Current;
    float m_Voltage;
    float m_Throttle;
    float m_ControllerTemp;
  }
}
