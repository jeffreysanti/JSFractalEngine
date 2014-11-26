/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package org.jeffreysanti.fractalengine;

import info.monitorenter.gui.chart.Chart2D;
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
import org.jeffreysanti.fractalengine.Context.Artifact;

/**
 *
 * @author jeffrey
 */
public class PanelViewer extends JPanel implements ServerReplyer {
    
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
        
        // now add the artifact tabs
        if(c.isComplete()){
            for(Artifact a : c.getArtifacts()){
                if(a.type.equals("IMGS")){                    
                    addPane(new ViewPaneTransformableImage(c, a.data), "Image");
                }
            }
        }
        
        
        addPane(new ViewPaneLog(c), "Log");
        if(!c.isUpdateableState()){
            addPane(new ViewPaneParams(c, c.getParamsOutput()), "Results");
            for(Chart2D graph : c.getGraphs()){
                addPane(new ViewPaneHistogram(c, graph), graph.getName());
            }
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
            ServerConnection.getInst().addPacketToQueue("RCTX", ""+c.getJobID(), this); // request all avali info
        }
    }
    
    private Context c;
    private PanelProperties props;
    private JTabbedPane pane;
    private Timer timer;
    private ArrayList<ViewPaneAbstract> P;
    
    @Override
    public void onReceiveReply(String head, int len, DataInputStream data) {
        if(c == null || c.getJobID() < 0)
            return;
        
        if(head.equals("RCTX")){
            try {
                if(data.available() >=4){
                    int j = data.readInt();
                    if(j == c.getJobID()){
                        c.updateInfoFromRCTX(data);
                        for(ViewPaneAbstract p : P){
                            p.contextDataReceieved();
                        }
                    }
                }
            } catch (IOException ex) {
                Logger.getLogger(PanelViewer.class.getName()).log(Level.SEVERE, null, ex);
            }
        }
    }
}
