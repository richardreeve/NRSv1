package xml_parser;

import java.util.*; // For Linked Lists
/* For Sax Parser */

import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.io.UnsupportedEncodingException;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;
import org.xml.sax.SAXParseException;
import org.xml.sax.XMLReader;
import org.xml.sax.helpers.DefaultHandler;

import java.io.BufferedWriter; // writer/reader
import java.io.FileWriter;

import javax.swing.JPanel;

/* for package / links */
//import misc.printer; // for printing (doesnt work yet)
import world_objects.*;// for node/container/link objects
import gui_stuff.*;//Stuff for drawing/making gui
import variable_stuff.*;//Stuff for representing variables of different types
import main_method.GUI; // Main Method

/** Support Classes
 * By Anthony Mckale 2003 -
* This is basically a class for holding desciptive Information
* about the simulator and also for obtaining it
* Ie String Names, Neuron Values etc */

public class arrays {


    //
    // Constants
    //

    /** Default parser name. */
    static String
        DEFAULT_PARSER_NAME = GUI.DEFAULT_PARSER_NAME ;


    private static boolean setValidation    = false; //defaults
    private static boolean setNameSpaces    = true;
    private static boolean setSchemaSupport = true;
    private static boolean setSchemaFullSupport = false;

    //
    // My Vars
    //
    
    /** This is a variable used to make sure every variable is unique in arrays*/
    public static int Counter = 0;

    /** Container Vars containing every */
    private static String s[];
    
    // Used for construction
    static LinkedList a = new LinkedList();

    
    /** Important */
    /** Where node types stored */
    public static String[] Neuron_Names;
    /** Where node types variables stored */
    public static Variable Neuron_Vars[][];

    /** Where link types stored */
    public static String Synapse_Names[];
    /** Where link types variables stored */
    public static Variable Synapse_Vars[][];

    /** Where container types stored */
    public static String Container_Names[];
    /** Where container types variables stored */
    public static Variable Container_Vars[][];

    /** This is a list of every variable in the simulator */
    public static HashMap Variable_List;

    /** Where world types stored (should only be one but could be fun to try more :P*/
    public static String World_Names[];
    /** Where world variables stored */
    public static Variable World_Vars[][];

    private static String  parserName = DEFAULT_PARSER_NAME;

    /** main method for testing parsing alone without gui */
    public static void main(String argv[]) {
	try{
	    parse_sim_spec_xml(GUI.Simulator_Description_Location);
	}catch(Exception e){
	    System.out.println("Oh");
	}
    }


    /** Method for calling the parsing process */
    public arrays()  {
	try{
	    parse_sim_spec_xml(GUI.Simulator_Description_Location);
	}catch(Exception e){
	    System.out.println("There was a problem reading xml simulator description");
	    e.printStackTrace();
	    System.exit(1);
	}
    }

    /** Prints the output from the SAX callbacks. */
    public static void parse_sim_spec_xml(String uri) {
        try {
            DefaultHandler handler = new Sim_Spec_Read();

            XMLReader parser = (XMLReader)Class.forName(parserName).newInstance();

            parser.setFeature( "http://xml.org/sax/features/validation",
                                                setValidation);
            parser.setFeature( "http://xml.org/sax/features/namespaces",
                                                setNameSpaces );
            parser.setFeature( "http://apache.org/xml/features/validation/schema",
                                                setSchemaSupport );
            parser.setFeature( "http://apache.org/xml/features/validation/schema-full-checking",
                                                setSchemaFullSupport );


            parser.setContentHandler(handler);
            parser.setErrorHandler(handler);
            parser.parse(uri);
        }
        catch (Exception e) {
	    System.err.println("There was a problem reading the simulator description file");
            e.printStackTrace(System.err);
	    System.exit(1);
        }

    }


