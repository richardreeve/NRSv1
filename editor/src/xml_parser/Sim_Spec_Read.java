package xml_parser;

import xml_parser.helpers.AttributesImpl;// xml node helper
import java.util.*; // For Linked Lists + HashMaps


import org.xml.sax.Attributes;
//import org.xml.sax.Parser;
import org.xml.sax.SAXException;
import org.xml.sax.SAXNotRecognizedException;
import org.xml.sax.SAXNotSupportedException;
import org.xml.sax.SAXParseException;
import org.xml.sax.XMLReader;
import org.xml.sax.ext.LexicalHandler;
import org.xml.sax.helpers.DefaultHandler;
import org.xml.sax.helpers.ParserAdapter;
//import org.xml.sax.helpers.ParserFactory;
import org.xml.sax.helpers.XMLReaderFactory;

/* for package / links */
import variable_stuff.*;//Stuff for representing variables of different types
import main_method.GUI; // Main Method

/** By Anthony Mckale 2003  - this class contains the sax event handlers for parsing the simulator description xml */
public class  Sim_Spec_Read
    extends DefaultHandler{
    
    /** This is a int to set the offset to remove namespace (prosumes only one namespace is used*/
    protected int offset = 0;
    protected boolean first_time = true;
    
    //
    // Data Storage
    //
    
    // Main Types
    // Main Storage
    private HashMap list_of_vars = new HashMap();
    private HashMap list_of_links = new HashMap();
    private HashMap list_of_containers = new HashMap();
    private HashMap list_of_nodes = new HashMap();
    private HashMap list_of_worlds = new HashMap();
    // Temp Vars
    private HashMap Temp_Node_List = new HashMap();
    private HashMap Temp_Node = new HashMap();
    private LinkedList Temp_attr_List;
    private String Temp_Name;
    private String Temp_Element_Default;
    private String Temp_Type;
    private Double Temp_Default;
    private String Temp_Units;
    private Double Temp_Min;
    private Double Temp_Max;
    private LinkedList Temp_List;
    private Boolean Temp_is_int;
    private variable_num Temp_Variable;
    private variable_list Temp_Variable_List;
    private variable_string Temp_Variable_String;
    private variable_bool Temp_Variable_Bool;
    private String Temp_Desc;
    
    // State Variables
    /* 
       The idea here is the parsing process is split into 3 stages (global_mode) dealing with the variable desciptions
       and this has seperate stage too (current_Type), then it goes to terminals the first is a stage alone becuz it's 
       the world terminal (no seperate stages), then individual terminals node_desc with sub stages (object_mode) 
    */
    
    private int start = 4;
    private int global_params = 5;
    private int node_desc = 6;
    /** this describes the parser global mode */
    private int global_mode = start;
    
    private int Null = 999;
    private int node = 0;
    private int link =1;
    private int container = 2;
    /** this describes the current Object type being parsed (used during last stage) */
    private int object_mode = Null;
    
    private int Num_Type = 7;
    private int List_Type = 8;
    private int String_Type = 9;
    private int Bool_Type = 10;
    /** this describes the parser current terminal desc type being parsed */
    private int current_Type = Null;
    
    //
    // Constructors
    //
    
    /** Default constructor. */
    public Sim_Spec_Read() {
    } // <init>()
    
    //
    // Public methods
    //
    
    //
    // ContentHandler methods
    //
    
    /** Start document. */
    public void startDocument() throws SAXException {
	
    } // startDocument()
    
    /** End document. */
    public void endDocument() throws SAXException {	
	convert_internals();
	debug_method_print_lists();
	make_variable_hashmap();
    } // endDocument()
    
    /** This method changes the key from the String name to the Integer serial of the variables and transfers this into arrays */
    private void make_variable_hashmap(){
	arrays.Variable_List = new HashMap();
	Set hash_map =  list_of_vars.entrySet();
	Iterator iterator = hash_map.iterator();
	
	while(iterator.hasNext()){
	    Map.Entry mapentry = (Map.Entry) iterator.next();
	    Variable tmp_var = (Variable)mapentry.getValue();
	    arrays.Variable_List.put((Object)new Integer(tmp_var.serial),(Object)tmp_var);
	    //String tmp_name = (String)mapentry.getKey();
	}
	
    }
    
    /** Processing instruction. */
    public void processingInstruction(String target, String data)
        throws SAXException {
    } // processingInstruction(String,String)
    
    /** Start element. */
    public void startElement(String uri, String local, String raw,
                             Attributes attrs) throws SAXException {
	// Get Offset
	if(first_time){
	    find_offset(raw);
	}
	
	// Do Stuff
	
	/********************/
	/* STATIC  PARAMS */
	/********************/
	if(global_mode == start){
	    parse_start_section1(local, attrs);
	}else{
	    parse_end_section1(local, attrs);
	}
	
	
    } // startElement(String,String,String,Attributes)
    
    
    /** End element. */
    public void endElement(String uri, String local, String raw)
        throws SAXException {
	
	if(global_mode == start){
	    parse_start_section2(local);
	}else{
	    parse_end_section2(local);
	}
	
    } // endElement(String)
    
    /**********************/
    /** Parse Start Section */
    /**********************/
    private void parse_start_section1(String name, Attributes  attrs){
	if(current_Type == Null){
	    if(name.equals("ParamNumDefault")){
		// This as far as i can see is as it defines global num limit which are overided by every num instance (silly really)
		// obselete but richard included it for completeness
	    }
	    // TRANSSION stage
	    else if(name.equals("GlobalParams")){
		global_mode = global_params;
		Temp_Node = new HashMap();
		Temp_Name = "world";
		Temp_Node_List = list_of_worlds;
	    }
	    
	    //param_num_desc
	    else if(name.equals("ParamNumDescription")){
		current_Type = Num_Type;
		Temp_Name = null;
		Temp_Default = null;
		Temp_Units =  null;
		Temp_Min = null;
		Temp_Max = null;
		Temp_is_int =  null;
		
		if (attrs != null) {
		    attrs = sortAttributes(attrs);
		    int len = attrs.getLength();
		    for (int i = 0; i < len; i++) {
			String attr_local_name;
			if(attrs.getLocalName(i) != null){
			    attr_local_name = attrs.getLocalName(i);
			}else{
			    attr_local_name = attrs.getQName(i).substring(offset);
			}
			
			if(attr_local_name.equals("name")){
			    Temp_Name = attrs.getValue(i);
			}else if(attr_local_name.equals("defaultVal")){
			    Temp_Default = new Double(attrs.getValue(i));
			}else if(attr_local_name.equals("units")){
			    Temp_Units= attrs.getValue(i);
			}else if(attr_local_name.equals("maxVal")){
			    try{
				Temp_Max = new Double(attrs.getValue(i));
			    }catch(Exception e){
				Temp_Max = new Double(Double.POSITIVE_INFINITY); 
			    }
			}else if(attr_local_name.equals("minVal")){
			    try{
				Temp_Min = new Double(attrs.getValue(i));
			    }catch(Exception e){
				Temp_Min = new Double(Double.NEGATIVE_INFINITY);
			    }
			}else if(attr_local_name.equals("isInt")){
			    Temp_is_int = new Boolean(attrs.getValue(i));
			}
		    }
		}
		if(Temp_Name != null && Temp_Default != null && Temp_Units != null){
		    Temp_Variable = new variable_num(Temp_Name, Temp_Min.doubleValue(), Temp_Max.doubleValue(), Temp_Default.doubleValue(), Temp_Units);	
		    if(Temp_is_int != null) Temp_Variable.is_int = Temp_is_int.booleanValue();
		}else{
		    System.err.println("There was a problem making the variable "+Temp_Name);
		    System.exit(1);
		}
	    }
	    //param_bool_desc
	    else if(name.equals("ParamBoolDescription")){
		current_Type = Bool_Type;
		Temp_List = new LinkedList();
		Temp_Name = null;
		Temp_Element_Default = null; 
		if (attrs != null) {
		    attrs = sortAttributes(attrs);
		    int len = attrs.getLength();
		    for (int i = 0; i < len; i++) {
			String attr_local_name;
			if(attrs.getLocalName(i) != null){
			    attr_local_name = attrs.getLocalName(i);
			}else{
			    attr_local_name = attrs.getQName(i).substring(offset);
			}
			if(attr_local_name.equals("name")){
			    Temp_Name = attrs.getValue(i);
			}else if(attr_local_name.equals("defaultVal")){
			    Temp_Element_Default = attrs.getValue(i);
			}
		    }
		}
		Temp_Variable_Bool = new variable_bool(Temp_Name, 0);
		if(Temp_Name == null || Temp_Element_Default == null){
		    System.err.println("Problem with Bool Variable and null values");
		}
		//new Integer(Temp_Variable_Bool.List.indexOf(Temp_Element_Default)));
	    }
	    //param_string_desc
	    else if(name.equals("ParamStringDescription")){
		current_Type = String_Type;
		Temp_Name = null;
		String Temp_DefaultS = null;
		Boolean Temp_is_filename =  null;
		
		if (attrs != null) {
		    attrs = sortAttributes(attrs);
		    int len = attrs.getLength();
		    for (int i = 0; i < len; i++) {
			String attr_local_name;
			if(attrs.getLocalName(i) != null){
			    attr_local_name = attrs.getLocalName(i);
			}else{
			    attr_local_name = attrs.getQName(i).substring(offset);
			}
			
			if(attr_local_name.equals("name")){
			    Temp_Name = attrs.getValue(i);
			}else if(attr_local_name.equals("defaultVal")){
			    Temp_DefaultS = attrs.getValue(i);
			}else if(attr_local_name.equals("isFilename")){
			    Temp_is_filename = new Boolean(attrs.getValue(i));
			}
		    }
		    if(Temp_Name != null){
			if(Temp_DefaultS == null) Temp_DefaultS=" ";
			Temp_Variable_String = new variable_string(Temp_Name, Temp_DefaultS);	
			if(Temp_is_filename != null) Temp_Variable_String.is_filename = Temp_is_filename.booleanValue();
		    }else{
			System.err.println("There was a problem making the variable "+Temp_Name);
			System.exit(1);
		    }
		}
	    }
	    //param_list_desc
	    else if(name.equals("ParamListDescription")){
		current_Type = List_Type;
		Temp_List = new LinkedList();
		Temp_Name = null;
		Temp_Element_Default = null; 
		if (attrs != null) {
		    attrs = sortAttributes(attrs);
		    int len = attrs.getLength();
		    for (int i = 0; i < len; i++) {
			String attr_local_name;
			if(attrs.getLocalName(i) != null){
			    attr_local_name = attrs.getLocalName(i);
			}else{
			    attr_local_name = attrs.getQName(i).substring(offset);
			}
			if(attr_local_name.equals("name")){
			    Temp_Name = attrs.getValue(i);
			}else if(attr_local_name.equals("defaultVal")){
			    Temp_Element_Default = attrs.getValue(i);
			}
		    }
		}
		Temp_Variable_List = new variable_list(Temp_Name, Temp_List, 0);
	    }
	
	}else{
	    if(name.equals("Description")){
		/* Three stage to desc making init (this), building up the string in characters,
		   then setting it to the variables desc in start section 2 */
		Temp_Desc = new String();
	    }
	    if(current_Type == List_Type){
		if(name.equals("Element")){
		    if (attrs != null) {
			attrs = sortAttributes(attrs);
			int len = attrs.getLength();
			for (int i = 0; i < len; i++) {
			    String attr_local_name;
			    if(attrs.getLocalName(i) != null){
				attr_local_name = attrs.getLocalName(i);
			    }else{
				attr_local_name = attrs.getQName(i).substring(offset);
			    }
			    if(attr_local_name.equals("name")){
				Temp_List.add(attrs.getValue(i));
			    }
			}
		    }
		}
	    }
	}
    }
    
    private void parse_start_section2(String name){
	if(current_Type == Num_Type){
	    if(name.equals("Description")){
		Temp_Variable.desc = Temp_Desc;
	    }
	    else if(name.equals("ParamNumDescription")){
		current_Type = Null;
		list_of_vars.put((Object)Temp_Name,(Object)Temp_Variable); 
	    }
	}else if(current_Type == Bool_Type){
	    if(name.equals("Description")){
		Temp_Variable_Bool.desc = Temp_Desc;
	    }else if(name.equals("ParamBoolDescription")){
		current_Type = Null;
		Temp_Variable_Bool.Default = new Integer(Temp_Variable_Bool.List.indexOf(Temp_Element_Default));
		list_of_vars.put((Object)Temp_Name,(Object)Temp_Variable_Bool);
	    }
	}else if(current_Type == List_Type){
	    if(name.equals("Description")){
		Temp_Variable_List.desc = Temp_Desc;
	    }else if(name.equals("ParamListDescription")){
		current_Type = Null;
		Temp_Variable_List.Default = new Integer(Temp_Variable_List.List.indexOf(Temp_Element_Default));
		list_of_vars.put((Object)Temp_Name,(Object)Temp_Variable_List);
	    }
	}else if(current_Type == String_Type){
	    if(name.equals("Description")){
		Temp_Variable_String.desc = Temp_Desc;
	    }
	    else if(name.equals("ParamStringDescription")){
		current_Type = Null;
		list_of_vars.put((Object)Temp_Name,(Object)Temp_Variable_String); 
	    }
	}
	
    }
    
    /**********************/
    /** Parse End Section */
    /**********************/
    
    private void parse_end_section1(String name, Attributes  attrs) {
	if(global_mode == node_desc) {
	    if(object_mode == link){
		Temp_Node_List = list_of_links;
	    }
	    if(object_mode == node){
		Temp_Node_List = list_of_nodes;
	    }
	    if(object_mode == container){
		Temp_Node_List = list_of_containers;
	    }
	}
	
	//Now Cases for each kind of Element
	
	// GLOBAL VAR -> NODE DESC || NODE DESC -> NODE DESC 
	if(name.equals("NodeDescription")){
	    // Set mode to node desc
	    global_mode = node_desc;
	    // finish what was being done
	    if (attrs != null) {
		attrs = sortAttributes(attrs);
		int len = attrs.getLength();
		for (int i = 0; i < len; i++) {
		    String attr_local_name;
		    if(attrs.getLocalName(i) != null){
			attr_local_name = attrs.getLocalName(i);
		    }else{
			attr_local_name = attrs.getQName(i).substring(offset);
		    }
		    
		    if(attr_local_name.equals("name")){
			Temp_Name = attrs.getValue(i);
		    }
		    if(attr_local_name.equals("type")){
			Temp_Type = attrs.getValue(i);
		    }
		}
	    }
	    if(Temp_Type.equals("link")){
		object_mode = link;
	    }else if(Temp_Type.equals("terminal")){
		object_mode = node;
	    }else if(Temp_Type.equals("container")){
		object_mode = container;
	    }else{
		object_mode = Null;
	    }	
	}
	
	//Now the individual variable inside
	else if(name.equals("Description")){
	    Temp_Desc = new String();
	}
	
	// Do Vars
	else if(name.equals("ParamNum")||name.equals("ParamBool")||name.equals("ParamString")||name.equals("ParamList")){
	    String attr_Name = null;
	    if (attrs != null) {
		attrs = sortAttributes(attrs);
		int len = attrs.getLength();
		for (int i = 0; i < len; i++) {
		    String attr_local_name;
		    if(attrs.getLocalName(i) != null){
			attr_local_name = attrs.getLocalName(i);
		    }else{
			attr_local_name = attrs.getQName(i).substring(offset);
		    }
		    if(attr_local_name.equals("name")){
			attr_Name = attrs.getValue(i);
		    }
		}
	    }
	    if(attr_Name != null){
		Temp_Node.put((Object)attr_Name,list_of_vars.get((Object)attr_Name)); 
	    }else{
		System.err.println("There was a problem making the variable "+attr_Name+" inside node "+Temp_Name+" of type "+Temp_Type);
		System.exit(1);
	    }
	}
    }	
    
    private void parse_end_section2(String name){
	if(name.equals("GlobalParams")||name.equals("NodeDescription")){
	    // finish what was being done
	    Temp_Node_List.put((Object)Temp_Name, (Object)Temp_Node);
	    Temp_Node = new HashMap();
	}
	
    }
    /** Characters. */
    public void characters(char ch[], int start, int length)
	throws SAXException {
	Temp_Desc = Temp_Desc + new String(ch, start, length)+"\n";
    } // characters(char[],int,int);
    
    /** Bits for converting parser representation into arrays representation */    
    private void convert_internals(){
	HashMap From=new HashMap();
	String[] To_Name;
	Variable[] Cast_Away_To[];
	Variable[] Temp;
	
	debug_method_print_lists();
	
	try{
	    for(int a1 = 0; a1<4; a1++){
		if(a1==0) From = list_of_links;
		if(a1==1) From = list_of_containers;
		if(a1==2) From = list_of_nodes;
		if(a1==3) From = list_of_worlds;
		
		To_Name = new String[From.size()];
		Cast_Away_To = new  Variable[From.size()][];
		
		Set hash_map =  From.entrySet();
		Iterator iterator = hash_map.iterator();
		int counter = 0;
		
		while(iterator.hasNext()){
		    Map.Entry mapentry = (Map.Entry) iterator.next();
		    HashMap tmp_var = (HashMap)mapentry.getValue();    
		    String tmp_name = (String)mapentry.getKey();
		    To_Name[counter] = tmp_name;//=temp_name
		    
		    Set hash_map1 =  tmp_var.entrySet();
		    Iterator iterator1 = hash_map1.iterator();
		    Temp = new Variable[tmp_var.size()];
		    int counter2 = 0;
		    
		    while(iterator1.hasNext()){
			Map.Entry mapentry1 = (Map.Entry) iterator1.next();
			Variable tmp_var1 = (Variable)mapentry1.getValue();    
			String tmp_name1 = (String)mapentry1.getKey();
			
			Temp[counter2]=tmp_var1;
			counter2++;
		    }
		    
		    Cast_Away_To[counter]=Temp;
		    counter++;
		}
		
		if(a1==0) {
		    arrays.Synapse_Names = To_Name;
		    arrays.Synapse_Vars = Cast_Away_To;
		    if(GUI.debug_mode) System.out.println("S");
		}
		if(a1==1) {
		    arrays.Container_Names = To_Name;
		    arrays.Container_Vars = Cast_Away_To;
		    if(GUI.debug_mode) System.out.println("C");
		}
		if(a1==2) {
		    arrays.Neuron_Names = To_Name;
		    arrays.Neuron_Vars = Cast_Away_To;
		    if(GUI.debug_mode) System.out.println("N");
		}
		if(a1==3) {
		    arrays.World_Names = To_Name;
		    arrays.World_Vars = Cast_Away_To;
		    if(GUI.debug_mode) System.out.println("W");
		}
	    }
	}catch(Exception e){
	    e.printStackTrace();
	}
	debug_method_print_arrays();
    }
    
    
    /** Ignorable whitespace. */
    public void ignorableWhitespace(char ch[], int start, int length)
	throws SAXException {
	characters(ch, start, length);
    } // ignorableWhitespace(char[],int,int);
    
    //
    // ErrorHandler methods
    //
    
    /** Warning. */
    public void warning(SAXParseException ex) throws SAXException {
    } // warning(SAXParseException)
    
    /** Error. */
    public void error(SAXParseException ex) throws SAXException {
    } // error(SAXParseException)
    
    /** Fatal error. */
    public void fatalError(SAXParseException ex) throws SAXException {
	throw ex;
    } // fatalError(SAXParseException)
    
    //
    // Protected methods
    //
    
    /** Returns a sorted list of attributes. */
    protected Attributes sortAttributes(Attributes attrs) {
	
	AttributesImpl attributes = new AttributesImpl();
	
	int len = (attrs != null) ? attrs.getLength() : 0;
	for (int i = 0; i < len; i++) {
	    String name = attrs.getQName(i);
	    int count = attributes.getLength();
	    int j = 0;
	    while (j < count) {
		if (name.compareTo(attributes.getQName(j)) < 0) {
		    break;
		}
		j++;
	    }
	    attributes.insertAttributeAt(j, name, attrs.getType(i),
					 attrs.getValue(i));
	}
	
	return attributes;
	
    } // sortAttributes(AttributeList):AttributeList
    
    /** Find Offset */
    private void find_offset(String name){
	if(name.indexOf(':') == -1){ 
	    // as an after thought -1+1 = 0 anyway but at some point
	    // i may which to do something here (or important likely richard may :D
	    offset  = 0 ;
	}else{
	    offset = name.indexOf(':')+1;
	}
    }
    
    private void debug_method_print_lists(){
	if(GUI.debug_mode){
	    System.out.println(list_of_vars);
	    System.out.println(list_of_links);
	    System.out.println(list_of_containers);
	    System.out.println(list_of_nodes);
	    System.out.println(list_of_worlds);
	}
    }
    
    private void debug_method_print_arrays(){
	if(GUI.debug_mode){
	    /** Where link types stored */
	    String temp_Names[];
	    /** Where link types variables stored */
	    Object temp_Vars[][];
	    
	    /** <Hack> so java will compile code*/
	    temp_Names = arrays.Neuron_Names;
	    temp_Vars = arrays.Neuron_Vars;
	    /** </Hack> */
	    
	    for(int a1 = 0; a1<4; a1++){
		if(a1==0){
		    System.out.println("Neuron");
		    temp_Names = arrays.Neuron_Names;
		    temp_Vars = arrays.Neuron_Vars;
		}
		if(a1==1){
		    System.out.println("Synapse");
		    temp_Names = arrays.Synapse_Names;
		    temp_Vars = arrays.Synapse_Vars;
		}
		if(a1==2){
		    System.out.println("Container");
		    temp_Names = arrays.Container_Names;
		    temp_Vars = arrays.Container_Vars;
		}
		if(a1==3){
		    System.out.println("World");
		    temp_Names = arrays.World_Names;
		    temp_Vars = arrays.World_Vars;
		}
		for(int a2 = 0; a2<temp_Names.length;a2++){
		    System.out.println(temp_Names[a2]);
		    for(int a3 = 0; a3<temp_Vars[a2].length;a3++){
			if(temp_Vars[a2][a3] instanceof variable_list){
			    variable_list a6 = (variable_list)temp_Vars[a2][a3];
			    System.out.println(a6.List);
			}
			System.out.println(temp_Vars[a2][a3]);
		    }
		}
	    }
	}
    }
    
} // class Writer
