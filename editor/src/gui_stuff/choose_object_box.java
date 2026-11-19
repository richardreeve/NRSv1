package gui_stuff;

/* for package / links */
import misc.printer; // for printing (doesnt work yet)
import world_objects.*;// for node/container/link objects
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

import javax.swing.event.*;

import java.util.*; // For Linked Lists

/** By Anthony Mckale 2003 - Is a combo box which is used to select what type of object u want to create, 
 * whether it be a node, container, or connector */

public class choose_object_box extends JComboBox  implements ItemListener{
    int Type;
    int Selected_Type;
    World_Object template;

    /** Default constructor add Type wanted as defined in World_Objects */
    public choose_object_box(int Type){
	this.Type = Type;
	remake();
	addItemListener(this);
	//return Object_Type_Selector;
    }
    
    public void itemStateChanged(ItemEvent e){
	if(e.getStateChange() == ItemEvent.SELECTED){
	    String Selected_Type_ = (String)e.getItem();
	    // Inefficent but java doesnt give an index or allow for tagging items 
	    // in a combo box with a index or key (grrr) 
	    
	    // This checks EVERY neuron type to get the index of the one selected
	    // Then changes the neuron type and redraws the info bar
	    for(int counter =0; counter < template.Type_Names.length; counter++){ // Then Add the Other Options of Neuron
		if( Selected_Type_.equals(template.Type_Names[counter])){
		    Selected_Type = counter;
				    
		    // Then remakes combo box like new
		    remake();
		    
		    // Then set mode to appriorate type
		    if(Type == World_Object.Neuron){
			//template = new Dummy_Neuron();
			//addItem(new String("new Neuron"));
			GUI.ViewTypePane.graph.current_neuron = new Dummy_Neuron(Selected_Type);
			GUI.ViewTypePane.graph.neuron_making_state = 0;
			GUI.ButtonPane.mode = GUI.ButtonPane.create_neuron_mode;
			GUI.ViewTypePane.graph.current_neuron.fill_in_stats();
			GUI.status_bar.setText("Please click the Network you want the "+arrays.Neuron_Names[Selected_Type]+" to be connected to");
		    }else if(Type == World_Object.Synapse){
			//ViewTypePane.graph.template = new Link();
			GUI.ViewTypePane.graph.current_synapse = new Link(Selected_Type);
			GUI.ButtonPane.mode = GUI.ButtonPane.create_synapse_mode;
			GUI.ViewTypePane.graph.synapse_making_state = 0;
			GUI.ViewTypePane.graph.current_synapse.fill_in_stats();
			GUI.status_bar.setText("Please click the Neuron the Synapse is to be Coming From");
		    }else if(Type == World_Object.Network){
			//template = new Dummy_Network();
			GUI.ButtonPane.mode = GUI.ButtonPane.start;
			GUI.refresh_graphics();
			Dummy_Network dummy1 = new Dummy_Network(Selected_Type);
			dummy1.connect();
			World_Object.add_component((World_Object) GUI.world, (Object)dummy1);
			dummy1.fill_in_stats();
		    }
		}
	    }
	}
    }
    
    /** this destroys then rebuilds the combo box to get ordering right */
    void remake(){
	removeAllItems();

	if(Type == World_Object.Neuron){
	    template = new Dummy_Neuron();
	    addItem(new String("new Neuron"));
	}else if(Type == World_Object.Synapse){
	    template = new Link();
	    addItem(new String("new Synapse"));
	}else if(Type == World_Object.Network){
	    template = new Dummy_Network();
	    addItem(new String("new Network"));
	}

	// populate the combo box with the different types of object
	for(int counter =0; counter < template.Type_Names.length;counter++){ // Then Add the Other Options of object
		addItem(template.Type_Names[counter]);
	}
    }
}
