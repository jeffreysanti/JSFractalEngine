/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package org.jeffreysanti.fractalengine;

import java.awt.KeyEventDispatcher;
import java.awt.KeyboardFocusManager;
import java.awt.AWTEvent;
import java.awt.event.AWTEventListener;
import javax.swing.AbstractAction;
import java.awt.event.InputEvent;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.awt.geom.NoninvertibleTransformException;
import java.awt.geom.Point2D;
import java.awt.geom.AffineTransform;
import java.awt.Graphics2D;
import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Image;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.ComponentEvent;
import java.awt.event.ComponentListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.image.BufferedImage;
import java.awt.image.ImageFilter;
import java.io.File;
import java.io.IOException;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.imageio.ImageIO;
import javax.swing.ImageIcon;
import javax.swing.JFileChooser;
import javax.swing.JLabel;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPopupMenu;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.KeyStroke;
import javax.swing.filechooser.FileFilter;
import static org.jeffreysanti.fractalengine.LibraryTile.FDBS_UNLINKED;
import org.json.simple.JSONObject;

/**
 *
 * @author jeffrey
 */
public class ViewPaneImage extends ViewPaneAbstract implements ComponentListener {
    
    
    public ViewPaneImage(Context c){
        super(c);
        isLoaded = false;
        
        trans = new AffineTransform();
        
        projectedWidth = 0;
        projectedHeight = 0;
        
        this.setLayout(new BorderLayout());
        
        img = new JLabel();
        this.add(img, BorderLayout.CENTER);
        
        img.addMouseListener(new ContextMenuListener());
        
        this.addComponentListener(this);
        contextDataReceieved();
        
        keyHndlr = new KeyHandler();
    }

    @Override
    synchronized void contextDataReceieved() {
        if(context.getFullImage() == null || context.getFullImage().getHeight() < 1)
            return;
        
        if(this.getWidth() == 0 || this.getHeight() == 0)
            return;
        
        isLoaded = true;
        
        double aspectRatio = (double)context.getFullImage().getWidth() / 
                        (double)context.getFullImage().getHeight();
        Image i;
        if((double)this.getHeight() * aspectRatio > this.getWidth()){ // need to scale on width-constrait
            projectedWidth = this.getWidth();
            projectedHeight = (int)((double)this.getWidth() / aspectRatio);
        }else{
            projectedHeight = this.getHeight();
            projectedWidth = (int)((double)this.getHeight() * aspectRatio);
        }
        
        // now get the subimage baded on transformations
        BufferedImage imgTransform = new BufferedImage(context.getFullImage().getWidth(), context.getFullImage().getHeight(), 
                BufferedImage.TYPE_INT_RGB);
        Graphics2D dc = imgTransform.createGraphics();
        dc.setBackground(Color.MAGENTA);
        boolean ret = dc.drawImage(context.getFullImage(), trans, null);
        imgTransform.flush();
        
        i = imgTransform.getScaledInstance(projectedWidth, projectedHeight, Image.SCALE_SMOOTH);
        img.setIcon(new ImageIcon(i));
        this.revalidate();
        
        if(trans.getTranslateX() != 0 || trans.getTranslateY() != 0 ||
                trans.getScaleX() != 1 || trans.getScaleY() != 1)
            context.markDirty(); // a change has been made
        
        if(context.getParams().containsKey("basic") && context.getParams().get("basic") instanceof JSONObject){
            JSONObject json = (JSONObject)context.getParams().get("basic");
            json.put("translateX", -Math.round(trans.getTranslateX()));
            json.put("translateY", -Math.round(trans.getTranslateY()));
            json.put("scaleX", trans.getScaleX());
            json.put("scaleY", trans.getScaleY());
        }
    }
    
    synchronized void saveImage(){
        if(context.isModified())
            JOptionPane.showMessageDialog(null, "It has been reported that this context was modified. If so, the\n"+
                "image may appear differently, particuarly if windowing was changed.");
        
        JFileChooser fc = new JFileChooser();
        fc.setAcceptAllFileFilterUsed(false);
        fc.addChoosableFileFilter(new FileFilter(){
            public String getExtension(File f) {
                String ext = null;
                String s = f.getName();
                int i = s.lastIndexOf('.');

                if (i > 0 &&  i < s.length() - 1) {
                    ext = s.substring(i+1).toLowerCase();
                }
                return ext;
            }
            
            @Override
            public boolean accept(File f) {
                if (f.isDirectory()) {
                    return true;
                }

                String extension = getExtension(f);
                if (extension != null) {
                    if (extension.equalsIgnoreCase("png"))
                        return true;
                }
                return false;
            }

            @Override
            public String getDescription() {
                return "PNG Image (.png)";
            }
        });
        int returnVal = fc.showSaveDialog(JavaDesktop.getInst());
        if (returnVal == JFileChooser.APPROVE_OPTION) {
            File file = fc.getSelectedFile();
            if(!file.getAbsolutePath().endsWith(".png")){
                file = new File(file + ".png");
            }
            try {
                ImageIO.write(context.getFullImage(), "png", file);
            } catch (IOException ex) {
                Logger.getLogger(ViewPaneImage.class.getName()).log(Level.SEVERE, null, ex);
            }
        }
    }
    
