package pmdw10;
import java.net.InetAddress;
import java.net.UnknownHostException;

public class RefData implements java.io.Serializable {

	public String getHostName()
	{
		String retVal = "Unknown";
		try
		{
		    InetAddress addr;
		    addr = InetAddress.getLocalHost();
		    retVal = addr.getHostName();
		}
		catch (UnknownHostException ex)
		{
		    System.out.println("Hostname can not be resolved");
		}
		return retVal;
	}
	
	public String getHostAddress()
	{
		String retVal = "Unknown";
		try
		{
		    InetAddress addr;
		    addr = InetAddress.getLocalHost();
		    retVal = addr.getHostAddress();
		}
		catch (UnknownHostException ex)
		{
		    System.out.println("Host address can not be resolved");
		}
		return retVal;
	}
	
	public String getInfo()
	{
		return getHostAddress()+" "+getHostName();
	}
}
