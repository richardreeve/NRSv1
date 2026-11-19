/***************************************************************************************************/
/* By Anthony Mckale 2003  - this is contains all the world objects for internal representation of the neuron system */
/***************************************************************************************************/

package world_objects;

/* for package / links */
import misc.printer; // for printing (doesnt work yet)
import gui_stuff.*;//Stuff for drawing/making gui
import xml_parser.*;//Stuff for inputing/output xml files/reading xml types description file
import variable_stuff.*;//Stuff for representing variables of different types
import main_method.GUI; // Main Method

/* World Object includes */
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

import java.util.*; // For Linked Lists

/* Support Classes includes */

/*<none>*/

/**************************************************************/
/* World Classes for Synapses and Neurons Networks and Files */
/*************************************************************/

// One should remember that Java has a wierd way of storing variable witha half and half system
// of absolute and relative reference calling hence alot of the "arrays" here are just pointers to the 
// same array meaning very little space space "should" be used

// Also it means u should be very careful when changed arrays inside Objects to make sure
// ur not editing a table of parameters or something

// Also CHECK FOR NULLS (note mainly to myself) as they are used to convey information

/** Super Class :- all drawible objects (links/nodes/containers/world) a member of this class */

public class World_Object {

    /** For if connected or not
     * this is a hack from where object were create when the user had decided when to use them
     * now objects are pre made with connections to other item made later
     * ie for a synapse to be connected means it does not have 
     * then when the object is created a copy of the non connected object is added */
    public boolean Connected = true;

    // the is how the synapses are counted
    // there is a frame that is the current frame
    /** used for telling how many synapse are connected */ 
    // at that point in draw hence the offset that should be applyed

    /** for creating a unquie id */
    static int id_ref =0;
    /** Unique Serial for it's self so when refreshing the info bar no vars is updates another by mistake*/
    public int id;
    /** This is used to stop Objects updating one another*/
    public static int current_id  = 0;
    
    /** global track keeper */
    public static int current_frame=0;
    /** objects current frame */
    public int frame=0;
    
    /** DrawingVar used counting number of synapse or stuff connected at point in time */
    public int incoming = 0;
    /** DrawingVar used counting number of synapse or stuff connected at point in time */
    public int outgoing = 0;
    
    /** Contains the parent of object */
    public World_Object parent;//parent = GUI.world; 
    
    // Since there should only be one world this increments
    // frame when draw called there
    
    public int x=0;
    public int y=0;
    
    /** String Name of Object */
    public String name = "World_Object - Default Name";

    /** String comments of object */
    public String comment = "";
    
    /** The int Object_Type specify's what type of Object
     * Ie "Neuron" or "Network"
     * The list below helps show/specify the values */
    public int Object_Type;
    public static final int Default = 0;
    public static final int Neuron = 1;
    public static final int Synapse = 2;
    public static final int Network = 3;
    public static final int World = 4;


    /** This is used to determine the draw mode (ie if to draw selected
     * neuron or not */
    public static final int Selected_Draw_Mode = 5;
    public static final int Normal_Draw_Mode = 6;
    
    public static final int Refresh_Mode = 7;
    
    /** Used for keeping track of the component inside object */
    public LinkedList list_of_Parts = new LinkedList();
    
    /** Used in hit detection */
    public World_Object  current_hit;
    
    // Used to Store Variables and Displaying Info Bar
    /** Pointer to List of Object Type's [Names] */
    public String Type_Names[];
    /** Pointer to List of Object Type's [Names][parameters] */
    public Object Parameters_used_by_Type[][];
    
    /** Where the Values of parameters of the Object should be Kept*/
    public vars vars[];

    /** The int Type specify's what type of Object that type of Object is
     * Ie Object "Motor Neuron" or "Spiking Neuron" of "Neuron" Object
     */
    public int Type = 0;

    /** Used for info_box */
    JTextField Name = new JTextField(name);
    /** Used for info_box */
    JTextArea Comments = new JTextArea(comment);

    /** This method is used to increment the global frame for drawing
     * To prevent over flow after while resets */
    public void increment_current_frame(){
	if(current_frame<100){
	    current_frame++;
	}else{
	    current_frame = 69;
	}
    }

