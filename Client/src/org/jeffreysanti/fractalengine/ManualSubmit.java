/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package org.jeffreysanti.fractalengine;

import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowEvent;
import java.io.DataInputStream;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;

/**
 *
 * @author jeffrey
 */
public class ManualSubmit extends JFrame implements ServerReplyer {
    ManualSubmit(){
        super();
        this.setLayout(new BorderLayout());
        this.setSize(new Dimension(400, 400));
        this.setTitle("Manual Submit");
        
        final JTextArea txt = new JTextArea();
        txt.setColumns(100);
        txt.setRows(40);
        
        final JButton btnSubmit = new JButton("Submit");
        btnSubmit.addActionListener(new ActionListener(){
            @Override
            public void actionPerformed(ActionEvent e) {
                ServerConnection.getInst().addPacketToQueue("SJOB",txt.getText(), ManualSubmit.this); // request all avali info
                setDefaultCloseOperation(javax.swing.WindowConstants.DO_NOTHING_ON_CLOSE);
                txt.setEnabled(false);
                btnSubmit.setEnabled(false);
            }
        });
        
        
        
        
        this.add(BorderLayout.SOUTH, btnSubmit);
        this.add(BorderLayout.CENTER, new JScrollPane(txt));
        
        setDefaultCloseOperation(javax.swing.WindowConstants.DISPOSE_ON_CLOSE);
    }

    @Override
    public void onReceiveReply(String head, int len, DataInputStream data) {
        int jid = Integer.parseInt(ServerPacket.extractString(data, len));
        System.out.println("Submitted JOB: " + jid);
        
        Context c = JavaDesktop.getInst().getWorkspacePanel().getContextLibrary().openNewContextFromScratch();
        JavaDesktop.getInst().getWorkspacePanel().changeContext(c);
        c.becameSubmittedJob(jid);
        
        setDefaultCloseOperation(javax.swing.WindowConstants.DISPOSE_ON_CLOSE);
        dispatchEvent(new WindowEvent(this, WindowEvent.WINDOW_CLOSING));
    }
    
    
}
