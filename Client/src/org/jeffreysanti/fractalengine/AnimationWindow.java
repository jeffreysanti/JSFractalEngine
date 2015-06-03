/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package org.jeffreysanti.fractalengine;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import static javax.swing.ScrollPaneConstants.HORIZONTAL_SCROLLBAR_NEVER;
import static javax.swing.ScrollPaneConstants.VERTICAL_SCROLLBAR_ALWAYS;
import org.json.simple.JSONArray;
import org.json.simple.JSONObject;
import org.json.simple.parser.JSONParser;
import org.json.simple.parser.ParseException;

/**
 *
 * @author jeffrey
 */
public class AnimationWindow extends javax.swing.JFrame {
    
    public static enum AnimationParamType{
        APT_COLOR,
	APT_INT,
	APT_REAL,
	APT_TEXT,
	APT_COMPLEX,
	APT_SELECTOR_NO_AFFECT,

	APT_NON_TYPE
    };
    
    public static class AnimationParam{
        public AnimationParam(String a, AnimationParamType t, JSONObject s){
            addr = a;
            type = t;
            schema = s;
        }
        
        public String addr;
        public AnimationParamType type;
        public JSONObject schema;
    };
    
    public AnimationWindow(JSONObject p, PanelProperties pprops, HashMap<String, AnimationParam> ap){
        cb = pprops;
        params = p;
        AP = ap;
        P = new HashMap();
        
        if(!p.containsKey("anim") || !(p.get("anim") instanceof JSONObject)){
            p.put("anim", new JSONObject());
        }
        if(!((JSONObject)p.get("anim")).containsKey("keyframes") || !(((JSONObject)p.get("anim")).get("keyframes")
                instanceof JSONArray)){
            ((JSONObject)p.get("anim")).put("keyframes", new JSONArray());
        }
        
        initComponents();
        updatedAnimationParams();
        
        this.setDefaultCloseOperation(DISPOSE_ON_CLOSE);
        this.setVisible(true);
    }
    
    private void initComponents() {
        getContentPane().setLayout(new BoxLayout(getContentPane(), BoxLayout.PAGE_AXIS));
        this.setTitle("Fractal Animation");
        this.setSize(600, 400);
        
        // constant properties: framecount / rate, etc.
        JPanel pnlInfo = new JPanel();
        pnlInfo.setLayout(new BoxLayout(pnlInfo, BoxLayout.LINE_AXIS));
        JSONParser parser = new JSONParser();
        JSONObject schemaFrameCount;
        try {
            schemaFrameCount = (JSONObject)parser.parse("{\"id\":\"frames\", \"caption\":\"Frame Count\", \"type\":\"integer\", "
                    + "\"min\":2, \"allowZero\":false, \"default\":150}");
        } catch (ParseException ex) {
            Logger.getLogger(AnimationWindow.class.getName()).log(Level.SEVERE, null, ex);
            return;
        }
        frameCount = new ParamsElementIntegral(schemaFrameCount, params.get("anim"), cb, -1, "/");
        pnlInfo.add(frameCount.getInnerElm());
        getContentPane().add(pnlInfo);
        
        timeLine = new JPanel();
        timeLine.setLayout(new BoxLayout(timeLine, BoxLayout.PAGE_AXIS));
        JScrollPane scroll = new JScrollPane(timeLine, VERTICAL_SCROLLBAR_ALWAYS, HORIZONTAL_SCROLLBAR_NEVER);
        getContentPane().add(timeLine);
        
        selKF = new JButton("Select/Add Key Frame");
        selKF.addActionListener(new ActionListener(){
            @Override
            public void actionPerformed(ActionEvent ae) {
                ArrayList<String> tmpList = new ArrayList(APrevised.keySet());
                Collections.sort(tmpList);
                AnimationSelectKF askf = new AnimationSelectKF(AnimationWindow.this);
                askf.setUp(tmpList, frameCount.getIntVal());
                askf.setVisible(true);
            }
        });
        getContentPane().add(selKF);
        
    }
    
    public void updatedAnimationParams(){
        for(AnimationParamPanel p : P.values()){
            timeLine.remove(p);
        }
        P.clear();
        
        cb.removeAnimationParamType("/");
        
        APrevised.clear();
        for(String s : AP.keySet()){
            if(AP.get(s).type.equals(AnimationParamType.APT_NON_TYPE))
                continue;
            APrevised.put(s, AP.get(s));
        }
        
        JSONArray keyframes = (JSONArray)((JSONObject)params.get("anim")).get("keyframes");
        Iterator<Object> it = keyframes.iterator();
        while (it.hasNext()){
            Object tmp = it.next();
            if(!(tmp instanceof JSONObject)){
                it.remove();
                continue;
            }
            
            // first verify data
            JSONObject kf = (JSONObject)tmp;
            if(!kf.containsKey("frame") || !(kf.get("frame") instanceof Number)){
                it.remove();
                continue;
            }
            if(!kf.containsKey("val")){
                it.remove();
                continue;
            }
            if(!kf.containsKey("param") || !(kf.get("param") instanceof String)){
                it.remove();
                continue;
            }
            String param = (String)kf.get("param");
            if(!APrevised.containsKey(param)){
                it.remove();
                continue;
            }
            if(!kf.containsKey("interp") || !(kf.get("interp") instanceof String)){
                kf.put("interp", "none");
            }
            String interp = (String)kf.get("interp");
            if(!"none".equals(interp) && !"linear".equals(interp) && !"cube".equals(interp) && !"square".equals(interp) &&
                    !"sqroot".equals(interp) && !"cuberoot".equals(interp)){
                kf.put("interp", "none");
                interp = "none";
            }
            
            if(!P.containsKey(param)){
                AnimationParamPanel p = new AnimationParamPanel(APrevised.get(param), 
                                                keyframes, frameCount.getIntVal(), this);
                if(param.equals(selectedParam)){
                    p.setSelectedFrame(selectedFrame);
                }
                P.put(param, p);
            }
        }
        
        for(AnimationParamPanel p : P.values()){
            p.updateDisplay();
            timeLine.add(p);
        }
        this.revalidate();
    }
    
    public void frameSelected(String param, int fno){
        System.out.println(param + " : "+ fno);
        selectedFrame = fno;
        selectedParam = param;
        updatedAnimationParams();
    }
    
    private JPanel timeLine;
    private ParamsElementIntegral frameCount;
    private JSONObject params;
    private PanelProperties cb;
    private HashMap<String, AnimationParam> AP;
    private HashMap<String, AnimationParam> APrevised = new HashMap();
    private HashMap<String, AnimationParamPanel> P;
    private JButton selKF;
    
    private int selectedFrame;
    private String selectedParam;
}