    /** returns a string representation of the type */
    public String getType(){
	return "World Object";
    }
    
    /** This method sets up all the parameters initising them all, requires Parameters_used_by_Type set*/
    void setup_vars(){
	vars = new vars[Parameters_used_by_Type[Type].length]; 
	for (int counter =0; counter < Parameters_used_by_Type[Type].length; counter++) {
	    if(Parameters_used_by_Type[Type][counter] instanceof variable_num){
		vars[counter]= new vars(((variable_num)Parameters_used_by_Type[Type][counter]).serial,(Object)Parameters_used_by_Type[Type][counter]);
	    }else if(Parameters_used_by_Type[Type][counter] instanceof variable_list){
		vars[counter]= new vars(((variable_list)Parameters_used_by_Type[Type][counter]).serial,(Object)Parameters_used_by_Type[Type][counter]);
	    }else if(Parameters_used_by_Type[Type][counter] instanceof variable_string){
		vars[counter]= new vars(((variable_string)Parameters_used_by_Type[Type][counter]).serial,(Object)Parameters_used_by_Type[Type][counter]);
	    }else if(Parameters_used_by_Type[Type][counter] instanceof variable_bool){
		vars[counter]= new vars(((variable_bool)Parameters_used_by_Type[Type][counter]).serial,(Object)Parameters_used_by_Type[Type][counter]);
	    }	
	}
    }

    /** this adds a compent to a network (world type or network type or Neuron(for synapses only)) 
	while maintaining the list of neurons/synapses/networks contained in world*/
    public static void add_component(World_Object network, Object to_be_added){
	boolean valid = false;
	if(((World_Object)network).Object_Type == World_Object.World ){
	    if(GUI.debug_mode) System.out.println(network.name);
	    ((Dummy_World)network).list_of_Parts.add(to_be_added);
	    valid = true;
	}else if(((World_Object)network).Object_Type == World_Object.Network ){
	    ((Dummy_Network)network).list_of_Parts.add(to_be_added);
	    valid = true;
	}else if(((World_Object)network).Object_Type == World_Object.Neuron ){
	    ((Dummy_Neuron)network).list_of_Parts.add(to_be_added);
	    if(((World_Object)to_be_added).Object_Type == World_Object.Synapse )
		GUI.world.Synapse_List.add(to_be_added);
	}

	if(valid){
	    if(((World_Object)to_be_added).Object_Type == World_Object.Neuron )
		GUI.world.Neuron_List.add(to_be_added);
	    if(((World_Object)to_be_added).Object_Type == World_Object.Synapse )
		GUI.world.Synapse_List.add(to_be_added);
	    if(((World_Object)to_be_added).Object_Type == World_Object.Network )
		GUI.world.Network_List.add(to_be_added);
	}
	((World_Object)to_be_added).parent = network;
    }
    
    /** Update Parameter, this takes a parameter name given as a String and if that 
     * parameter is inside this terminals description it updates the parameter to the String value
     * validating this value to be allowed given and converting it to apprioate class */
    
    public void Update_Parameter(String Parameter_name, String value){
	// vars is the list of all the parameters this terminal contains
	for(int counter=0; counter < vars.length;counter++){
	    if(vars[counter].variable.Name.equals(Parameter_name)){
		set(vars[counter], value);
		break;
	    }
	}
	if(Object_Type == World_Object.Network){
	    for(int counter=0; counter < ((Dummy_Network)this).list_of_nodes_links_vars.length;counter++){//list_of_nodes_links_vars
		if(((Dummy_Network)this).list_of_nodes_links_vars[counter]!= null)
		    if(((Dummy_Network)this).list_of_nodes_links_vars[counter].variable.Name.equals(Parameter_name)){
			set(vars[counter], value);
			break;
		    }
	    }
	}
	if(Object_Type == World_Object.World){
	    for(int counter=0; counter < ((Dummy_World)this).list_of_nodes_links_vars.length;counter++){//list_of_nodes_links_vars
		if(((Dummy_World)this).list_of_nodes_links_vars[counter]!= null)
		    if(((Dummy_World)this).list_of_nodes_links_vars[counter].variable.Name.equals(Parameter_name)){
			set(vars[counter], value);
			break;
		    }
	    }
	}
    }
    
