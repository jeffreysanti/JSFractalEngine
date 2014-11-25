/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package org.jeffreysanti.fractalengine;

import info.monitorenter.gui.chart.Chart2D;
import info.monitorenter.gui.chart.IAxis.AxisTitle;
import info.monitorenter.gui.chart.ITrace2D;
import info.monitorenter.gui.chart.labelformatters.LabelFormatterNumber;
import info.monitorenter.gui.chart.traces.Trace2DSimple;
import info.monitorenter.gui.chart.traces.painters.TracePainterFill;
import info.monitorenter.gui.chart.traces.painters.TracePainterVerticalBar;
import info.monitorenter.util.Range;
import java.awt.BorderLayout;
import java.text.NumberFormat;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Locale;

/**
 *
 * @author jeffrey
 */
public class ViewPaneHistogram extends ViewPaneAbstract {

    ViewPaneHistogram(Context x, Chart2D g){
        super(x);
        
        this.setLayout(new BorderLayout());
        chart = g;
        this.add(chart, BorderLayout.CENTER);
    }
    
    static Chart2D parseGraph(HashMap<String,Object> json){
        Chart2D chart = new Chart2D();
        chart.removeAllTraces();
        ITrace2D trace = new Trace2DSimple();
        chart.addTrace(trace);
        
        // now load the trace points
        trace.setTracePainter(new TracePainterVerticalBar(chart));
        for(Object o : (ArrayList)json.get("trace")){
            ArrayList<Number> pts = (ArrayList)o;
            trace.addPoint(pts.get(0).doubleValue(), pts.get(1).doubleValue());
        }
        
        chart.getAxisX().setRange(new Range(((Number)json.get("xmin")).doubleValue(), ((Number)json.get("xmax")).doubleValue()));
        chart.getAxisY().setRange(new Range(((Number)json.get("ymin")).doubleValue(), ((Number)json.get("ymax")).doubleValue()));
        chart.getAxisX().setMinorTickSpacing(1.0);
        chart.getAxisY().setMinorTickSpacing(1.0);
        chart.getAxisX().setAxisTitle(new AxisTitle((String)json.get("xaxis")));
        chart.getAxisY().setAxisTitle(new AxisTitle((String)json.get("yaxis")));
        chart.getAxisX().setFormatter(new LabelFormatterNumber(NumberFormat.getIntegerInstance(Locale.ENGLISH)));
        chart.getAxisY().setFormatter(new LabelFormatterNumber(NumberFormat.getIntegerInstance(Locale.ENGLISH)));
        
        chart.setUseAntialiasing(true);
        chart.setName((String)json.get("title"));
        
        return chart;
    }
    
    private Chart2D chart;

    @Override
    void contextDataReceieved() {
    }
    
}
