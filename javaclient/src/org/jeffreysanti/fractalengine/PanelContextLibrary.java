/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package org.jeffreysanti.fractalengine;

import java.awt.Dimension;
import java.awt.image.BufferedImage;
import java.io.DataInputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.swing.BoxLayout;
import javax.swing.JPanel;

/**
 *
 * @author jeffrey
 */
public class PanelContextLibrary extends JPanel {
    
    PanelContextLibrary(){
        setLayout(new BoxLayout(this, BoxLayout.LINE_AXIS));
        C = new ArrayList();
        
        this.setPreferredSize(new Dimension(2000, 120));
    }
    
    public void recievedFractalStatusUpdate(int jid, int status, String title, String author, int uid, BufferedImage img){
        for(ContextTile c : C){
            if(c.getContext().getJobID() == jid){
                if(!c.getContext().isUpdateableState()) // we cannot update this context
                    continue;
                if(c.getContext().getStatus() != status){ // context must be replaced
                    C.remove(c);
                    this.remove(c);
                    
                    Context cont = new Context(jid, title, author, uid, status, img);
                    ContextTile contTile = new ContextTile(cont);
                    C.add(contTile);
                    this.add(contTile);
                    
                    if(JavaDesktop.getInst().getWorkspacePanel().getContext() == c.getContext()){ // it's current
                        JavaDesktop.getInst().getWorkspacePanel().changeContext(cont);
                    }
                    this.revalidate();
                    break;
                }
            }
        }
    }
    
    public Context openNewContext(LibraryTile tile){
        Context cont = new Context(tile.getJobID(), tile.getTitle(), tile.getAuthor(), tile.getUserID(), 
                tile.getStatus(), tile.getFullSizeImage());
        
        // only one context is allowed for an "updatable"
        for(ContextTile c : C){
            if(c.getContext().getJobID() == cont.getJobID() && c.getContext().isUpdateableState()){
                return c.getContext();
            }
        }
        
        // add context to list
        ContextTile ctile = new ContextTile(cont);
        this.add(ctile);
        C.add(ctile);
        this.revalidate();
        
        return cont;
    }
    
    public Context openNewContextFromScratch(){
        Context cont = new Context(-1, "", "", ServerConnection.getInst().getUID(), 
                LibraryTile.FDBS_UNLINKED, null);
        
        // add context to list
        ContextTile ctile = new ContextTile(cont);
        this.add(ctile);
        C.add(ctile);
        this.revalidate();
        
        return cont;
    }
    
    private ArrayList<ContextTile> C;
}
