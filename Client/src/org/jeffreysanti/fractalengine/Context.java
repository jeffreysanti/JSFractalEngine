/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package org.jeffreysanti.fractalengine;

import info.monitorenter.gui.chart.Chart2D;
import info.monitorenter.gui.chart.ITrace2D;
import info.monitorenter.gui.chart.traces.Trace2DSimple;
import info.monitorenter.gui.chart.traces.painters.TracePainterVerticalBar;
import java.awt.image.BufferedImage;
import java.io.BufferedReader;
import java.io.DataInputStream;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.logging.Level;
import java.util.logging.Logger;
import org.json.simple.JSONObject;
import org.json.simple.parser.JSONParser;
import org.json.simple.parser.ParseException;

/**
 *
 * @author jeffrey
 */

// A context is a local instance of a fractal that can be edited
// it holds the image of an old render, paramaters (either modified or not),
//      log info, histograms, etc

public class Context implements ServerReplyer {
    
    Context(int job, String nm, String auth, int userid, int stat, BufferedImage full){
        jid = job;
        name = nm;
        author = auth;
        uid = userid;
        status = stat;
        imgFull = full;
        
        modified = false;
        params = new JSONObject();
        paramsOut = new JSONObject();
        charts = new ArrayList();
        //isFetched = true;
        
        if(jid >= 0){
            //isFetched = false;
            ServerConnection.getInst().addPacketToQueue("RCTX", ""+jid, this); // request all avali info
            isUpdated = false;
        }else
            isUpdated = true;
    }
    
    // should we listen to any server "updates" -> if so the context may be thrown away at will
    public boolean isUpdateableState(){
        return (status == LibraryTile.FDBS_QUEUED || status == LibraryTile.FDBS_RENDERING || status == LibraryTile.FDBS_RENDER_SUBMITTED);
    }
    
    // can user edit paramaters?
    public boolean isForkable(){
        return (!isUpdateableState() && status != LibraryTile.FDBS_ERR && status != LibraryTile.FDBS_RENDER_SUBMITTED); // must be done and not err
    }
    
    public boolean isComplete(){
        return status == LibraryTile.FDBS_COMPLETE;
    }
    
    public int getJobID(){
        return jid;
    }
    public int getStatus(){
        return status;
    }
    public BufferedImage getFullImage(){
        return imgFull;
    }
    
    public JSONObject getParams(){
        return params;
    }
    
    public JSONObject getParamsOutput(){
        return paramsOut;
    }
    
    public boolean isDataImportedYet(){
        return isUpdated;
    }
    
    /*public boolean isReady(){
        return isFetched;
    }*/
    
    public String getTitle(){
        return name;
    }
    public String getAuthor(){
        return author;
    }
    
    public boolean isModified(){
        return modified;
    }
    
    public void markDirty(){
        modified = true;
        if(callBack != null)
            callBack.onContextModifed();
    }
    
    public void updateInfoFromRCTX(DataInputStream ds){
        try {
            int len = ds.readInt();
            if(len > 0){
                byte[] tmp = new byte[len];
                ds.readFully(tmp);
                rawParams = new String(tmp);
                JSONParser parser = new JSONParser();
                Object t = null;
                try {
                    t = parser.parse(rawParams);
                } catch (ParseException ex) {
                    // all is okay
                }
                if(t instanceof JSONObject)
                    params = (JSONObject)t;
            }
            len = ds.readInt();
            if(len > 0){
                byte[] tmp = new byte[len];
                ds.readFully(tmp);
                rawParamsOut = new String(tmp);
                JSONParser parser = new JSONParser();
                Object t = null;
                try {
                    t = parser.parse(rawParamsOut);
                } catch (ParseException ex) {
                    // all is okay
                }
                if(t instanceof JSONObject)
                    paramsOut = (JSONObject)t;
            }
            len = ds.readInt();
            if(len > 0){
                byte[] tmp = new byte[len];
                ds.readFully(tmp);
                log = new String(tmp);
            }
            
            seperateGraphsFromResults();
            
        } catch (IOException ex) {
            Logger.getLogger(Context.class.getName()).log(Level.SEVERE, null, ex);
        }
    }
    
    private void seperateGraphsFromResults(){
        charts.clear();
        if(!paramsOut.containsKey("graphs"))
            return;
        ArrayList<HashMap<String,Object>> G = (ArrayList)paramsOut.get("graphs");
        for(HashMap<String,Object> g : G){
            charts.add(ViewPaneHistogram.parseGraph(g));
        }
        paramsOut.put("graphs", null);
    }
    
    public ArrayList<Chart2D> getGraphs(){
        return charts;
    }
    
    private void onReceiveRCTX(DataInputStream ds){
        updateInfoFromRCTX(ds);
        isUpdated = true;
            
        if(JavaDesktop.getInst().getWorkspacePanel().getContext() == this){
            JavaDesktop.getInst().getWorkspacePanel().changeContext(this); // force panel to update
        }
    }
    
    public String getLog(){
        return log;
    }
    
    public void becameSubmittedJob(int j){
        jid = j;
        uid = ServerConnection.getInst().getUserID();
        status = LibraryTile.FDBS_RENDER_SUBMITTED;
        
        if(params.get("basic") != null){
            name = (String)((JSONObject)params.get("basic")).get("name");//params.getValue("title", "[Just Submitted]");
            author = (String)((JSONObject)params.get("basic")).get("author");//params.getValue("author", "...");
        }else{
            name = "Manually Submitted";
            author = "NA";
        }
        
        JavaDesktop.getInst().getWorkspacePanel().getContextLibrary().recievedFractalStatusUpdate(jid, 
                        LibraryTile.FDBS_QUEUED, name, author, uid, null);
    }
    
    
    
    private int jid; // -1 if new
    private String name;
    private String author;
    private int uid;
    private JSONObject params, paramsOut;
    private String rawParams, rawParamsOut;
    private String log;
    
    ArrayList<Chart2D> charts;
    
    public ContextTile callBack = null;
    
    private int status;
    private BufferedImage imgFull;
    
    private boolean modified;
    //private volatile boolean isFetched
    
    private boolean isUpdated;

    @Override
    public void onReceiveReply(String head, int len, DataInputStream data) {
        try {
            if(data.available() >=4){
                int j = data.readInt();
                if(j == jid){
                    onReceiveRCTX(data);
                }
            }
        } catch (IOException ex) {
            Logger.getLogger(Context.class.getName()).log(Level.SEVERE, null, ex);
        }
    }
    
}
