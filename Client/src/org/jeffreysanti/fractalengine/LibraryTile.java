/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package org.jeffreysanti.fractalengine;

import java.awt.Image;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.image.BufferedImage;
import java.io.ByteArrayInputStream;
import java.io.DataInputStream;
import java.io.File;
import java.io.IOException;
import java.net.URL;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.imageio.ImageIO;
import javax.swing.ImageIcon;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import static javax.swing.JOptionPane.YES_NO_OPTION;
import static javax.swing.JOptionPane.YES_OPTION;
import javax.swing.JPopupMenu;
import javax.swing.Timer;

/**
 *
 * @author jeffrey
 */
public class LibraryTile extends javax.swing.JPanel {

    public static final int FDBS_UNLINKED = -10;
    public static final int FDBS_RENDER_SUBMITTED = -5;
    public static final int FDBS_RENDERING = 0;
    public static final int FDBS_COMPLETE = 10;
    public static final int FDBS_QUEUED = 20;
    public static final int FDBS_CANCEL = 30;
    public static final int FDBS_TIMEOUT = 40;
    public static final int FDBS_ERR = 50;
    
    /**
     * Creates new form LibraryTile
     */
    
    static ImageIcon icoNo, icoCanceled, icoTimeout, icoError, icoInQueue, icoRendering;
    static{
        icoNo = new ImageIcon(LibraryTile.class.getResource("/images/noimg.png"), "No Image Avaliable");
        
        icoCanceled = new ImageIcon(LibraryTile.class.getResource("/images/canceled.png"), "Rendering Canceled");
        icoTimeout = new ImageIcon(LibraryTile.class.getResource("/images/timedout.png"), "Rendering Timed Out");
        icoError = new ImageIcon(LibraryTile.class.getResource("/images/error.png"), "Error Occured");
        icoInQueue = new ImageIcon(LibraryTile.class.getResource("/images/inqueue.png"), "Render Waiting in Queue");
        icoRendering = new ImageIcon(LibraryTile.class.getResource("/images/rendering.png"), "Currently Rendering");
    }
    
    
    
    public LibraryTile(int jid) {
        job = jid;
        uid = -1;
        status = FDBS_UNLINKED;
        initComponents();
        
        lblTitle.setText("");
        lblAuthor.setText("");
        lblNum.setText(Integer.toString(job));
        img.setIcon(icoNo);
        
        this.addMouseListener(new ContextMenuListener());
        lblTitle.addMouseListener(new ContextMenuListener());
        lblAuthor.addMouseListener(new ContextMenuListener());
        lblNum.addMouseListener(new ContextMenuListener());
        img.addMouseListener(new ContextMenuListener());
        
        updateTimer = new Timer(5000, new ActionListener(){
            @Override
            public void actionPerformed(ActionEvent ae) {
                updateData(0);
                updateTimer.stop();
            }
        });
        updateTimer.setRepeats(false);
        updateTimer.stop();
    }
    
    public void receivedData(int userId, int stat, String title, String author, byte[] img){
        try{
            lblTitle.setText(title);
            lblAuthor.setText(author);
            lblNum.setText(Integer.toString(job));
            status = stat;
            uid = userId;
            if(status == FDBS_COMPLETE){
                if(img != null){
                    try {
                        BufferedImage imag=ImageIO.read(new ByteArrayInputStream(img));
                        Image simag = imag.getScaledInstance(150, 150, Image.SCALE_SMOOTH);
                        ImageIcon icon = new ImageIcon(simag);
                        this.img.setIcon(icon);

                        image = imag; // save full copy
                    } catch (IOException ex) {
                        Logger.getLogger(LibraryTile.class.getName()).log(Level.SEVERE, null, ex);
                    }
                }else{
                    this.img.setIcon(icoNo);
                    image = null;
                }
            }else if(status == FDBS_RENDERING){
                this.img.setIcon(icoRendering);
                image = null;
            }else if(status == FDBS_QUEUED){
                this.img.setIcon(icoInQueue);
                image = null;
            }else if(status == FDBS_CANCEL){
                this.img.setIcon(icoCanceled);
                image = null;
            }else if(status == FDBS_TIMEOUT){
                this.img.setIcon(icoTimeout);
                image = null;
            }else if(status == FDBS_ERR){
                this.img.setIcon(icoError);
                image = null;
            }else{
                this.img.setIcon(icoNo);
                image = null;
            }
        }catch(Exception e){
            status = FDBS_UNLINKED;
            this.img.setIcon(icoNo);
            image = null;
        }
    }
    
    public void updateData(int deferTime){
        
        if(updateTimer.isRunning()) // do not force another request
            return;
        
        // retrieve data for job
        if(deferTime > 0){
            updateTimer.stop();
            updateTimer.setInitialDelay(deferTime);
            updateTimer.start();
        }else{
            ServerConnection.getInst().addPacketToQueue("RJIF", ""+job, null);
        }
    }
    
