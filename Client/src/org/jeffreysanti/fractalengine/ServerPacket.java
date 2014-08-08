/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package org.jeffreysanti.fractalengine;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 *
 * @author jeffrey
 */
public class ServerPacket {
    public class ServerPacketHeader{
        public int magic;
	public int len;
	public int replyAddr;
	public byte[] pckType;
        
        public ServerPacketHeader(){
            magic = ServerConnection.MAGIC;
            len = 0;
            replyAddr = 0;
            pckType = new byte[4];
        }
    };
    
    public byte[] data;
    public ServerPacketHeader head;
    private int dataBytesRead;
    private boolean readHeader;
    
    public ServerPacket(){
        data = new byte[0];
        head = new ServerPacketHeader();
        dataBytesRead = 0;
        readHeader = false;
    }
    
    // Returns 0: okay, 1: finished packet, 2: disconnect
    public int readOperation(DataInputStream i){
        try{
            if(!readHeader){
                if(i.available() < 16)
                    return 0;
                int mg = i.readInt();
                if(mg != ServerConnection.MAGIC)
                    return 2;
                int len = i.readInt();
                if(len > ServerConnection.MAXSZ)
                    return 2;
                head.magic = mg;
                head.len = len;
                head.replyAddr = i.readInt();
                head.pckType = new byte[4];
                if(i.read(head.pckType) != 4)
                    return 2;
                readHeader = true;
                dataBytesRead = 0;
                data = new byte[head.len];
            }
            if(readHeader){
                if(i.available() > 0){
                    int count = i.read(data, dataBytesRead, head.len-dataBytesRead);
                    if(count < 0)
                        return 2;
                    dataBytesRead += count;
                }
                if(dataBytesRead == head.len){
                    return 1;
                }
            }
        }catch(IOException ex){
            Logger.getLogger(ServerConnection.class.getName()).log(Level.SEVERE, null, ex);
            return 2;
        }
        return 0;
    }
    
    // Returns 0: okay, 2: disconnect
    public int writeOperation(DataOutputStream o){
        head.len = data.length;
        head.magic = ServerConnection.MAGIC;
        try{
            o.writeInt(head.magic);
            o.writeInt(head.len);
            o.writeInt(head.replyAddr);
            o.write(head.pckType);
            o.write(data);
        }catch(IOException ex){
            Logger.getLogger(ServerConnection.class.getName()).log(Level.SEVERE, null, ex);
            return 2;
        }
        return 0;
    }
    
    
    public byte[] toByteArray() throws IOException {
        ByteArrayOutputStream b = new ByteArrayOutputStream();
        DataOutputStream o = new DataOutputStream(b);
        
        head.len = data.length;
        
        o.writeInt(head.magic);
        o.writeInt(head.len);
        o.writeInt(head.replyAddr);
        o.write(head.pckType);
        
        o.write(data);
        
        return b.toByteArray();
    }
    
    public static byte[] strToByteArray(String s) throws IOException{
        ByteArrayOutputStream b = new ByteArrayOutputStream();
        DataOutputStream o = new DataOutputStream(b);
        
        o.writeBytes(s);
        return b.toByteArray();
    }
    
    public static String byteArrayToStr(byte[] barr) throws IOException{
        String dta = new String(barr);
        return dta;
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
    
    public boolean forceReadPacket(DataInputStream i) throws IOException{
        Long start = System.currentTimeMillis();
        
        // first read the header
        while(true){
            if(System.currentTimeMillis() - start > 6000L){ // something went wrong -> give up
                return false;
            }
            if(i.available() >= 16){ // header should be avaliable
                int mg = i.readInt();
                if(mg != ServerConnection.MAGIC)
                    return false;
                int len = i.readInt();
                if(len > ServerConnection.MAXSZ)
                    return false;
                head.magic = mg;
                head.len = len;
                head.replyAddr = i.readInt();
                head.pckType = new byte[4];
                if(i.read(head.pckType) != 4)
                    return false;
                break;
            }
        }
        data = new byte[head.len];
        int bread = 0;
        while(bread < head.len){
            if(System.currentTimeMillis() - start > 6000L){ // something went wrong -> give up
                return false;
            }
            if(i.available() > 0){
                bread += i.read(data, bread, head.len - bread);
            }
        }
        return true;
    }
}
