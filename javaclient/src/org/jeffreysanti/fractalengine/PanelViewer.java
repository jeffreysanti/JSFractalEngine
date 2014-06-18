/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package org.jeffreysanti.fractalengine;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.DataInputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.swing.JPanel;
import javax.swing.JTabbedPane;
import javax.swing.Timer;

/**
 *
 * @author jeffrey
 */
public class PanelViewer extends JPanel implements ASyncPoolAcceptor {
    
    PanelViewer(PanelProperties p){
        c = null;
        props = p;
        P = new ArrayList();
        
        this.setBackground(Color.BLACK);
        this.setLayout(new BorderLayout());
        
        pane = new JTabbedPane();
        this.add(pane, BorderLayout.CENTER);
        
        timer = new javax.swing.Timer(1000, new ActionListener() { // once per minute
            @Override
            public void actionPerformed(ActionEvent e) {
                update();
            }
        });
        timer.start();
    }
    
    public void switchToContext(){
        c = JavaDesktop.getInst().getWorkspacePanel().getContext();
        props.switchContext(c);
        int counter = pane.getTabCount();
        for(int i=0; i< counter; i++){
            P.remove(0);
            pane.removeTabAt(0);
        }
        
        // now add the nessesary tabs
        if(c.isComplete()){
            addPane(new ViewPaneImage(c), "Image");
            addPane(new ViewPaneHistogram(c), "Iteration Histogram");
        }
        addPane(new ViewPaneLog(c), "Log");
        if(!c.isUpdateableState()){
            addPane(new ViewPaneParams(c, c.getParamsOutput()), "Results");
        }
        
        addPane(new ViewPaneParams(c, c.getParams()), "Paramaters");
    }
    
    private void addPane(ViewPaneAbstract p, String title){
        pane.addTab(title, p);
        P.add(p);
    }
    
    public void update(){
        if(c == null)
            return;
        if(c.getJobID() >= 0 && c.isUpdateableState()){
            ServerConnection.getInst().sendPacket("RCTX" + c.getJobID()); // request all avali info
            ServerConnection.getInst().getASyncPool().addASyncAcceptor("RCTX", this);
        }
    }
    
    private Context c;
    private PanelProperties props;
    private JTabbedPane pane;
    private Timer timer;
    private ArrayList<ViewPaneAbstract> P;
    
    @Override
    public boolean poolDataReceived(String head, DataInputStream ds) {
        if(c == null || c.getJobID() < 0)
            return false;
        
        if(head.equals("RCTX")){
            try {
                if(ds.available() >=4){
                    int j = ds.readInt();
                    if(j == c.getJobID()){
                        c.updateInfoFromRCTX(ds);
                        for(ViewPaneAbstract p : P){
                            p.contextDataReceieved();
                        }
                        return true;
                    }
                }
            } catch (IOException ex) {
                Logger.getLogger(PanelViewer.class.getName()).log(Level.SEVERE, null, ex);
            }
        }
        return false;
    }
}
