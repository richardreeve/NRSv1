/** By Anthony Mckale 2003 - this in the class which variables  are stored in
 * remember will need help menu option too - note that index and value should be considered a union*/

package variable_stuff;

/* for package / links */
import misc.printer; // for printing (doesnt work yet)
import world_objects.*;// for node/container/link objects
import gui_stuff.*;//Stuff for drawing/making gui
import xml_parser.*;//Stuff for inputing/output xml files/reading xml types description file
import main_method.GUI; // Main Method

/** This is a general purpose storgae class providing a easy way to store the value of params that can be of several diffreent types */
public class vars{
    /** Stores which variable this corresponds too */
    public int serial;	
    /** Stores value - int points to memeber of the list is this param's value*/
    protected Integer index= new Integer(0);
    /** Stores value - double is the value of this parameter when not inherrited*/
    private Double value= new Double(0);
    /** Stores value - string is the text which this parameter represents*/
    private String string=" ";
    /** Stores pointer parent variable */
    public Variable variable;
    /** This says whether or not this vars value is determined by it's parents defaults */ 
    public boolean inherrited=true; 
    
    /** default constructor */
    public vars(int serial, Object variable){
	this.serial = serial;
	this.variable = (Variable)variable;
    }
    
    public void set(Object a){
	if(this.variable instanceof variable_bool || this.variable instanceof variable_list){
	    if(this.variable.valid(a)){ 
		index = (Integer)a;
		this.inherrited = false;
	    }else{
		this.inherrited = true;
	    }
	}else if(this.variable instanceof variable_string) {
	    if(this.variable.valid(a)){ 
		if(((String)a).equals(" ") || ((String)a).equals("")){
		    this.inherrited = true;
		}else{
		    string = (String)a;
		    this.inherrited = false;
		}
	    }else{
		this.inherrited = true;
	    }
	}else if(this.variable instanceof variable_num) {
	    if(this.variable.valid(a)){ 
		value = (Double)a;
		this.inherrited = false;
	    }else{
		this.inherrited = true;
	    }
	}
    }
    
    public Object get(){
	if(this.variable instanceof variable_bool || this.variable instanceof variable_list){
	    return (Object)index;
	}else if(this.variable instanceof variable_num) {
	    return (Object)value;
	}else if(this.variable instanceof variable_string) {
	    if(string.equals("")) return (Object)" ";
	    else return (Object)string;
	}
	// Wont get here
	System.exit(1);
	return null;
    }
    
    public String get_String(){
	if(this.variable instanceof variable_bool || this.variable instanceof variable_list){
	    return (String)this.variable.List.get(this.index.intValue());
	}else if(this.variable instanceof variable_num) {
	    return value.toString();
	}else if(this.variable instanceof variable_string) {
	    if(string.equals("")) return " ";
	    else return string;
	}
	// Wont get here
	System.exit(1);
	return null;
    }
}
