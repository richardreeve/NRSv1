package main_method;

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

import java.io.*; // writer/reader

import java.util.*; // For Linked Lists

/* for package / links */
import misc.printer; // for printing (doesnt work yet)
import world_objects.*;// for node/container/link objects
import gui_stuff.*;//Stuff for drawing/making gui
import xml_parser.*;//Stuff for inputing/output xml files/reading xml types description file
import variable_stuff.*;//Stuff for representing variables of different types

//import main_method.GUI; // Main Method

/** By Anthony Mckale 2003  - this is the main method */
public class GUI extends Frame{

    /** this is the icon that will appear in the top left hand corner on the menu bar */
    private static final Image icon = Toolkit.getDefaultToolkit().getImage("icon/duke.gif"); 
    // Only one // Icon cannot be multi part gif
    /** This stores the file currently being read to be the simulator description */
    public static String Simulator_Description_Location = "simulator_description.xml";    
    /** This stores the currently opened file */
    public static String current_open_file = "example_xml_file.xml";
    /** The states weither or not the simulator is being debuged or not */
    public static boolean debug_mode = false;

    /** This stores the wiether the source viewer should edit real code */
    public static boolean source_editible = true;
    /** The states which side text should be jusified to on the info panel */
    public static boolean jusified_left = true;
    /** The states how the info panel should be drawn old method or new method
     * new method can draw offscreen, old method resizes to wierd shapes */
    public static boolean new_draw_method = false;


    /** this is the default sax parser used by the editor */
    public static String DEFAULT_PARSER_NAME = "org.apache.xerces.parsers.SAXParser";
    
    /** this is the status bar that appears on the bottom of the main window use 
     *GUI.label.setText(<String>) to change what appears there */
    public static Label status_bar;

    /** This is a variable for checking if antialias should be allowed */
    private static boolean antialias = true;
    /** This is a variable for checking if synapse names should be drawn */
    public  static boolean draw_synapse_names = true;
    
    // MAIN WINDOW
    /** Resolution Size (designed for 1024x800) */
    private static int x = 1024;
    /** Resolution Size (designed for 1024x800) */
    private static int y = 800;
    private static Dimension res = new Dimension(x,y);
    
    // HTML VIEWER WINDOW
    /** x of html viewer */
    public static int html_viewer_x = 800;
    /** y of html viewer */
    public static int html_viewer_y = 600;
    /** offset x of html viewer */
    public static int html_viewer_x_offset = 0;
    /** offset y of html viewer */
  public static int html_viewer_y_offset = 0;
    
    /** Old constant variable used for detecting output/input file type */
    public static final int none = 0;
    //private static final int Nfile = 1;
    //private static final int Imr = 2;
    /** Old constant variable used for detecting output/input file type */
    public static final int XML = 3;

    /** This is for input/output and know what class to use, the what the int value corresponse to changing see inside GUI.java*/
    public static int output_format = XML;
    
    /** class used for setting up variables */
    private static arrays a;
    
    /** For Storing the current world being edited and displayed */
    public static Dummy_World world; //world = new Dummy_World();

    // Get Sub Windows Ready
    /** Button column */
    public static ButtonPane ButtonPane;// = new ButtonPane();
    /** Tabbed window on the right */
    public static ViewTypePane ViewTypePane;// = new ViewTypePane();
    
    /** Sets window to look like system default, so on windows XP or whatever theme set is used etc*/
    public static void setNativeLookAndFeel() {
	try {
	    UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
	} catch(Exception e) {
	    System.out.println("Error setting native LAF: " + e);
	}
    }
    
    /** This is used to standardize the jusification of the text in menu bars */
    public static void setLayoutHoz(JComponent a){
	if(jusified_left){ 
	    a.setLayout(new FlowLayout(FlowLayout.LEFT));
	}else{
	    a.setLayout(new FlowLayout(FlowLayout.RIGHT));
	}
    }
    
    /** This is used to standardize the way menu bars are drawn vertically*/
    public static void setLayoutVert(JComponent a){
	if(new_draw_method){
	    int align = VerticalLayout.LEFT;
	    
	    if(!jusified_left){ 
		align = VerticalLayout.RIGHT;
	    }
	    
	    a.setLayout(new VerticalLayout(5, align, VerticalLayout.TOP));
	}else{
	    a.setLayout(new BoxLayout(a,BoxLayout.Y_AXIS));
	}
    }

    /** These methods should be used for turning on and off antialiasing during drawing */

