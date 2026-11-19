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
import world_objects.*;// For world construction

public class  XML_reader
    extends DefaultHandler {

    //
    // Data
    //

    private LinkedList list_of_parents = new LinkedList();
    private LinkedList list_of_Synapse_To = new LinkedList();
    private LinkedList list_of_Synapse_From = new LinkedList();
    private Dummy_World world;
    private World_Object currently_being_made = new World_Object();
    private String Temp_Desc;
    
    //
    // Constructors
    //

    /** Default constructor. */
    public XML_reader() {
    } // <init>()

    //
    // ContentHandler methods
    //

    /** Start document. */
    public void startDocument() throws SAXException {
	if(GUI.debug_mode) System.out.println("Starting XML file read");
	world = new Dummy_World();
	GUI.world = world;
    } // startDocument()

    /** End document. */
    public void endDocument() throws SAXException {	
	connect_synapses();
	if(GUI.debug_mode) System.out.println("Finished XML file read");
    } // endDocument()
    
    /** Processing instruction. */
    public void processingInstruction(String target, String data)
        throws SAXException {
    } // processingInstruction(String,String)

    /** Start element. */
    public void startElement(String uri, String local, String raw,
                             Attributes attrs) throws SAXException {

	// Check if graphical element (if so add graphic info into the currently being made world_object)
	if(local.equals("GraphicalInfo")){
	    Temp_Desc = "";
	    if(GUI.debug_mode) System.out.println("G");
	    if (attrs != null) {
		attrs = sortAttributes(attrs);
		int len = attrs.getLength();
		for (int i = 0; i < len; i++) {
		    String attr_local_name = attrs.getQName(i);
		    if(attr_local_name.equals("x")){
			currently_being_made.x = Integer.parseInt(attrs.getValue(i));
		    }else if(attr_local_name.equals("y")){
			currently_being_made.y = Integer.parseInt(attrs.getValue(i));
		    }else if(attr_local_name.equals("description")){
			currently_being_made.comment = attrs.getValue(i);
		    }
		}
	    }
	}
	
	// Do Parameter
	if(local.equals("Param")){
	    String name="";

	    String value="";
	    if (attrs != null) {
		attrs = sortAttributes(attrs);
		int len = attrs.getLength();
		for (int i = 0; i < len; i++) {
		    String attr_local_name = attrs.getQName(i);
		    if(attr_local_name.equals("name")){
			name = attrs.getValue(i);
		    }else
		    if(attr_local_name.equals("value")){
			value = attrs.getValue(i);
		    }
		}
	    }
	    if(!name.equals("") && !value.equals("")){
		if(GUI.debug_mode) System.out.println("Parameter "+currently_being_made.name+" "+name+" "+value);
		currently_being_made.Update_Parameter(name, value);
	    }
	}
	
	// Do Start
	if(local.equals("World")){
	    if(GUI.debug_mode) System.out.println("Making World");
	    currently_being_made = world;
	    list_of_parents.add(currently_being_made);
	    String name="";
	    String type="";
	    if (attrs != null) {
		attrs = sortAttributes(attrs);
		int len = attrs.getLength();
		for (int i = 0; i < len; i++) {
		    String attr_local_name = attrs.getQName(i);
		    if(attr_local_name.equals("name")){
			name = attrs.getValue(i);
		    }//else
			//if(attr_local_name.equals("type")){
			//type = attrs.getValue(i);
			//}
		}
	    }
	    if(name != null)
		world.name = name;
	}
	
	// Do Nodes
	
	// Do Container
	if(local.equals("Container")){
	    if(GUI.debug_mode) System.out.println("Making Network");
	    String name="";
	    String type="";
	    if (attrs != null) {
		attrs = sortAttributes(attrs);
		int len = attrs.getLength();
		for (int i = 0; i < len; i++) {
		    String attr_local_name = attrs.getQName(i);
		    if(attr_local_name.equals("name")){
			name = attrs.getValue(i);
		    }else
			if(attr_local_name.equals("type")){
			    type = attrs.getValue(i);
			}
		}
	    }

	    if(!name.equals("") && !type.equals("")){
		for(int counter = 0; counter < arrays.Container_Names.length;counter++){	
		    if(arrays.Container_Names[counter].equals(type)){
			currently_being_made = new Dummy_Network(counter);
			currently_being_made.name = name;
			currently_being_made.parent = (World_Object)list_of_parents.getLast();
			list_of_parents.add(currently_being_made);
			World_Object.add_component((World_Object) currently_being_made.parent, (Object)currently_being_made);
			break;
		    }
		}
	    }else{
		
	    }
	}

	// Check if element is a neuron
	if(local.equals("Terminal")){
	    if(GUI.debug_mode) System.out.println("Making Neuron");
	    String name="";
	    String type="";
	    if (attrs != null) {
		attrs = sortAttributes(attrs);
		int len = attrs.getLength();
		for (int i = 0; i < len; i++) {
		    String attr_local_name = attrs.getQName(i);
		    if(attr_local_name.equals("name")){
			name = attrs.getValue(i);
		    }else
			if(attr_local_name.equals("type")){
			    type = attrs.getValue(i);
			}
		}
	    }
	    if(!name.equals("") && !type.equals("")){
		for(int counter = 0; counter < arrays.Neuron_Names.length;counter++){
		    if(type.equals(arrays.Neuron_Names[counter])){
			currently_being_made = new Dummy_Neuron(counter);
			currently_being_made.name = name;
			currently_being_made.Connected = true;
			currently_being_made.parent = (World_Object)list_of_parents.getLast();
			World_Object.add_component((World_Object) currently_being_made.parent, (Object)currently_being_made);
			break;
		    }
		}
	    }else{
		
	    }
	}
	
	// Check if element is a synapse
	if(local.equals("Link")){
	    if(GUI.debug_mode) System.out.println("Making Synapse");
	    String name="";
	    String type="";
	    String To="";
	    String From="";
	    if (attrs != null) {
		attrs = sortAttributes(attrs);
		int len = attrs.getLength();
		for (int i = 0; i < len; i++) {
		    String attr_local_name = attrs.getQName(i);
		    if(attr_local_name.equals("name")){
			name = attrs.getValue(i);
		    }else if(attr_local_name.equals("type")){
			type = attrs.getValue(i);
		    }else if(attr_local_name.equals("to")){
			if(GUI.debug_mode) System.out.println("To_Link : "+attrs.getValue(i));
			To = attrs.getValue(i);//list_of_Synapse_To.add((Object)attrs.getValue(i));
		    }else if(attr_local_name.equals("from")){
			if(GUI.debug_mode) System.out.println("From_Link : "+attrs.getValue(i));
			From = attrs.getValue(i);//list_of_Synapse_From.add((Object)attrs.getValue(i)); 
		    }
		}
	    }
	    if(!name.equals("") && !type.equals("") && !To.equals("") && !From.equals("")){
		for(int counter = 0; counter < arrays.Synapse_Names.length;counter++){
		    if(type.equals(arrays.Synapse_Names[counter])){
			currently_being_made = new Link(counter);
			currently_being_made.name = name;
			currently_being_made.parent = (World_Object)list_of_parents.getLast();
			World_Object.add_component((World_Object) currently_being_made.parent, (Object)currently_being_made);
			list_of_Synapse_To.add((Object)To);
			list_of_Synapse_From.add((Object)From); 
			break;
		    }
		}
	    }else{
		
	    }
	}
    } // startElement(String,String,String,Attributes)
    
    
    /** End element. */
    public void endElement(String uri, String local, String raw)
        throws SAXException {
	
	// Do Nodes
	
	// Check if element is a container first
	if(local.equals("Container")){
		list_of_parents.removeLast();
		currently_being_made = null;
	}
	
	// Check if element is a neuron
	if(local.equals("Terminal")){
	    currently_being_made = null;
	}
	
	// Check if element is a synapse
	if(local.equals("Link")){
	    currently_being_made = null;
	}
    } // endElement(String)

    /** To make sure this is used ensuring data safety only use after list of synapses have been finalized 
     * (ie end of parse)*/
    private void connect_synapses(){
	if(GUI.debug_mode) System.out.println("connecting synapses");
	for(int counter = 0; counter < GUI.world.Synapse_List.size();counter++){
	    Link current = (Link)GUI.world.Synapse_List.get(counter);  
	    if(GUI.debug_mode) System.out.println("Synapse : "+current.name);
	    current.To=(Dummy_Neuron)GUI.world.XPath((String)list_of_Synapse_To.get(counter));
	    current.From=(Dummy_Neuron)GUI.world.XPath((String)list_of_Synapse_From.get(counter));
	    
	    if(current.To != null && current.From != null){
		current.Connected = true;
		//System.out.println("0");
	    }else{
		System.err.println("There was a problem connecting synapse "+current.name);
		System.err.println((String)list_of_Synapse_To.get(counter));
		System.err.println(current.To);
		System.err.println((String)list_of_Synapse_From.get(counter));
		System.err.println(current.From);
		System.err.println("1");
	    }
	}
    }

    /** Characters. */
    public void characters(char ch[], int start, int length)
        throws SAXException {
    } // characters(char[],int,int);


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
}
