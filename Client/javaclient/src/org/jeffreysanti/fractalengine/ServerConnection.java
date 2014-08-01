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
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.PrintWriter;
import java.net.Socket;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.swing.event.EventListenerList;

/**
 *
 * @author jeffrey
 */
public class ServerConnection {
    
    private static final int MAGIC = 0xFA42AB;
    
    public static final int MAXSZ = 33554432; /* 32MB */
    
    public final boolean SHOW_DEBUG = false;
    
    private ServerConnection(){
        //serverUpdateList = new EventListenerList();
        aSyncPool = new ASyncPool();
    }
    
    public synchronized ASyncPool getASyncPool(){
        return aSyncPool;
    }
    
    public synchronized String getJobsMessage(){
        if(isConnected())
            return jobMsg;
        return "No Server Connection";
    }
    public synchronized String getServerMessage(){
        return getAddress();
    }
    
    public synchronized void processPacket(String head, DataInputStream ds){
        try {
            if(ds.available() > 0){
                byte[] dta = new byte[ds.available()];
                ds.readFully(dta);
                ds = new DataInputStream(new ByteArrayInputStream(dta));
                if(aSyncPool.receivePacket(head, dta))
                    return;
            }else{
                if(aSyncPool.receivePacket(head, new byte[0]))
                    return;
            }

            if(head.equals("UPDT")){
                jobMsg = extractString(ds);
                JavaDesktop.getInst().getStatusBar().actionPerformed(null);
            }if(head.equals("SSLT")){ // metaDataUpdate, searchStringList
                JavaDesktop.getInst().getLibraryPanel().handleServerPackets(head, ds);
            }if(head.equals("MDUD")){
                JavaDesktop.getInst().getLibraryPanel().handleServerPackets(head, ds);
            }
        } catch (IOException ex) {
            Logger.getLogger(ServerConnection.class.getName()).log(Level.SEVERE, null, ex);
        }
    }
    
    public synchronized void update(){
        if(isConnected()){
            DataInputStream rd = readPacketBG(false);
            try {
                if(rd.available() >= 4){
                    // first 4 characters
                    byte[] BA = new byte[4];
                    BA[0] = rd.readByte(); BA[1] = rd.readByte(); BA[2] = rd.readByte(); BA[3] = rd.readByte();
                    String head = new String(BA);
                    if(SHOW_DEBUG)
                        System.out.println("update() caught: "+head);
                    processPacket(head, rd);
                }
            } catch (IOException ex) {
                Logger.getLogger(ServerConnection.class.getName()).log(Level.SEVERE, null, ex);
            }

            //ActionEvent evt = new ActionEvent(this, 0, "requested");
            //fireServerUpdate(evt);
        }
    }
    
    public synchronized void connectNow(String addr, int port, String user, String pass){
        this.user = user;
        this.pass = pass;        
        serverAddr = addr;
        serverPort = port;

        if(sock != null){
            try {
                sock.close();
            } catch (Exception ex) {}
        }

        sock = null;
        out = null;
        in = null;

        try{
            sock = new Socket(serverAddr, serverPort);
            out = sock.getOutputStream();
            in = new DataInputStream(sock.getInputStream());

            sendPacket("AUTH"+user+":"+pass);
            DataInputStream rd = readPacket("AUTH");
            String resp = extractString(rd);
            if(resp.contains(":YESADMIN")){
                admin = true;
                uid = Integer.parseInt(resp.substring(resp.indexOf('/')+1));
            }else if(resp.contains(":YES")){
                admin = false;
                uid = Integer.parseInt(resp.substring(resp.indexOf('/')+1));
            }else{
                sock.close();
                sock = null;
                out = null;
                in = null;
                onConnectionDrop();
            }
        } catch (Exception e) {
            sock = null;
            out = null;
            in = null;
            onConnectionDrop();
        }
        JavaDesktop.getInst().getLibraryPanel().update();
        //fireServerUpdate(new ActionEvent(this, 0, "connection_start"));
    }
    public synchronized String getAddress(){
        return serverAddr;
    }
    public synchronized int getPort(){
        return serverPort;
    }
    
    public synchronized String getUser(){
        return user;
    }
    public synchronized String getPassword(){
        return pass;
    }
    
    public synchronized void updateSocket(){
        
    }
    
    private synchronized void onConnectionDrop(){
        ServerConnectionDialog.openDialog();
    }
    
    public synchronized void sendPacket(String str){
        try {
            ByteArrayOutputStream buf = new ByteArrayOutputStream();
            DataOutputStream ds = new DataOutputStream(buf);
            ds.writeBytes(str);
            ds.flush();
            buf.flush();
            sendPacket(buf);
        } catch (IOException ex) {
            Logger.getLogger(ServerConnection.class.getName()).log(Level.SEVERE, null, ex);
        }
    }
    