    public static void Antialias_off(Graphics2D g2){ 
	// As antialias only checks a truth value to fail if already turned off
	// No real gain, apart from function call overhead
	g2.setRenderingHint(RenderingHints.KEY_ANTIALIASING,
	       		RenderingHints.VALUE_ANTIALIAS_OFF);
    }

    /** These methods should be used for turning on and off antialiasing during drawing */

    public static void Antialias_on(Graphics2D g2){
	if(antialias){
	g2.setRenderingHint(RenderingHints.KEY_ANTIALIASING,
                            RenderingHints.VALUE_ANTIALIAS_ON);
	}
    }
        
    /** The is the Main Loader Class that creates all the other classes and Windows */
    
    public static void main(String argv[]) {
	load_variables();
	a= new arrays();
	world = new Dummy_World();
	ButtonPane = new ButtonPane();
	ViewTypePane = new ViewTypePane();
	/** temp variable used during menu creation */
	LinkedList temp_file = new LinkedList();
	
	res = new Dimension(x,y);
	setNativeLookAndFeel(); // Sets window to look like system default
	
	JFrame f = new JFrame("Graphical Neuron Editor - Beta GUI Version");

	f.addWindowListener(new WindowAdapter() {
		public void windowClosing(WindowEvent e) {
		    save_variables();
		    System.exit(0);
		}
	    }); // To make app close on closing window

	JTabbedPane viewtypes = new JTabbedPane();

	// Make ViewTypePane ////////// The class in seperate file //////////
	f.getContentPane().setLayout(new BorderLayout());
	//f.getContentPane().add("West",ButtonPane);
	//f.getContentPane().add("Center",ViewTypePane);
	
	
	f.getContentPane().add(new JSplitPane(JSplitPane.HORIZONTAL_SPLIT, ButtonPane, ViewTypePane));
	
	// Make Status Bar //////////////////////////////////////////////////
	status_bar = new Label("Status Bar"); // Make Status Bar
	f.getContentPane().add("South", status_bar);

	// Make Menu Bar ////////////////////////////////////////////////////
	JMenuBar menubar = new JMenuBar();
	
	// Make file Menu
	
	JMenu file = new JMenu("File");
	ImageIcon new_icon = new ImageIcon("icon/new.jpg");
	JMenuItem New = new JMenuItem("New",new_icon);
	ImageIcon open_icon = new ImageIcon("icon/open.gif");
	JMenuItem Open = new JMenuItem("Open File",open_icon);
	ImageIcon save_icon = new ImageIcon("icon/save.jpg");
	JMenuItem Save = new JMenuItem("Save",save_icon);
	ImageIcon picture_icon = new ImageIcon("icon/picture.gif");
	JMenuItem SaveasJpeg = new JMenuItem("Save picture as jpeg",picture_icon);
	ImageIcon saveas_icon = new ImageIcon("icon/saveas.gif");
	JMenuItem Saveas = new JMenuItem("Save as ...",saveas_icon);
	//ImageIcon print_icon = new ImageIcon("icon/print.gif");
	//JMenuItem Print = new JMenuItem("Print",print_icon);
	ImageIcon quit_icon = new ImageIcon("icon/quit.gif");
	JMenuItem Quit = new JMenuItem("Quit",quit_icon);
	
	New.addActionListener(new ActionListener(){
		public void actionPerformed(ActionEvent event){
		    world = new Dummy_World();
		    refresh_graphics();
		}});
	Open.addActionListener(new ActionListener(){
		public void actionPerformed(ActionEvent event){
		    xml_load_dialog a = new xml_load_dialog();
		    refresh_graphics();
		}});
	Save.addActionListener(new ActionListener(){
		public void actionPerformed(ActionEvent event){
		    arrays.write_xml_file();
		}});
	Saveas.addActionListener(new ActionListener(){
		public void actionPerformed(ActionEvent event){
		    xml_save_dialog a = new xml_save_dialog();
		}});
	SaveasJpeg.addActionListener(new ActionListener(){
		public void actionPerformed(ActionEvent event){
		    jpeg_save_dialog a = new jpeg_save_dialog();
		}});
	/*
	Print.addActionListener(new ActionListener(){
		public void actionPerformed(ActionEvent event){
		    GUI.status_bar.setText("Starting Printing ...");
		    printer printer = new printer();
		    printer.print_world(world);
		    GUI.status_bar.setText("Printing Finished");
		}});
	*/
	Quit.addActionListener(new ActionListener(){
		public void actionPerformed(ActionEvent event){
		    save_variables();
		    System.exit(0);
		}});
	
	file.add(New);
	file.addSeparator();
	file.add(Open);
	file.addSeparator();
	file.add(Save);
	file.add(Saveas);
	file.add(SaveasJpeg);
	file.addSeparator();
	//file.add(Print);
	file.addSeparator();
	file.add(Quit);
	
	file.getPopupMenu().setLightWeightPopupEnabled(false);
	menubar.add(file);
	
	// Make Options Menu
	JMenu options = new JMenu("Options");
	final JCheckBoxMenuItem antialiasing = new JCheckBoxMenuItem("no antialias");
	antialiasing.addActionListener(new ActionListener(){
		public void actionPerformed(ActionEvent event){
		   antialias = !antialiasing.getState();  
		   refresh_graphics();
		}});
	
	options.add(antialiasing);

	final JCheckBoxMenuItem synapse_names = new JCheckBoxMenuItem("dont draw synapse names");
	synapse_names.addActionListener(new ActionListener(){
		public void actionPerformed(ActionEvent event){
		   draw_synapse_names = !synapse_names.getState();
		   refresh_graphics();
		}});
	
	options.add(synapse_names);

	options.getPopupMenu().setLightWeightPopupEnabled(false);
	menubar.add(options);
	
	// Make Options Menu
	JMenu View = new JMenu("View");
	options.getPopupMenu().setLightWeightPopupEnabled(false);
	
	JMenuItem refresh = new JMenuItem("Refresh");
	refresh.addActionListener(new ActionListener(){
		public void actionPerformed(ActionEvent event){
		    refresh_graphics();
		}});
	View.add(refresh);
	
	menubar.add(View);

	// Make Help Menu
	JMenu help = new JMenu("Help");
	
	//JMenuItem glossary = new JMenuItem("This Simulator Glossary");
	//glossary.addActionListener(new ActionListener(){
	//	public void actionPerformed(ActionEvent event){
	//	    html_viewer_window a = new html_viewer_window("http://www.google.com");
	//	    a.show();
	//	}});
	//help.add(glossary);
	
	JMenu sim_help = new JMenu("How to Use This Simulator : a Tutorial");
	File folder = new File("tutorials");// prosumes that tutorial folder exists
	File[] files = folder.listFiles();
	if( files != null){
	    for (int i = 0; i < files.length; i++) {
		if (!(files[i].isDirectory())) {
		    final int c = i;
		    final File b = files[c];
		    JMenuItem new_tutorial = new JMenuItem(b.getName());
		    new_tutorial.addActionListener(new ActionListener(){
			    public void actionPerformed(ActionEvent event){
				try{
				    html_viewer_window a = new html_viewer_window("file:"+b.getCanonicalPath() );
				    a.show();
				}catch(IOException e){
				    System.err.println("could not find file "+b.getAbsolutePath());
				}
			    }});
		    sim_help.add(new_tutorial); 
		}
	    }
	    sim_help.getPopupMenu().setLightWeightPopupEnabled(false);
	    help.add(sim_help);
	}

	JMenu text_help = new JMenu("How to Use This Simulator : TextFiles");
	folder = new File("text_files");// prosumes that tutorial folder exists
	files = folder.listFiles();
	if( files != null){
	    for (int i = 0; i < files.length; i++) {
		if (!(files[i].isDirectory())) {
		    final int c = i;
		    final File b = files[c];
		    JMenuItem new_tutorial = new JMenuItem(b.getName());
		    new_tutorial.addActionListener(new ActionListener(){
			    public void actionPerformed(ActionEvent event){
				try{
				    html_viewer_window a = new html_viewer_window("file:"+b.getCanonicalPath() );
				    a.show();
				}catch(IOException e){
				    System.err.println("could not find file "+b.getAbsolutePath());
			    }
			    }});
		    text_help.add(new_tutorial); 
		}
	    }
	    text_help.getPopupMenu().setLightWeightPopupEnabled(false);
	    help.add(text_help);
	}
	help.addSeparator();

	JMenuItem about = new JMenuItem("about");
	about.addActionListener(new ActionListener(){
		public void actionPerformed(ActionEvent event){
		    aboutbox a = new aboutbox();
		    a.show();
		}});
	help.add(about);

	help.getPopupMenu().setLightWeightPopupEnabled(false);

	menubar.add(help);

	// Load the Last Opened File
	arrays.parse_xml_file(current_open_file);

	// Set Menu Bar
	f.setJMenuBar(menubar);
	
	// Finish Off Making Window
	f.pack();
	f.setSize(res);
	f.setIconImage(icon); // Set Window Icon
	f.show();
    }
    
