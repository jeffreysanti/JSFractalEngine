/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package org.jeffreysanti.fractalengine;

import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Queue;
import java.util.logging.Level;
import java.util.logging.Logger;
import org.json.simple.JSONArray;
import org.json.simple.JSONObject;
import org.json.simple.parser.JSONParser;
import org.json.simple.parser.ParseException;

/**
 *
 * @author jeffrey
 */
public class SchemaManager {
    
    SchemaManager(){
        String schemaPath = "/home/jeffrey/dev/JSFractalEngine/Server/etc/schema.json";
        JSONParser parser = new JSONParser();
        Object objRoot;
        try {
            objRoot = parser.parse(new FileReader(schemaPath));
        } catch (ParseException ex) {
            System.err.println("Error in schema initialization [SchemaManager::initialize]!");
            System.err.println("   File failed to parse: '" + schemaPath + "'");
            System.err.println("   Is json invalid?");
            System.err.println(ex.toString());
            System.exit(1);
            return;
        } catch (FileNotFoundException ex) {
            System.err.println("Error in schema initialization [SchemaManager::initialize]!");
            System.err.println("   File failed to parse: '" + schemaPath + "'");
            System.err.println("   File does not exist");
            System.exit(1);
            return;
        } catch (IOException ex) {
            System.err.println("Error in schema initialization [SchemaManager::initialize]!");
            System.err.println("   File failed to parse: '" + schemaPath + "'");
            System.err.println("   Does this file not exist, or is json invalid?");
            System.exit(1);
            return;
        }
        if(!(objRoot instanceof JSONObject)){
            System.err.println("Error in schema initialization [SchemaManager::initialize]!");
            System.err.println("   Root must be object type!");
            System.exit(1);
            return;
        }
        root = (JSONObject)objRoot;
        
        if(!root.containsKey("groups") || !(root.get("groups") instanceof JSONArray) || 
                ((JSONArray)root.get("groups")).isEmpty()){
            System.err.println("Error in schema initialization [SchemaManager::initialize]!");
            System.err.println("   Schema must contain array of groups!");
            System.exit(1);
            return;
        }
        
        JSONArray groupList = (JSONArray)root.get("groups");
        for(Object o : groupList){
            if(o instanceof JSONObject){
                JSONObject g = (JSONObject)o;
                if(g.containsKey("elms") && g.containsKey("id") && g.get("elms") instanceof JSONArray){
                    G.put((String)g.get("id"), (JSONObject)g);
                    OG.add((String)g.get("id"));
                }
            }
        }
    }
    
    public void findConditionlessGroups(ArrayList<String> lst)
    {
        for(String id : G.keySet()){
            if(G.get(id).containsKey("active")){
                lst.add(id);
            }
        }
    }
    public ArrayList<String> findAllOrderedGroups()
    {
        return OG;
    }
    
    public ArrayList<String> getGroupList(JSONObject r){
        ArrayList<String> L = new ArrayList();
        findConditionlessGroups(L);
        
        return L;
    }
    
    public JSONObject getGroup(String nm){
        if(G.containsKey(nm) && G.get(nm) instanceof JSONObject){
            return G.get(nm);
        }
        return null;
    }
    
    public HashSet<String> expandGroupSet(ArrayList<String> set){
        HashSet<String> ret = new HashSet();
        
        for(String s : set){
            if(s.charAt(s.length()-1) != '*'){
                ret.add(s);
            }else{
                String tmp = s.substring(0, s.length()-1);
                for(String g : G.keySet()){
                    if(g.contains(tmp)){
                        ret.add(g);
                    }
                }
            }
        }
        
        return ret;
    }
    
    
    private JSONObject root;
    private HashMap<String, JSONObject> G = new HashMap();
    private ArrayList<String> OG = new ArrayList(); // ordered groups
    
    public static SchemaManager getInst(){
        return schmMngr;
    }
    
    private static final SchemaManager schmMngr = new SchemaManager();
    
}
