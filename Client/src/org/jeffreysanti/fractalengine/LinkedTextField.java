/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package org.jeffreysanti.fractalengine;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.InputMethodEvent;
import java.awt.event.InputMethodListener;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.beans.PropertyVetoException;
import java.beans.VetoableChangeListener;
import javax.swing.BoxLayout;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JTextField;

/**
 *
 * @author jeffrey
 */
public class LinkedTextField extends JPanel {    
    
    LinkedTextField(Context cont, String paramname, String lbl){
        c = cont;
        pName = paramname;
        paramLbl = lbl;
        schem = new ParamaterSchema("SCHEMA_ALL");
        
        this.setLayout(new BorderLayout());
        
        this.lbl = new JLabel(paramLbl);
        this.lbl.setMaximumSize(this.lbl.getPreferredSize());
        this.add(this.lbl, BorderLayout.LINE_START);
        
        field = new JTextField(15);
        field.setMaximumSize(field.getPreferredSize());
        field.setText(schem.getDefaultOrCurrent(pName, c.getParams()));
        this.add(field, BorderLayout.LINE_END);
        
        if(!c.isForkable())
            field.setEnabled(false);
        
        isValid = true;
        
        field.addKeyListener(new KeyListener(){

            @Override
            public void keyTyped(KeyEvent ke) { }

            @Override
            public void keyPressed(KeyEvent ke) {  }

            @Override
            public void keyReleased(KeyEvent ke) {
                String val = field.getText();
                c.getParams().setValue(pName, val);
                if(!schem.validate(c.getParams(), pName, val)){
                    LinkedTextField.this.lbl.setForeground(Color.red);
                    isValid = false;
                }
                else{
                    LinkedTextField.this.lbl.setForeground(Color.black);
                    isValid = true;
                }
                c.markDirty();
            }
            
        });
        
    }
    
    
    private JTextField field;
    private JLabel lbl;
    private Context c;
    private ParamaterSchema schem;
    private String pName;
    private String paramLbl;
    private boolean isValid;
}