    JLabel img;

    @Override
    public synchronized void componentResized(ComponentEvent ce) {
        contextDataReceieved();
    }

    @Override
    public synchronized void componentMoved(ComponentEvent ce) { }

    @Override
    public synchronized void componentShown(ComponentEvent ce) {
        KeyboardFocusManager manager = KeyboardFocusManager.getCurrentKeyboardFocusManager();
        manager.addKeyEventDispatcher(keyHndlr);
    }

    @Override
    public synchronized void componentHidden(ComponentEvent ce) {
        KeyboardFocusManager manager = KeyboardFocusManager.getCurrentKeyboardFocusManager();
        manager.removeKeyEventDispatcher(keyHndlr);
    }
    
    
    public synchronized Point2D getCenterPoint(){
        // x is left-aligned, y is center-aligned
        double curImgX = projectedWidth/2;
        double curImgY = projectedHeight/2;

        // convert curImg to fullsize coords
        curImgX = (double)curImgX * (double)context.getFullImage().getWidth()/(double)projectedWidth;
        curImgY = (double)curImgY * (double)context.getFullImage().getHeight()/(double)projectedHeight;

        // undo old transformation
        Point2D pt = new Point2D.Double();
        try {
            trans.inverseTransform(new Point2D.Double(curImgX, curImgY), pt);
        } catch (NoninvertibleTransformException ex) {
            Logger.getLogger(ViewPaneImage.class.getName()).log(Level.SEVERE, null, ex);
            return null;
        }
        return pt;
    }
    
    public synchronized void centerTransform(double scaleX, double scaleY, Point2D cent){
        trans = new AffineTransform();

        // now apply scale
        trans.translate(context.getFullImage().getWidth()/2, context.getFullImage().getHeight()/2);
        trans.scale(scaleX, scaleY);
        trans.translate(-context.getFullImage().getWidth()/2, -context.getFullImage().getHeight()/2);

        // apply final translation
        trans.translate(context.getFullImage().getWidth()/2 - cent.getX(), 
                            context.getFullImage().getHeight()/2 - cent.getY());
    }
    
    public synchronized void scaleAboutXCenter(double factor){
        Point2D pt = getCenterPoint();
        if(pt != null){
            double scaleX = trans.getScaleX() * factor;
            double scaleY = trans.getScaleY();
            centerTransform(scaleX, scaleY, pt);

            contextDataReceieved();
        }
    }
    public synchronized void scaleAboutYCenter(double factor){
        Point2D pt = getCenterPoint();
        if(pt != null){
            double scaleX = trans.getScaleX();
            double scaleY = trans.getScaleY() * factor;
            centerTransform(scaleX, scaleY, pt);

            contextDataReceieved();
        }
    }
    public synchronized void translateAboutX(double factor){
        double tX = factor * context.getFullImage().getWidth() * 1/trans.getScaleX();
        trans.translate(tX, 0);
        contextDataReceieved();
    }
    public synchronized void translateAboutY(double factor){
        double tY = factor * context.getFullImage().getHeight() * 1/trans.getScaleY();
        trans.translate(0, tY);
        contextDataReceieved();
    }
    
    
    private boolean isLoaded;
    private volatile AffineTransform trans;
    
    private int projectedWidth, projectedHeight;
    private KeyHandler keyHndlr;
    
    class ContextMenuListener extends MouseAdapter {
        @Override
        public void mousePressed(MouseEvent e){
            if(!isLoaded)
                return;
            if (e.isPopupTrigger())
                showContextMenu(e);
            else{
                
            }
        }
        @Override
        public void mouseReleased(MouseEvent e){
            if(!isLoaded)
                return;
            if (e.isPopupTrigger())
                showContextMenu(e);
        }

        private void showContextMenu(MouseEvent e){
            ContextMenu menu = new ContextMenu(e);
            menu.show(e.getComponent(), e.getX(), e.getY());
        }
    };
    
    class ContextMenu extends JPopupMenu {
        