    /** Unprotected method for setting parameter (only really used to reduce coupling so only need to change
     * code in one place */
    private void set(vars a, String b){
	if(a.variable instanceof variable_string){
	    if(GUI.debug_mode) System.out.println("setting string");
	    a.set(b);
	}
	if(a.variable instanceof variable_bool){
	    if(GUI.debug_mode) System.out.println("setting bool");
	    a.set(new Integer(a.variable.List.indexOf(b)));
	}
	if(a.variable instanceof variable_list){
	    if(GUI.debug_mode) System.out.println("setting list");
	    a.set(new Integer(a.variable.List.indexOf(b)));
	}
	if(a.variable instanceof variable_num){
	    if(GUI.debug_mode) System.out.println("setting num");
	    a.set(new Double(b));
	}
    }
    
    /** This draws the networks/synapse then neurons in that order */
    public void draw (Graphics2D g2, int refresh_or_not) {//int incoming now obselete
	final int obselete = 0;
	increment_current_frame(); // This is for Network drawing (for knowing which frame in)

	if(GUI.world.Network_List != null)
	    for(int counter =0; counter < GUI.world.Network_List.size() ;counter++) {
		((Dummy_Network)GUI.world.Network_List.get(counter)).drawme(g2, Normal_Draw_Mode);
	    }
	if(GUI.world.Synapse_List != null)
	    for(int counter =0; counter < GUI.world.Synapse_List.size() ;counter++) {
		((Link)GUI.world.Synapse_List.get(counter)).drawme(g2, Normal_Draw_Mode);
	    }
	if(GUI.world.Neuron_List != null)
	    for(int counter =0; counter < GUI.world.Neuron_List.size() ;counter++) {
		((Dummy_Neuron)GUI.world.Neuron_List.get(counter)).drawme(g2, Normal_Draw_Mode);
	    }
	if(GUI.world.Currently_Selected_List != null)
	    for(int counter =0; counter < GUI.world.Currently_Selected_List.size() ;counter++) {
		((World_Object)GUI.world.Currently_Selected_List.get(counter)).drawme(g2, Selected_Draw_Mode);
	    }
    }
    
    /** This draws the non selected networks/synapse then neurons in that order */
    public void draw_non_selected (Graphics2D g2) {//int incoming now obselete
	increment_current_frame(); // This is for Network drawing (for knowing which frame in)

	if(GUI.world.Network_List != null)
	    for(int counter =0; counter < GUI.world.Network_List.size() ;counter++) {
		((Dummy_Network)GUI.world.Network_List.get(counter)).drawme(g2, Normal_Draw_Mode);
	    }
	if(GUI.world.Synapse_List != null)
	    for(int counter =0; counter < GUI.world.Synapse_List.size() ;counter++) {
		((Link)GUI.world.Synapse_List.get(counter)).drawme(g2, Normal_Draw_Mode);
	    }
	if(GUI.world.Neuron_List != null)
	    for(int counter =0; counter < GUI.world.Neuron_List.size() ;counter++) {
		((Dummy_Neuron)GUI.world.Neuron_List.get(counter)).drawme(g2, Normal_Draw_Mode);
	    }
    }

    /** This draws the selected networks/synapse then neurons in that order (wont work with networks */
    public void draw_selected (Graphics2D g2) {//int incoming now obselete
    	if(GUI.world.Currently_Selected_List != null)
	    for(int counter =0; counter < GUI.world.Currently_Selected_List.size() ;counter++) {
		if(((World_Object)GUI.world.Currently_Selected_List.get(counter)).Object_Type == World_Object.Neuron )
		    ((Dummy_Neuron)GUI.world.Currently_Selected_List.get(counter)).drawme(g2, Selected_Draw_Mode);
		if(((World_Object)GUI.world.Currently_Selected_List.get(counter)).Object_Type == World_Object.Synapse )
		    ((Link)GUI.world.Currently_Selected_List.get(counter)).drawme(g2, Selected_Draw_Mode);
		if(((World_Object)GUI.world.Currently_Selected_List.get(counter)).Object_Type == World_Object.Network )
		    ((Dummy_Network)GUI.world.Currently_Selected_List.get(counter)).drawme(g2, Selected_Draw_Mode);
	    }
    }
    
