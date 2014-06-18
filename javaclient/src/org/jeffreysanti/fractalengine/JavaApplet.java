/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package org.jeffreysanti.fractalengine;

import java.awt.*;
import java.lang.reflect.InvocationTargetException;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.swing.*;

/**
 *
 * @author jeffrey
 */
public class JavaApplet extends JApplet {

    TextField input,output;
   Label label1,label2;
   Button b1;
   JLabel lbl;
   int num, sum = 0;
    @Override
   public void init(){
        setLayout(new FlowLayout()); 
      label1 = new Label("please enter number : ");
      add(label1);
      label1.setBackground(Color.yellow);
      label1.setForeground(Color.magenta);
      input = new TextField(5);
      add(input);
      label2 = new Label("Sum : ");
      add(label2);
      label2.setBackground(Color.yellow);
      label2.setForeground(Color.magenta);
      output = new TextField(20);
      add(output);
      b1 = new Button("Add");
      add(b1);
      lbl = new JLabel("Swing Applet Example. ");
      add(lbl);
      setBackground(Color.yellow);
        try {
            SwingUtilities.invokeAndWait(new Runnable () { 
                @Override
              public void run() { 
                
              } 
            });
        } catch (InterruptedException ex) {
            Logger.getLogger(JavaApplet.class.getName()).log(Level.SEVERE, null, ex);
        } catch (InvocationTargetException ex) {
            Logger.getLogger(JavaApplet.class.getName()).log(Level.SEVERE, null, ex);
        }
   }
    
    @Override
    public void start() { 
    // Not used by this applet. 
  } 
 
  // Called when the applet is stopped. 
    @Override
  public void stop() { 
    // Not used by this applet. 
  } 
 
  // Called when applet is terminated.  This is 
  // the last method executed. 
    @Override
  public void destroy() { 
    // Not used by this applet. 
  } 
   
}
