package gui_stuff;

import java.awt.*;
import javax.swing.*;
import java.awt.event.*;
import java.io.IOException;
import javax.swing.event.*;
import javax.swing.text.html.*;
import main_method.*;

import java.util.LinkedList;

/** This class should provide a window for view html files in */
public class html_viewer_window extends JFrame{
    JEditorPane jeditorpane = new JEditorPane();
    JTextField textField = new JTextField();
    LinkedList Back = new LinkedList();
    String current_page =" ";
    /** Contains a pointer to last created html_viewer, used for reaching inside inner classes */
    protected static html_viewer_window hack;
    public html_viewer_window(String uri){
	super("Document Viewer");
	current_page = uri;
	if(GUI.debug_mode) System.out.println("Starting Html Viewer values");
	if(GUI.debug_mode) System.out.println(GUI.html_viewer_x+" "
					      +GUI.html_viewer_y+" "
					      +GUI.html_viewer_x_offset+" "
					      +GUI.html_viewer_y_offset);
	
	hack = this;
	this.setBackground(Color.white);
	this.setDefaultCloseOperation(DISPOSE_ON_CLOSE);
	this.addWindowListener(new WindowAdapter() {
		public void windowClosing(WindowEvent e) {
		    if(html_viewer_window.hack != null){
			if(GUI.debug_mode) System.out.println("Setting values");
			GUI.html_viewer_x= html_viewer_window.hack.getWidth();
			GUI.html_viewer_y= html_viewer_window.hack.getHeight();
			GUI.html_viewer_x_offset= html_viewer_window.hack.getX();
			GUI.html_viewer_y_offset= html_viewer_window.hack.getY();
			if(GUI.debug_mode) System.out.println(GUI.html_viewer_x+" "
							      +GUI.html_viewer_y+" "
							      +GUI.html_viewer_x_offset+" "
							      +GUI.html_viewer_y_offset);
		    }
		}
	    });
	Container contentPane = getContentPane();
	
	/** Area for entering new place to look */
	JPanel panel = new JPanel();
	
	textField.setText(current_page);
	panel.setLayout(new BorderLayout());
	JButton Back_button = new JButton("Back");
	Back_button.addActionListener(new BackButtonListener());
	panel.add(Back_button, BorderLayout.WEST);
	panel.add(new JLabel("Location:  "), BorderLayout.CENTER);
	panel.add(textField, BorderLayout.EAST);
	textField.addActionListener(new TextFieldListener());
	contentPane.add(panel, BorderLayout.NORTH);
	
	/** For making the html view bit and containing it in a scroll Pane */
	jeditorpane.setEditable(false);
	jeditorpane.setEditorKit(new javax.swing.text.html.HTMLEditorKit());

	jeditorpane.addHyperlinkListener( new LinkFollower());
	try{
	    jeditorpane.setPage(current_page);
	}catch(IOException e){
	    jeditorpane.setText("Page could not be loaded");
	}
	JScrollPane scrollPane = new JScrollPane(jeditorpane);
	
	contentPane.add(scrollPane, BorderLayout.CENTER);

	//this.pack();
	this.setBounds(GUI.html_viewer_x_offset,GUI.html_viewer_y_offset,GUI.html_viewer_x,GUI.html_viewer_y);
	this.setSize(new Dimension(GUI.html_viewer_x,GUI.html_viewer_y));
	//this.setSize(new Dimension(200,200));
	this.setVisible(true);
    }

    class TextFieldListener implements ActionListener {

        public void actionPerformed(ActionEvent e) {
            try {
		
		Back.add((Object)current_page);
		current_page = textField.getText();
		jeditorpane.setPage(current_page);
            } catch (IOException ex) {
                jeditorpane.setText("Page "+textField.getText()+" could not be loaded");
            }
        }
    }
    
    class BackButtonListener implements ActionListener {

        public void actionPerformed(ActionEvent e) {
            try {
		if(!Back.isEmpty()){
		    current_page = (String)Back.getLast();
		    Back.removeLast();
		    jeditorpane.setPage(current_page);
		    textField.setText(current_page);
		}
            } catch (IOException ex) {
                jeditorpane.setText("Page "+textField.getText()+" could not be loaded");
            }
        }
    }
    
    class LinkFollower implements HyperlinkListener {
	public void hyperlinkUpdate(HyperlinkEvent e) {
	    if (e.getEventType() == HyperlinkEvent.EventType.ACTIVATED) {
		JEditorPane pane = (JEditorPane) e.getSource();
		if (e instanceof HTMLFrameHyperlinkEvent) {
		    HTMLFrameHyperlinkEvent  evt = (HTMLFrameHyperlinkEvent)e;
		    HTMLDocument doc = (HTMLDocument)pane.getDocument();
		    doc.processHTMLFrameHyperlinkEvent(evt);
		} else {
		    try {
			Back.add((Object)current_page);
			current_page=e.getURL().getFile();
			pane.setPage(e.getURL());
			textField.setText(current_page);
			jeditorpane.setText(current_page);
		    } catch (Throwable t) {
			if(GUI.debug_mode)t.printStackTrace();
			jeditorpane.setText("Page could not be loaded");
		    }
		}
	    }
	}
    }
}
