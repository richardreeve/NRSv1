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

/** File Chooser                      Obslete minus file filters which need to encaplsulated    */

public class filechooserfilter extends JPanel implements ActionListener{
    JFileChooser filechooser = new JFileChooser();
    JButton button = new JButton("Display dir chooser");
    JButton refresh = new JButton("write files");
    JTextField text = new JTextField(20);
    public filechooserfilter(){
	super();
	setLayout(new FlowLayout());
	
	button.setToolTipText("Double click on folders to recurse through them and select one and click open to choose it");
	add(button);
	
	refresh.addActionListener(new ActionListener(){
		public void actionPerformed(ActionEvent event){
		    //GUI.output(text.getText());
		}
	    });
	refresh.setToolTipText("Write format to file");
	add(refresh);
	
	add(text);

	button.addActionListener(this);
	
	
	//filechooser.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY); // to select directories only
	//filechooser.addChoosableFileFilter(new nfile_filter());
	filechooser.addChoosableFileFilter(new xml_filter());
    }
    
    public void actionPerformed(ActionEvent e){
	int result = filechooser.showOpenDialog(null);
	if(result == filechooser.APPROVE_OPTION){
	    arrays.parse_xml_file(filechooser.getSelectedFile().getPath());
	    GUI.status_bar.setText(filechooser.getSelectedFile().getPath()+"has been loaded into Simulator");
	}
    }
}


/** File filters
 * this prosumes that nfile have n as a extension */

class all_files extends javax.swing.filechooser.FileFilter{
    public boolean accept(File fileobj){
	String extension = "";

	if(fileobj.getPath().lastIndexOf('.')>0)
	    extension = fileobj.getPath().substring(fileobj.getPath().lastIndexOf('.') + 1).toLowerCase();
	
	if(extension != "")
	    return true;
	else
	    return fileobj.isDirectory();
    }

    public String getDescription(){
	return "All files (*)";
    }
}

/** File filters
 * this prosumes that imr files have imr as a extension */
/*
class imr_filter extends javax.swing.filechooser.FileFilter{
    public boolean accept(File fileobj){
	String extension = "";

	if(fileobj.getPath().lastIndexOf('.')>0)
	    extension = fileobj.getPath().substring(fileobj.getPath().lastIndexOf('.') + 1).toLowerCase();
	
	if(extension != "")
	    return extension.equals("imr");
	else
	    return fileobj.isDirectory();
    }

    public String getDescription(){
	return "imr files (*.imr)";
    }
}
*/
