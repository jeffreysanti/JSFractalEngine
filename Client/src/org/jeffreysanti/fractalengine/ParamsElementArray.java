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
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.util.ArrayList;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JComponent;
import javax.swing.JLabel;
import javax.swing.JList;
import javax.swing.JMenuItem;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
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
public class ParamsElementArray extends ParamsElement {
    public ParamsElementArray(JSONObject schemaDefn, Object paramsContainer, PanelProperties cb, int arrIndex, String addr){
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
        minQuant = 0;
        if(schem.containsKey("min"))
            minQuant = ((Number)schem.get("min")).intValue();
        
        if(!valueExists() || !(getValue() instanceof JSONArray) ||
                ((JSONArray)getValue()).size() < minQuant){
            if(schem.containsKey("default"))
                setValue(schem.get("default"));
            else{
                // fill with default defaults
                JSONArray arr = new JSONArray();
                for(int i=0; i<minQuant; i++){
                    arr.add(new JSONObject());
                }
                setValue(arr);
            }
            callback.markDirty();
        }
        
        E.clear();
        lst.removeAll();
        
        JSONArray val = (JSONArray)getValue();
        JSONObject elmsSchema = (JSONObject)schem.get("elm");
        String type = (String)elmsSchema.get("type");
        callback.removeAnimationParamType(addr+":");
        for(int i=0; i<val.size(); i++){
            String newAddr = addr + ":" + i;
            
            // add element to list & verify it
            ParamsElement e;
            if(type.equals("text")){
                e = new ParamsElementText(elmsSchema, val, callback, i, newAddr);
            }else if(type.equals("integer")){
                e = new ParamsElementIntegral(elmsSchema, val, callback, i, newAddr);
            }else if(type.equals("selector")){
                e = new ParamsElementSelector(elmsSchema, val, callback, i, newAddr);
            }else if(type.equals("color")){
                e = new ParamsElementColor(elmsSchema, val, callback, i, newAddr);
            }else if(type.equals("real")){
                e = new ParamsElementReal(elmsSchema, val, callback, i, newAddr);
            }else if(type.equals("complex")){
                e = new ParamsElementComplex(elmsSchema, val, callback, i, newAddr);
            }else if(type.equals("array")){
                e = new ParamsElementArray(elmsSchema, val, callback, i, newAddr);
            }else if(type.equals("tuple")){
                e = new ParamsElementTuple(elmsSchema, val, callback, i, newAddr);
            }
            else{
                continue;
            }
            E.add(e);
            
            // Add List Entry
            JPanel lstEntry = new JPanel();
            lstEntry.setLayout(new BorderLayout());
            JLabel actionLbl = new JLabel("["+Integer.toString(E.size()+1)+"]");
            final JPopupMenu menu = new JPopupMenu("Popup");
            JMenuItem itm = new JMenuItem("Delete");
            menu.add(itm);
            final int arrIndex = E.size() - 1;
            itm.addActionListener(new ActionListener(){
                @Override
                public void actionPerformed(ActionEvent e) {
                    // delete item from array
                    JSONArray val = (JSONArray)getValue();
                    if(val.size() <= minQuant)
                        return;
                    val.remove(arrIndex);
                    verify();
                    callback.reportAnimationParamChange();
                }
            });
            actionLbl.addMouseListener(new MouseAdapter(){
                @Override
                public void mousePressed(MouseEvent ev) {
                    if (ev.isPopupTrigger()) {
                        menu.show(ev.getComponent(), ev.getX(), ev.getY());
                    }
                }

                @Override
                public void mouseReleased(MouseEvent ev) {
                    if (ev.isPopupTrigger()) {
                        menu.show(ev.getComponent(), ev.getX(), ev.getY());
                    }
                }
            });
            
            lstEntry.add(BorderLayout.WEST, actionLbl);
            JComponent comp = e.getInnerElm();
            lstEntry.add(BorderLayout.CENTER, comp);
            
            Color bg = null;
            if(i % 2 == 0)
                bg = new Color(65, 129, 127);
            else
                bg = new Color(103, 155, 153);
            
            lstEntry.setBackground(bg);
            if(comp instanceof JPanel){
                comp.setBackground(bg);
            }
            
            lst.add(lstEntry);
        }
        JButton btnNew = new JButton("Add New Entry");
        btnNew.addActionListener(new ActionListener(){
            @Override
            public void actionPerformed(ActionEvent e) {
                // Add item from array
                JSONArray val = (JSONArray)getValue();
                val.add(null);
                verify();
                callback.reportAnimationParamChange();
            }
        });
        lst.add(btnNew);
        lst.revalidate();
        return true;
    }
    
    @Override
    public JComponent getInnerElm(){
        JPanel pnl = new JPanel();
        
        pnl.setLayout(new BorderLayout());
        pnl.add(lbl, BorderLayout.LINE_START);
        pnl.add(lst, BorderLayout.CENTER);
        return pnl;
    }
    
    private JLabel lbl;
    private JPanel lst;
    
    private int minQuant;
    
    private ArrayList<ParamsElement> E = new ArrayList();
}

