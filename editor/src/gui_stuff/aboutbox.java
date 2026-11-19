package gui_stuff;

import java.awt.*; 
import java.awt.event.*; 
//import java.io.*; 
//import java.util.*; 
import javax.swing.*; 
import javax.swing.event.*; 
import javax.swing.border.*;

/** This class makes a about box which should appear in the help menu */
public class aboutbox extends JDialog { 
    public aboutbox(){//Frame owner) { 
	super((Frame)null, "About Swing Menu", true); 
	this.setDefaultCloseOperation(DISPOSE_ON_CLOSE);
	JLabel lbl = new JLabel(new ImageIcon("icon/duke.gif")); 
	JPanel p = new JPanel(); 
	Border b1 = new BevelBorder(BevelBorder.LOWERED); 
	Border b2 = new EmptyBorder(5, 5, 5, 5); 
	lbl.setBorder(new CompoundBorder(b1, b2)); 
	p.add(lbl); 
	getContentPane().add(p, BorderLayout.WEST); 
	String message = "Graphical user inteface (beta) developed by Anthony Mckale for the nrs program, developed by richard reeve"; 
	JTextArea txt = new JTextArea(message); 
	txt.setBorder(new EmptyBorder(5, 10, 5, 10)); 
	txt.setFont(new Font("Helvetica", Font.BOLD, 12)); 
	txt.setEditable(false); 
	txt.setBackground(getBackground()); 
	p = new JPanel(); 
	p.setLayout(new BoxLayout(p, BoxLayout.Y_AXIS)); 
	p.add(txt); 
	message = "For more information about this software or bug reporting email A.Mckale@sms.ed.ac.uk"; 
	txt = new JTextArea(message); 
	txt.setBorder(new EmptyBorder(5, 10, 5, 10)); 
	txt.setFont(new Font("Arial", Font.PLAIN, 12)); 
	txt.setEditable(false); 
	txt.setBackground(getBackground()); 
	p.add(txt); 
	getContentPane().add(p, BorderLayout.CENTER); 
	JButton btOK = new JButton("OK"); 
	ActionListener lst = new ActionListener() { 
		public void actionPerformed(ActionEvent e) { 
		    setVisible(false); 
		} 
	    }; 
	btOK.addActionListener(lst); 
	p = new JPanel(); 
	p.add(btOK); 
	getContentPane().add(p, BorderLayout.SOUTH); 
	pack(); setResizable(false); 
    } 
}
