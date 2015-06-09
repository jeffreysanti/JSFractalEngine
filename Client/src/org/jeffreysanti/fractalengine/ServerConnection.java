/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package org.jeffreysanti.fractalengine;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.BufferedReader;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.PrintWriter;
import java.net.Socket;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.util.ArrayDeque;
import java.util.HashMap;
import java.util.HashSet;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.swing.SwingUtilities;
import javax.swing.event.EventListenerList;

/**
 *
 * @author jeffrey
 */
public class ServerConnection implements Runnable {
    
    public static final int MAGIC = 0xFA42AB;
    public static final int MAXSZ = 33554432; /* 32MB */
    
    private Socket sock = null;
    private DataOutputStream out = null;
    private DataInputStream in = null;
    
    String noSocketsPath;
    
    private int portno;
    private String addr, pass, user;
    private int uid;
    private boolean admin;
    private boolean nosockets;
    
    private ArrayDeque<ServerPacket> O = new ArrayDeque();
    private ArrayDeque<ServerPacket> R = new ArrayDeque();
    private ServerPacket Pin = new ServerPacket();
    private ServerPacket Pout = null;
    int bytesIn;
    int bytesOut;
    
    private HashMap<String, HashSet<ServerMessageListener>> L = new HashMap();
    
    private HashMap<Integer, ServerReplyer> REP = new HashMap();
    
    
    synchronized public boolean isAdmin(){
        return admin;
    }
    
    synchronized public int getUserID(){
        return uid;
    }
    
    synchronized public String getSocketsPath(){
        return noSocketsPath + "/";
    }
    
    synchronized public String getAddress(){
        return addr;
    }
    
    synchronized private void readSocket(){
        
        if(nosockets){
            File f = new File(noSocketsPath + "/sout.lck");
            if(f.exists()){
                f = new File(noSocketsPath + "/sout.sck");
                if(f.exists()){
                    FileInputStream input;
                    try {
                        input = new FileInputStream(noSocketsPath + "/sout.sck");
                        int ret = Pin.readOperation(new DataInputStream(input));
                        if(ret == 2){
                            connectionDropped("Read Operation Failed");
                        }else if(ret == 1){
                            R.addFirst(Pin);
                            Pin = new ServerPacket();
                        }
                        input.close();
                    } catch (FileNotFoundException ex) {
                        Logger.getLogger(ServerConnection.class.getName()).log(Level.SEVERE, null, ex);
                    } catch (IOException ex) {
                        Logger.getLogger(ServerConnection.class.getName()).log(Level.SEVERE, null, ex);
                    }
                }
                f = new File(noSocketsPath + "/sout.lck");
                f.delete();
            }
        }else{
            int ret = Pin.readOperation(in);
            if(ret == 2){
                connectionDropped("Read Operation Failed");
            }else if(ret == 1){
                R.addFirst(Pin);
                Pin = new ServerPacket();
            }
        }
    }
    synchronized private void writeSocket(){
        if(Pout == null && O.size() > 0){
            Pout = O.removeFirst();
        }
        if(Pout != null && nosockets){
            File f = new File(noSocketsPath + "/sin.lck");
            if(!f.exists()){ 
                try {
                    // lock must be gone to write
                    FileOutputStream output = new FileOutputStream(noSocketsPath + "/sin.sck");
                    int ret = Pout.writeOperation(new DataOutputStream(output));
                    if(ret == 2){
                        connectionDropped("Write Operation Failed");
                        return;
                    }
                    output.close();
                    output = new FileOutputStream(noSocketsPath + "/sin.lck"); // write lock
                    output.write(new byte[0]);
                    output.close(); 
                    Pout = null;
                } catch (FileNotFoundException ex) {
                    Logger.getLogger(ServerConnection.class.getName()).log(Level.SEVERE, null, ex);
                } catch (IOException ex) {
                    Logger.getLogger(ServerConnection.class.getName()).log(Level.SEVERE, null, ex);
                }
            }
        }else if(Pout != null){
            int ret = Pout.writeOperation(out);
            if(ret == 2){
                connectionDropped("Write Operation Failed");
            }
            Pout = null;
        }
    }
    
