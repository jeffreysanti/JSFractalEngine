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
import java.io.DataInputStream;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.SwingConstants;
import javax.swing.SwingUtilities;
import javax.swing.border.BevelBorder;

/**
 *
 * @author jeffrey
 */
public class StatusBar extends JPanel implements MouseListener, ServerMessageListener {
    
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
        
        ServerConnection.getInst().addServerMessageListener("UPDT", this);
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

    @Override
    public void onReceivePacket(String head, int len, DataInputStream data) {
        lblJobs.setText(ServerPacket.extractString(data, len));
    }
    
    public void setConnectionString(final String host){
        SwingUtilities.invokeLater(new Runnable(){
            @Override
            public void run() {
                lblServer.setText(host);
            }
        });
    }
    
}
