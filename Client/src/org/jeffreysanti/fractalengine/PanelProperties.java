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
                E = new ParamsElementText(elm, paramGroup, this);
            }else if(type.equals("integer")){
                E = new ParamsElementIntegral(elm, paramGroup, this);
            }else if(type.equals("selector")){
                E = new ParamsElementSelector(elm, paramGroup, this);
            }else{
                continue;
            }
            pnl.add(E.getInnerElm());
        }
        
        G.put(nm, pnl);
        this.add(pnl);
        this.revalidate();
    }
    
    public void removeGroup(String nm){
        JPanel pnl = G.get(nm);
        if(pnl != null){
            this.remove(pnl);
            G.remove(nm);
        }
        this.revalidate();
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
    
    /*public void addProperty(Component comp) {
        this.add(comp);
        P.add(comp);
    }*/
    
    public void submit(){
        /*for(Component p : P){
            p.setEnabled(false);
        }*/
        
        //ServerConnection.getInst().addPacketToQueue("SJOB",c.getParams().dumpToString(), this); // request all avali info
    }
    
    
    private Context c;
    //private ArrayList<Component> P;
    
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