    /** The method should be implemented seperately as this is the draw method for that object */
    void drawme (Graphics2D g2, int draw_mode) {
    	System.out.println("Draw method not implemented for this Object");
    }

    
    /** This default hit Recurively checks each Object contained then checks it's self */
    public World_Object hit(int X, int Y){
	if(GUI.debug_mode) System.out.println(name);
	if(GUI.debug_mode) System.out.println(list_of_Parts);
	if(list_of_Parts != null)
	    for(int counter =0; counter < list_of_Parts.size();counter++) {
		current_hit = (World_Object) list_of_Parts.get(counter);
		current_hit = current_hit.hit(X, Y);
		if(current_hit != null){
		    return current_hit;
		}
	    }
	return hitme(X, Y);
    }

    
    /* The method should be implemented seperately as this is the hit method for that object */
    World_Object hitme(int X, int Y){
	System.out.println("Hit me method not implemented for this Object");
	return null;
    }

    public JPopupMenu getPopUpMenu(){
	JPopupMenu popup_menu =  new JPopupMenu();
	JMenuItem remove_object = new JMenuItem("remove object");
	remove_object.addActionListener(new ActionListener(){
		public void actionPerformed(ActionEvent event){
		    String Text = "Removing world object " + GUI.ViewTypePane.graph.popup_hit.name + " from ";
		    Text = Text + GUI.ViewTypePane.graph.popup_hit.parent.name + ".";
		    GUI.status_bar.setText(Text);
		    GUI.ViewTypePane.graph.popup_hit.remove();
		    GUI.ViewTypePane.graph.refresh();
		}});
	popup_menu.add(remove_object);

	JMenu change_parent = new JMenu("change parent");
	popup_menu.add(change_parent);
	ButtonGroup group = new ButtonGroup();
	ActionListener groupActionListener = new ActionListener(){
		public void actionPerformed(ActionEvent event){
		    for(int counter = 0; counter < GUI.world.Network_List.size(); counter++){
			String returned = (String)((JMenuItem)event.getSource()).getText();
			String check_network = ((Dummy_Network)GUI.world.Network_List.get(counter)).name;
			if(returned.equals(check_network)){
			    GUI.status_bar.setText(GUI.ViewTypePane.graph.popup_hit.name+"'s parent changed to "+check_network);
			    GUI.ViewTypePane.graph.popup_hit.change_parent((World_Object)GUI.world.Network_List.get(counter));
			    GUI.ViewTypePane.graph.refresh();
			    counter = GUI.world.Network_List.size() + 6;
			}
		    }
		}
	    };
	for(int counter = 0; counter < GUI.world.Network_List.size(); counter++){
	    Dummy_Network network = (Dummy_Network)GUI.world.Network_List.get(counter);
	    JCheckBoxMenuItem network_Menu_Item = new JCheckBoxMenuItem(network.name);
	    group.add(network_Menu_Item); 
	    network_Menu_Item.addActionListener(groupActionListener);
	    change_parent.add(network_Menu_Item);
	}

	//JMenu change_colour = new JMenu("change colour");
	//popup_menu.add(change_colour);
	//JMenuItem change_colour_custom = new JMenuItem("change to custom colour");
	//popup_menu.add(change_colour_custom);
	return popup_menu;
    }
    
    /** This returns that default value for a parameter given it's uniqine serial */
    public vars get_inheritance(int serial){
	System.out.println("add method not implemented for this Object");
	return null;
    }
    
