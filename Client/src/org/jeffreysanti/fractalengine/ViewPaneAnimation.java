/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package org.jeffreysanti.fractalengine;

import java.awt.BorderLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.ByteArrayInputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.imageio.ImageIO;
import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JFileChooser;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JProgressBar;
import javax.swing.filechooser.FileFilter;
import org.apache.commons.net.ftp.FTP;
import org.apache.commons.net.ftp.FTPClient;
import org.apache.commons.net.ftp.FTPFile;

/**
 *
 * @author jeffrey
 */
public class ViewPaneAnimation extends ViewPaneAbstract {

    public ViewPaneAnimation(Context c, byte[] data) {
        super(c);
        
        try {
            DataInputStream ds = new DataInputStream(new ByteArrayInputStream(data));
            final int pno = ds.readInt();
            
            btnReqAnim = new JButton("Download Video");
            btnReqAnim.addActionListener(new ActionListener(){
                @Override
                public void actionPerformed(ActionEvent ae) {
                    download(pno);
                }
            });
            
            this.setLayout(new BorderLayout());
            this.add(btnReqAnim, BorderLayout.CENTER);
        } catch (IOException ex) {
            Logger.getLogger(ViewPaneAnimation.class.getName()).log(Level.SEVERE, null, ex);
        }
    }

    
    
    @Override
    void contextDataReceieved() {
    }
    
    JButton btnReqAnim;

    public void download(int port){
        
        File downloadFile = new File(getSaveFile());
        
        
        final JDialog dlg = new JDialog(JavaDesktop.getInst(), "Progress Dialog", true);
        JProgressBar dpb = new JProgressBar(0, 500);
        dlg.add(BorderLayout.CENTER, dpb);
        dlg.add(BorderLayout.NORTH, new JLabel("Progress..."));
        dlg.setDefaultCloseOperation(JDialog.DISPOSE_ON_CLOSE);
        dlg.setSize(300, 75);
        dlg.setLocationRelativeTo(JavaDesktop.getInst());
        dlg.setVisible(false);
        
        if(port < 1){ // need local
            if(ServerConnection.getInst().getSocketsPath().isEmpty())
                return;
            
            String remoteFile = ServerConnection.getInst().getSocketsPath()+
                    "renders/"+context.getJobID()+".mkv";
            File remo = new File(remoteFile);
            if(!remo.exists()){
                JOptionPane.showMessageDialog(JavaDesktop.getInst(), "Download Failed");
                return;
            }
            long kb = remo.length() / 1024;
            
            dpb.setMaximum((int)kb);
            dpb.setMinimum(0);
            dpb.setValue(0);
            
            Thread t = new Thread(new Runnable() {
                @Override
                public void run() {
                    dlg.setVisible(true);
                }
            });
            t.start();
            
            try{
                OutputStream outputStream = new BufferedOutputStream(new FileOutputStream(downloadFile));
                InputStream inputStream = new BufferedInputStream(new FileInputStream(remo));
                byte[] bytesArray = new byte[4096];
                long totalRead = 0;
                int bytesRead = -1;
                while ((bytesRead = inputStream.read(bytesArray)) != -1) {
                    outputStream.write(bytesArray, 0, bytesRead);
                    totalRead += bytesRead;

                    long totalReadKb = totalRead / 1024;
                    dpb.setValue((int)totalReadKb);
                }
                dlg.setVisible(false);
                JOptionPane.showMessageDialog(JavaDesktop.getInst(), "Download Completed!");
               
                outputStream.close();
                inputStream.close();
            }catch(IOException ex){
                dlg.setVisible(false);
                JOptionPane.showMessageDialog(JavaDesktop.getInst(), "Download Failed");
            }
            return;
        }
        
        FTPClient ftpClient = new FTPClient();
        try {
            
            ftpClient.connect(ServerConnection.getInst().getAddress(), port);
            ftpClient.enterLocalPassiveMode();
            ftpClient.login("anonymous", "");
            
            ftpClient.setFileType(FTPClient.BINARY_FILE_TYPE);
            
            String remoteFile = context.getJobID()+".mkv";
            long kb = 0;
            
            FTPFile[] files = ftpClient.listFiles("/");
            boolean found = false;
            for (FTPFile file : files) {
                if(file.getName().equals(remoteFile)){
                    found = true;
                    kb = file.getSize() / 1024;
                    break;
                }
            }
            if(!found){
                JOptionPane.showMessageDialog(JavaDesktop.getInst(), "Download Failed");
                return;
            }
            
            dpb.setMaximum((int)kb);
            dpb.setMinimum(0);
            dpb.setValue(0);
            
            Thread t = new Thread(new Runnable() {
                @Override
                public void run() {
                    dlg.setVisible(true);
                }
            });
            t.start();
            
            OutputStream outputStream = new BufferedOutputStream(new FileOutputStream(downloadFile));
            InputStream inputStream = ftpClient.retrieveFileStream("/"+remoteFile);
            byte[] bytesArray = new byte[4096];
            long totalRead = 0;
            int bytesRead = -1;
            while ((bytesRead = inputStream.read(bytesArray)) != -1) {
                outputStream.write(bytesArray, 0, bytesRead);
                totalRead += bytesRead;
                
                long totalReadKb = totalRead / 1024;
                dpb.setValue((int)totalReadKb);
            }
            dlg.setVisible(false);
 
            boolean success = ftpClient.completePendingCommand();
            if(success) {
                JOptionPane.showMessageDialog(JavaDesktop.getInst(), "Download Completed!");
            }else{
                JOptionPane.showMessageDialog(JavaDesktop.getInst(), "Download Failed");
            }
            outputStream.close();
            inputStream.close();
        } catch (IOException ex) {
            dlg.setVisible(false);
            JOptionPane.showMessageDialog(JavaDesktop.getInst(), "Download Failed");
        } finally {
            try {
                if (ftpClient.isConnected()) {
                    ftpClient.logout();
                    ftpClient.disconnect();
                }
            } catch (IOException ex) {
                ex.printStackTrace();
            }
        }
    }
    
    public String getSaveFile() {
        JFileChooser fc = new JFileChooser();
        fc.setAcceptAllFileFilterUsed(false);
        fc.addChoosableFileFilter(new FileFilter(){
            public String getExtension(File f) {
                String ext = null;
                String s = f.getName();
                int i = s.lastIndexOf('.');

                if (i > 0 &&  i < s.length() - 1) {
                    ext = s.substring(i+1).toLowerCase();
                }
                return ext;
            }
            
            @Override
            public boolean accept(File f) {
                if (f.isDirectory()) {
                    return true;
                }

                String extension = getExtension(f);
                if (extension != null) {
                    if (extension.equalsIgnoreCase("mkv"))
                        return true;
                }
                return false;
            }

            @Override
            public String getDescription() {
                return "Matroska H.264 Video (.mkv)";
            }
        });
        int returnVal = fc.showSaveDialog(JavaDesktop.getInst());
        if (returnVal == JFileChooser.APPROVE_OPTION) {
            File file = fc.getSelectedFile();
            if(!file.getAbsolutePath().endsWith(".mkv")){
                file = new File(file + ".mkv");
            }
            return file.getAbsolutePath();
        }
        return "";
    }
    
}
