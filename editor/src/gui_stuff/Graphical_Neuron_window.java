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

/** This is a canvas which shows the Network and File Structure of the Network */    
public class Graphical_Neuron_window extends Canvas implements MouseListener, MouseMotionListener {
    
    BufferedImage bi;
    Graphics2D big;
    Graphics2D refresh_part;
    BufferedImage refresh_par;
    
    /** Holds the coordinates of the user's last mousePressed event.*/
    int last_x, last_y;
    /** For updating the graphicals the first time */
    boolean firstTime = true;
    /** for hit detection */
    Rectangle area;
    
    TexturePaint fillPolka, strokePolka;// obolsete
    
    /** For keeping count of the stages in synapse creation
     * 0 = start, 1 = 1st Neuron Choosen, 2 = 2nd Neuron Choosen, <3 = null */
    public int synapse_making_state = 0;
    World_Object Link1;
    World_Object Link2;

    /** For keeping count of the stages in Neuron creation
     * 0 = start, 1 = 1st Network Choosen, 2 = 2nd Placement Choosen, <3 = null */
    public int neuron_making_state = 0;
    
    /** Drawing States - this is set to true if it is the first refresh set true every time new move started */
    public boolean first_refresh = true;
        
    /** True if the user pressed, dragged or released the mouse outside of the neuron; false otherwise */
    boolean pressOut = false;
    
    /** for hit detection and Object Creation */
    public World_Object popup_hit;
    World_Object current_hit;
    Dummy_Network current_network;
    Dummy_Neuron current_neuron;
    Link current_synapse;
    int Hit = 0;
    
    public Graphical_Neuron_window(){
	GUI.setNativeLookAndFeel();
	setBackground(Color.white);
	addMouseMotionListener(this);
	addMouseListener(this);
	

	// Alot of this code stolen from bits and places
	// hence there are obsolete bits such as this with no
	// purpose that i could be bothered removing
	// and debuging the changes
	
	bi = new BufferedImage(5, 5, BufferedImage.TYPE_INT_RGB);
	big = bi.createGraphics();
	big.setColor(Color.pink);
	big.fillRect(0, 0, 7, 7);
	Rectangle r = new Rectangle(0,0,5,5);
	fillPolka = new TexturePaint(bi, r);
	big.dispose();
	
	//Creates the stroke texture paint pattern.
	bi = new BufferedImage(5, 5, BufferedImage.TYPE_INT_RGB);
	big = bi.createGraphics();		
	big.setColor(Color.cyan);
	big.fillRect(0, 0, 7, 7);
	r = new Rectangle(0,0,5,5);
	strokePolka = new TexturePaint(bi, r);
	big.dispose();	
    }


    // Methods to make everything a bit easier to check what mode ur in
    public boolean in_select_mode () { return GUI.ButtonPane.mode == GUI.ButtonPane.select_mode;}
    public boolean in_move_mode () { return GUI.ButtonPane.mode == GUI.ButtonPane.move_mode;}
    public boolean in_remove_mode () { return GUI.ButtonPane.mode == GUI.ButtonPane.remove_mode;}

    public boolean in_create_neuron_mode () { return GUI.ButtonPane.mode == GUI.ButtonPane.create_neuron_mode;}
    public boolean in_create_synapse_mode () { return GUI.ButtonPane.mode == GUI.ButtonPane.create_synapse_mode;}
    
