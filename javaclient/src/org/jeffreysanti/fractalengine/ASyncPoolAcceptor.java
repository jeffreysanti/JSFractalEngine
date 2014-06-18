/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package org.jeffreysanti.fractalengine;

import java.io.DataInputStream;

/**
 *
 * @author jeffrey
 */
public interface ASyncPoolAcceptor {
    
    // returns true if data was received happily by recipiant; false if not applicable -> will pass on
    public boolean poolDataReceived(String head, DataInputStream ds);
    
}
