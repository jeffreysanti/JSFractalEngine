/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package org.jeffreysanti.fractalengine;

import java.io.BufferedReader;
import java.io.FileReader;
import java.util.HashMap;

/**
 *
 * @author jeffrey
 */
public class ParamaterSchema {
    
    enum PARAM_TYPE{
	PARAM_TYPE_STRING,
	PARAM_TYPE_WHOLE,
	PARAM_TYPE_REAL,
	PARAM_TYPE_BOOL,
	PARAM_TYPE_COLOR
    }

    enum EXTERMA_TYPE{
        EXT_IGNORE,
        EXT_INCLUSIVE,
        EXT_EXCLUSIVE
    }

    class ParamConstraints{
        String name;

        PARAM_TYPE type;
        boolean zeroNullOkay; // can this be zero (if numeric) or empty string
        String defValue;

        boolean setOf;

        // only for numeric types
        double min; EXTERMA_TYPE emin;
        double max; EXTERMA_TYPE emax;

        int setMin;
        int setMax;
    }
    
    ParamaterSchema(String schemaFile) {
        C = new HashMap();
        try{
            BufferedReader br = new BufferedReader(new FileReader(schemaFile));
            String line;
            while ((line = br.readLine()) != null) {
               if(line.length() < 2)
                    continue;
                ParamConstraints c = lineParse(line);
                C.put(c.name, c);
            }
	}catch(Exception e){
            System.out.println("Error: Schema File Missing:" + schemaFile + "\n");
	}

    }

    // Format of line as follows:
    // <paramName>:<VariableType>/<AllowNull? Y/N>(range)|<defValue>
    // Variable Types: S for strings, R for real values, W for whole number, B for boolean, C for color
    // range is in set format [ ( inclusive / exclusive ) ]
    // a question mark in paramName indicates a set-of followed by min;max # of elements
    // if no min or max just leave empty ie: "colorSet?;" or "colorSet?1;"
    final ParamConstraints lineParse(String line)
    {
        ParamConstraints c = new ParamConstraints();
        c.setOf = false;
        c.min = 0; c.emin = EXTERMA_TYPE.EXT_IGNORE;
        c.max = 0; c.emax = EXTERMA_TYPE.EXT_IGNORE;
        c.setOf = false;
        c.setMin = -1;
        c.setMax = -1;

        c.name = line.substring(0, line.indexOf(':'));
        if(line.indexOf('?') != -1){
            c.setOf = true; // TODO: Implement Set min/max
        }
        line = line.substring(line.indexOf(':')+1);
        char type = line.charAt(0);
        c.type = PARAM_TYPE.PARAM_TYPE_STRING;
        if(type == 'R') c.type = PARAM_TYPE.PARAM_TYPE_REAL;
        if(type == 'W') c.type = PARAM_TYPE.PARAM_TYPE_WHOLE;
        if(type == 'B') c.type = PARAM_TYPE.PARAM_TYPE_BOOL;
        if(type == 'C') c.type = PARAM_TYPE.PARAM_TYPE_COLOR;

        line = line.substring(line.indexOf('/')+1);
        type = line.charAt(0);
        if(type == 'Y') c.zeroNullOkay = true;
        else c.zeroNullOkay = false;

        line = line.substring(1);
        if(line.charAt(0) == '(' || line.charAt(0) == '['){ // range exclusive
                c.emin = EXTERMA_TYPE.EXT_EXCLUSIVE;
                if(line.charAt(0) == '[')
                        c.emin = EXTERMA_TYPE.EXT_INCLUSIVE;

                line = line.substring(1);
                c.min = Double.parseDouble(line.substring(0, line.indexOf(',')));
                line = line.substring(line.indexOf(',') + 1);
                if(line.indexOf(')') != -1){
                        c.emax = EXTERMA_TYPE.EXT_EXCLUSIVE;
                        c.max = Double.parseDouble(line.substring(0, line.indexOf(')')));
                        line = line.substring(line.indexOf(')')+1);
                }else{
                        c.emax = EXTERMA_TYPE.EXT_INCLUSIVE;
                        c.max = Double.parseDouble(line.substring(0, line.indexOf(']')));
                        line = line.substring(line.indexOf(']')+1);
                }
        }

        line = line.substring(line.indexOf('|')+1);
        c.defValue = line;
        return c;
    }
    
    public String getDefault(String pname){
        if(!C.containsKey(pname)){
            System.err.println("Schema has no paramater: " + pname);
            return "";
        }
        ParamConstraints c = C.get(pname);
        return c.defValue;
    }
    public String getDefaultOrCurrent(String pname, Paramaters p){
        if(!C.containsKey(pname)){
            System.err.println("Schema has no paramater: " + pname);
            return "";
        }
        ParamConstraints c = C.get(pname);
        return p.getValue(pname, c.defValue);
    }
    
    public boolean validate(Paramaters p, String pname, String val){
        // TODO: More work
        if(!C.containsKey(pname)){
            System.err.println("Schema has no paramater: " + pname);
            return false;
        }
        ParamConstraints c = C.get(pname);
        if(!c.zeroNullOkay && val.equals(""))
            return false;
        if(c.type == PARAM_TYPE.PARAM_TYPE_REAL || c.type == PARAM_TYPE.PARAM_TYPE_WHOLE){
            try{
                double x = Double.parseDouble(val);
                if(!c.zeroNullOkay && x == 0)
                    return false;
                
            }catch(NumberFormatException ex){
                return false;
            }
        }
        
        return true;
    }
    
    
    private HashMap<String, ParamConstraints> C;
}
