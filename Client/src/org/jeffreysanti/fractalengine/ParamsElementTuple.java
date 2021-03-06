/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package org.jeffreysanti.fractalengine;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.Rectangle;
import java.awt.datatransfer.Transferable;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.util.ArrayList;
import javax.swing.BoxLayout;
import javax.swing.JComponent;
import javax.swing.JLabel;
import javax.swing.JList;
import javax.swing.JPanel;
import javax.swing.JTextField;
import javax.swing.ListCellRenderer;
import javax.swing.TransferHandler;
import org.json.simple.JSONArray;
import org.json.simple.JSONObject;
import org.json.simple.JSONValue;

/**
 *
 * @author jeffrey
 */
public class ParamsElementTuple extends ParamsElement {
    public ParamsElementTuple(JSONObject schemaDefn, Object paramsContainer, PanelProperties cb, int arrIndex, String addr){
        super(schemaDefn, paramsContainer, cb, arrIndex, addr);
        
        lbl = new JLabel((String)schemaDefn.get("caption"));
        
        lst = new JPanel();
        lst.setLayout(new BoxLayout(lst, BoxLayout.Y_AXIS));
        
        verify(); // assures some value is there
        
        callback.registerAnimationParamType(new AnimationWindow.AnimationParam(addr, 
                AnimationWindow.AnimationParamType.APT_NON_TYPE, schem));
    }
    
    @Override
    public boolean verify()
    {
        if(!valueExists() || !(getValue() instanceof JSONObject)){
            if(schem.containsKey("default"))
                setValue(schem.get("default"));
            else{
                setValue(new JSONObject());
            }
            JSONObject O = (JSONObject)getValue();
            for(Object elmSchem : (JSONArray)schem.get("elms")){
                if(!O.containsKey(((JSONObject)elmSchem).get("id"))){
                    O.put(((JSONObject)elmSchem).get("id"), null);
                }
            }
            callback.markDirty();
        }
        
        E.clear();
        lst.removeAll();
        
        // elements of tuple
        callback.removeAnimationParamType(addr+":");
        for(Object elmSchem : (JSONArray)schem.get("elms")){
            String type = (String)((JSONObject)elmSchem).get("type");
            String newAddr = addr + ":" + (String)((JSONObject)elmSchem).get("id");
            
            ParamsElement e;
            if(type.equals("text")){
                e = new ParamsElementText((JSONObject)elmSchem, getValue(), callback, -1, newAddr);
            }else if(type.equals("integer")){
                e = new ParamsElementIntegral((JSONObject)elmSchem, getValue(), callback, -1, newAddr);
            }else if(type.equals("selector")){
                e = new ParamsElementSelector((JSONObject)elmSchem, getValue(), callback, -1, newAddr);
            }else if(type.equals("color")){
                e = new ParamsElementColor((JSONObject)elmSchem, getValue(), callback, -1, newAddr);
            }else if(type.equals("real")){
                e = new ParamsElementReal((JSONObject)elmSchem, getValue(), callback, -1, newAddr);
            }else if(type.equals("complex")){
                e = new ParamsElementComplex((JSONObject)elmSchem, getValue(), callback, -1, newAddr);
            }else if(type.equals("array")){
                e = new ParamsElementArray((JSONObject)elmSchem, getValue(), callback, -1, newAddr);
            }else if(type.equals("tuple")){
                e = new ParamsElementTuple((JSONObject)elmSchem, getValue(), callback, -1, newAddr);
            }
            else{
                continue;
            }
            E.add(e);
            lst.add(e.getInnerElm());
        }
        lst.revalidate();
        return true;
    }
    
    @Override
    public JComponent getInnerElm(){
        JPanel pnl = new JPanel(){
            @Override
            public void setBackground(Color bg) {
                super.setBackground(bg);
                for(Component comp : this.getComponents()){
                    if(comp instanceof JPanel){
                        comp.setBackground(bg);
                        for(Component comp2 : ((JPanel)comp).getComponents()){
                            comp2.setBackground(bg);
                        }
                    }
                }
            }
        };
        
        pnl.setLayout(new BorderLayout());
        pnl.add(lbl, BorderLayout.LINE_START);
        pnl.add(lst, BorderLayout.CENTER);
        return pnl;
    }
    
    private JLabel lbl;
    private JPanel lst;
    
    private ArrayList<ParamsElement> E = new ArrayList();
}

