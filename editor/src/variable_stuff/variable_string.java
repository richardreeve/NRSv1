package variable_stuff;

/* for package / links */
import misc.printer; // for printing (doesnt work yet)
import world_objects.*;// for node/container/link objects
import gui_stuff.*;//Stuff for drawing/making gui
import xml_parser.*;//Stuff for inputing/output xml files/reading xml types description file
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

import java.io.*; // For File Chooser

import javax.swing.event.*;

/** By Anthony Mckale 2003 - this in the class which boolean string descriptions  are stored in
 * remember will need help menu option too */


public class variable_string extends Variable{
    public boolean is_filename=false;
    public JTextField input = new JTextField(" ",5);

    /** default Constructor */
    public variable_string(String Name, String Default) {
	this.Name = Name;
	if(!this.valid(Default)){
	    System.err.println("Serious problem with sim description default for parameter "+Name+" invalid, default == "+Default);
	    System.exit(1);
	}
	this.Default = (Object)Default;
	this.serial = arrays.Counter++;
    }
    
    /** Method for checking if input is valid or not */
    public boolean valid(Object input) {
	/* works is all doubles == Doubles*/	
	return (input instanceof String);
    }
    
    
    
    /** Methods for producing a panel, add componets on to a existing JPanel pasted thru */
    public void make_panel(JPanel editing, vars current, vars defaulted){
	if (check_two_vars(current, defaulted)){
	    // Make Panel with format - default value (for when defaulted to world/network
	    // default) - current value - then units of measurement
	    panel = new JPanel();
	    GUI.setLayoutHoz(panel);
	    panel.add(new JLabel((String)defaulted.get()));
	    if(current.inherrited){
		input = new JTextField(" ",5);
	    }else{
		input = new JTextField((String)current.get(),5);
	    }
	    
	    panel.add(input);
	    // If filename type add option so can choose file
	    if(is_filename){
		JButton button = new JButton("getFile");
		button.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent event){
			    JFileChooser filechooser = new JFileChooser();
			    filechooser.setDialogTitle("get file name"); 
			    int result = filechooser.showOpenDialog(null);
			    if(result == filechooser.APPROVE_OPTION){
				String file  = filechooser.getSelectedFile().getPath();
				input.setText(file);
			    }
			}});
		button.setToolTipText("Click here to bring up a get file prompt");
		panel.add(button);
	    }
	    // Add Title and tooltip then other bits
	    JPanel a = new JPanel();
	    GUI.setLayoutHoz(a);

	    ImageIcon question_icon = new ImageIcon("icon/question.gif");
	    JButton tooltip = new JButton(question_icon);
	    tooltip.setToolTipText(desc);
	    tooltip.setContentAreaFilled(false);
	    a.add(tooltip);
	    a.add(new JLabel(Name));

	    // Add Everything Now
	    editing.add(a);
	    editing.add(panel);
	}else{
	    System.err.println("Doh ! Tried to assign param a value for another type");
	}
    }
    
    /** get input from info box*/
    public Object getInput(){
	if(input.getText() == null)
	    return (Object)"";
	else
	    return (Object)input.getText();
    }
}
