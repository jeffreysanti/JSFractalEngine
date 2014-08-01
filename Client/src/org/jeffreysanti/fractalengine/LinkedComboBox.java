/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package org.jeffreysanti.fractalengine;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.util.HashMap;
import javax.swing.JComboBox;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JTextField;

/**
 *
 * @author jeffrey
 */
public class LinkedComboBox extends JPanel {
    
    LinkedComboBox(Context cont, String paramname, String lbl, HashMap<String, String> vals){
        c = cont;
        pName = paramname;
        paramLbl = lbl;
        schem = new ParamaterSchema("SCHEMA_ALL");
        E = new HashMap();
        
        this.setLayout(new BorderLayout());
        
        txtLbl = new JLabel(paramLbl);
        txtLbl.setMaximumSize(txtLbl.getPreferredSize());
        this.add(txtLbl, BorderLayout.LINE_START);
        
        combo = new JComboBox();
        int i=0;
        for(String key : vals.keySet()){
            combo.addItem(vals.get(key));
            E.put(i, key);
            i++;
        }
        
        String defValue = schem.getDefaultOrCurrent(pName, c.getParams());
        for(int key : E.keySet()){
            if(E.get(key).equals(defValue)){
                combo.setSelectedIndex(key);
                break;
            }
        }
        c.getParams().setValue(pName, defValue);
        
        //field.setMaximumSize(field.getPreferredSize());
        //field.setText(schem.getDefault(pName));
        this.add(combo, BorderLayout.LINE_END);
        
        if(!c.isForkable())
            combo.setEnabled(false);
        
        combo.addItemListener(new ItemListener(){

            @Override
            public void itemStateChanged(ItemEvent ie) {
                int key = combo.getSelectedIndex();
                if(E.containsKey(key)){
                    c.getParams().setValue(pName, E.get(key));
                    c.markDirty();
                }
            }
        });
    }
    
    private JLabel txtLbl;
    private JComboBox combo;
    
    private HashMap<Integer, String> E;
    
    private Context c;
    private ParamaterSchema schem;
    private String pName;
    private String paramLbl;
}
