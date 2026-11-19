package btn.ds ;

import btn.ds.* ;
import btn.stuff.* ;
import java.io.File;
import javax.swing.filechooser.FileFilter;

public class DSEarFileFilter extends FileFilter 
{

	public static final int EAR = 1 ;
	public static final int PNG = 2 ;
	public static final int SVG = 3 ;
	public static final int TXT = 4 ;
	
	private int type ;

    public DSEarFileFilter(int type) 
    {
    	this.type = type ;
    }

    public boolean accept(File f) 
    {
        if (f != null) 
        {
            if (f.isDirectory()) 
            {
                return true;
	        }
            String extension = getExtension(f);
            
            switch(type)
            {
            	case EAR :
            	{
            		if (extension != null && extension.equalsIgnoreCase("txt")) 
            			return true ;	
            	} ; break ;	
            	case PNG :
            	{
            		if (extension != null && extension.equalsIgnoreCase("png")) 
            			return true ;	
            	} ; break ;	
            	case SVG :
            	{
            		if (extension != null && extension.equalsIgnoreCase("svg")) 
            			return true ;	
            	} ; break ;	
            	case TXT :
            	{
            		if (extension != null && extension.equalsIgnoreCase("txt")) 
            			return true ;	
            	} ; break ;	
            	default :
            	{
            		return false ;	
            	}          	
            }
        }
        
        return false ;
    }

    private String getExtension(File f) 
    {
        if(f != null) 
        {
            String filename = f.getName();
            int i = filename.lastIndexOf('.');
            if (i > 0 && i < filename.length()-1) 
            {
                return filename.substring(i+1).toLowerCase();
            }
        }
        return null;
    }

    public String getDescription() 
    {
	    if(type == EAR)
	    	return "Ear configuration file (*.txt)" ;
	    if(type == PNG)
	    	return "Portable Network Graphics (*.png)" ;
	    if(type == SVG)
	    	return "Scalable Vector Graphics (*.svg)" ;
	    if(type == TXT)
	    	return "Scalable Vector Graphics (*.svg)" ;
	    	
	    return "" ;
    }
}
