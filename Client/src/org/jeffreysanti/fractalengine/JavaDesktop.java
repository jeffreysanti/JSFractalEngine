/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package org.jeffreysanti.fractalengine;

import info.monitorenter.gui.chart.Chart2D;
import info.monitorenter.gui.chart.ITrace2D;
import info.monitorenter.gui.chart.traces.Trace2DSimple;
import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.util.Random;
import javax.swing.BorderFactory;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JScrollPane;
import javax.swing.UIManager;
import javax.swing.UnsupportedLookAndFeelException;
import org.json.simple.JSONValue;

/**
 *
 * @author jeffrey
 */
public class JavaDesktop extends JFrame {
    
    public static void main(String[] args) {
        try {
        UIManager.setLookAndFeel(
            UIManager.getSystemLookAndFeelClassName());
        } 
        catch (UnsupportedLookAndFeelException e) {
           // handle exception
        }
        catch (ClassNotFoundException e) {
           // handle exception
        }
        catch (InstantiationException e) {
           // handle exception
        }
        catch (IllegalAccessException e) {
           // handle exception
        }
        
        getInst().setVisible(true);
        
        Thread networkThread = new Thread(ServerConnection.getInst());
        networkThread.run();
    }
    
    private JavaDesktop(){
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        this.setTitle("JSFractalEngine DEV");
        
        setLayout(new BorderLayout(0, 0));
        setSize(800, 600);

        sb = new StatusBar();
        add(sb, BorderLayout.SOUTH);
        
        lib = new PanelLibrary();
        JScrollPane scrollPane = new JScrollPane(lib, JScrollPane.VERTICAL_SCROLLBAR_ALWAYS, JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
        scrollPane.getVerticalScrollBar().setUnitIncrement(10);
        scrollPane.setBorder(BorderFactory.createEmptyBorder());
        add(scrollPane, BorderLayout.EAST);
        
        wsp = new PanelWorkspace();
        add(wsp, BorderLayout.CENTER);
    }
    
    public static JavaDesktop getInst(){
        return mainWnd;
    }
    
    public StatusBar getStatusBar(){
        return sb;
    }
    public PanelLibrary getLibraryPanel(){
        return lib;
    }
    public PanelWorkspace getWorkspacePanel(){
        return wsp;
    }
    
    
    private static final JavaDesktop mainWnd = new JavaDesktop();
    
    
    private StatusBar sb;
    private PanelLibrary lib;
    private PanelWorkspace wsp;
}