    // Handles the event of the user pressing down the mouse button.
    public void mousePressed (MouseEvent e) { // note to people looking at this code i hate java becuz it has no pointers
	if ( in_move_mode() ) { // if in move mode
	    // Checks whether or not the cursor is inside of the neuron while the user is pressing the mouse.
	    current_hit = GUI.world.hit(e.getX(), e.getY());
	    if ( current_hit != null ) {	
		// This sets info Panel in the buttons Panel
		current_hit.fill_in_stats();
		first_refresh = true;
		if(current_hit.Object_Type == World_Object.Neuron)// instanceof Dummy_Neuron)
		    GUI.world.Currently_Selected_List = ((Dummy_Neuron)current_hit).get_list_of_connected_stuff();
		last_x = current_hit.x - e.getX();
		last_y = current_hit.y - e.getY();
		
		GUI.status_bar.setText(current_hit.name + " selected.");
		
		Hit = current_hit.Object_Type;
		
		updateLocation(e, current_hit);
		repaint();
	    } else {
		GUI.ButtonPane.updateInfoPanel(new JPanel());
		pressOut = true;
	    }    
	}
    }
    
    // Handles the event of a user dragging the mouse while holding down the mouse button.
    public void mouseDragged(MouseEvent e){
	if(in_move_mode()){
	    if(!pressOut){
		    updateLocation(e, current_hit); // This is teh generic world object mover
	    } else {
		GUI.status_bar.setText("First position the cursor on the rectangle and then drag.");
	    }
	}
    }

