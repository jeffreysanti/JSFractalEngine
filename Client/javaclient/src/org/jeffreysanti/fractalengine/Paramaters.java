/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package org.jeffreysanti.fractalengine;

import java.io.PrintWriter;
import java.util.HashMap;

/**
 *
 * @author jeffrey
 */
public class Paramaters {
    
    public Paramaters(){
        P = new HashMap();
        id = -1;
    }
    
    public void  setID(int i)
    {
        if(id != -1) // cannot be changed
            return;
        id = i;
    }

    public int getID()
    {
            return id;
    }

    public void setValue(String key, String val)
    {
        P.put(key, val);
    }

    public String getValue(String key, String defValue)
    {
        if(!P.containsKey(key))
            return defValue;
        return P.get(key);
    }

    public void loadFromStream(byte[] byteArray){
        String str = new String(byteArray);
        String[] lines = str.split("\n");
        for(String line : lines){
            if(!line.contains(":"))
                continue;
            String key = line.substring(0, line.indexOf(':'));
            String val = line.substring(line.indexOf(':')+1);
            P.put(key, val);
        }
    }
    
    public String dumpToString(){
        String str = "";
        for(String key : P.keySet()){
            str += key + ":"+P.get(key) + "\n";
        }
        return str;
    }
    
    /*boolean writeToFile(String flName)
    {
        try{
            PrintWriter writer = new PrintWriter(flName, "UTF-8");
            for(String key : P.keySet()){
                writer.print(key + ":" + P.get(key) + "\n");
            }
            writer.close();
        }catch(Exception e){
            return false;
        }
        return true;
    }*/

    
    private HashMap<String, String> P;
    private int id;
}
