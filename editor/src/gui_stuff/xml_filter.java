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


/** File filters
 * this prosumes that xml have .xml as a extension */

class xml_filter extends javax.swing.filechooser.FileFilter{
    public boolean accept(File fileobj){
	String extension = "";

	if(fileobj.getPath().lastIndexOf('.')>0)
	    extension = fileobj.getPath().substring(fileobj.getPath().lastIndexOf('.') + 1).toLowerCase();
	
	if(extension != "")
	    return extension.equals("xml");
	else 
	    return fileobj.isDirectory();
    }

    public String getDescription(){
	return "XML files (*.xml)";
    }
}