    // Handles the event of a user releasing the mouse button.
    public void mouseReleased(MouseEvent e){
	//first_refresh = true;
	if(e.isMetaDown()){// Right Click
	    popup_hit = GUI.world.hit(e.getX(), e.getY());
	    if ( popup_hit != null ) {	
		final JPopupMenu popupMenu = popup_hit.getPopUpMenu();
		
		popupMenu.setDefaultLightWeightPopupEnabled(false);
		popupMenu.show(this,e.getX(),e.getY());
		repaint();
	    }
	}else{ // Any other button
	    if(in_move_mode()){
		GUI.world.Currently_Selected_List = new LinkedList();
		first_refresh = true;
		// Checks whether or not the cursor is inside of a neuron when the user releases the mouse button.
		if(current_hit != null && current_hit.hit(e.getX(), e.getY()) != null){ 
		    current_hit.fill_in_stats(); // Sets the info panel to that neuron
		    updateLocation(e, current_hit); // This is the generic world object location updaer
		}else{
		    GUI.status_bar.setText("First position the cursor on a component and then drag.");
		    pressOut = false;
		}
	    }else if (in_select_mode()){ // Select a component
		current_hit = GUI.world.hit(e.getX(), e.getY());
		if(current_hit != null && current_hit.hit(e.getX(), e.getY()) != null){ 
		    GUI.world.Currently_Selected_List = new LinkedList();
		    GUI.world.Currently_Selected_List.add(current_hit);
		    current_hit.fill_in_stats();
		    repaint();
		}else{
		    GUI.ButtonPane.updateInfoPanel(new JPanel());
		    GUI.status_bar.setText("First position the cursor on a component and click to select.");
		}
	    }else if (in_remove_mode()){ // Select the component to remove
		
		World_Object remove = GUI.world.hit(e.getX(), e.getY());
		if(remove != null && remove.hit(e.getX(), e.getY()) != null){
		    String Text = "Removing world object " + remove.name + " from ";
		    Text = Text + remove.parent.name;
		    Text = Text + ".";
		    GUI.status_bar.setText(Text);
		    remove.remove();
		    GUI.world.Currently_Selected_List = new LinkedList();
		    GUI.ButtonPane.updateInfoPanel(new JPanel());
		}else{
		    GUI.status_bar.setText("First position the cursor on a component and click to remove.");
		}
		repaint();
	    }else if (in_create_neuron_mode()){ // Create a Neuron
		
		if(neuron_making_state == 0){ // Stage 1 : link network
		    current_hit = GUI.world.hit(e.getX(), e.getY());
		    if(current_hit != null && (current_hit.Object_Type == World_Object.Network)){
			current_network = (Dummy_Network)current_hit;// Verified this was true cast above
			current_neuron.parent = current_network;
			neuron_making_state++;
			GUI.status_bar.setText("Neuron "
					       +current_neuron.name
					       +" linked to network "+current_network.name+".");
		    }else{
			GUI.status_bar.setText("Please click on the Network you want Neuron "
					       +current_neuron.name+" to be part of.");
		    }
		}else if(neuron_making_state == 1){
		    // Makes new neuron inside selected network
		    current_neuron.x  = e.getX()-25;
		    current_neuron.y =  e.getY()-25;
		    World_Object.add_component((World_Object) current_network, (Object)current_neuron);
		    current_neuron.Connected = true;
		    check_Object_inbounds((World_Object)current_neuron); // Makes sure neuron inside the boundary area
		    GUI.status_bar.setText("Made new neuron in " 
					   + current_network.name 
					   + " click in more places to create other neurons in the same network.");
		    neuron_making_state++;
		    repaint(); 
		}else if(neuron_making_state == 2){
		    //If clicked on another Network obviously want to make a new neuron inside that network
		    current_hit = GUI.world.hit(e.getX(), e.getY());
		    if(current_hit != null && (current_hit.Object_Type == World_Object.Network)) {
			/********** Need method to copy current_neuron ***********/
			current_neuron = new Dummy_Neuron(current_neuron.Type);
			current_network = (Dummy_Network)current_hit;// Verified this was true cast above
			current_neuron.parent = current_network;
			current_neuron.Connected = true;
			neuron_making_state=1;
			GUI.status_bar.setText("Neuron "
					       +current_neuron.name
					       +" linked to network "+current_network.name+".");
		    }else{
			// Else they have clicked because they want to make another
			// Neuron inside the selected network
			
			// Makes new neuron inside selected network
			
			/********** Need method to copy current_neuron ***********/
			current_neuron = new Dummy_Neuron(current_neuron.Type);
			
			current_neuron.x  = e.getX()-25;
			current_neuron.y =  e.getY()-25;
			current_neuron.parent = current_network;
			current_neuron.Connected = true;
			World_Object.add_component((World_Object) current_network, (Object)current_neuron);
			check_Object_inbounds((World_Object)current_neuron); // Makes sure neuron inside the boundary area
			GUI.status_bar.setText("Made new neuron in " 
					       + current_network.name 
					       + " click in more places to create other neurons in the same network.");
			neuron_making_state = 2;
			repaint();
		    }
		}
		
	    }else if (in_create_synapse_mode()){ // Create a Synapse link
		
		if(synapse_making_state == 0){ // Stage 1 : link first neuron
		    current_hit = GUI.world.hit(e.getX(), e.getY());
		    if(current_hit != null && (current_hit.Object_Type == World_Object.Neuron)){
			// This sets info Panel in the buttons Panel
			//current_hit.fill_in_stats();
			
			last_x = current_hit.x - e.getX();
			last_y = current_hit.y - e.getY();
			
			Link1 = current_hit;// Verified this was true cast above
			synapse_making_state++;
			GUI.status_bar.setText("Synapse linked from "+Link1.name+".");
		    }else{
			GUI.status_bar.setText("Click the Neuron the synapse is to be connected from.");
		    }
		}else if(synapse_making_state == 1){ // Stage 2 : link second neuron and create synapses
		    current_hit = GUI.world.hit(e.getX(), e.getY());
		    if(current_hit != null && (current_hit.Object_Type == World_Object.Neuron)){	
			// This sets info Panel in the buttons Panel
			//current_hit.fill_in_stats();
			
			last_x = current_hit.x - e.getX();
			last_y = current_hit.y - e.getY();
			
			Link2 = current_hit; // Verified this was true cast above
			synapse_making_state=0; // reset state
			
			current_synapse.From = Link1;
			current_synapse.To = Link2;
			current_synapse.Connected = true;
			
			World_Object.add_component((World_Object) Link1, (Object)current_synapse);
			
			current_synapse = new Link(current_synapse.Type);
			current_synapse.fill_in_stats();
			GUI.status_bar.setText("Synapse linked from "+Link1.name+" to "+Link2.name+".");
			repaint();   
		    }else{
			GUI.status_bar.setText("Click the Neuron the synapse is to be connected to.");
		    }
		}
	    }
	}
    }
    
