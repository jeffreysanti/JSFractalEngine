/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package org.jeffreysanti.fractalengine;

import java.awt.Color;
import java.util.ArrayList;

/**
 *
 * @author jeffrey
 */
public class ColorPalette {
    
    public class ColorTrait{
        public Color col;
        public int maxIter;
        
        public ColorTrait(Color c, int max){
            col = c;
            maxIter = max;
        }
        public ColorTrait(Color c){
            this(c, -1);
        }
    }
    
    
    public ColorPalette(Paramaters p){
        C = new ArrayList();
        params = p;
        schem = new ParamaterSchema("./SCHEMA_ALL");
    }
    
    public void loadColorsList(){
        try{
            C.clear();
            int count = Integer.parseInt(schem.getDefaultOrCurrent("fillColPalCount", params));
            if(count < 1){
                loadDefaultColorsList();
                return;
            }
            
            for(int i=1; i<=count; i++){
                String s = params.getValue("fillColPal$"+i, "RGB0,0,0");
                if(!s.substring(0, 3).equals("RGB") || s.length() < 8){
                    loadDefaultColorsList();
                    return;
                }
                s = s.substring(3);
                String [] arr = s.split(",");
                if(arr.length != 3){
                    loadDefaultColorsList();
                    return;
                }
                Color col = new Color(Integer.parseInt(arr[0]), Integer.parseInt(arr[1]), Integer.parseInt(arr[2]));
                int maxIter = Integer.parseInt(params.getValue("fillColPalIterMax$"+i, "-1"));
                C.add(new ColorTrait(col, maxIter));
            }
            
        }catch(NumberFormatException e){
            loadDefaultColorsList();
        }
    }
    public void loadDefaultColorsList(){
        C.clear();
        C.add(new ColorTrait(Color.RED));
        C.add(new ColorTrait(Color.ORANGE));
        C.add(new ColorTrait(Color.YELLOW));
        C.add(new ColorTrait(Color.GREEN));
        C.add(new ColorTrait(Color.BLUE));
        C.add(new ColorTrait(Color.BLACK));
        writeToParamaters();
    }
    
    public void writeToParamaters(){
        params.setValue("fillColPalCount", Integer.toString(C.size()));
        for(int i=0; i<C.size(); i++){
            params.setValue("fillColPal$"+(i+1), "RGB"+C.get(i).col.getRed()+","+C.get(i).col.getGreen()+","+
                    C.get(i).col.getBlue());
            params.setValue("fillColPalIterMax$"+(i+1), Integer.toString(C.get(i).maxIter));
        }
    }
    
    public void addColorTrait(Color c, int maxIter){
        ColorTrait trait = new ColorTrait(c, maxIter);
        C.add(trait);
    }
    
    public ArrayList<ColorTrait> getColors(){
        return C;
    }
    
    
    private Paramaters params;
    private ParamaterSchema schem;
    private ArrayList<ColorTrait> C;
}
