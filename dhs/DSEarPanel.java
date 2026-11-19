package btn.ds ;

import btn.ds.* ;
import btn.stuff.* ;
import java.io.* ;
import java.awt.* ;
import java.awt.event.*;
import java.awt.geom.* ;
import javax.swing.border.* ;
import javax.swing.* ;

public class DSEarPanel extends JPanel
{
	private Object parent ;
	
	private Container contentPane ;
	
	public DSEarPanel(Object parent)
	{
		super() ;
		this.parent = parent ;	
		
		setLayout(new GridLayout(1,1)) ;
		add(new DSEarComponent(parent)) ;
	}	
}