    // This is always performed inside EDT for Swing
    synchronized private void handleReceivingQueue() throws IOException{
        if(R.size() > 0){
            ServerPacket pkt = R.removeFirst();
            
            String head = ServerPacket.byteArrayToStr(pkt.head.pckType);
            
            int repNum = pkt.head.replyAddr;
            if(repNum > 0){
                ServerReplyer replyer = REP.get(repNum);
                if(replyer != null){
                    replyer.onReceiveReply(head, pkt.head.len, new DataInputStream(new ByteArrayInputStream(pkt.data)));
                    REP.remove(repNum);
                    return;
                }
            }
            
            if(!L.containsKey(head)){
                System.err.println("Lost Packet Received HEAD: "+head);
                return;
            }
            HashSet<ServerMessageListener> list = L.get(head);
            for(ServerMessageListener listener : list){
                listener.onReceivePacket(head, pkt.head.len, new DataInputStream(new ByteArrayInputStream(pkt.data)));
            }
        }
    }
    
    synchronized public void addServerMessageListener(String head, ServerMessageListener listener){
        if(L.containsKey(head)){
            L.get(head).add(listener);
        }else{
            HashSet<ServerMessageListener> list = new HashSet();
            list.add(listener);
            L.put(head, list);
        }
    }
    
    synchronized public boolean isConnected(){
        return (nosockets && !noSocketsPath.isEmpty()) || (sock != null);
    }
    
    synchronized private void connectionDropped(String reason){
        try {
            if(sock != null)
                sock.close();
        } catch (IOException ex) {}
        sock = null;
        out = null;
        in = null;
        O.clear();
        R.clear();
        Pin = new ServerPacket();
        Pout = null;
        JavaDesktop.getInst().getStatusBar().setConnectionString(addr);
        if(!reason.isEmpty()){
            System.err.println("Connection Dropped: "+reason+"\n");
        }
    }
    
    synchronized private void forcePushEntirePacket(ServerPacket pkt) throws IOException{
        
        if(nosockets){
            FileOutputStream out = new FileOutputStream(noSocketsPath + "/sin.sck");
            out.write(pkt.toByteArray());
            out.close();
            
            out = new FileOutputStream(noSocketsPath + "/sin.lck");
            out.close();
        }else{
            out.write(pkt.toByteArray());
            out.flush();
        }
    }
    
    private void authenticate() throws IOException{
        
        if(nosockets){
            // Delete any locks in place [From Previous Connections]
            File f = new File(noSocketsPath + "/sout.lck");
            f.delete();
            f = new File(noSocketsPath + "/sin.lck");
            f.delete();
            
            f = new File(noSocketsPath + "/sin.sck");
            f.delete();
        }
        
        ServerPacket authPkt = new ServerPacket();
        authPkt.head.pckType = new byte[] {'A', 'U', 'T', 'H'};
        authPkt.head.replyAddr = 1;
        authPkt.data = ServerPacket.strToByteArray(user+":"+pass);
        forcePushEntirePacket(authPkt);

        boolean ret = false;
        if(nosockets){
            File f = new File(noSocketsPath + "/sout.lck");
            int attempts = 0;
            while(!f.exists()){
                try {
                    Thread.sleep(100);
                } catch (InterruptedException ex) {
                    Logger.getLogger(ServerConnection.class.getName()).log(Level.SEVERE, null, ex);
                }
                attempts ++;
                if(attempts > 60){
                    connectionDropped("AUTH Packet not received");
                    return;
                }
            }
            // now read message
            f = new File(noSocketsPath + "/sout.sck");
            if(!f.exists()){
                connectionDropped("AUTH Packet not received");
                return;
            }
            FileInputStream input = new FileInputStream(noSocketsPath + "/sout.sck");
            ret = authPkt.forceReadPacket(new DataInputStream(input));
            input.close();
            
            f = new File(noSocketsPath + "/sout.lck");
            f.delete();
        }else{
            ret = authPkt.forceReadPacket(in);
        }
        if(ret && ServerPacket.byteArrayToStr(authPkt.head.pckType).equals("AUTH")){
            String resp = ServerPacket.byteArrayToStr(authPkt.data);
            if(resp.contains("YESADMIN")){
                admin = true;
                uid = Integer.parseInt(resp.substring(resp.indexOf('/')+1));
                System.out.println("Authenticated: Admin Rights "+uid+"\n");
            }else if(resp.contains("YES")){
                admin = false;
                uid = Integer.parseInt(resp.substring(resp.indexOf('/')+1));
                System.out.println("Authenticated: Standard User "+uid+"\n");
            }else{
                connectionDropped("AUTH denied");
            }
        }else{
            connectionDropped("AUTH Packet not received");
        }
    }
    
