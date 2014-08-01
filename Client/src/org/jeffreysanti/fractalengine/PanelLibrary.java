/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package org.jeffreysanti.fractalengine;

import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.awt.image.BufferedImage;
import java.io.ByteArrayInputStream;
import java.io.DataInputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.imageio.ImageIO;
import javax.swing.BorderFactory;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JTextField;
import javax.swing.Timer;

/**
 *
 * @author jeffrey
 */
public class PanelLibrary extends JPanel implements ASyncPoolAcceptor {
    
    private final int REQUEST_GAP = 1000;
    private final long REQUEST_GAP_SSLT = 2000L;
    private final int REQUEST_GAP_RE_SSLT = 60000;
    private final int REQUEST_GAP_REREQ = 10000;
    
    public PanelLibrary(){
        
        lastUpdate = System.currentTimeMillis();
        
        setLayout(new BoxLayout(this, BoxLayout.PAGE_AXIS));
        this.setBorder(BorderFactory.createEmptyBorder());
        
        txtSearch = new JTextField(15);
        txtSearch.setMaximumSize(txtSearch.getPreferredSize());
        txtSearch.setText("");
        
        txtSearch.addKeyListener(new KeyListener() {
            @Override
            public void keyTyped(KeyEvent ke) {}
            @Override
            public void keyPressed(KeyEvent ke) {}
            @Override
            public void keyReleased(KeyEvent ke) {update();}
        });
        add(txtSearch);
        
        btnNew = new JButton("New");
        btnNew.addActionListener(new ActionListener(){

            @Override
            public void actionPerformed(ActionEvent ae) {
                JavaDesktop.getInst().getWorkspacePanel().openNewContextFromScratch();
            }
        });
        this.add(btnNew);
        
        T = new ArrayList();
        
        timer = new javax.swing.Timer(REQUEST_GAP_RE_SSLT, new ActionListener() { // once per minute
            @Override
            public void actionPerformed(ActionEvent e) {
                update();
            }
        });
        timer.setRepeats(false);
        timer2 = new javax.swing.Timer(REQUEST_GAP_REREQ, new ActionListener() { // once per minute
            @Override
            public void actionPerformed(ActionEvent e) {
                updateReRequest();
            }
        });
        timer2.start();
        timer2.setRepeats(true);
    }
    
    public void handleServerPackets(String head, DataInputStream ds){
        int jid = 0;
        if(head.equals("MDUD")){ // we received an update
            try {
                jid = ds.readInt();
                int uid = ds.readInt();
                String title = ServerConnection.extractString(ds, ds.readInt());
                String author = ServerConnection.extractString(ds, ds.readInt());
                int status = ds.readInt();
                byte[] imgData = null;
                int sz = ds.readInt();
                if(sz > 0 && sz <= ServerConnection.MAXSZ){
                    imgData = new byte[sz];
                    ds.readFully(imgData);
                }
                
                BufferedImage imag = null;
                if(imgData != null){
                    imag=ImageIO.read(new ByteArrayInputStream(imgData));
                }

                for(LibraryTile t : T){
                    if(t.getJobID() == jid){
                        t.receivedData(uid, status, title, author, imgData);
                        break;
                    }
                }
                
                JavaDesktop.getInst().getWorkspacePanel().getContextLibrary().recievedFractalStatusUpdate(jid, status, title, author, uid, imag);
            } catch (Exception ex) {
                Logger.getLogger(PanelLibrary.class.getName()).log(Level.SEVERE, null, ex);
                System.err.println("Error Improting: MDUD: " + jid);
            }
        }
    }
    
    public void update(){
        timer.stop();
        long now = System.currentTimeMillis();
        if(now - lastUpdate > REQUEST_GAP_SSLT){
            System.out.println("Requesting Library Refresh!!!");
            timer.restart(); // so that we do not auto update right after manually updating
            if(ServerConnection.getInst().isConnected()){
                ServerConnection.getInst().sendPacket("SSLT20|" + txtSearch.getText());
                ServerConnection.getInst().getASyncPool().addASyncAcceptor("SSLT", this);
            }
            lastUpdate = now;
            timer.setDelay(REQUEST_GAP_RE_SSLT);
            timer.setInitialDelay(REQUEST_GAP_RE_SSLT);
        }else{
            timer.setDelay(REQUEST_GAP_RE_SSLT);
            timer.setInitialDelay((int)REQUEST_GAP_SSLT);
        }
        timer.start();
    }
    
    // send any nessesary re-requests if they failed to come at first
    public void updateReRequest(){
        int deferTimer = 0;
        for(LibraryTile t : T){
            if(t.getStatus() == LibraryTile.FDBS_UNLINKED){
                t.updateData(deferTimer);
                deferTimer += REQUEST_GAP;
            }
        }
    }
    
    
    private JTextField txtSearch;
    private JButton btnNew;
    private ArrayList<LibraryTile> T;
    private Timer timer;
    private Timer timer2;
    private long lastUpdate;

    @Override
    public boolean poolDataReceived(String head, DataInputStream ds) {
        String resp = ServerConnection.extractString(ds);
        if(resp.length() > 0){
            String[] arr = resp.split(",");
            HashMap<Integer, LibraryTile> old = new HashMap();
            for(LibraryTile t : T){
                old.put(t.getJobID(), t);
                this.remove(t);
            }
            T.clear();
            int deferTimer = 0;
            
            deferTimer += REQUEST_GAP; // wait for initial request
            for(String s : arr){
                int jid = Integer.parseInt(s);

                // if we already have info (inc. title) -> do not recheck for it
                if(old.containsKey(jid) && ! old.get(jid).isUpdatableStatus()){
                    T.add(old.get(jid));
                    this.add(old.get(jid));
                }else{
                    LibraryTile t = new LibraryTile(jid);
                    t.updateData(deferTimer);
                    this.add(t);
                    T.add(t);
                    deferTimer += REQUEST_GAP;
                }
            }
            old.clear();
            this.revalidate();
        }
        return true;
    }
}