    /** this refreshes the window where graphics are displayed */
    public static void refresh_graphics(){
	ViewTypePane.graph.refresh();
    }
        
    /** load variables, loads all the gui related variables such as wiether antialiasing should be used or not
     * into a file called gui.ini stored at the editors root directory
     * very simple Stream Tokenizer method used for loading variable
     * as there is no structure just "VAR = VALUE \\n" pairs*/
    public static void load_variables(){
	try{
	    StreamTokenizer reader = new StreamTokenizer(new FileReader("gui.ini"));
	    reader.slashStarComments(true);
	    //reader.slashSlashComments(true);
	    reader.wordChars('_', '_');
	    reader.wordChars('/', '/');
	    reader.wordChars('.', '.');

	    // idea - 
	    // Check what is the current variable being parsed
	    // then do apporiate action
	    int Current_Var = 0;
	    final int antival = 1;
	    final int synval = 2;
	    final int xval = 3;
	    final int yval = 4;
	    final int Simulator_Description_Locationval = 5; 
	    final int current_open_fileval = 6;
	    final int debug_modeval = 7;
	    final int DEFAULT_PARSER_NAMEval = 8;
	    final int html_viewer_xval = 9;
	    final int html_viewer_yval = 10;
	    final int html_viewer_x_offsetval = 11;
	    final int html_viewer_y_offsetval = 12;
	    final int jusified_leftval = 13;
	    final int new_draw_methodval = 14;
	    final int source_editibleval = 15;
	    while(reader.nextToken() != StreamTokenizer.TT_EOF){
		if(reader.ttype == StreamTokenizer.TT_WORD){

		    if(reader.sval.equals("antialias")){
			Current_Var = antival;
			if(debug_mode) System.out.println("parsing var:antialias");
		    }
		    if(reader.sval.equals("drawsynapsenames")){
			Current_Var = synval;
			if(debug_mode) System.out.println("parsing var:synapse names");
		    }
		    if(reader.sval.equals("debugmode")){
			Current_Var = debug_modeval;
			if(debug_mode) System.out.println("parsing var:parsing debug mode");
		    }
		    if(reader.sval.equals("jusified_left")){ 
			Current_Var = jusified_leftval;
			if(debug_mode) System.out.println("parsing var:jusified_left");
		    }
		    if(reader.sval.equals("source_editible")){ 
			Current_Var = source_editibleval;
			if(debug_mode) System.out.println("parsing var:source_editible");
		    }
		    if(reader.sval.equals("new_draw_method")){ 
			Current_Var = new_draw_methodval;
			if(debug_mode) System.out.println("parsing var:new_draw_method");
		    }
		    if(reader.sval.equals("x")){
			if(debug_mode) System.out.println("parsing var:y");
			Current_Var = xval;
		    }
		    if(reader.sval.equals("y")){
			Current_Var = yval;
			if(debug_mode) System.out.println("parsing var:y");
		    }
		    if(reader.sval.equals("html_viewer_x")){
			Current_Var =  html_viewer_xval;
			if(debug_mode) System.out.println("parsing var:html_viewer_x");
		    }
		    if(reader.sval.equals("html_viewer_y")){
			Current_Var =  html_viewer_yval;
			if(debug_mode) System.out.println("parsing var:html_viewer_y");
		    }
		    if(reader.sval.equals("html_viewer_x_offset")){
			Current_Var =  html_viewer_x_offsetval;
			if(debug_mode) System.out.println("parsing var:html_viewer_x_offset");
		    }
		    if(reader.sval.equals("html_viewer_y_offset")){
			Current_Var =  html_viewer_y_offsetval;
			if(debug_mode) System.out.println("parsing var:html_viewer_y_offset");
		    }
		    if(reader.sval.equals("SimulatorDescriptionLocation")){
			Current_Var = Simulator_Description_Locationval;
			if(debug_mode) System.out.println("parsing var:sim loc file");
		    }
		    if(reader.sval.equals("currentopenfile")){
			if(debug_mode) System.out.println("parsing var:currently open file");
			Current_Var = current_open_fileval;
		    }
		    if(reader.sval.equals("DEFAULT_PARSER_NAME")){
			if(debug_mode) System.out.println("parsing var:DEFAULT_PARSER_NAME");
			 Current_Var = DEFAULT_PARSER_NAMEval;
		    }
		    // Useful for finding what is being reader in
		    // System.out.println(reader.sval);
		    
		    reader.nextToken();
		    
		    if(reader.nextToken() != StreamTokenizer.TT_EOF){
			if(reader.ttype == StreamTokenizer.TT_NUMBER){
			    switch(Current_Var){
			    case antival:
				if(reader.nval == 1)
				    antialias=true;
				else
				    antialias=false;
				break;
			    case synval:
				if(reader.nval == 1)
				    draw_synapse_names=true;
				else
				    draw_synapse_names=false;
				break;
			    case debug_modeval:
				if(reader.nval == 1)
				    debug_mode = true;
				else
				    debug_mode = false;
				break;
			    case jusified_leftval:
				if(reader.nval == 1)
				    jusified_left = true;
				else
				    jusified_left = false;
				break;
			case source_editibleval:
				if(reader.nval == 1)
				    source_editible = true;
				else
				    source_editible = false;
				break;
			    case new_draw_methodval:
				if(reader.nval == 1)
				    new_draw_method = true;
				else
				    new_draw_method = false;
				break;
			    case xval:
				x =(int)reader.nval;
				break;
			    case yval:
				y =(int)reader.nval;
				break;
			    case html_viewer_xval:
				html_viewer_x=(int)reader.nval;
				break;
			    case html_viewer_yval:
				html_viewer_y=(int)reader.nval;
				break;
			    case html_viewer_x_offsetval:
				html_viewer_x_offset=(int)reader.nval;
				break;
			    case html_viewer_y_offsetval:
				html_viewer_y_offset=(int)reader.nval;
				break;
			    }
			    
			    // Useful for finding what is being reader in
			    // System.out.println(reader.nval);
			}else if(reader.ttype == StreamTokenizer.TT_WORD){
			     switch(Current_Var){
			     case Simulator_Description_Locationval:
				 Simulator_Description_Location = reader.sval;
				 break;
			     case current_open_fileval:
				 current_open_file = reader.sval;
				 break;
			     case DEFAULT_PARSER_NAMEval:
				 DEFAULT_PARSER_NAME = reader.sval;
				 break;
			     }
			}
		    }
		}
	    }
	    
	}catch(IOException e){
	    System.err.println("There was a problem reading init.");
	    
	}
    }
    
