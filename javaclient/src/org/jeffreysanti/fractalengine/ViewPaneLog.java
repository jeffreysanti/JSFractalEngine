/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package org.jeffreysanti.fractalengine;

import java.awt.BorderLayout;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;

/**
 *
 * @author jeffrey
 */
public class ViewPaneLog extends ViewPaneAbstract{
    
    public ViewPaneLog(Context c){
        super(c);
        
        this.setLayout(new BorderLayout());
        
        text = new JTextArea();
        text.setEditable(false);
        JScrollPane scroll = new JScrollPane(text);
        this.add(scroll, BorderLayout.CENTER);
        
        contextDataReceieved();
    }


    @Override
    void contextDataReceieved() {
        String logFile = context.getLog();
        text.setText(logFile);
    }
    
    private JTextArea text;
}
