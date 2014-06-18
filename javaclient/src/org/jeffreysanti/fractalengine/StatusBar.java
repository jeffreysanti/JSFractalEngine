/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package org.jeffreysanti.fractalengine;

import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.SwingConstants;
import javax.swing.border.BevelBorder;

/**
 *
 * @author jeffrey
 */
public class StatusBar extends JPanel implements ActionListener, MouseListener {
    
    public StatusBar(){        
        setBorder(new BevelBorder(BevelBorder.LOWERED));
        setPreferredSize(new Dimension(10000, 16));
        setLayout(new BoxLayout(this, BoxLayout.X_AXIS));
        
        lblJobs = new JLabel("status");
        lblJobs.setHorizontalAlignment(SwingConstants.LEFT);
        lblJobs.addMouseListener(this);
        add(lblJobs);
        
        add(Box.createHorizontalGlue());
        
        lblServer = new JLabel("connection");
        lblServer.setHorizontalAlignment(SwingConstants.RIGHT);
        lblServer.addMouseListener(this);
        add(lblServer);
        
        
    }

    @Override
    public void actionPerformed(ActionEvent arg0) {
        lblJobs.setText(ServerConnection.getInst().getJobsMessage());
        lblServer.setText(ServerConnection.getInst().getServerMessage());
    }
    

    @Override
    public void mouseClicked(MouseEvent arg0) {
        if(arg0.getSource() == lblServer){ // request connection dialogue
            ServerConnectionDialog dlg = new ServerConnectionDialog();
            dlg.show();
        }else{
            ServerStatusDialog dlg = new ServerStatusDialog();
            dlg.show();
        }
    }

    @Override
    public void mousePressed(MouseEvent arg0) {}
    @Override
    public void mouseReleased(MouseEvent arg0) {}
    @Override
    public void mouseEntered(MouseEvent arg0) {}
    @Override
    public void mouseExited(MouseEvent arg0) {}
    
    
    
    
    private JLabel lblServer;
    private JLabel lblJobs;
}
