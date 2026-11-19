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

import java.io.*; // For File Chooser


/** The is the panel with the graphical representations of the Networks
 * Ie the middle one */ 

public class ViewTypePane extends JTabbedPane implements ChangeListener {
    static int a=0;
    /** Hack - to prevent system notancing action when in middle of building gui*/
    private boolean Buzy = true;
    /** Hack - to alow for going back to orginal source without change*/
    private boolean dont_rebuild_source =false;


    public Graphical_Neuron_window graph = new Graphical_Neuron_window();
    public Source_Viewer source_viewer = new Source_Viewer();
    /** Default Constructer */
    public ViewTypePane(){
	GUI.setNativeLookAndFeel();
	addTab("Graphical View", graph);
	addTab("Source View", source_viewer);
	addChangeListener(this);
    }
    
/** Hack Contitued - should ignore any thing done when in not Buzy mode */
    public void stateChanged(ChangeEvent e) {
	ViewTypePane source = (ViewTypePane)e.getSource();
	int choosen = source.getSelectedIndex();
	if(GUI.debug_mode) if(Buzy) System.out.println(this.getTitleAt(choosen));
	if(choosen == 0){
	    if(GUI.source_editible && Buzy) {
		if(Source_Viewer.changed){
		    // Bring up dialog for Editing
		    //Custom button text
		    Object[] options = {"Cancel", "Discard", "Parse"};
		    String Text = "Changes have been made to the source.\n"
			+"Would you like to parse the text or discard these changes ?";
		    int reply = JOptionPane.showOptionDialog(this, 
							 Text,
							 "Change In Source XML",
							 JOptionPane.YES_NO_CANCEL_OPTION,
							 JOptionPane.QUESTION_MESSAGE,
							 null,
							 options,
							 options[0]);
		    if(GUI.debug_mode) System.out.println("User choose "+reply);
		    if(reply == 0){ // Cancel
			dont_rebuild_source =true;
			setSelectedIndex(1);
		    }
		    if(reply == 1){ // Discard
			// Do nothing :D
		    }
		    if(reply == 2){ // Parse
			reply = arrays.try_write_xml_file();
			if(reply == 0){ // Great text parsed with no problem
			    // go to graphics and tell user the great news
			    GUI.status_bar.setText("Changed source parsed with no problems.");
			} else if(reply == 1){ // Oh dear there was a problem when parsing the text
			    // Go back to text and tell user the problem
			    dont_rebuild_source =true;
			    setSelectedIndex(1);
			    GUI.status_bar.setText("Changed source could not be parsed.");
			}
		    }
		}
	    }
	}
	if(choosen == 1){
	    if(dont_rebuild_source){ 
		dont_rebuild_source = false;
	    }else{
		if(Buzy){
		    Buzy = false;
		    remove(source_viewer);
		    //GUI.status_bar.setText("In "+new Integer(a).toString());a++;
		    source_viewer = new Source_Viewer();
		    addTab("Source View", source_viewer);
		    setSelectedIndex(1);
		    Buzy = true;
		}
	    }
	}
    }
    /*

    /** Hack Contitued 
    public void stateChanged(ChangeEvent e) {
	ViewTypePane source = (ViewTypePane)e.getSource();
	int choosen = source.getSelectedIndex();
	/**if(choosen == 0){//Graphical Window
		if(this.getTitleAt(0).equals("Graphical View")){
		    // Bring up dialog for Editing
		    //Custom button text
		    Object[] options = {"Cancel", "Discard", "Parse"};
		    String Text = "Changes have been made to the source.\n"
			+"Would you like to parse the text or discard these changes ?";
		    int n = JOptionPane.showOptionDialog(this, 
							 Text,
							 "Change In Source XML",
							 JOptionPane.YES_NO_CANCEL_OPTION,
							 JOptionPane.QUESTION_MESSAGE,
							 null,
							 options,
							 options[0]);
		    System.out.println(n);
		}
		}
	
	if(choosen == 1){// Source
	    Buzy = false;
	    remove(source_viewer);
	    a++;
	    source_viewer = new Source_Viewer();
	    addTab("Source View", source_viewer);
	    setSelectedIndex(1);
	    Buzy = true;
	}
    }
    */
}
