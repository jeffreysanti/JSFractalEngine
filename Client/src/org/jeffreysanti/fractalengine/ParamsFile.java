/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package org.jeffreysanti.fractalengine;

import java.util.logging.Level;
import java.util.logging.Logger;
import org.json.simple.parser.JSONParser;
import org.json.simple.parser.ParseException;

/**
 *
 * @author jeffrey
 */
public class ParamsFile {
    
    public ParamsFile(String data){
        
        open_correct = true;
        
        JSONParser parser = new JSONParser();
        try {
            parser.parse(data);
        } catch (ParseException ex) {
            open_correct = false;
            //Logger.getLogger(ParamsFile.class.getName()).log(Level.SEVERE, null, ex);
        }
        
    }
    
    private boolean open_correct;
    
}