    synchronized private void attemptConnection() throws IOException{
        ServerConnectionDialog dlg = new ServerConnectionDialog();
        dlg.openDialog("localhost", 14860, "", "");
        while(dlg.isOpen()){
            try {
                Thread.sleep(200);
            } catch (InterruptedException ex) {
                Logger.getLogger(ServerConnection.class.getName()).log(Level.SEVERE, null, ex);
            }
        }
        // now try to connect to it
        nosockets = dlg.isNoSockets();
        portno = dlg.getPort();
        addr = dlg.getAddress();
        user = dlg.getUser();
        pass = dlg.getPass();
        
        JavaDesktop.getInst().getStatusBar().setConnectionString(addr);
        
        // now make connection
        if(!nosockets){
            sock = new Socket(addr, portno);
            out = new DataOutputStream(sock.getOutputStream());
            in = new DataInputStream(sock.getInputStream());
            
            authenticate();
            
        }else{
            noSocketsPath = dlg.getNoSocketsDir();
            authenticate();
        }
    }
    
    private int repCounter = 2;
    synchronized public void addPacketToQueue(String head, String data, ServerReplyer rep){
        ServerPacket p = new ServerPacket();
        p.head.pckType = head.getBytes();
        assert(p.head.pckType.length == 4);
        try {
            p.data = ServerPacket.strToByteArray(data);
        } catch (IOException ex) {
            Logger.getLogger(ServerConnection.class.getName()).log(Level.SEVERE, null, ex);
        }
        if(rep != null){
            p.head.replyAddr = repCounter;
            REP.put(repCounter, rep);
            repCounter ++;
        }else{
            p.head.replyAddr = 0;
        }
        O.add(p);
    }
    
    @Override
    public void run() {
        while(true){
            try{
                // First make sure that we are connected
                if(sock == null && !nosockets){ // force new connection
                    attemptConnection();
                    continue;
                }
                if(nosockets && noSocketsPath.isEmpty()){
                    attemptConnection();
                    continue;
                }

                // now, take care of any data
                readSocket();
                writeSocket();
                
                // now send server update messages
                SwingUtilities.invokeLater(new Runnable(){
                    @Override
                    public void run() {
                        try {
                            handleReceivingQueue();
                        } catch (IOException ex) {
                            Logger.getLogger(ServerConnection.class.getName()).log(Level.SEVERE, null, ex);
                        }
                    }
                });
                
                Thread.sleep(5);
                
            }catch (IOException ex) {
                Logger.getLogger(ServerConnection.class.getName()).log(Level.SEVERE, null, ex);
            }catch(InterruptedException ex){
                Logger.getLogger(ServerConnection.class.getName()).log(Level.SEVERE, null, ex);
            }
        }
    }
    
    public static ServerConnection getInst(){
        return inst;
    }
    private static final ServerConnection inst = new ServerConnection();

    private ServerConnection(){ // prevents initalization outside of class
         
    }
}
