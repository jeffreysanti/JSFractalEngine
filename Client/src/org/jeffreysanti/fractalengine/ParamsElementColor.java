/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package org.jeffreysanti.fractalengine;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Rectangle;
import java.awt.datatransfer.Transferable;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import javax.swing.JComponent;
import javax.swing.JLabel;
import javax.swing.JList;
import javax.swing.JPanel;
import javax.swing.JTextField;
import javax.swing.TransferHandler;
import org.json.simple.JSONArray;
import org.json.simple.JSONObject;
import org.json.simple.JSONValue;

/**
 *
 * @author jeffrey
 */
public class ParamsElementColor extends ParamsElement {
    public ParamsElementColor(JSONObject schemaDefn, JSONObject paramsContainer, PanelProperties cb){
        super(schemaDefn, paramsContainer, cb);
        
        lbl = new JLabel((String)schemaDefn.get("caption"));
        
        colBox = new JPanel();
        colBox.addMouseListener(new MouseListener(){
            @Override
            public void mouseClicked(MouseEvent e) {
                if (e.getClickCount() == 2 && !e.isConsumed()) {
                    e.consume();
                    ColorChooser chooserForm = new ColorChooser();
                    chooserForm.setColor(colBox.getBackground());
                    chooserForm.setVisible(true);
                }
            }
            
            @Override
            public void mousePressed(MouseEvent e) {}
            @Override
            public void mouseReleased(MouseEvent e) {}
            @Override
            public void mouseEntered(MouseEvent e) {}
            @Override
            public void mouseExited(MouseEvent e) {}
        });
        
        ColorChooser.DropTargetList dropTargetList = new ColorChooser.DropTargetList(colBox){
            @Override
            public void handleDroppedColor(Color c){
                JSONArray arr = new JSONArray();
                arr.add(c.getRed());
                arr.add(c.getGreen());
                arr.add(c.getBlue());
                grp.put(id, arr);
                verify();
            }
        };
        
        verify(); // assures some value is inside text editor
    }
    
    @Override
    public boolean verify()
    {
        if(!grp.containsKey(id) || !(grp.get(id) instanceof JSONArray) || 
                ((JSONArray)grp.get(id)).size() < 3){
            if(schem.containsKey("default"))
                grp.put(id, schem.get("default"));
            else
                grp.put(id, new int[] {0, 0, 0});
            callback.markDirty();
        }
        
        JSONArray val = (JSONArray)grp.get(id);
        try{
            for(int i=0; i<3; i++){
                int v = Integer.parseInt(val.get(i).toString());
                v = Math.min(v, 255);
                v = Math.max(v, 0);
                val.set(i, v);
            }
        }catch(NumberFormatException e){
            // okay
            if(schem.containsKey("default"))
                grp.put(id, schem.get("default"));
            else
                grp.put(id, new int[] {0, 0, 0});
            callback.markDirty();
        }
        colBox.setBackground(new Color((Integer)val.get(0), (Integer)val.get(1), (Integer)val.get(2)));
        return true;
    }
    
    @Override
    public JComponent getInnerElm(){
        JPanel pnl = new JPanel();
        
        pnl.setLayout(new BorderLayout());
        pnl.add(lbl, BorderLayout.LINE_START);
        pnl.add(colBox, BorderLayout.CENTER);
        return pnl;
    }
    
    private JLabel lbl;
    private JPanel colBox;
}