    // This method required by MouseListener.
    public void mouseMoved(MouseEvent e){}

    // These methods are required by MouseMotionListener.
    public void mouseClicked(MouseEvent e){}
    public void mouseExited(MouseEvent e){}
    public void mouseEntered(MouseEvent e){}

    public void paint(Graphics g){
	update(g);
    }
    
    /** Ok how this code works, firstTime is a boolean which is true at the first frame ever then false
     * ever afterwards, firstRefresh is used in move mode in the firstRefresh all non selected item cached
     * and not needed to be redrawn then set to false, when the user stops dragging then the firstRefresh 
     * is true again */
    public void update(Graphics g){
	Graphics2D g2 = (Graphics2D)g;
	if(firstTime){
	    Dimension dim = getSize();
	    int w = dim.width;
	    int h = dim.height;
	    area = new Rectangle(dim);
	    bi = (BufferedImage)createImage(w, h);
	    big = bi.createGraphics();
	    firstTime = false;

	    refresh_par = (BufferedImage)createImage(w, h);
	    refresh_part = (Graphics2D) refresh_par.getGraphics();
	} 
	
	// Clears that was previously drawn.
	big.setColor(Color.white);
	big.clearRect(0, 0, area.width, area.height);

	if(in_move_mode()){
	    if(first_refresh){
		refresh_part.setColor(Color.white);
		refresh_part.clearRect(0, 0, area.width, area.height);
		GUI.world.draw_non_selected(refresh_part);
		first_refresh = false;
	    }
	    
	    big.drawImage(refresh_par, 0, 0, this);
	    GUI.world.draw_selected(big);
	    g2.drawImage(bi, 0, 0, this); // Draws the buffered image to the screen.
	}else{
	    // Draws World
	    GUI.world.draw(big, 0);
	    g2.drawImage(bi, 0, 0, this); // Draws the buffered image to the screen.
	}
    }

    /** This method is called where a neuron is moved it makes sure that the neuron is not moved out of bounds */

    public void updateLocation(MouseEvent e, World_Object Object){
	if(Hit == World_Object.Neuron){
	    updateNeuronLocation(e, current_hit);
	}else if(Hit == World_Object.Synapse){
	}else if(Hit == World_Object.Network){
	    
	} // Clicking anything else does nothing
    }

    /** Refresh - this method forces the screen to be drawn completely again */
    public void refresh(){
	firstTime=true;
	first_refresh=true;
	repaint();
    }
    /** This method is called where a neuron is moved it makes sure that the neuron is not moved out of bounds */
    
    public void updateNeuronLocation(MouseEvent e, World_Object Neuron){
	Neuron.x = last_x + e.getX();
	Neuron.y = last_y + e.getY();
	if (!check_Object_inbounds(Neuron)) //GUI.status_bar.setText("Neuron located at "+Neuron.x+", "+Neuron.y);
	    //else
	    GUI.status_bar.setText("Please don't try to drag the Neuron outside the area.");
	repaint();
    }

    /**
     * Checks if the Neuron is contained within the applet window.  If the rectangle
     * is not contained withing the applet window, it is redrawn so that it is adjacent
     * to the edge of the window and just inside the window. */

    boolean check_Object_inbounds(World_Object Neuron){
	if (area == null)  return false;
	if(area.contains(Neuron.x, Neuron.y, 50, 50)) return true;
	
	int new_x = Neuron.x;
	int new_y = Neuron.y;
	
	if((Neuron.x+50)>area.width) new_x = area.width-51;
	if(Neuron.x < 0) new_x = 0;
	if((Neuron.y+50)>area.height) new_y = area.height-51;
	if(Neuron.y < 0) new_y = 0;
	
	Neuron.x = new_x;
	Neuron.y = new_y;
	
	return false;
    }
}
