/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package org.jeffreysanti.fractalengine;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import javax.swing.JComponent;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JTextField;
import org.json.simple.JSONObject;

/**
 *
 * @author jeffrey
 */
public class ParamsElementIntegral extends ParamsElement {
    public ParamsElementIntegral(JSONObject schemaDefn, Object paramsContainer, PanelProperties cb, int arrIndex){
        super(schemaDefn, paramsContainer, cb, arrIndex);
        
        lbl = new JLabel((String)schemaDefn.get("caption"));
        
        txt = new JTextField(15);
        txt.setMaximumSize(txt.getPreferredSize());
        txt.addKeyListener(new KeyListener(){
             @Override
             public void keyTyped(KeyEvent ke) { }
             @Override
             public void keyPressed(KeyEvent ke) {  }
             @Override
             public void keyReleased(KeyEvent ke) {
                 String val = txt.getText();
                 setValue(val);
                 if(!verify()){
                     lbl.setForeground(Color.red);
                 }
                 else{
                    lbl.setForeground(Color.black);
                 }
                 callback.markDirty();
             }
         });
        
        verify(); // assures some value is inside text editor
        txt.setText(getValue().toString());
    }
    
    @Override
    public boolean verify()
    {
        if(!valueExists()){
            if(schem.containsKey("default"))
                setValue(schem.get("default").toString());
            else
                setValue(0);
            callback.markDirty();
        }
        
        
        String val = getValue().toString();
        try{
            long integral = Integer.parseInt(val);
            setValue(integral);
            
            // now verify it
            return verifyMinMaxZero(integral);
        }catch(NumberFormatException e){
            // okay
        }
        return false;
    }
    
    @Override
    public JComponent getInnerElm(){
        JPanel pnl = new JPanel();
        pnl.setLayout(new BorderLayout());
        pnl.add(lbl, BorderLayout.LINE_START);
        pnl.add(txt, BorderLayout.LINE_END);
        return pnl;
    }
    
    private JLabel lbl;
    private JTextField txt;
}

