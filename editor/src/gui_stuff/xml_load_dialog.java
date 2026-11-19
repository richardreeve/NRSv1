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

/** File Chooser                          */

public class xml_load_dialog{
    JFileChooser filechooser = new JFileChooser();
    public xml_load_dialog(){
	if(GUI.current_open_file.lastIndexOf('/') != -1)
	    filechooser.setCurrentDirectory(new File(GUI.current_open_file.substring(0,GUI.current_open_file.lastIndexOf('/'))));
	filechooser.setApproveButtonText("Load xml file"); 
	filechooser.addChoosableFileFilter(new xml_filter());
	filechooser.setDialogTitle("load xml"); 
	pop_up_dialog();
    }
    
    public void pop_up_dialog(){
	int result = filechooser.showOpenDialog(null);
	if(result == filechooser.APPROVE_OPTION){
	    // arrays.parse_xml_file(current_open_file);
	    String file  = filechooser.getSelectedFile().getPath();
	    arrays.parse_xml_file(file);
	    GUI.current_open_file = file;
	    GUI.status_bar.setText(file+"has been loaded into Simulator");
	}
    }
}
