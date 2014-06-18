/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package org.jeffreysanti.fractalengine;

import java.awt.BorderLayout;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;

/**
 *
 * @author jeffrey
 */
public class ViewPaneParams extends ViewPaneAbstract {
    ViewPaneParams(Context c, Paramaters p){
        super(c);
        params = p;
        
        this.setLayout(new BorderLayout());
        
        text = new JTextArea();
        text.setEditable(false);
        JScrollPane scroll = new JScrollPane(text);
        this.add(scroll, BorderLayout.CENTER);
        
        contextDataReceieved();
    }

    @Override
    void contextDataReceieved() {
        String txt = params.dumpToString();
        text.setText(txt);
    }
    
    private JTextArea text;
    private Paramaters params;
}