    public int getJobID(){
        return job;
    }
    public int getStatus(){
        return status;
    }
    public boolean isUpdatableStatus(){
        return status == FDBS_RENDERING || status == FDBS_QUEUED || status == FDBS_UNLINKED;
    }
    public int getUserID(){
        return uid;
    }
    public String getTitle(){
        return lblTitle.getText();
    }
    public String getAuthor(){
        return lblAuthor.getText();
    }
    public BufferedImage getFullSizeImage(){
        return image;
    }
    
    public boolean canUserEdit(){
        if(uid < 0)
            return false;
        if(ServerConnection.getInst().isConnected() && ServerConnection.getInst().isAdmin())
            return true;
        if(ServerConnection.getInst().getUserID() == uid)
            return true;
        return false;
    }
    
    class ContextMenu extends JPopupMenu {
        JMenuItem itmSetTitle, itmSetAuthor, itmDel;
        public ContextMenu(){
            itmSetTitle = new JMenuItem("Set Fractal Title");
            itmSetTitle.addActionListener(new ActionListener(){
                @Override
                public void actionPerformed(ActionEvent ae) {
                    String old = lblTitle.getText();
                    String newVal = JOptionPane.showInputDialog("Fractal Title?", old);
                    if(newVal.length() > 0){
                        ServerConnection.getInst().addPacketToQueue("SFTI", job+"|"+newVal, null);
                    }
                }
            });
            add(itmSetTitle);
            
            itmSetAuthor = new JMenuItem("Set Fractal Author");
            itmSetAuthor.addActionListener(new ActionListener(){
                @Override
                public void actionPerformed(ActionEvent ae) {
                    String old = lblAuthor.getText();
                    String newVal = JOptionPane.showInputDialog("Fractal Author?", old);
                    if(newVal.length() > 0){
                        ServerConnection.getInst().addPacketToQueue("SFAU", job+"|"+newVal, null);
                    }
                }
            });
            add(itmSetAuthor);
            
            itmDel = new JMenuItem("Delete Fractal");
            itmDel.addActionListener(new ActionListener(){
                @Override
                public void actionPerformed(ActionEvent ae) {
                    int resp = JOptionPane.showConfirmDialog(null, "Delete Fractal", "Are you sure", YES_NO_OPTION);
                    if(resp == YES_OPTION){
                        ServerConnection.getInst().addPacketToQueue("DELF", job+"", null);
                    }
                }
            });
            add(itmDel);
            
            if(!canUserEdit()){
                itmSetTitle.setEnabled(false);
                itmSetAuthor.setEnabled(false);
                itmDel.setEnabled(false);
            }
        }
    }
    
    class ContextMenuListener extends MouseAdapter {
        @Override
        public void mousePressed(MouseEvent e){
            if(status == FDBS_UNLINKED)
                return;
            if (e.isPopupTrigger())
                showContextMenu(e);
            else{ // spawn a context based on this
                if(JOptionPane.showConfirmDialog(null, "Establish Local Fractal Context?","Open Confirmation", JOptionPane.YES_NO_OPTION) == JOptionPane.YES_OPTION){
                    JavaDesktop.getInst().getWorkspacePanel().openNewContext(LibraryTile.this);
                }
            }
        }
        @Override
        public void mouseReleased(MouseEvent e){
            if(status == FDBS_UNLINKED)
                return;
            if (e.isPopupTrigger())
                showContextMenu(e);
        }

        private void showContextMenu(MouseEvent e){
            ContextMenu menu = new ContextMenu();
            menu.show(e.getComponent(), e.getX(), e.getY());
        }
    };

    /**
     * This method is called from within the constructor to initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is always
     * regenerated by the Form Editor.
     */
    @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        img = new javax.swing.JLabel();
        lblNum = new javax.swing.JLabel();
        lblTitle = new javax.swing.JLabel();
        lblAuthor = new javax.swing.JLabel();

        setMaximumSize(new java.awt.Dimension(150, 200));
        setMinimumSize(new java.awt.Dimension(150, 200));
        setPreferredSize(new java.awt.Dimension(150, 200));
        setLayout(new javax.swing.BoxLayout(this, javax.swing.BoxLayout.PAGE_AXIS));

        img.setIcon(new javax.swing.ImageIcon(getClass().getResource("/images/noimg.png"))); // NOI18N
        add(img);

        lblNum.setText("1350");
        add(lblNum);

        lblTitle.setText("Sunshine");
        add(lblTitle);

        lblAuthor.setText("Author");
        add(lblAuthor);
    }// </editor-fold>//GEN-END:initComponents
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JLabel img;
    private javax.swing.JLabel lblAuthor;
    private javax.swing.JLabel lblNum;
    private javax.swing.JLabel lblTitle;
    // End of variables declaration//GEN-END:variables

    private int job;
    private int uid;
    private int status;
    BufferedImage image = null; // this holds the true image, while displayed is scaled :P
    
    private Timer updateTimer;
}
