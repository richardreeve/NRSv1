package gui_stuff;

/* for package / links */
import misc.printer; // for printing (doesnt work yet)
import world_objects.*;// for node/container/link objects
import xml_parser.*;//Stuff for inputing/output xml files/reading xml types description file
import variable_stuff.*;//Stuff for representing variables of different types
import main_method.GUI; // Main Method


import java.io.BufferedWriter; // writer
import java.io.FileWriter;

import java.awt.*;
import javax.swing.*;
import javax.swing.event.*;

/** This is the source code that is being produced by the program
 * When xml files can be loaded this will allow for editing of this until this
 * It just shows a pretty view */

public class Source_Viewer extends JInternalFrame {
    JTextArea j = new JTextArea("raar");
    static boolean changed=false;// Static for simplicity not nessaccary
    public Source_Viewer(){
	changed=false;
	JScrollPane scrollbar = new JScrollPane(
						j, 
						ScrollPaneConstants.VERTICAL_SCROLLBAR_ALWAYS, 
						ScrollPaneConstants.HORIZONTAL_SCROLLBAR_AS_NEEDED);

	if(GUI.output_format == GUI.none){
	    j.setText("No Output Format Choosen");
	    /*
	      }else if(GUI.output_format == GUI.Nfile){ 
	      j.setText(dummy_nfile_io.make_nfile(GUI.world));
	      }else if(GUI.output_format == GUI.Imr){
	      String a = IMR.WorldtoIMR(GUI.world);
	      if(a != null && !a.equals(""))
	      j.setText(a);
	    */
 	}else if(GUI.output_format == GUI.XML){
	     String a = XML_writer.World_to_XML_string(GUI.world);
	     if(a != null && !a.equals(""))
		 j.setText(a);
	}
	j.getDocument().addDocumentListener(new change_listener());
	Container c = getContentPane();
	c.add(scrollbar, BorderLayout.CENTER);
	//c.add(new filechooserfilter(), BorderLayout.SOUTH);
    }

    public void writer_text_to_file(){
	try{
	BufferedWriter writer = new BufferedWriter(new FileWriter(".temp_source"));
	writer.write( j.getText());
	writer.close();
	}catch(Exception e ){
	System.err.println("Error when trying to write file .temp_source (gui_stuff/Source_Viewer.java)");
		e.printStackTrace(System.err);
	}
    }

    class change_listener implements DocumentListener{
	public void changedUpdate(DocumentEvent e){
	    //Gives notification that an attribute or set of attributes changed.
	    Source_Viewer.changed=true;
	}
	public void insertUpdate(DocumentEvent e){
	    //Gives notification that there was an insert into the document.
	    Source_Viewer.changed=true;
	}
	public void removeUpdate(DocumentEvent e) {
	    // Gives notification that a portion of the document has been removed.
	    Source_Viewer.changed=true;
	}
    }
}
