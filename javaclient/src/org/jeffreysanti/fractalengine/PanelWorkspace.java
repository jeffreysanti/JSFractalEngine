/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package org.jeffreysanti.fractalengine;

import java.awt.BorderLayout;
import java.io.DataInputStream;
import java.io.IOException;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.swing.JPanel;
import javax.swing.JScrollPane;

/**
 *
 * @author jeffrey
 */
public class PanelWorkspace extends JPanel {
    
    PanelWorkspace(){
        context = null;
        
        setLayout(new BorderLayout());

        contextLib = new PanelContextLibrary();
        JScrollPane scrollPane = new JScrollPane(contextLib, JScrollPane.VERTICAL_SCROLLBAR_NEVER, JScrollPane.HORIZONTAL_SCROLLBAR_ALWAYS);
        scrollPane.getHorizontalScrollBar().setUnitIncrement(10);
        add(scrollPane, BorderLayout.PAGE_END);
        
        PanelProperties props = new PanelProperties();
        scrollPane = new JScrollPane(props, JScrollPane.VERTICAL_SCROLLBAR_ALWAYS, JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
        scrollPane.getVerticalScrollBar().setUnitIncrement(10);
        add(scrollPane, BorderLayout.LINE_START);
        
        viewer = new PanelViewer(props);
        add(viewer, BorderLayout.CENTER);
        
    }
    
    public void openNewContext(LibraryTile tile){
        changeContext(contextLib.openNewContext(tile));
    }
    
    public void openNewContextFromScratch(){
        changeContext(contextLib.openNewContextFromScratch());
    }
    
    
    public void changeContext(Context cont){
        context = cont;
        //if(cont.isReady())
            viewer.switchToContext();
    }
    
    public PanelViewer getViewer(){
        return viewer;
    }
    public PanelContextLibrary getContextLibrary(){
        return contextLib;
    }
    
    public Context getContext(){
        return context;
    }
    
    
    private PanelContextLibrary contextLib;
    private PanelViewer viewer;
    private Context context; // current one
}
