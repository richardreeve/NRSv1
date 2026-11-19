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
/*    NETWORK   */
/****************/

public class Dummy_Network extends World_Object {

    static Color normal_box_wall = Color.blue;
    static Color normal_box_inside = Color.pink;

    static Color normal_text_name = Color.black;
    static Color normal_text_type = Color.black;

    static Color selected_box_wall = Color.blue;
    static Color selected_box_inside = Color.red;

    static Color selected_text_name = Color.black;
    static Color selected_text_type = Color.black;

     // Used to give an orginal name
    private static int number = 1; // The keeps track of how many networks have been made
    
    // Objects used for drawing
    private Rectangle rect;

    final static int Square_size = 50;

    /* part of interesting drawing method */
    private static int y_offset=0;
    
    /** Used for quickly getting the value for inherrance */
    public HashMap map_of_nodes_links_vars;
    /** Used for storing the value for inherriance */
    vars[] list_of_nodes_links_vars;

    /** Constructor for making a new Network */
    public Dummy_Network() {
	Type =0;
	set_up_vars();
    }

    /** Constructor for making a new Network */
    public Dummy_Network(int Type) {
	this.Type = Type;
	set_up_vars();
    }
      // This is a Method to set up the Neurons
    // rather than type the same thing twice

    public void connect(){
	Connected = true;
	parent = GUI.world;
    }

    private void set_up_vars(){
	// Set Up name
	Object_Type= Network;

	this.id = id_ref++;

	name = "Network_" + Integer.toString(number++);
	
	// Set Up Variables Stuff
	list_of_Parts = new LinkedList();
		
	Type_Names = arrays.Container_Names;
	Parameters_used_by_Type = arrays.Container_Vars;

	// Set up own parameters
	setup_vars();
	
	// Set up default vars
	create_node_and_link_var_list();
    }

    void drawme(Graphics2D g2, int draw_mode) {
	//draw Network

	if(draw_mode == Normal_Draw_Mode){
	    Color box_wall = normal_box_wall;
	    Color box_inside = normal_box_inside;

	    Color text_name = normal_text_name;
	    Color text_type = normal_text_type;

	if(GUI.world.Currently_Selected_List.contains((Object)this)){
	    box_wall =  selected_box_wall;
	    box_inside = selected_box_inside;
	    text_name = selected_text_name;
	    text_type = selected_text_type;
	}
	
	// Get offset properly done
	if(current_frame != parent.frame){
	    parent.outgoing = 0;
	    parent.frame = current_frame;
	    y_offset = 0;
	    /* part of interesting method */
	    
	}
	
	/* Interesting method wraps the y counter around to prevent it going off the screen
	 * well at least in the x direction */
	if(parent.outgoing == 5){
	    y_offset++;
	    parent.outgoing = 0;
	}
	// Could bit shift for extra speed but hey it's java
	int out_x_offset = (int)((double)(parent.outgoing+0.25) *(double)Square_size * 2);
	int out_y_offset =  (int)((double)y_offset*(double)Square_size * 2); //(int)(parent.outgoing *Square_size* 2);
	
	parent.outgoing++;
	
	int x1 = x + out_x_offset;
	int y1 = y + out_y_offset;
	
	
	//Draw Big Box
	g2.setColor(box_wall);
	rect = new Rectangle(x1, y1, 50, 50);
	g2.draw(rect);
	g2.fill(rect);
	
	//Draw Smaller Box
	g2.setColor(box_inside);
	rect = new Rectangle(x1+2, y1+2, 46, 46);
	g2.draw(rect);
	g2.fill(rect);
	
	//Write Pretty Name
	GUI.Antialias_on(g2);
	
	FontRenderContext frc = g2.getFontRenderContext();
	Font font = g2.getFont();
	TextLayout tl = new TextLayout(name, font, frc);
	float sw = (float) tl.getBounds().getWidth();
	float sh = (float) tl.getAscent() + tl.getDescent();
	g2.setColor(text_name);
	tl.draw(g2, (float) x1+25-sw/2, y1+25+sh/2);
	
	// Write Network Belongs to below
	g2.setColor(text_type);
	TextLayout type_text = new TextLayout(Type_Names[Type], font, frc);
	sw = (float) type_text.getBounds().getWidth();
	sh = (float) type_text.getAscent() + type_text.getDescent();
	type_text.draw(g2, (float) x1+25-sw/2, (float)(y1+25+1.5*sh));
	
	GUI.Antialias_off(g2);
	}
    }
    