        public ContextMenu(MouseEvent e){
            me = e;
            JMenuItem menuItem = new JMenuItem("Set Center");
            menuItem.addActionListener(new ActionListener(){

                @Override
                public void actionPerformed(ActionEvent ae) {
                    Point2D pt = mouseToFullSizeCoord();
                    if(pt != null){
                        System.out.println("PT: " + pt.getX() + ", "+pt.getY());
                        double scaleX = trans.getScaleX();
                        double scaleY = trans.getScaleY();
                        centerTransform(scaleX, scaleY, pt);
                        
                        contextDataReceieved();
                    }
                }
            });
            add(menuItem);
            
            menuItem = new JMenuItem("Reset Window");
            menuItem.addActionListener(new ActionListener(){
                @Override
                public void actionPerformed(ActionEvent ae) {
                    trans = new AffineTransform();
                    contextDataReceieved();
                }
            });
            add(menuItem);
            
            menuItem = new JMenuItem("Zoom In 25%");
            menuItem.addActionListener(new ActionListener(){
                @Override
                public void actionPerformed(ActionEvent ae) {
                    Point2D pt = mouseToFullSizeCoord();
                    if(pt != null){
                        double scaleX = trans.getScaleX() * 1.25;
                        double scaleY = trans.getScaleY() * 1.25;
                        centerTransform(scaleX, scaleY, pt);
                        
                        contextDataReceieved();
                    }
                }
            });
            add(menuItem);
            
            menuItem = new JMenuItem("Zoom Out 25%");
            menuItem.addActionListener(new ActionListener(){
                @Override
                public void actionPerformed(ActionEvent ae) {
                    Point2D pt = mouseToFullSizeCoord();
                    if(pt != null){
                        double scaleX = trans.getScaleX() * 0.75;
                        double scaleY = trans.getScaleY() * 0.75;
                        centerTransform(scaleX, scaleY, pt);
                        
                        contextDataReceieved();
                    }
                    contextDataReceieved();
                }
            });
            add(menuItem);
            
            menuItem = new JMenuItem("Zoom In 5% On X [Ctrl-L]");
            menuItem.addActionListener(new ActionListener(){
                @Override
                public void actionPerformed(ActionEvent ae) {
                    scaleAboutXCenter(1.05);
                }
            });
            add(menuItem);
            menuItem = new JMenuItem("Zoom Out 5% On X [Ctrl-J]");
            menuItem.addActionListener(new ActionListener(){
                @Override
                public void actionPerformed(ActionEvent ae) {
                    scaleAboutXCenter(0.95);
                }
            });
            add(menuItem);
            menuItem = new JMenuItem("Zoom In 5% On Y [Ctrl-I]");
            menuItem.addActionListener(new ActionListener(){
                @Override
                public void actionPerformed(ActionEvent ae) {
                    scaleAboutYCenter(1.05);
                }
            });
            add(menuItem);
            menuItem = new JMenuItem("Zoom Out 5% On Y [Ctrl-K]");
            menuItem.addActionListener(new ActionListener(){
                @Override
                public void actionPerformed(ActionEvent ae) {
                    scaleAboutYCenter(0.95);
                }
            });
            add(menuItem);
            menuItem = new JMenuItem("Save Image");
            menuItem.addActionListener(new ActionListener(){
                @Override
                public void actionPerformed(ActionEvent ae) {
                    saveImage();
                }
            });
            add(menuItem);
            
            
        }
        public Point2D mouseToFullSizeCoord(){
            // x is left-aligned, y is center-aligned
            double curImgX = me.getX();
            double curImgY = (double)me.getY() - ((double)ViewPaneImage.this.getHeight() - (double)projectedHeight) / 2.0;

            // convert curImg to fullsize coords
            curImgX = (double)curImgX * (double)context.getFullImage().getWidth()/(double)projectedWidth;
            curImgY = (double)curImgY * (double)context.getFullImage().getHeight()/(double)projectedHeight;

            // undo old transformation
            Point2D pt = new Point2D.Double();
            try {
                trans.inverseTransform(new Point2D.Double(curImgX, curImgY), pt);
            } catch (NoninvertibleTransformException ex) {
                Logger.getLogger(ViewPaneImage.class.getName()).log(Level.SEVERE, null, ex);
                return null;
            }
            return pt;
        }
        private MouseEvent me;
    };
    
    class KeyHandler implements KeyEventDispatcher{
        @Override
        public boolean dispatchKeyEvent(KeyEvent evt) {
            if(!evt.isControlDown())
                return false;
            if(evt.getID() != KeyEvent.KEY_RELEASED)
                return false;
            
            double shiftVal = 0.01;
            if(evt.isShiftDown())
                shiftVal = 0.05;
            
            if(evt.getKeyCode() == KeyEvent.VK_L)
                scaleAboutXCenter(1.05);
            else if(evt.getKeyCode() == KeyEvent.VK_I)
                scaleAboutYCenter(1.05);
            else if(evt.getKeyCode() == KeyEvent.VK_J)
                scaleAboutXCenter(0.95);
            else if(evt.getKeyCode() == KeyEvent.VK_K)
                scaleAboutYCenter(0.95);
            else if(evt.getKeyCode() == KeyEvent.VK_RIGHT)
                translateAboutX(shiftVal);
            else if(evt.getKeyCode() == KeyEvent.VK_LEFT)
                translateAboutX(-shiftVal);
            else if(evt.getKeyCode() == KeyEvent.VK_UP)
                translateAboutY(-shiftVal);
            else if(evt.getKeyCode() == KeyEvent.VK_DOWN)
                translateAboutY(shiftVal);
            else
                return false;
            return false;
        }
    };
}
