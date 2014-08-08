/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package org.jeffreysanti.fractalengine;

import java.io.DataInputStream;

/**
 *
 * @author jeffrey
 */
public interface ServerReplyer {
    public void onReceiveReply(String head, int len, DataInputStream data);
}