    /** This method will try to return a object based it's XPath ie 'world6.network6.neuron2' 
     * would return the neuron 'neuron2' */
    public World_Object XPath(String XPath){
	boolean leaf = false;
	if(GUI.debug_mode) System.out.println("Name: "+name+" XPath :"+XPath);
	int offset = XPath.indexOf('.');
	// Hack
	// This is for case where offset is big bigger than String XPath becuz it's at a end terminal 
	// with no '.' hence return bigger offset than possible 
	if(GUI.debug_mode) System.out.println(offset + " vs. "+XPath.length());
	if(offset == -1){
	    leaf = true;
	    if(GUI.debug_mode) System.out.println("leaf");
	} 
	for(int counter = 0; counter < this.list_of_Parts.size();counter++){
	    if(leaf){
		if(GUI.debug_mode) System.out.println(((World_Object)list_of_Parts.get(counter)).name + " vs. "+XPath);
		if(((World_Object)list_of_Parts.get(counter)).name.equals(XPath))
		    return (World_Object)list_of_Parts.get(counter);
	    }else{
		if(GUI.debug_mode) System.out.println(((World_Object)list_of_Parts.get(counter)).name + " vs. "+XPath.substring(0,offset));
		if(((World_Object)list_of_Parts.get(counter)).name.equals(XPath.substring(0,offset)))
		    return ((World_Object)list_of_Parts.get(counter)).XPath(XPath.substring(offset+1));
	    }
	}
	System.out.println("Damn");
	return null;
    }
    