    public synchronized void sendPacket(ByteArrayOutputStream dta){
        if(isConnected()){
            ByteArrayOutputStream outputter = new ByteArrayOutputStream();
            DataOutputStream outputterDS = new DataOutputStream(outputter);
            if(dta.size() == 0)
                return;
            try {
                outputterDS.writeInt(MAGIC);
                outputterDS.writeInt(dta.size());
                byte[] bytes = dta.toByteArray();
                outputterDS.write(bytes);
                outputterDS.flush();

                if(bytes.length >= 4){
                    byte[] bytes_head = new byte[4];
                    bytes_head[0] = bytes[0];
                    bytes_head[1] = bytes[1];
                    bytes_head[2] = bytes[2];
                    bytes_head[3] = bytes[3];
                    String header = new String(bytes_head);
                    if(SHOW_DEBUG)
                        System.out.println("SEND::: "+header);
                }

                out.write(outputter.toByteArray());
                out.flush();
            }catch (IOException ex) {
                onConnectionDrop();
            }
        }
    }
    
    private synchronized DataInputStream readPacketBG(boolean block){
        if(!isConnected())
            return new DataInputStream(new ByteArrayInputStream(new byte[0]));
        try {
            if(block){
                Long start = System.currentTimeMillis();
                while(in.available() < 8){
                    if(System.currentTimeMillis() - start > 3000L){ // something went wrong -> give up
                        return new DataInputStream(new ByteArrayInputStream(new byte[0]));
                    }
                }
            }
            if(in.available() > 8){
                int magic = in.readInt();
                if(magic != MAGIC){
                    System.out.println("Corrupt Packet Received... Clearing Buffer");
                    in.readFully(new byte[in.available()]);
                    return new DataInputStream(new ByteArrayInputStream(new byte[0]));
                }
                int pktSize = in.readInt();
                if(pktSize > MAXSZ) // over 32MB uh oh
                    return new DataInputStream(new ByteArrayInputStream(new byte[0]));
                byte [] buf = new byte[pktSize];
                int recvd = 0;
                Long start = System.currentTimeMillis();
                while(recvd < pktSize){
                    if(System.currentTimeMillis() - start > 3000L){ // something went wrong -> give up
                        return new DataInputStream(new ByteArrayInputStream(new byte[0]));
                    }
                    recvd += in.read(buf, recvd, Math.min(in.available(), pktSize-recvd));
                }
                return new DataInputStream(new ByteArrayInputStream(buf));
            }
        } catch (IOException ex) {
            Logger.getLogger(ServerConnection.class.getName()).log(Level.SEVERE, null, ex);
        }
        return new DataInputStream(new ByteArrayInputStream(new byte[0]));
    }
    
    private synchronized DataInputStream readPacket(String header){
        Long start = System.currentTimeMillis();
        while(true){
            if(System.currentTimeMillis() - start > 6000L){ // something went wrong -> give up
                return new DataInputStream(new ByteArrayInputStream(new byte[0]));
            }

            DataInputStream ds = readPacketBG(true);
            try {
                if(ds.available() >= 4){
                    // first 4 characters
                    byte[] BA = new byte[4];
                    BA[0] = ds.readByte(); BA[1] = ds.readByte(); BA[2] = ds.readByte(); BA[3] = ds.readByte();
                    String head = new String(BA);
                    if(SHOW_DEBUG)
                        System.out.println("readPacket("+header+") caught: "+head);
                    if(header.length() < 1 || head.equals(header))
                        return ds;
                    processPacket(head, ds);
                }
            } catch (IOException ex) {
                Logger.getLogger(ServerConnection.class.getName()).log(Level.SEVERE, null, ex);
            }
        }
    }
    
    public synchronized boolean isConnected(){
        
        if(sock == null || out == null || sock.isConnected() == false || sock.isClosed()){
            onConnectionDrop();
            return false;
        }
        return true;
    }
    
    public synchronized boolean isAdmin(){
        return admin;
    }
    public synchronized int getUID(){
        return uid;
    }
    
    public static String extractString(DataInputStream ds){
        try {
            byte arr[] = new byte[ds.available()];
            ds.readFully(arr);
            String dta = new String(arr);
            return dta;
        } catch (IOException ex) {
            Logger.getLogger(ServerConnection.class.getName()).log(Level.SEVERE, null, ex);
        }
        return "";
    }
    public static String extractString(DataInputStream ds, int len){
        try {
            byte arr[] = new byte[len];
            ds.readFully(arr);
            String dta = new String(arr);
            return dta;
        } catch (IOException ex) {
            Logger.getLogger(ServerConnection.class.getName()).log(Level.SEVERE, null, ex);
        }
        return "";
    }
    
    
    
    //private EventListenerList serverUpdateList;
    private String serverAddr = "localhost";
    private int serverPort = 14860;
    
    private String user, pass;
    private int uid=-1;
    
    private Socket sock = null;
    private OutputStream out = null;
    private DataInputStream in = null;
    
    private String jobMsg;
    
    private ASyncPool aSyncPool;
    
    private boolean admin = false;
    
    public static ServerConnection getInst(){
        return inst;
    }
    private static final ServerConnection inst = new ServerConnection();
}
