/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package org.jeffreysanti.fractalengine;

import javax.swing.JComponent;
import org.json.simple.JSONArray;
import org.json.simple.JSONObject;
import org.json.simple.JSONValue;

/**
 *
 * @author jeffrey
 */
public class ParamsElement {
    public ParamsElement(JSONObject schemaDefn, Object paramsContainer, PanelProperties cb, int arrIndex, String addr){
        callback = cb;
        schem = schemaDefn;
        grp = paramsContainer;
        this.addr = addr;
        
        id = (String)schem.get("id");
        aind = arrIndex;
    }
    
    public boolean verify()
    {
        return false;
    }
    
    public JComponent getInnerElm(){
        return null;
    }
    
    public boolean verifyMinMaxZero(double val){
        if(schem.containsKey("allowZero") && schem.get("allowZero") instanceof Boolean){
            if(val == 0 && !((Boolean)schem.get("allowZero")))
                return false; // zero 
        }
        if(schem.containsKey("min")){
            if(val < ((Number)schem.get("min")).doubleValue())
                return false;
        }
        if(schem.containsKey("max")){
            if(val > ((Number)schem.get("max")).doubleValue())
                return false;
        }
        
        return true;
    }
    
    protected boolean valueExists(){
        if(aind >= 0)
            return ((JSONArray)grp).get(aind) != null;
        return ((JSONObject)grp).containsKey(id) && ((JSONObject)grp).get(id) != null;
    }
    protected Object getValue(){
        if(aind >= 0)
            return ((JSONArray)grp).get(aind);
        return ((JSONObject)grp).get(id);
    }
    protected void setValue(Object o){
        if(aind >= 0)
            ((JSONArray)grp).set(aind, o);
        else
            ((JSONObject)grp).put(id, o);
    }
    
    
    protected JSONObject schem;
    protected Object grp;
    protected PanelProperties callback;
    protected String id;
    protected int aind;
    protected String addr;
}
