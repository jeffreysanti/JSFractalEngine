/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package org.jeffreysanti.fractalengine;

import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import com.google.gson.JsonElement;
import com.google.gson.JsonParser;
import java.awt.BorderLayout;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import org.json.simple.JSONObject;

/**
 *
 * @author jeffrey
 */
public class ViewPaneParams extends ViewPaneAbstract {
    ViewPaneParams(Context c, JSONObject p){
        super(c);
        params = p;
        
        this.setLayout(new BorderLayout());
        
        text = new JTextArea();
        text.setEditable(false);
        JScrollPane scroll = new JScrollPane(text);
        this.add(scroll, BorderLayout.CENTER);
        
        contextDataReceieved();
    }

    @Override
    void contextDataReceieved() {
        // Use Gson to convert ugly simplejson output to nice format
        Gson gson = new GsonBuilder().setPrettyPrinting().create();
        JsonParser jp = new JsonParser();
        JsonElement je = jp.parse(params.toJSONString());
        String txt = gson.toJson(je);
        text.setText(txt);
    }
    
    private JTextArea text;
    private JSONObject params;
}
