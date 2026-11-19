package btn.stuff ;

import java.util.* ;

public class Chrono
{
	private long startT ;
	private long stopT ;
	private Date date ;
	
	public Chrono()
	{
		Date date = new Date() ;	
	}
	
	public void start()
	{
		startT = new Date().getTime() ;	
	}
	
	public void stop()
	{
		stopT = new Date().getTime() ;	
	}	
	
	public long getMillis()
	{
		return stopT - startT ;	
	}
}