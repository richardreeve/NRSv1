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

import javax.swing.event.*;

/** By Anthony Mckale 2003 - this in the class which number variable descriptions  are stored in
 * remember will need help menu option too */

public class variable_num extends Variable{
    public double Min;
    public double Max;
    public String Units;

    public boolean is_int=false;
    public JTextField input;

    /** default Constructor */
    public variable_num(String Name, double Min, double Max, double Default, String Units) {
	this.Name = Name;
	this.Min= Min;
	this.Max = Max;
	Double DefaultO = new Double(Default);
	if(!this.valid(DefaultO)){
	    System.err.println("Serious problem with sim description default for parameter "+Name+" invalid, default == "+DefaultO);
	    System.exit(1);
	}
	this.Default = (Object)DefaultO;
	this.Units = Units;
	this.serial = arrays.Counter++;
    }
    
    /** Method for checking if input is valid or not */
    public boolean valid(Object input) {
	/* works is all doubles == Doubles*/
	try{
	if(((Double)input).doubleValue() <= Max && ((Double)input).doubleValue() >= Min)
	    return true;
	return false;
	}catch(Exception e){
	    return false;
	}
    }
    
    
    
    /** Methods for producing a panel, add componets on to a existing JPanel pasted thru */
    public void make_panel(JPanel editing, vars current,  vars defaulted){
	// Make Panel with format - default value (for when defaulted to world/network
	// default) - current value - then units of measurement
	if (check_two_vars(current, defaulted)){
	    panel = new JPanel();
	    
	    GUI.setLayoutHoz(panel);
	    if(current.inherrited){
		panel.add(new JLabel(Double.toString(((Double)defaulted.get()).doubleValue())));
		input = new JTextField(" ",5);
	    }else{
		panel.add(new JLabel(Double.toString(((Double)current.get()).doubleValue())));
		input = new JTextField(Double.toString(((Double)current.get()).doubleValue()),5);
	    }
	    panel.add(input);
	    panel.add(new JLabel(Units));
	    
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
	try{
	    if(is_int){
		return (Object) new Double(new Integer(input.getText()).intValue());
	    }else{
		return (Object) new Double(input.getText());
	    }
	}catch(Exception e){ 
	    return (Object)new Double(Double.NaN);
	    // Cast will fail and come here if String not double
     	}
    }
}
