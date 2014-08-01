/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package org.jeffreysanti.fractalengine;

import java.awt.Component;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.DataInputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JPanel;

/**
 *
 * @author jeffrey
 */
public class PanelProperties extends JPanel implements ASyncPoolAcceptor {
    
    PanelProperties(){
        this.setLayout(new BoxLayout(this, BoxLayout.PAGE_AXIS));
        P = new ArrayList();
    }
    
    public void switchContext(Context cont){
        c = cont;
        
        for(Component p : P){
            this.remove(p);
        }
        P.clear();
        
        if(cont.isDataImportedYet()){ // otherwise we need to wait
        
            addProperty(new LinkedTextField(cont, "title", "Title"));
            addProperty(new LinkedTextField(cont, "author", "Author"));
            addProperty(new LinkedTextField(cont, "timeOut", "Time Out (sec)"));
            addProperty(new LinkedTextField(cont, "imgWidth", "Width (px)"));
            addProperty(new LinkedTextField(cont, "imgHeight", "Height (px)"));

            HashMap<String, String> types = new HashMap();
            types.put("multibrot", "Mandlebrot Type");
            types.put("julia", "Julia Type");
            addProperty(new LinkedComboBox(cont, "type", "Fractal Type", types));

            addProperty(Box.createVerticalStrut(20));

            addProperty(new LinkedTextField(cont, "centR", "X-Center"));
            addProperty(new LinkedTextField(cont, "centI", "Y-Center"));
            addProperty(new LinkedTextField(cont, "radR", "X-Radius"));
            addProperty(new LinkedTextField(cont, "radI", "Y-Radius"));
            addProperty(new LinkedTextField(cont, "iters", "Iteration Count"));
            addProperty(new LinkedTextField(cont, "funct", "Function"));
            addProperty(new LinkedTextField(cont, "threshold", "Escape Threshold"));
            addProperty(new JLabel("Julia Set Only:"));
            addProperty(new LinkedTextField(cont, "Cr", "C (Real)"));
            addProperty(new LinkedTextField(cont, "Ci", "C (Imag)"));

            addProperty(Box.createVerticalStrut(20));

            types = new HashMap();
            types.put("histogram", "Histogram");
            types.put("simplin", "Simple Linear");
            types.put("linear", "Linear");
            types.put("root2", "Square Root");
            types.put("root3", "Cubic Root");
            types.put("root4", "Quartic Root");
            types.put("log", "Logarithmic (ln)");
            types.put("rankorder", "Rank Order [Rearranges Palette]");
            types.put("none", "No Shading");
            addProperty(new LinkedComboBox(cont, "shading", "Color Algorithm", types));

            // TODO: BG COLOR & COLORS

            types = new HashMap();
            types.put("discrete", "Discrete Coloring");
            types.put("continuous", "Continuous Palette");
            types.put("iterMax", "Manual Iteration Amounts");
            addProperty(new LinkedComboBox(cont, "fillColPalType", "Color Palette Type", types));
            
            JButton colorDlgBtn = new JButton("Edit Color Palette");
            colorDlgBtn.addActionListener(new ActionListener(){
                @Override
                public void actionPerformed(ActionEvent ae) {
                    PaletteEditor editor = new PaletteEditor(new ColorPalette(c.getParams()));
                    editor.setVisible(true);
                }
            });
            addProperty(colorDlgBtn);
            
            addProperty(Box.createVerticalStrut(20));

            types = new HashMap();
            types.put("none", "No Tracing");
            types.put("blend", "Blend Surrounding Colors");
            types.put("solid", "Solid Tracing (Set Color)");
            addProperty(new LinkedComboBox(cont, "tracing", "Tracing Style", types));

            // TODO: Trace Color

            addProperty(new LinkedTextField(cont, "traceRadius", "Trace Radius"));
            addProperty(new LinkedTextField(cont, "traceBlur", "Tracing Blur Factor"));
            addProperty(new LinkedTextField(cont, "traceOpacity", "Trace Opacity"));

            addProperty(Box.createVerticalStrut(20));

            addProperty(new LinkedTextField(cont, "imgBlur", "Image Blur Factor"));
            addProperty(new LinkedTextField(cont, "imgSharpen", "Image Sharpen Factor"));

            btnSubmit = new JButton("Submit Job");
            btnSubmit.addActionListener(new ActionListener(){
                @Override
                public void actionPerformed(ActionEvent ae) {
                    submit();
                }
            });

            addProperty(btnSubmit);
        }
        
        this.revalidate();
    }
    
    public void addProperty(Component comp) {
        this.add(comp);
        P.add(comp);
    }
    
    public void submit(){
        for(Component p : P){
            p.setEnabled(false);
        }
        
        ServerConnection.getInst().sendPacket("SJOB" + c.getParams().dumpToString()); // request all avali info
        ServerConnection.getInst().getASyncPool().addASyncAcceptor("SJOB", this);
    }
    
    
    private Context c;
    private ArrayList<Component> P;
    private JButton btnSubmit;

    @Override
    public boolean poolDataReceived(String head, DataInputStream ds) {
        int jid = Integer.parseInt(ServerConnection.extractString(ds));
        System.out.println("Submitted JOB: " + jid);
        c.becameSubmittedJob(jid);
        return true;
    }
}