    /** save variables, saves all the gui related variables such as wiether antialiasing should be used or not
     * into a file called gui.ini stored at the editors root directory*/
    public static void save_variables(){
	if (debug_mode) System.out.println("Saving Variables");
	try{
	    BufferedWriter writer = new BufferedWriter(new FileWriter("gui.ini"));
	    String Answer, Text;

	    // Boolean
	    if (debug_mode) Answer = "1"; else Answer = "0";
	    Text = "debugmode = " +  Answer + "\n";
	    if (antialias) Answer = "1"; else Answer = "0";
	    Text = Text + "antialias = " +  Answer + "\n";
	    if (draw_synapse_names) Answer = "1"; else Answer = "0";
	    Text = Text + "drawsynapsenames = " +  Answer + "\n";
	    if (jusified_left) Answer = "1"; else Answer = "0";
	    Text = Text + "jusified_left = " +  Answer + "\n";
	    if (new_draw_method) Answer = "1"; else Answer = "0";
	    Text = Text + "new_draw_method = " +  Answer + "\n";
	    if (source_editible) Answer = "1"; else Answer = "0";
	    Text = Text + "source_editible = " +  Answer + "\n";
	    // ints
	    Text = Text + "x = "+ new Integer(x).toString() + "\n";
	    Text = Text + "y = "+ new Integer(y).toString() + "\n";

	    Text = Text + "html_viewer_x = "+ new Integer(html_viewer_x).toString() + "\n";
	    Text = Text + "html_viewer_y = "+ new Integer(html_viewer_y).toString() + "\n";
	    Text = Text + "html_viewer_x_offset = "+ new Integer(html_viewer_x_offset).toString() + "\n";
	    Text = Text + "html_viewer_y_offset = "+ new Integer(html_viewer_y_offset).toString() + "\n";

	    // Strings
	    Text = Text + "SimulatorDescriptionLocation = "+ Simulator_Description_Location + "\n";
	    Text = Text + "currentopenfile = "+ current_open_file+"\n";

	    Text = Text + "DEFAULT_PARSER_NAME = "+DEFAULT_PARSER_NAME +"\n";

	    writer.write(Text);
	    writer.close();
	}catch(IOException e){
	    System.err.println("There was a problem writing init file.");
	}
    }
}

