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

/** These Classes Make Windows and swing Parts
 * By Anthony Mckale 2003  - this contains all the GUI windows
 * this pane is the panel with the buttons on the left hand side
 * so far this only has a buttons tab */

public class ButtonPane extends JTabbedPane {
    static int start = 0;
    static int select_mode = 1;
    static int move_mode = 2;

    static int create_neuron_mode = 3;
    static int create_synapse_mode = 4;
    
    static int remove_mode = 5;

    static int global_Vars_mode = 8;

    static int mode = start;
    
    /* Init-ing the buttons tab */
    /** Main Panel on left hand side if GUI */
    public JPanel buttons = new JPanel();
    
    /** This is the position that the above info panel will be inserted
     * is used for inserting/removing info */
    public int hightlighted_info_pos = 2;//5;

    public ButtonPane() {
	/* This Bit only sets up the buttons Tab other tabs will follow */
	GUI.setNativeLookAndFeel();
	
	/* Init-ing the buttons tab */
	
	// Tool Bar
	//JToolBar jtoolbar = new JToolBar("Tool Bar");
	
	JPanel inside_toolbar = new JPanel();
	JPanel top_toolbar = new JPanel();
	JPanel bottom_toolbar = new JPanel();
	
	JLabel title = new JLabel("Mode Selection Area"); // Small DeScription of the Pane
	
	JButton select = new JButton(new ImageIcon("icon/select.gif"));
	JButton move = new JButton(new ImageIcon("icon/move.gif"));
	JButton remove = new JButton(new ImageIcon("icon/remove.gif"));
	JButton global_Vars = new JButton(new ImageIcon("icon/world.gif"));
	
	JPanel hightlighted_info = new JPanel();
	
	
	select.addActionListener(new ActionListener() {
		public void actionPerformed(ActionEvent event){
		    mode = select_mode;
		    GUI.status_bar.setText("Click the first you want to select");
		}
	    });
	select.setToolTipText("Select");
	
	move.addActionListener(new ActionListener(){
		public void actionPerformed(ActionEvent event){
		    mode = move_mode;
		    GUI.status_bar.setText("Click and Drag to move Stuff");
		}
	    });
	
	remove.setToolTipText("Remove");
		
	remove.addActionListener(new ActionListener(){
		public void actionPerformed(ActionEvent event){
		    mode = remove_mode;
		    GUI.status_bar.setText("Click what u want to remove");
		}
	    });

	move.setToolTipText("Move");
	
	global_Vars.addActionListener(new ActionListener(){
		public void actionPerformed(ActionEvent event){
		    mode = global_Vars_mode;
		    GUI.ViewTypePane.graph.first_refresh = true;
		    GUI.status_bar.setText("In Global Variable Changing Area");
		    GUI.world.fill_in_stats();
		}
	    });
	global_Vars.setToolTipText("Editor World Variables");
	GUI.setLayoutVert(buttons);//.setLayout(new BoxLayout(buttons,BoxLayout.Y_AXIS));
		
	/******************* GUI Hack *************/
	
	// The Buttons werent looking nice so i introducted this hack into improve the layout
	//JPanel Hack = new JPanel();
	//Hack.setLayout(new BoxLayout(Hack, BoxLayout.X_AXIS));
	//Hack.add(title);
	//Hack.add(new JLabel("ok"));
	//buttons.add(Hack);
	
	
	// This is the buttons toolbar
	//top bit
	top_toolbar.add(select);
	top_toolbar.add(move);
	
	top_toolbar.add(remove);
	top_toolbar.add(global_Vars);

	GUI.setLayoutVert(inside_toolbar);//.setLayout(new BoxLayout(inside_toolbar,BoxLayout.Y_AXIS));
	inside_toolbar.add(top_toolbar);
	
	//bottom bit
	inside_toolbar.add(new choose_object_box(World_Object.Synapse));
	inside_toolbar.add(new choose_object_box(World_Object.Neuron));
	inside_toolbar.add(new choose_object_box(World_Object.Network));
	
	// add everything to the buttons bar
	buttons.add(title);
   
	buttons.add(inside_toolbar);

	GUI.setLayoutVert(hightlighted_info);//.setLayout(new BoxLayout(hightlighted_info,BoxLayout.Y_AXIS));
	buttons.add(hightlighted_info, hightlighted_info_pos);
	
	addTab("Buttons", buttons);
    }
    
    /** This updates the info field in the buttons tab, 
     * <i>note :-  it is important the int variable hightlighted_info_pos 
     * to be the correct position of where the info_panel where stats
     * of the currently Selected and to be created objects are put</i> */
    
    // void updateInfoPanel(JScrollPane new_info) {
    public void updateInfoPanel(JPanel new_info) {
	buttons.remove(hightlighted_info_pos);
	buttons.add(new_info, hightlighted_info_pos);
	repaint();
	
    }
}
