package xml_parser;

import java.util.*; // For Linked Lists

import java.io.BufferedWriter; // writer
import java.io.FileWriter;

/* for package / links */
//import misc.printer; // for printing (doesnt work yet)
import world_objects.*;// for node/container/link objects
import variable_stuff.*;//Stuff for representing variables of different types
import main_method.GUI; // Main Method

/** Support Classes
 * By Anthony Mckale 2003 -
 * This is basically a class for holding desciptive Information
 * about the simulator and also for obtaining it
 * Ie String Names, Neuron Values etc */

public class XML_writer {
    static int indent = 0;
    
    /** Method for calling the parsing process */
    public XML_writer()  {
    }
    
    /** This will write the current world being edited to a xml file */
    public static void World_to_XML_file(String file, Dummy_World world) throws Exception{
	BufferedWriter writer = new BufferedWriter(new FileWriter(file));
	writer.write(make_XML_rep(world));
	writer.close();
    }
    
    /** This will return a string of the xml of current world being edited to a xml file */
    public static String World_to_XML_string(Dummy_World world) {
	indent = 0;
	// indent
	return indent()+"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"+make_XML_rep(world);
    }
    
    private static String indent(){
	String a = "";
	for(int counter = 0; counter < indent;counter++){
	    a = a+"    ";
	}
	return a;
    }
    
    private static String make_XML_rep(World_Object object){
	String send = "";
	switch(object.Object_Type){ 
	case World_Object.Default:
	    send = indent()+"<ERROR></ERROR>\n\n";
	    break;
	case World_Object.Neuron:
	    send=indent()+"<Terminal name = '"+object.name+
		"' type = '"+object.Type_Names[object.Type].toString()+"'>\n";
	    indent++;
	    send = send+Standard_Info(object)
		+Graphical_info(object);
	    indent--;
	    send = send+indent()+"</Terminal> \n\n"
		+write_out_list_of_parts(object);
	    break;
	case World_Object.Synapse:
	    if(((Link)object).To != null && ((Link)object).From != null ){
		send=indent()+"<Link name = '"+object.name+
		    "' type='"+object.Type_Names[object.Type].toString()+"' "
		    +Synapse_Link_Info((Link)object)+">\n";
		indent++;
		send = send+Standard_Info(object)
		    +Graphical_info(object);
		indent--;
		send = send+indent()+"</Link> \n\n";
	    }
	    break;
	case World_Object.Network:
	    send=indent()+"<Container name = '"+object.name+
		"' type = '"+object.Type_Names[object.Type].toString()+"'>\n";
	    indent++;
	    send = send+Standard_Info(object)+Graphical_info(object)
		+write_out_list_of_parts(object) ;
	    indent--;
	    send = send+indent()+"</Container> \n\n";
	    break;
	case World_Object.World:
	    send = indent()+"<World  name = '"+object.name+"'>\n";
	    indent++;
	    send = send+Standard_Info(object)+Graphical_info(object)+ write_out_list_of_parts(object);
	    indent--;
	    send = send+indent()+"</World>";
	    break;
	}
	return send;
    }
    
    /** Malformed Networks that do not end in world will cause endless loops */
    private static String Synapse_Link_Info(Link object){
	String send="";
	World_Object temp1;
	String To_Link="";
	temp1 = object.To;
	int counter = 0;
	To_Link = temp1.name;
	temp1 = temp1.parent;
	while(!(temp1.Object_Type == World_Object.World )){
	    To_Link = temp1.name+"."+To_Link;
	    temp1 = temp1.parent;
	    if(counter == 40){
		System.err.println("There is a problem with the synapse being process for XML file resursived too deep to find link name "); 
	    }
	}
	// This adds world name to XPath
	//To_Link = temp1.name+"."+To_Link;

	String From_Link="";
	temp1 = object.From;
	counter = 0;
	From_Link = temp1.name;
	temp1 = temp1.parent;
	while(!(temp1.Object_Type == World_Object.World )){
	    From_Link = temp1.name+"."+From_Link;
	    temp1 = temp1.parent;
	    if(counter == 40){
		System.err.println("There is a problem with the synapse being process for XML file resursived too deep to find link name "); 
	    }
	}
	// This adds world name to XPath
	//From_Link = temp1.name+"."+From_Link;
	
	send = send + "to = '"+To_Link+"' ";
	send = send + "from = '"+From_Link+"' ";
	return send;
    }
    
    private static String Standard_Info(World_Object object){
	String send="";
	if(object.Type_Names != null){
	    for (int counter =0; counter < object.Parameters_used_by_Type[object.Type].length; counter++) {
		if(!object.vars[counter].inherrited){
		    send = send +indent()+"<Param name = '"+ ((Variable)object.Parameters_used_by_Type[object.Type][counter]).Name 
			+ "' value = '"+object.vars[counter].get_String() + "'>\n";
		    send = send +indent()+"</Param>\n";
		}
	    }
	}

	/* If the object is a container*/
	if(object.Object_Type == World_Object.Network || object.Object_Type == World_Object.World){
	    Set hash_map;
	    if(object.Object_Type == World_Object.Network){   
		// if network
		hash_map =  ((Dummy_Network)object).map_of_nodes_links_vars.entrySet();
	    }else{
		// else World
		hash_map =  ((Dummy_World)object).map_of_nodes_links_vars.entrySet();
	    }
	    
	    Iterator iterator = hash_map.iterator();   
	    while(iterator.hasNext()){
		Map.Entry mapentry = (Map.Entry) iterator.next();
		vars temp = (vars)mapentry.getValue();    
		if(!temp.inherrited){
		    send = send +indent()+"<Param name = '"+ ((Variable)(temp.variable)).Name + "' value = '"+temp.get_String()+"'>\n";
		    send = send +indent()+"</Param>\n";
		}
	    }
	}
	return send;
    }

    private static String Graphical_info(World_Object object) {
	String send = "";
	//if(object.Object_Type == World_Object.Neuron){
	if(object.x != 0 || object.y != 0 || !object.comment.equals("")){
	    send = send + indent()+"<GraphicalInfo ";
	    if(object.x != 0) 
		send = send + "x = '"+object.x+ "' ";
	    if(object.y != 0) 
		send = send + "y = '"+object.y+ "' ";
	    if(!object.comment.equals(""))
	    send = send+" description = '"+object.comment+ "' ";
	    send = send + ">";
	    indent++;
	    // inside tag if needed
	    indent--;
	    send = send + "</GraphicalInfo>\n";
	}
	return send;
    }

    private static String write_out_list_of_parts(World_Object object){
	String send ="";
	if(object.list_of_Parts!=null){
	    for (int counter = 0 ; counter <object.list_of_Parts.size(); counter++){
		send = send + make_XML_rep((World_Object)object.list_of_Parts.get(counter));
	    }
	}
	return send;
    }
}
