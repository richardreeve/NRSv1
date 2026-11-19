package misc;

/* for package / links */
import world_objects.*;// for node/container/link objects
import gui_stuff.*;//Stuff for drawing/making gui
import xml_parser.*;//Stuff for inputing/output xml files/reading xml types description file
import variable_stuff.*;//Stuff for representing variables of different types
import main_method.GUI; // Main Method

import java.awt.*;
import java.awt.geom.*;
import java.awt.event.*;
import java.awt.font.TextLayout;
import java.awt.font.FontRenderContext;
import javax.swing.*;

import java.awt.*;
import java.awt.event.*;
import java.applet.Applet;
import java.awt.image.*;

/** currently not working due to problem, with null value - 
 * process getToolkit().getPrintJob returns a null !! not good*/

public class printer extends Frame {
    public void print_world(Dummy_World world ){
	
	PrintJob printjob = getToolkit().getPrintJob(this,
                               "World Printout", null, null);
	//PrintJob printjob = getToolkit().getPrintJob(this, world.name, null);
	this.pack();
	Graphics g = printjob.getGraphics();
	if(g != null)
	    GUI.world.draw((Graphics2D)g, 0);
	    //g = printjob.getGraphics();
	//printjob.dispose();
	printjob.end();
    }
}
