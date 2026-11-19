package gui_stuff;

/* for package / links */
import misc.*; // for printing (doesnt work yet)
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

public class jpeg_save_dialog{
    JFileChooser filechooser = new JFileChooser();
    public jpeg_save_dialog(){
	if(GUI.current_open_file.lastIndexOf('/') != -1)
	    filechooser.setCurrentDirectory(new File(GUI.current_open_file.substring(0,GUI.current_open_file.lastIndexOf('/'))));
	filechooser.setApproveButtonText("Save"); 
	filechooser.setDialogTitle("save image of network as jpeg"); 
	pop_up_dialog();
    }
    
    public void pop_up_dialog(){
	int result = filechooser.showOpenDialog(null);
	if(result == filechooser.APPROVE_OPTION){
	    try{
		jpeg_maker a = new jpeg_maker(filechooser.getSelectedFile().getPath(), GUI.ViewTypePane.graph.bi); 
		GUI.status_bar.setText("The image of the current network has been saved as a jpeg to "+filechooser.getSelectedFile().getPath()+".");
	    }catch(Exception e){
		GUI.status_bar.setText("Image writing failed for file "+filechooser.getSelectedFile().getPath()+".");
	    }
	}
    }
}
