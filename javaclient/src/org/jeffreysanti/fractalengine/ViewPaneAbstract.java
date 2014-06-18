/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package org.jeffreysanti.fractalengine;

import javax.swing.JPanel;

/**
 *
 * @author jeffrey
 */
public abstract class ViewPaneAbstract extends JPanel {
    
    ViewPaneAbstract(Context c){
        context = c;
    }
    
    abstract void contextDataReceieved();
    
    protected Context context;
}
