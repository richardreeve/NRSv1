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

/************************/
/*            WORLD                 */
/************************/

/* This is the top class all other object are inside this object */

public class Dummy_World extends World_Object{
    // Used to give an orginal name
    static int number = 1; // The keeps track of how many networks have been made

    //Dummy_Network dummy1 = new Dummy_Network();
    
    /** Used for quickly getting the value for inherrance */
    public HashMap map_of_nodes_links_vars;
    
    /** For Storing a list of all neuron currently inside world */
    public LinkedList Neuron_List = new LinkedList();

     /** For Storing a list of all synapses currently inside world */
    public LinkedList Synapse_List = new LinkedList();

    /** For Storing a list of all networks currently inside world */
    public LinkedList Network_List = new LinkedList();
    
    /** For containing the currently selected elements in the world */
    public LinkedList Currently_Selected_List = new LinkedList();
    
    /** Used for storing the value for inherriance */
    vars[] list_of_nodes_links_vars;
    
    public Dummy_World(){
	set_up_vars();
    }
    
    // This is a Method to set up the Sensors
    // rather than type the same thing twice

    private void set_up_vars(){
	// Set Up name
	Object_Type= World;
	Type = 0;
	parent = this;
	
	this.id = id_ref++;

	name = "World_" + Integer.toString(number++);

	// Set Up Components
	list_of_Parts = new LinkedList();
		
	Type_Names = arrays.World_Names;
	Parameters_used_by_Type = arrays.World_Vars;
	
	// Set up own parameters
	setup_vars();

	// Set up default vars fro inherriance
	create_node_and_link_var_list();
    }

    public String getType(){
	return "World";
    }

    void drawme(Graphics2D g2, int incoming){
    }
    
    World_Object hitme(int X, int Y){
	return null;
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
    
    /** This method returns the parameter value inheritted from it's parent, 
     * in this case that would be the inbuildt default for the parameter */
    public vars get_inheritance(int serial){
	if(map_of_nodes_links_vars.containsKey(new Integer(serial))){
	    vars temp = (vars)map_of_nodes_links_vars.get(new Integer(serial));
	    temp.inherrited = false;
	    return temp;
	
	    // Umm confused if this becomes a problem just remove serial then insert new one with same serial
	    //This defaults to sim descirption defined defaults if that's not allowed just System.exit(1)
	    //System.err.println("World Should really have inherriance values for all variables does not for "+temp.variable.Name);
	    //System.exit(1);
	    //return null;

	    // Not exactly defined what's suppose to happen in this situation, i believe nrs just crashes at this point ......
	}
	// In case that parameter in list this means that the parameter is a container parameter or someone didnt read my note,
	// about not mixing container and terminal parameter becuz of teh general confusion about what should happen
	if(arrays.Variable_List.containsKey(new Integer(serial))){
	    Variable temp = (Variable)arrays.Variable_List.get(new Integer(serial));
	    vars a = new vars(temp.serial, temp);
	    a.inherrited=false;
	    a.set(temp.Default);
	    return a;
	}else{
	    try{
		throw new Exception("Tried to get a non existant parameter from inherriance hashmap serial : "+serial);
	    }catch(Exception e){
		e.printStackTrace();
		System.err.println("inheeriance Hap Map : "+map_of_nodes_links_vars);
		System.err.println("all parameter Hap Map : "+arrays.Variable_List);
	    }
	    System.exit(1);
	}
	return null;
    }
}
