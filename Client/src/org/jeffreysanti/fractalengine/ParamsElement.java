/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package org.jeffreysanti.fractalengine;

import javax.swing.JComponent;
import org.json.simple.JSONObject;

/**
 *
 * @author jeffrey
 */
public class ParamsElement {
    public ParamsElement(JSONObject schemaDefn, JSONObject paramsContainer, PanelProperties cb){
        callback = cb;
        schem = schemaDefn;
        grp = paramsContainer;
        
        id = (String)schem.get("id");
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
    
    
    protected JSONObject schem;
    protected JSONObject grp;
    protected PanelProperties callback;
    protected String id;
}
