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

/** By Anthony Mckale 2003 - this in the class which list parameter descriptions are stored in */
public class variable_list extends Variable  implements ItemListener {
        
    /** This is shows the member of the combo box currented highlighted at all times*/
    private  double ComboSelected =0;
    
    /** default Constructor */
    public variable_list(String Name, LinkedList List, double Default) {
	this.Name = Name;
	this.List = List;
	Integer DefaultO = new Integer((int)Default);

	this.Default = (Object)DefaultO;
	this.serial = arrays.Counter++;
    }
    
    /** Method for checking if input is valid or not */
    public boolean valid(Object input){
	try{
	    int test = ((Integer)input).intValue();
	    if(List.size() > test && test > -1)
		return true;
	    else
		return false;
	    }catch(Exception e){
	    return false;
	}
    }
    
    /** Methods for producing a panel, add componets on to a existing JPanel pasted thru */
    public void make_panel(JPanel editing, vars current ,vars defaulted){
	if (check_two_vars(current, defaulted)){
	    int selected;
	    if(current.inherrited) selected = ((Integer)defaulted.get()).intValue();
	    else selected = ((Integer)current.get()).intValue();
	    // Make ComboBox with selected item first then the other items
	    panel = new JPanel();
	    GUI.setLayoutHoz(panel);

	    panel.add(new JLabel((String)List.get(selected)));

	    JComboBox combo_panel = new JComboBox();
	    combo_panel.addItemListener(this);
	    
	    if(current.inherrited){
		((JComboBox)combo_panel).addItem("Defaulted");
		ComboSelected = Double.NaN; 
	    }else{
		((JComboBox)combo_panel).addItem(List.get(selected));
		ComboSelected=(int)selected;
	    }
	 
	    for(int counter = 0; counter < List.size(); counter++){
		    combo_panel.addItem(List.get(counter));
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
	    if(Selected_Type.equals("Defaulted")){
		ComboSelected = Double.NaN; 
	    }else{
		for(int counter =0; counter < List.size(); counter++){ // Then Add the Other Options of Neuron
		    if( Selected_Type.equals((String)List.get(counter))){
			ComboSelected = (int)counter;
		    }
		}
	    }
	}
    }
    
    
}
