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
import java.util.Locale;

/**
 *
 * @author jeffrey
 */
public class ViewPaneHistogram extends ViewPaneAbstract {

    ViewPaneHistogram(Context x){
        super(x);
        
        this.setLayout(new BorderLayout());
        chart = new Chart2D();
        this.add(chart, BorderLayout.CENTER);
        
        contextDataReceieved();
    }
    
    @Override
    void contextDataReceieved() {
        // create a chart
        chart.removeAllTraces();
        ITrace2D trace = new Trace2DSimple();
        chart.addTrace(trace);
        
        // now load the trace points
        int[] histogram = context.getHistogram();
        if(histogram.length == 0){
            trace.setName("No Histogram Avaliable");
            return;
        }
        trace.setTracePainter(new TracePainterVerticalBar(chart));
        trace.setPhysicalUnits("Iterations", "Pixels");
        int max = 0;
        for(int i=0; i<histogram.length; i++){
            trace.addPoint(i+1, histogram[i]);
            if(histogram[i] > max)
                max = histogram[i];
        }
        chart.getAxisX().setRange(new Range(1, histogram.length));
        chart.getAxisY().setRange(new Range(0, max));
        chart.getAxisX().setMinorTickSpacing(1.0);
        chart.getAxisY().setMinorTickSpacing(1.0);
        chart.getAxisX().setAxisTitle(new AxisTitle("Iteration Count"));
        chart.getAxisY().setAxisTitle(new AxisTitle("Pixel Count"));
        chart.getAxisX().setFormatter(new LabelFormatterNumber(NumberFormat.getIntegerInstance(Locale.ENGLISH)));
        chart.getAxisY().setFormatter(new LabelFormatterNumber(NumberFormat.getIntegerInstance(Locale.ENGLISH)));
        
        chart.setUseAntialiasing(true);
        chart.setName("Histogram");
    }
    
    private Chart2D chart;
    
}
