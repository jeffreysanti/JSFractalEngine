/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package org.jeffreysanti.fractalengine;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import javax.swing.BoxLayout;
import javax.swing.JLabel;
import javax.swing.JPanel;
import org.jeffreysanti.fractalengine.AnimationWindow.AnimationParam;
import org.json.simple.JSONArray;
import org.json.simple.JSONObject;

/**
 *
 * @author jeffrey
 */
public class AnimationParamPanel extends JPanel implements MouseListener {
    
    public AnimationParamPanel(AnimationParam ap, JSONArray kf, int fcount, AnimationWindow cb){
        param = ap;
        keyframes = kf;
        frames = fcount;
        callback = cb;
        selectedFrame = 0;
        
        for(Object o : keyframes){
            JSONObject frame = (JSONObject)o;
            if(!frame.get("param").equals(param.addr))
                continue;
            
            int frameno = ((Number)frame.get("frame")).intValue();
            if(frameno > frames){
                frame.put("frame", frames);
                frameno = frames;
            }
        }
        
        initComponents();
    }
    
    private void initComponents(){
        this.setLayout(new GridBagLayout());
        GridBagConstraints c = new GridBagConstraints();
        
        lbl = new JLabel();
        lbl.setMinimumSize(new Dimension(200, 50));
        lbl.setMaximumSize(new Dimension(200, 50));
        lbl.setPreferredSize(new Dimension(200, 50));
        c.fill = GridBagConstraints.NONE;
        c.fill = GridBagConstraints.VERTICAL;
        this.add(lbl, c);
        
        framePanel = new JPanel(){
            @Override
            public void paintComponent(Graphics g) {
                super.paintComponent(g);
                paintGraph(g, this.getWidth(), this.getHeight());
            }
        };
        framePanel.addMouseListener(this);
        framePanel.setMinimumSize(new Dimension(400, 50));
        //framePanel.setMaximumSize(new Dimension(4000, 100));
        c.fill = GridBagConstraints.BOTH;
        c.weightx = 1;
        this.add(framePanel, c);
    }
    
    public void updateDisplay(){
        lbl.setText(param.addr);
        framePanel.repaint();
    }
    
    public void paintGraph(Graphics g, int w, int h){
        g.setColor(Color.white);
        g.fillRect(0, 0, w, h);
        
        double wpf = (double)w / frames;
        int ppf = 5;
        
        // draw first frame
        g.setColor(Color.DARK_GRAY);
        g.fillRect(0, 0, ppf, h);
        
        for(Object o : keyframes){
            JSONObject frame = (JSONObject)o;
            if(!frame.get("param").equals(param.addr))
                continue;
            
            int frameno = ((Number)frame.get("frame")).intValue();            
            g.setColor(Color.CYAN);
            g.fillRect((int)Math.round((frameno-1)*wpf), 0, ppf, h);
        }
        
        // draw selection
        if(selectedFrame != 0){
            g.setColor(Color.RED);
            g.drawRect((int)((selectedFrame-1)*wpf), 0, ppf, h);
        }
    }
    
    public int getSelectedFrame(){
        return selectedFrame;
    }
    public void setSelectedFrame(int i){
        selectedFrame = i;
        if(selectedFrame > frames)
            selectedFrame = frames;
        if(selectedFrame < 2 && selectedFrame != 0)
            selectedFrame = 2;
        updateDisplay();
    }
    
    private AnimationParam param;
    private JSONArray keyframes;
    private int frames;
    AnimationWindow callback;
    
    private JLabel lbl;
    private JPanel framePanel;
    
    private int selectedFrame;

    @Override
    public void mouseClicked(MouseEvent me) {
        double wpf = frames / (double)me.getComponent().getWidth();
        int f = (int)Math.round(wpf * me.getX());
        int dist = Integer.MAX_VALUE;
        int sel = selectedFrame;
        for(Object o : keyframes){
            JSONObject frame = (JSONObject)o;
            if(!frame.get("param").equals(param.addr))
                continue;
            
            int frameno = ((Number)frame.get("frame")).intValue();
            if(Math.abs(f - frameno) < dist){
                sel = frameno;
                dist = Math.abs(f - frameno);
            }
        }
        
        setSelectedFrame(sel);
        callback.frameSelected(param.addr, selectedFrame);
    }

    @Override
    public void mousePressed(MouseEvent me) {}
    @Override
    public void mouseReleased(MouseEvent me) {}
    @Override
    public void mouseEntered(MouseEvent me) {}
    @Override
    public void mouseExited(MouseEvent me) {    }
}