    World_Object  hitme(int X, int Y) {
	if(rect !=null)
	    if(rect.contains(X, Y)) return this;
	return null;
   }

    public String getType(){
	return Type_Names[Type];
    }

    /** Container Vars containing every neuron and synapse variable */
    public void create_node_and_link_var_list(){
	vars[] temp_list_of_nodes_links_vars = new vars[99];
	HashMap temp_map_of_nodes_links_vars = new HashMap();
	int total = 0;
	
	try{
	// Add up all neuron
	for (int tempType =0; tempType < arrays.Neuron_Names.length; tempType++) { 
	    for (int counter =0; counter < arrays.Neuron_Vars[tempType].length; counter++) {
		Variable temp = (Variable)arrays.Neuron_Vars[tempType][counter];
		if(!temp_map_of_nodes_links_vars.containsKey(new Integer(temp.serial))){
		    total++;
		    // make new vars
		    temp_list_of_nodes_links_vars[counter] = new vars(temp.serial, (Object)temp);
		    // give it value according to it's type
		    temp_list_of_nodes_links_vars[counter].set((Object)temp.Default);
		    temp_map_of_nodes_links_vars.put(new Integer(temp.serial), temp_list_of_nodes_links_vars[counter]);
		}
	    } 
	}
	
	// Add up all synapse
	for (int tempType =0; tempType < arrays.Synapse_Names.length; tempType++) { 
	    for (int counter =0; counter < arrays.Synapse_Vars[tempType].length; counter++) { 
		Variable temp = (Variable)arrays.Synapse_Vars[tempType][counter];
		// Should a terminal be able to inherrant from it's self ? here's the code
		// TO STOP IT DOING THAT WILL NEED TO DO SOME NULL CHECKING
		// to make it work
		//if(!temp_map_of_nodes_links_vars.containsKey(new Integer(temp.serial))){
		    total++;
		    // make new vars
		    temp_list_of_nodes_links_vars[counter] = new vars(temp.serial, (Object)temp);
		    // give it value according to it's type
		    temp_list_of_nodes_links_vars[counter].set((Object)temp.Default);
		    temp_map_of_nodes_links_vars.put(new Integer(temp.serial), temp_list_of_nodes_links_vars[counter]);
		    //}
	    } 
	}
	}catch(Exception e){
	    System.err.println(" Hi if this is called then hack in Dummy world or Dummy newtork has failed inside method create_node_and_link_var_list change the vars array from vars[99] to something bigger Anthony 2003");
	    e.printStackTrace();
	}
	// Store these newly made values
	list_of_nodes_links_vars = new vars[temp_map_of_nodes_links_vars.size()];
	
	for(int counter = 0; list_of_nodes_links_vars.length < counter;counter++){
	    list_of_nodes_links_vars[counter] = temp_list_of_nodes_links_vars[counter];
	}
	
	map_of_nodes_links_vars = temp_map_of_nodes_links_vars ;
    }
    
    public vars get_inheritance(int serial){
	if(map_of_nodes_links_vars.containsKey(new Integer(serial))){
	    vars temp = (vars)map_of_nodes_links_vars.get(new Integer(serial));
	    if(!temp.inherrited)
		return temp;
	    else
		return parent.get_inheritance(serial);
	}
	try{
	    throw new Exception("Tried to get a non existant parameter from inherriance hashmap serial : "+serial);
	}catch(Exception e){
	    e.printStackTrace();
	    System.err.println("inheeriance Hap Map : "+map_of_nodes_links_vars);
	    System.err.println("all parameter Hap Map : "+arrays.Variable_List);
	}
	System.exit(1);
	return null;
    }
}