    /** Prints the output from the SAX callbacks. */
    public static void parse_xml_file(String uri) {
        try {
	    DefaultHandler handler = new XML_reader();
	    
	    XMLReader parser = (XMLReader)Class.forName(parserName).newInstance();
	    
	    parser.setFeature( "http://xml.org/sax/features/validation",
			       setValidation);
	    parser.setFeature( "http://xml.org/sax/features/namespaces",
			       setNameSpaces );
	    parser.setFeature( "http://apache.org/xml/features/validation/schema",
			       setSchemaSupport );
	    parser.setFeature( "http://apache.org/xml/features/validation/schema-full-checking",
			       setSchemaFullSupport );
	    
	    
	    parser.setContentHandler(handler);
	    parser.setErrorHandler(handler);
	    parser.parse(uri);
	    GUI.ButtonPane.updateInfoPanel(new JPanel());
        }
        catch (Exception e) {
	    System.err.println("There was a problem reading the xml file "+uri);
	    e.printStackTrace(System.err);
        }
    }

    /** Prints the output from the SAX callbacks. */
    public static void throwing_parse_xml_file(String uri) throws Exception {
	    DefaultHandler handler = new XML_reader();
	    
	    XMLReader parser = (XMLReader)Class.forName(parserName).newInstance();
	    
	    parser.setFeature( "http://xml.org/sax/features/validation",
			       setValidation);
	    parser.setFeature( "http://xml.org/sax/features/namespaces",
			       setNameSpaces );
	    parser.setFeature( "http://apache.org/xml/features/validation/schema",
			       setSchemaSupport );
	    parser.setFeature( "http://apache.org/xml/features/validation/schema-full-checking",
			       setSchemaFullSupport );
	    
	    
	    parser.setContentHandler(handler);
	    parser.setErrorHandler(handler);
	    parser.parse(uri);
	    GUI.ButtonPane.updateInfoPanel(new JPanel());
	}

    /** Writes the current world to the open file (last opened/saved) */
    public static void write_xml_file() {
	write_xml_file(GUI.current_open_file);
    }
    
    /** Writes the current world to the specificed file */
    public static void write_xml_file(String uri) {
	String safe = GUI.current_open_file;
	try{
	    XML_writer.World_to_XML_file(uri, GUI.world);
	    GUI.current_open_file = uri;
	    GUI.status_bar.setText("'"+uri+"' has been saved");
	}catch(Exception e){
	    GUI.current_open_file  = safe; // Recovers last opened file is error happens
	    GUI.status_bar.setText("Error occured while saving file '"+uri+"'");
	}
    }
 
    /** Save current world, then try to load a temp file, if fails then reload old world 
     * and return fail value (1), else return (0) */
    public static int try_write_xml_file(String uri) {
	try{
	    write_xml_file(".temp_xml_file");
	    throwing_parse_xml_file(uri);
	    return 0;
	}catch(Exception e){
	    try{
		throwing_parse_xml_file(".temp_xml_file");
	    }catch(Exception eh){
		System.err.println("Oh dear god, BIG problem with save/load system,"
				   +" old file was saved but couldnt be loaded "
				   +"properly :( \nxml_parser/arrays.java");
		System.err.println("Stack trace for attempted parse");
		e.printStackTrace(System.err);
		System.err.println("Stack trace for parse that should of worked");
		eh.printStackTrace(System.err);
	    }
	    return 1;
	}
    }
   
    /** Save current world, then try to load a temp file, if fails then reload old world 
     * and return fail value (1), else return (0) */
    public static int try_write_xml_file() {
	try{
	    GUI.ViewTypePane.source_viewer.writer_text_to_file();
	    write_xml_file(".temp_xml_file");
	    throwing_parse_xml_file(".temp_source");
	    return 0;
	}catch(Exception e){
	    try{
	    throwing_parse_xml_file(".temp_xml_file");
	    }catch(Exception eh){
		System.err.println("Oh dear god, BIG problem with save/load system,"
				   +" old file was saved but couldnt be loaded "
				   +"properly :( \nxml_parser/arrays.java");
		System.err.println("Stack trace for attempted parse");
		e.printStackTrace(System.err);
		System.err.println("Stack trace for parse that should of worked");
		eh.printStackTrace(System.err);
	    }
	    return 1;
	}
    }
}