    /** This sets the info panel (below the buttons tab of the right hand side) to show all the varaibles of this
     * Terminal and set up the listeners that allow them to be edited*/
    public void fill_in_stats(){
	/****************/
	/* INFO         BIT */
	/****************/

	GUI.world.Currently_Selected_List = new LinkedList();
	GUI.world.Currently_Selected_List.add((Object)this);

	// Hack
	Name = new JTextField(name);
	Comments = new JTextArea(comment);
	// End Hack
	    update_var_values(); // update current var values
	JPanel new_panel = new JPanel(); // Panel for everything to go in
	GUI.setLayoutVert(new_panel);//.setLayout(new BoxLayout(new_panel,BoxLayout.Y_AXIS));

	GUI.setNativeLookAndFeel();
	// Make Add Type Name
	JPanel selected_info_box = new JPanel();
	JLabel Type_Name; 
	if(Object_Type != Synapse){ // parent will be null or synapse hence other title needed
	    if(Connected){
		Type_Name = new JLabel(" - inside container - ");
		Type_Name.setHorizontalAlignment(JLabel.RIGHT);
		selected_info_box.add(Type_Name); // add Type
		Type_Name = new JLabel("["+parent.Type_Names[parent.Type] +"]"+parent.name);
		Type_Name.setHorizontalAlignment(JLabel.RIGHT);
		selected_info_box.add(Type_Name); // add Type
	    }else{
		Type_Name = new JLabel(" - UnConnected - ");
		Type_Name.setHorizontalAlignment(JLabel.RIGHT);
		selected_info_box.add(Type_Name); // add Type
	    }
	}else{
	    //Type_Name = new JLabel(Type_Names[Type]);
	    //Type_Name.setHorizontalAlignment(JLabel.LEFT);
	    //selected_info_box.add(Type_Name); // add Type
	    if(this.Connected){
		Type_Name = new JLabel("Linked From : "+((Link)this).From.name
				       +"["+((Link)this).From.Type_Names[((Link)this).From.Type] +"]");
		Type_Name.setHorizontalAlignment(JLabel.LEFT);
		selected_info_box.add(Type_Name); // add Type
		Type_Name = new JLabel("To : "+((Link)this).To.name
				       +"["+((Link)this).To.Type_Names[((Link)this).To.Type] +"]");
		Type_Name.setHorizontalAlignment(JLabel.LEFT);
	    }else{
		Type_Name = new JLabel("Not Currently Connected");
		Type_Name.setHorizontalAlignment(JLabel.LEFT);
		selected_info_box.add(Type_Name); // add Type
	    }
	    selected_info_box.add(Type_Name); // add Type
	    
	}
	
	selected_info_box.setBorder(BorderFactory.createTitledBorder("["+Type_Names[Type] +"]"+name));
	GUI.setLayoutVert(selected_info_box);//.setLayout(new BoxLayout(selected_info_box,BoxLayout.Y_AXIS));
	JScrollPane scrollbar2 = new JScrollPane(
						selected_info_box, 
						ScrollPaneConstants.VERTICAL_SCROLLBAR_AS_NEEDED, 
						ScrollPaneConstants.HORIZONTAL_SCROLLBAR_NEVER);
	//new_panel2.add(selected_info_box);
	   
	
	/****************/
	/* EDITING BIT */
	/****************/
	
	// This shows the parameters of the terminal
	
	JPanel selected_editing_box = new JPanel();

	// Make a new Info Panle to replace old one
	Name = new JTextField(name);
	Comments = new JTextArea(comment);
	Comments.setLineWrap(true);
	Comments.setRows(4);

	JButton set = new JButton("set changes");
     
	set.addActionListener(new ActionListener(){
		public void actionPerformed(ActionEvent event){
		    update_var_values();  // update current var values
		    fill_in_stats(); // redraw new info bar // inefficent but elegant
		}
	    });
	
	if(GUI.debug_mode) System.out.println(name);
	
	selected_editing_box.add(Name); // Name

	JPanel a = new JPanel();
	GUI.setLayoutHoz(a);
	a.add(new JLabel("Description"));// add Comment Box
	selected_editing_box.add(a);
	JScrollPane Comments_scrollbar = new JScrollPane(
						Comments, 
						ScrollPaneConstants.VERTICAL_SCROLLBAR_ALWAYS, 
						ScrollPaneConstants.HORIZONTAL_SCROLLBAR_NEVER);
	selected_editing_box.add(Comments_scrollbar);
	
	// For each variable for the terminal type of the selected terminal being edited, add it's change box to the editor panel
	// Code changes allow for Parameters_used_by_Type[Type][counter] to be replaced by vars.variable
	int Coun = 0;
	for(int counter =0; counter < Parameters_used_by_Type[Type].length;counter++){
	    ((Variable)Parameters_used_by_Type[Type][counter]).make_panel(selected_editing_box, vars[counter],get_inheritance(((Variable)Parameters_used_by_Type[Type][counter]).serial));
	}
       
	selected_editing_box.setBorder(BorderFactory.createTitledBorder("Selected's Parameters"));
	//new_panel2.setLayout(new GridBagLayout());
	GUI.setLayoutVert(selected_editing_box);//.setLayout(new BoxLayout(selected_editing_box,BoxLayout.Y_AXIS));
	
	/*******************************/
	/* INHERRIANCE EDITING BIT */
	/*******************************/
	
	// This lists for network and world all the params possible and allows u to change the default values
	// that it's childern will have
	
	JPanel defaults_editing_box = new JPanel();
	// Will need one for world too
	if(Object_Type== Network) {
	    Dummy_Network temp = (Dummy_Network)this;
	    Set hash_map =  temp.map_of_nodes_links_vars.entrySet();
	    Iterator iterator = hash_map.iterator();
	    
	    JPanel b = new JPanel();
	    GUI.setLayoutHoz(b);
	    b.add(new JLabel("Network Default Values"));
	    selected_editing_box.add(b);
	    
	    while(iterator.hasNext()){
		Map.Entry mapentry = (Map.Entry) iterator.next();
		vars temp_var = (vars)mapentry.getValue();
		((Variable)temp_var.variable).make_panel(selected_editing_box, temp_var, get_inheritance(temp_var.serial));
	    }
	}   
 
	if(Object_Type == World){
	    Dummy_World temp = (Dummy_World)this;
	    Set hash_map =  temp.map_of_nodes_links_vars.entrySet();
	    Iterator iterator = hash_map.iterator();

	    JPanel c = new JPanel();
	    GUI.setLayoutHoz(c);
	    c.add(new JLabel("World Default Values"));
	    selected_editing_box.add(c);

	    while(iterator.hasNext()){
		Map.Entry mapentry = (Map.Entry) iterator.next();
		vars temp_var = (vars)mapentry.getValue();
		((Variable)temp_var.variable).make_panel(selected_editing_box, temp_var, temp_var);
	    }
	}
	    
	// Enclose every thing inside scollbar
	JScrollPane scrollbar = new JScrollPane(
						selected_editing_box, 
						ScrollPaneConstants.VERTICAL_SCROLLBAR_ALWAYS, 
						ScrollPaneConstants.HORIZONTAL_SCROLLBAR_AS_NEEDED);
	/*******************************/
	/* PUTTING IT ALL TOGETHER */
	/*******************************/
	
	
	JPanel new_panel2 = new JPanel(); // Panel for everything to go in
	GUI.setLayoutVert(new_panel2);//.setLayout(new BoxLayout(new_panel2,BoxLayout.Y_AXIS));
	//GUI.setLayout(new_panel2);
	//new_panel2.setLayout(new GridBagLayout());

	new_panel2.add(scrollbar2);
	new_panel2.add(scrollbar);
	
	// Add the Set Variables Button
	new_panel2.add(set);

	//Replace old info panel
	GUI.ButtonPane.updateInfoPanel(new_panel2);
    }
    
