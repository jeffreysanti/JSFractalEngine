/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package org.jeffreysanti.fractalengine;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.util.ArrayList;
import javax.swing.JComboBox;
import javax.swing.JComponent;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JTextField;
import org.json.simple.JSONArray;
import org.json.simple.JSONObject;

/**
 *
 * @author jeffrey
 */
public class ParamsElementSelector extends ParamsElement {
    public ParamsElementSelector(JSONObject schemaDefn, Object paramsContainer, PanelProperties cb, int arrIndex){
        super(schemaDefn, paramsContainer, cb, arrIndex);
        
        lbl = new JLabel((String)schemaDefn.get("caption"));
        
        box = new JComboBox();
        JSONArray choices = (JSONArray)schem.get("choices");
        for(Object o : choices){
            JSONObject choice = (JSONObject)o;
            CID.add((String)choice.get("id"));
            box.addItem((String)choice.get("caption"));
        }
        box.addActionListener(new ActionListener(){
            /*
             public void keyReleased(KeyEvent ke) {
                 String val = txt.getText();
                 grp.put(id, val);
                 if(!verify()){
                     lbl.setForeground(Color.red);
                 }
                 else{
                    lbl.setForeground(Color.black);
                 }
                 callback.markDirty();
             }
*/
            @Override
            public void actionPerformed(ActionEvent e) {
                int cid = box.getSelectedIndex();
                String sel = CID.get(cid);
                setValue(sel);
                callback.markDirty();
                verify();
            }
         });
        
        verify(); // assures some value is inside text editor
        
        String def = getValue().toString();
        for(int i=0; i<CID.size(); i++){
            if(CID.get(i).equals(def)){
                box.setSelectedIndex(i);
                break;
            }
        }
    }
    
    @Override
    public boolean verify()
    {
        if(!valueExists()){
            setValue((String)schem.get("default"));
            callback.markDirty();
        }
        
        String val = getValue().toString();
        boolean found=false;
        for(int i=0; i<CID.size(); i++){
            if(CID.get(i).equals(val)){
                found = true;
                break;
            }
        }
        if(!found){
            val = CID.get(box.getSelectedIndex());
            setValue(val);
        }
        
        // now send callback
        JSONArray choices = (JSONArray)schem.get("choices");
        for(Object o : choices){
            JSONObject choice = (JSONObject)o;
            if(((String)choice.get("id")).equals(val)){
                // update actuators
                ArrayList<String> A = new ArrayList();
                if(choice.containsKey("hide")){
                    for(Object o2 : (JSONArray)choice.get("hide")){
                        A.add(o2.toString());
                    }
                }
                callback.hideActuator(SchemaManager.getInst().expandGroupSet(A));
                A.clear();
                if(choice.containsKey("show")){
                    for(Object o2 : (JSONArray)choice.get("show")){
                        A.add(o2.toString());
                    }
                }
                callback.showActuator(SchemaManager.getInst().expandGroupSet(A));
                break;
            }
        }
        
        return true;
    }
    
    @Override
    public JComponent getInnerElm(){
        JPanel pnl = new JPanel();
        pnl.setLayout(new BorderLayout());
        pnl.add(lbl, BorderLayout.LINE_START);
        pnl.add(box, BorderLayout.LINE_END);
        return pnl;
    }
    
    private JLabel lbl;
    private JComboBox<String> box;
    private ArrayList<String> CID = new ArrayList();
}

