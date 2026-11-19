package variable_stuff;

/* for package / links */
import misc.printer; // for printing (doesnt work yet)
import world_objects.*;// for node/container/link objects
import gui_stuff.*;//Stuff for drawing/making gui
import xml_parser.*;//Stuff for inputing/output xml files/reading xml types description file
import main_method.GUI; // Main Method

import java.util.*; // For Linked Lists

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

/** By Anthony Mckale 2003 - this in the class which boolean variable descriptions  are stored in
 * remember will need help menu option too */

public class variable_bool extends Variable implements ItemListener {

     /** This is shows the member of the combo box currented highlighted at all times*/
    public double ComboSelected =0;

    /** default Constructor */
    public variable_bool(String Name, Integer Default) {
	this.Name = Name;
	this.List = new LinkedList();

	List.add((Object)"true");
	List.add((Object)"false");
	Integer DefaultO = Default;
	if(!this.valid(DefaultO)){
	    System.err.println("Serious problem with sim description default for Bool parameter "+Name+" invalid, default == "+DefaultO);
	    System.exit(1);
	}
	this.Default = (Object)DefaultO;
	this.serial = arrays.Counter++;
    }

    /** default Constructor */
    public variable_bool(String Name, double Default) {
	this.Name = Name;
	this.List = new LinkedList();
	List.add((Object)"true");
	List.add((Object)"false");
	Integer DefaultO = new Integer((int)Default);
	if(!this.valid(DefaultO)){
	    System.err.println("Serious problem with sim description default for Bool parameter "+Name+" invalid, default == "+DefaultO);
	    System.exit(1);
	}
	this.Default = (Object)DefaultO;
	this.serial = arrays.Counter++;
    }
    
    /** Method for checking if input is valid or not */
    public boolean valid(Object input){
	try{
	    if(List.size() > ((Integer)input).intValue() &&  ((Integer)input).intValue()> -1)
		return true;
	    else
		return false;
	}catch(Exception e){
	    return false;
	}
    }
    
    /** Methods for producing a panel, add componets on to a existing JPanel pasted thru */
    public void make_panel(JPanel editing, vars selected, vars defaulted){
	if (check_two_vars(selected, defaulted)){
	    // Make ComboBox with selected item first then the other items
	    int select;

	    if(selected.inherrited) select = ((Integer)defaulted.get()).intValue();
	    else select = ((Integer)selected.get()).intValue();

	    panel = new JPanel();
	    GUI.setLayoutHoz(panel);

	    panel.add(new JLabel((String)List.get(select)));
	    
	    JComboBox combo_panel = new JComboBox();
	    ((JComboBox)combo_panel).addItemListener(this);

	    if(selected.inherrited){
		((JComboBox)combo_panel).addItem("Defaulted");
		ComboSelected = Double.NaN; 
	    }else{
		((JComboBox)combo_panel).addItem(List.get(select));
		ComboSelected=(int)select;
	    }

	    for(int counter = 0; counter < List.size(); counter++){
		    ((JComboBox)combo_panel).addItem(List.get(counter));
	    }
	    ((JComboBox)combo_panel).addItem("Defaulted");
	    panel.add(combo_panel);
	    
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
	if(new Double(ComboSelected).equals(new Double(Double.NaN))){
	    return (Object)new String("Default");
	}else{
	    return (Object)new Integer((int)ComboSelected);
	}
    }

    /** This is called when an item is selected from the selection box */
    public void itemStateChanged(ItemEvent e){
	if(e.getStateChange() == ItemEvent.SELECTED){
	    String Selected_Type = (String)e.getItem();
	    // Inefficent but java doesnt give an index or allow for tagging items 
	    // in a combo box with a index or key (grrr) 
	    if( Selected_Type.equals("Defaulted")){
		ComboSelected = Double.NaN; 
	    }else{
		for(int counter =0; counter < List.size(); counter++){
		    if( Selected_Type.equals((String)List.get(counter))){
			ComboSelected = (double)counter;
		    }
		}
	    }
	}
    }
}
