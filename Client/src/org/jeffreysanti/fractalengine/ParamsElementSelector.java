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
import java.util.HashMap;
import java.util.Map;
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
        subElm = new JPanel();
        
        box = new JComboBox();
        JSONArray choices = (JSONArray)schem.get("choices");
        for(Object o : choices){
            JSONObject choice = (JSONObject)o;
            CID.add((String)choice.get("id"));
            box.addItem((String)choice.get("caption"));
        }
        
        verify(); // assures some value is inside text editor
        
        String def = (String)((JSONObject)getValue()).get("selected");
        for(int i=0; i<CID.size(); i++){
            if(CID.get(i).equals(def)){
                box.setSelectedIndex(i);
                break;
            }
        }
        
        // moved down here to not dirty context when default selected
        box.addActionListener(new ActionListener(){
            @Override
            public void actionPerformed(ActionEvent e) {
                int cid = box.getSelectedIndex();
                String sel = CID.get(cid);
                JSONObject MP = new JSONObject();
                MP.put("selected", sel);
                setValue(MP);
                callback.markDirty();
                verify();
            }
         });
    }
    
    @Override
    public boolean verify()
    {
        subElm.removeAll();
        
        if(!valueExists() || !(getValue() instanceof JSONObject) ||
                !((JSONObject)getValue()).containsKey("selected")){
            JSONObject MP = new JSONObject();
            MP.put("selected", (String)schem.get("default"));
            setValue(MP);
            callback.markDirty();
        }
        
        JSONObject obj = (JSONObject)getValue();
        String val = (String)obj.get("selected");
        boolean found=false;
        for(int i=0; i<CID.size(); i++){
            if(CID.get(i).equals(val)){
                found = true;
                break;
            }
        }
        if(!found){
            val = CID.get(box.getSelectedIndex());
            obj.put("selected", val);
            setValue(obj);
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
                
                // now verify subelements
                if(choice.containsKey("elm") && choice.get("elm") instanceof JSONObject){
                    JSONObject subelm = (JSONObject)choice.get("elm");
                    String type = (String)subelm.get("type");

                    ParamsElement e = null;
                    if(type.equals("text")){
                        e = new ParamsElementText(subelm, getValue(), callback, -1);
                    }else if(type.equals("integer")){
                        e = new ParamsElementIntegral(subelm, getValue(), callback, -1);
                    }else if(type.equals("selector")){
                        e = new ParamsElementSelector(subelm, getValue(), callback, -1);
                    }else if(type.equals("color")){
                        e = new ParamsElementColor(subelm, getValue(), callback, -1);
                    }else if(type.equals("real")){
                        e = new ParamsElementReal(subelm, getValue(), callback, -1);
                    }else if(type.equals("complex")){
                        e = new ParamsElementComplex(subelm, getValue(), callback, -1);
                    }else if(type.equals("array")){
                        e = new ParamsElementArray(subelm, getValue(), callback, -1);
                    }else if(type.equals("tuple")){
                        e = new ParamsElementTuple(subelm, getValue(), callback, -1);
                    }
                    subElm.add(e.getInnerElm());
                    subElm.revalidate();
                }
                
                break;
            }
        }
        
        return true;
    }
    
    @Override
    public JComponent getInnerElm(){
        JPanel selectorContainer = new JPanel();
        selectorContainer.setLayout(new BorderLayout());
        selectorContainer.add(lbl, BorderLayout.LINE_START);
        selectorContainer.add(box, BorderLayout.LINE_END);
        
        JPanel pnl = new JPanel();
        pnl.setLayout(new BorderLayout());
        pnl.add(selectorContainer, BorderLayout.NORTH);
        pnl.add(subElm, BorderLayout.CENTER);
        
        return pnl;
    }
    
    private JLabel lbl;
    private JComboBox<String> box;
    private JPanel subElm;
    private ArrayList<String> CID = new ArrayList();
}

