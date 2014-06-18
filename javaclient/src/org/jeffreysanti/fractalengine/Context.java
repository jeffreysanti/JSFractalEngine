/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package org.jeffreysanti.fractalengine;

import java.awt.image.BufferedImage;
import java.io.DataInputStream;
import java.io.IOException;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 *
 * @author jeffrey
 */

// A context is a local instance of a fractal that can be edited
// it holds the image of an old render, paramaters (either modified or not),
//      log info, histograms, etc

public class Context implements ASyncPoolAcceptor {
    
    Context(int job, String nm, String auth, int userid, int stat, BufferedImage full){
        jid = job;
        name = nm;
        author = auth;
        uid = userid;
        status = stat;
        imgFull = full;
        histo = new int[0];
        
        modified = false;
        params = new Paramaters();
        paramsOut = new Paramaters();
        //isFetched = true;
        
        if(jid >= 0){
            //isFetched = false;
            ServerConnection.getInst().sendPacket("RCTX" + jid); // request all avali info
            ServerConnection.getInst().getASyncPool().addASyncAcceptor("RCTX", this);
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
    
    public Paramaters getParams(){
        return params;
    }
    
    public Paramaters getParamsOutput(){
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
                params.loadFromStream(tmp);
            }
            len = ds.readInt();
            if(len > 0){
                byte[] tmp = new byte[len];
                ds.readFully(tmp);
                paramsOut.loadFromStream(tmp);
            }
            len = ds.readInt();
            if(len > 0){
                byte[] tmp = new byte[len];
                ds.readFully(tmp);
                log = new String(tmp);
            }
            
            
            // TODO: histogram
            len = ds.readInt();
            if(len > 0){
                if(len % 4 == 0){ // should be list of integers
                    histo = new int[len/4];
                    for(int i=0; i<len/4; i++){
                        histo[i] = ds.readInt();
                    }
                }else{
                    byte[] tmp = new byte[len];
                    ds.readFully(tmp);
                }
            }
            
            
        } catch (IOException ex) {
            Logger.getLogger(Context.class.getName()).log(Level.SEVERE, null, ex);
        }
    }
    
    private void onReceiveRCTX(DataInputStream ds){
        updateInfoFromRCTX(ds);
        isUpdated = true;
            
        if(JavaDesktop.getInst().getWorkspacePanel().getContext() == this){
            JavaDesktop.getInst().getWorkspacePanel().changeContext(this); // force panel to update
        }
    }
    
    public int[] getHistogram(){
        return histo;
    }
    
    public String getLog(){
        return log;
    }
    
    public void becameSubmittedJob(int j){
        jid = j;
        uid = ServerConnection.getInst().getUID();
        status = LibraryTile.FDBS_RENDER_SUBMITTED;
        name = params.getValue("title", "[Just Submitted]");
        author = params.getValue("author", "...");
        
        JavaDesktop.getInst().getWorkspacePanel().getContextLibrary().recievedFractalStatusUpdate(jid, 
                        LibraryTile.FDBS_QUEUED, name, author, uid, null);
    }
    
    
    
    private int jid; // -1 if new
    private String name;
    private String author;
    private int uid;
    private Paramaters params, paramsOut;
    private String log;
    
    public ContextTile callBack = null;
    
    private int[] histo;
    
    private int status;
    private BufferedImage imgFull;
    
    private boolean modified;
    //private volatile boolean isFetched
    
    private boolean isUpdated;

    @Override
    public boolean poolDataReceived(String head, DataInputStream ds) {
        try {
            if(ds.available() >=4){
                int j = ds.readInt();
                if(j == jid){
                    onReceiveRCTX(ds);
                    return true;
                }
                
            }
        } catch (IOException ex) {
            Logger.getLogger(Context.class.getName()).log(Level.SEVERE, null, ex);
        }
        return false;
    }
    
}
