/***************************************************************************************************/
/* By Anthony Mckale 2003  - this is contains all the world objects for internal representation of the neuron system */
/***************************************************************************************************/

package world_objects;

/* for package / links */
import misc.printer; // for printing (doesnt work yet)
import gui_stuff.*;//Stuff for drawing/making gui
import xml_parser.*;//Stuff for inputing/output xml files/reading xml types description file
import variable_stuff.*;//Stuff for representing variables of different types
import main_method.GUI; // Main Method

/* World Object includes */
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

import java.util.*; // For Linked Lists

/****************/
/* SYNAPSE        */
/****************/

public class Link extends World_Object {
    private static int number = 0;
    
    static Color normal_line_colour = Color.black;
    static Color normal_text_name = Color.black;
    static Color selected_line_colour = Color.lightGray;
    static Color selected_text_name = Color.black;
    
    // Start / End Position Ints
    private int x1 =0;
    private int y1 = 0;
    private int x2 =0;
    private int y2 = 0;

    // Objects needed for drawing
    private Line2D line = new Line2D.Double();
    private Arc2D.Double To_arc = new Arc2D.Double(0);
    private Arc2D.Double From_arc = new Arc2D.Double(0);

    // References to the Linking Neurons
    public World_Object From;
    public World_Object To;
    
    // Synapse Constructor
    
    public Link() {
	From = null;
	To = null;
	parent = null;
	Connected = false;
	
	// Set Up Variable Stuff
	set_up_vars(false);
    }
    
    public Link(int Type) {
	From = null;
	To = null;
	parent = null;
	Connected = false;
	this.Type = Type;
	// Set Up Variable Stuff
	set_up_vars(true);
    }
    /*
    // Synapse Constructor
    public Link( World_Object TempFrom, World_Object TempTo) {
	From = TempFrom;
	To = TempTo;
	parent = From;
	set_up_vars();
    }
    */
    /** This is a Method to set up the Neurons
     * rather than type the same thing twice */

    private void set_up_vars(boolean increase){
	// Set Up name
	Object_Type= Synapse;
	if(increase){
	    name = "Synapse_" + Integer.toString(number++);
	}else{
	    name = "XXX";
	}
	this.id = id_ref++;
	
	// Set Up Variable Stuff
	
	Type_Names = arrays.Synapse_Names;
	Parameters_used_by_Type = arrays.Synapse_Vars;

	// Set up own parameters
	setup_vars();
    }
    
    void drawme(Graphics2D g2, int draw_mode){
	if(Connected){
	    // Case when drawing when in selected but doing normal non selected stage, 
	    // increament position counters but only draw when selected
	    if(GUI.world.Currently_Selected_List.contains((Object)this) && !(draw_mode == Selected_Draw_Mode)){
		if(current_frame != To.frame){
		    To.outgoing = 0;
		    To.incoming = 0;
		    To.frame = current_frame;
		} 
		
		if(current_frame != From.frame){
		    From.incoming = 0;
		    From.outgoing = 0;
		    From.frame = current_frame;
		} 
				
		From.incoming++;
		To.outgoing++;
	    }
	    
	    // Drawing Stage, plus drawing when selected stage
	    if(draw_mode == Selected_Draw_Mode ||(draw_mode == Normal_Draw_Mode && !(GUI.world.Currently_Selected_List.contains((Object)this))) ){
		Color line_color = normal_line_colour;
		Color text_name = normal_text_name;
		if(draw_mode == Selected_Draw_Mode){
		    line_color = selected_line_colour;
		    text_name = selected_text_name;
		}
		final int circle_size = 10;

		// Obtain up to date positions of Linking Neurons
		/* To fine tune position of synapse on Neuron do here 
		* When in slected move use old positions dont recalucate */
		if(draw_mode != Selected_Draw_Mode){
		    if(current_frame != To.frame){
			To.outgoing = 0;
			To.incoming = 0;
			To.frame = current_frame;
		    } 
		    
		    if(current_frame != From.frame){
			From.incoming = 0;
			From.outgoing = 0;
			From.frame = current_frame;
		    } 
		
		    incoming = From.incoming;
		    outgoing = To.outgoing;
		}

		int in_x_offset = (int)((double)incoming * (double)circle_size * 1.5);
		int out_x_offset =  (int)((double)outgoing * (double)circle_size * 1.5);
		
		if(draw_mode != Selected_Draw_Mode){
		    From.incoming++;
		    To.outgoing++;
		}

		int x1 =From.x + in_x_offset;
		int y1 = From.y+65;
		int x2 =To.x + out_x_offset;
		int y2 = To.y-15;
		
		GUI.Antialias_on(g2);
		
		// Draw Linking Line
		g2.setColor(line_color);
		line.setLine( (double)x1, (double)y1, (double)x2, (double)y2);	
		g2.draw(line);
		
		/* Draw Synapses Half Circles */
		
		// From Circle
		To_arc.setFrame((double) x1-5, (double) y1-circle_size, (double) circle_size, (double) circle_size);
		To_arc.setAngleStart(140);
		To_arc.setAngleExtent(270);
		g2.setColor(line_color);
		g2.draw(To_arc);
		
		// To Circle
		From_arc.setFrame((double) x2-5, (double) y2, (double) circle_size, (double) circle_size);
		From_arc.setAngleStart(315);
		From_arc.setAngleExtent(270);
		g2.setColor(line_color);
		g2.draw(From_arc);
		
		if(GUI.draw_synapse_names){
		    //Write Pretty Name
		    FontRenderContext frc = g2.getFontRenderContext();
		    Font font = g2.getFont();
		    TextLayout tl = new TextLayout(name, font, frc);
		    float sw = (float) tl.getBounds().getWidth();
		    float sh = (float) tl.getAscent() + tl.getDescent();
		    g2.setColor(text_name);
		    tl.draw(g2, (float) middle_of_line(x1,x2)-sw/2, middle_of_line(y1,y2)+sh/2);
		    
		    GUI.Antialias_off(g2);
		}
	    }
	}
    }
    
    World_Object hitme(int X, int Y){
	// The Synapses are Counted as being hit if one of there arc are clicked
	if(To_arc !=null && From_arc != null){
	    
	    // Check if To arc clicked
	    if(To_arc.contains(X, Y)) {
		return this;
	    }
	    
	    // Check if From arc clciked
	    if(From_arc.contains(X, Y)) {
		return this;
	    }
	}
	return null;
    }
    
    public String getType(){
	return Type_Names[Type];
    }

    private int middle_of_line(int x1,int x2){
	int Return = 0;
	if(x1>x2) Return = (x1-x2)/2 + x2;
	else Return = (x2-x1)/2 + x1;
	return Return;
    }

    public vars get_inheritance(int serial){
	if(From != null)
	    return From.get_inheritance(serial);
	else
	    return GUI.world.get_inheritance(serial);
    }
}
