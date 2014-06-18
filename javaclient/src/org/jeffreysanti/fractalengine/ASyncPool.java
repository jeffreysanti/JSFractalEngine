/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package org.jeffreysanti.fractalengine;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.util.ArrayList;
import java.util.HashMap;

/**
 *
 * @author jeffrey
 */
public class ASyncPool {
    
    ASyncPool(){
        E = new ArrayList();
    }
    
    public void addASyncAcceptor(String head, ASyncPoolAcceptor acceptor){
        ASyncEntry ent = new ASyncEntry();
        ent.head = head;
        ent.acceptor = acceptor;
        ent.timeExpire = System.currentTimeMillis() + 10000L;
        E.add(ent);
    }
    
    public boolean receivePacket(String head, byte[] data){
        long now = System.currentTimeMillis();
        int i=0;
        while(i < E.size()){
            if(E.get(i).timeExpire <= now){
                E.remove(i);
                continue;
            }
            
            if(E.get(i).head.equals(head)){
                // copy stream so acceptor can read from it
                ByteArrayInputStream output = new ByteArrayInputStream(data);
                DataInputStream dup = new DataInputStream(output);
                if(E.get(i).acceptor.poolDataReceived(head, dup)){
                    E.remove(i);
                    return true;
                }
            }
            i++;
        }
        return false;
    }
    
    
    
    private class ASyncEntry{
        public String head;
        public ASyncPoolAcceptor acceptor;
        public long timeExpire;
    }
    
    private ArrayList<ASyncEntry> E;
}
