/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package org.jeffreysanti.fractalengine;

import java.awt.Component;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.DataInputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JPanel;
import org.json.simple.JSONArray;
import org.json.simple.JSONObject;

/**
 *
 * @author jeffrey
 */
public class PanelProperties extends JPanel implements ServerReplyer {
    
    PanelProperties(){
        this.setLayout(new BoxLayout(this, BoxLayout.PAGE_AXIS));
        G = new HashMap();
        //P = new HashSet();
    }
    
    public void switchContext(Context cont){
        c = cont;
        
        for(JPanel p : G.values()){
            this.remove(p);
        }
        G.clear();
        
        if(cont.isDataImportedYet()){ // otherwise we need to wait
        
            ArrayList<String> grps = new ArrayList();
            SchemaManager.getInst().findConditionlessGroups(grps);
            
            for(String s : grps){
                addGroup(s);
            }
        }
        
        btnSubmit = new JButton("Submit Job");
        btnSubmit.addActionListener(new ActionListener(){
            @Override
            public void actionPerformed(ActionEvent ae) {
                submit();
            }
        });

        
        this.revalidate();
    }
    
    public void addGroup(String nm){
        JSONObject g = SchemaManager.getInst().getGroup(nm);
        if(g == null)
            return;
        JPanel pnl = new JPanel();
        pnl.setLayout(new BoxLayout(pnl, BoxLayout.PAGE_AXIS));
        if(g.containsKey("caption")){
            pnl.add(new JLabel((String)g.get("caption")));
        }
        
        // now prepare params file for group
        JSONObject paramGroup;
        if(c.getParams().containsKey(nm) && c.getParams().get(nm) instanceof JSONObject){
            paramGroup = (JSONObject)c.getParams().get(nm);
        }else{
            paramGroup = new JSONObject();
            c.getParams().put(nm, paramGroup);
        }
        
        JSONArray elms = (JSONArray)g.get("elms");
        for(Object o : elms){
            JSONObject elm = (JSONObject)o;
            String type = (String)elm.get("type");
            ParamsElement E;
            if(type.equals("text")){
                E = new ParamsElementText(elm, paramGroup, this, -1);
            }else if(type.equals("integer")){
                E = new ParamsElementIntegral(elm, paramGroup, this, -1);
            }else if(type.equals("selector")){
                E = new ParamsElementSelector(elm, paramGroup, this, -1);
            }else if(type.equals("color")){
                E = new ParamsElementColor(elm, paramGroup, this, -1);
            }else if(type.equals("real")){
                E = new ParamsElementReal(elm, paramGroup, this, -1);
            }else if(type.equals("array")){
                E = new ParamsElementArray(elm, paramGroup, this, -1);
            }else if(type.equals("tuple")){
                E = new ParamsElementTuple(elm, paramGroup, this, -1);
            }
            else{
                continue;
            }
            pnl.add(E.getInnerElm());
        }
        
        G.put(nm, pnl);
        updateGroupDisplay();
    }
    
    public void removeGroup(String nm){
        JPanel pnl = G.get(nm);
        if(pnl != null){
            G.remove(nm);
        }
        updateGroupDisplay();
    }
    
    public void showActuator(HashSet<String> set){
        for(String s : set){
            if(!G.containsKey(s)){
                addGroup(s);
            }
        }
    }
    public void hideActuator(HashSet<String> set){
        boolean added = true;
        while(added){ // used to prevent breaks from concurrent map access
            added = false;
            for(String g : G.keySet()){
                if(set.contains(g)){
                    removeGroup(g);
                    added = true;
                    break;
                }
            }
        }
    }
    
    public void updateGroupDisplay(){
        this.removeAll();
        ArrayList<String> ordered = SchemaManager.getInst().findAllOrderedGroups();
        for(String id  : ordered){
            if(G.containsKey(id)){
                this.add(G.get(id));
            }
        }
        this.add(btnSubmit);
        this.revalidate();
    }
    
    public void submit(){
        for(JPanel p : G.values()){
            p.setEnabled(false);
        }
        this.remove(btnSubmit);
        
        ServerConnection.getInst().addPacketToQueue("SJOB",c.getParams().toJSONString(), this); // request all avali info
    }
    
    
    private Context c;
    //private HashSet<Component> P;
    
    private HashMap<String, JPanel> G;
    
    private JButton btnSubmit;

    @Override
    public void onReceiveReply(String head, int len, DataInputStream ds) {
        int jid = Integer.parseInt(ServerPacket.extractString(ds, len));
        System.out.println("Submitted JOB: " + jid);
        c.becameSubmittedJob(jid);
    }
    
    void markDirty(){
        c.markDirty();
    }
}