    /** Method to remove this object from it's parent and erase any trace of it*/
    public void remove(){
	if(parent != null){
	    parent.list_of_Parts.remove((Object)this);

	    if(this.Object_Type == World_Object.Neuron){
		GUI.world.Neuron_List.remove((Object)this);
		for(int counter = 0; counter < GUI.world.Synapse_List.size();counter++){
		    if(((Link)GUI.world.Synapse_List.get(counter)).To == this 
		       || ((Link)GUI.world.Synapse_List.get(counter)).From == this){
			((World_Object)GUI.world.Synapse_List.get(counter)).remove();
		    }
		}
	    }

	    if(this.Object_Type == World_Object.Synapse )
	    	GUI.world.Synapse_List.remove((Object)this);
	    if(this.Object_Type == World_Object.Network ){
	    	GUI.world.Network_List.remove((Object)this);
	    
	    }
	
	}
	
	int a = list_of_Parts.size();
	for(int counter = 0; counter < a;counter++){
	    try{
		((World_Object)list_of_Parts.getFirst()).remove();
	    }catch(Exception e){
		// This is to get empty linkedlist errors
	    }
	}
    }
    
    /** This method will change the container the World_Object is inside */
    public void change_parent(World_Object new_parent){
	parent.list_of_Parts.remove((Object)this);
	parent = new_parent;
	parent.list_of_Parts.add((Object)this);
    }
    
    /** This method is to update the vars values from the info_panel */
    
    private void update_var_values(){
	// Only used to numercial values
	boolean global_error = false;
	String error_message = "";
	double test;
	if(current_id == id){
	// Update Name
	if(!name.equals("")) 
	    name = Name.getText();
	else { 
	    global_error = true;
	    error_message = "name (ie none set) , ";
	}// Known Bug if name set to "" then will crash nothing seems to work
	
	//Update Comments
	comment = Comments.getText();
	
	/** update values for normal Objects */

	/** This bit is quite confusing thou not nearly as confusing as the code b4 it, 
	 * it's been very simplifed at great time cost to make it appear readible 
	 * remember vars is the array of parameter values confusingly the class for storing 
	 * parameter values is also called vars */

	for (int counter =0; counter < vars.length; counter++) {
	    vars[counter].set(vars[counter].variable.getInput());
	}
	

	/** Both the word network work the same as the 
	
	/** INHERANCE FOR WORLD */
	if(Object_Type == World){
	    Dummy_World temp = (Dummy_World)this;
	    Set hash_map =  temp.map_of_nodes_links_vars.entrySet();
	    Iterator iterator = hash_map.iterator();
	    while(iterator.hasNext()){
		Map.Entry mapentry = (Map.Entry) iterator.next();
		vars temp_var = (vars)mapentry.getValue();
		temp_var.set(temp_var.variable.getInput());
		// World cannot be inherrited hence reset to default 
		if(temp_var.inherrited){
		    temp_var.inherrited = false;
		    temp_var.set(temp_var.variable.Default);
		}
	    }
	}
	
	/** INHERANCE FOR NETWORK */
	if(Object_Type == Network){
	    Dummy_Network temp = (Dummy_Network)this;
	    Set hash_map =  temp.map_of_nodes_links_vars.entrySet();
	    Iterator iterator = hash_map.iterator();
	    while(iterator.hasNext()){
		Map.Entry mapentry = (Map.Entry) iterator.next();
		vars temp_var = (vars)mapentry.getValue();
		temp_var.set(temp_var.variable.getInput());
	    }
	}
	
	// Show user results
	if(!global_error) GUI.status_bar.setText(getType()+" "+name+" updated");
	else GUI.status_bar.setText(getType()+" "+name+" updated, however errors were found in "+error_message);
	GUI.refresh_graphics();
	}
	current_id=id;
    }
}
