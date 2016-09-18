package pmdw10;

public class TestBean implements java.io.Serializable {
	private static int sequence = 0;
	
	public String getMessage() {
		sequence+=10;
		return new Integer(sequence).toString();
	}
}
