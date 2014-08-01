/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package org.jeffreysanti.fractalengine;

import java.awt.Color;
import java.awt.Component;
import java.awt.Rectangle;
import java.awt.datatransfer.DataFlavor;
import java.awt.datatransfer.Transferable;
import java.awt.datatransfer.UnsupportedFlavorException;
import java.awt.dnd.DnDConstants;
import java.awt.dnd.DragGestureEvent;
import java.awt.dnd.DragGestureListener;
import java.awt.dnd.DragSource;
import java.awt.dnd.DragSourceDragEvent;
import java.awt.dnd.DragSourceDropEvent;
import java.awt.dnd.DragSourceEvent;
import java.awt.dnd.DragSourceListener;
import java.io.IOException;
import java.io.Serializable;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.swing.DefaultListModel;
import javax.swing.JComponent;
import javax.swing.JLabel;
import javax.swing.JList;
import javax.swing.JPanel;
import javax.swing.ListCellRenderer;
import javax.swing.TransferHandler;
import javax.swing.colorchooser.ColorSelectionModel;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;
import org.jeffreysanti.fractalengine.ColorPalette.ColorTrait;

/**
 *
 * @author jeffrey
 */
public class PaletteEditor extends javax.swing.JFrame {   
    
    private class ColorPreview extends JPanel implements DragSourceListener, DragGestureListener, ListCellRenderer, Transferable{
        
        ColorPreview(Color c, int maxIter){
            super();
            
            col = c;
            upperBound = maxIter;
            
            t = new TransferHandler(){
                @Override
                public Transferable createTransferable(JComponent c){
                    return new ColorPreview(col, upperBound);
                }
            };
            setTransferHandler(t);
            
            source = new DragSource();
            source.createDefaultDragGestureRecognizer(this, DnDConstants.ACTION_COPY, this);
        }
        
        public void setColorValue(Color c, int bound){
            col = c;
            upperBound = bound;
            this.setBackground(col);
        }
        
        
        @Override
        public DataFlavor[] getTransferDataFlavors() {
            try {
                return new DataFlavor[]{(DataFlavor)FLAVOR.clone()};
            } catch (CloneNotSupportedException ex) {
                Logger.getLogger(PaletteEditor.class.getName()).log(Level.SEVERE, null, ex);
            }
            return new DataFlavor[]{};
        }

        @Override
        public boolean isDataFlavorSupported(DataFlavor df) {return true;}

        @Override
        public Object getTransferData(DataFlavor df) throws UnsupportedFlavorException, IOException {return this;}
        

        @Override
        public void dragEnter(DragSourceDragEvent dsde) {}

        @Override
        public void dragOver(DragSourceDragEvent dsde) {}

        @Override
        public void dropActionChanged(DragSourceDragEvent dsde) {}

        @Override
        public void dragExit(DragSourceEvent dse) {}

        @Override
        public void dragDropEnd(DragSourceDropEvent dsde) {}

        @Override
        public void dragGestureRecognized(DragGestureEvent dge) {
            source.startDrag(dge, DragSource.DefaultMoveDrop, new ColorPreview(col, upperBound), this);
        }
        
        @Override
        public Component getListCellRendererComponent(JList jlist, Object o, int i, boolean bln, boolean bln1) {
            if(o instanceof ColorPreview){
                ColorPreview prev = (ColorPreview)o;
                JLabel lbl = new JLabel("Color "+(i+1)+": Up to "+prev.upperBound+" Iterations");
                lbl.setBackground(prev.col);
                lbl.setOpaque(true);
                Color out = new Color(255-prev.col.getRed(), 255-prev.col.getGreen(), 
                                    255-prev.col.getBlue());
                lbl.setForeground(out);
                return lbl;
            }
            return null;
        }
        
        private DragSource source;
        private TransferHandler t;
        
        private Color col;
        private int upperBound;
        
    }
    
    /**
     * Creates new form PaletteEditor
     */
    public PaletteEditor(ColorPalette pal) {
        try {
            FLAVOR = new DataFlavor(DataFlavor.javaJVMLocalObjectMimeType + ";class=" + ColorPreview.class.getName());
        } catch (ClassNotFoundException ex) {
            Logger.getLogger(PaletteEditor.class.getName()).log(Level.SEVERE, null, ex);
        }
        
        palette = pal;
        pal.loadColorsList();
        
        initComponents();
        

        colorChooser.setPreviewPanel(new JPanel());
        
        colorModel = colorChooser.getSelectionModel();
        colorModel.addChangeListener(new ChangeListener() {
            
            @Override
            public void stateChanged(ChangeEvent evt) {
                ColorSelectionModel model = (ColorSelectionModel)evt.getSource();
                updateColor(model.getSelectedColor());
            }
        });
        colList.setCellRenderer(new ColorPreview(Color.BLACK, -1));
        
        updateColor(colorChooser.getColor());
        
        colList.setModel(listModel);
        colList.setTransferHandler(new TransferHandler() {
            @Override
            public boolean canImport(TransferHandler.TransferSupport support) {
                if (!support.isDataFlavorSupported(FLAVOR)) {
                    return false;
                }
                JList.DropLocation dl = (JList.DropLocation) support.getDropLocation();
                if (dl.getIndex() == -1) {
                    return false;
                } else {
                    return true;
                }
            }

            @Override
            public boolean importData(TransferHandler.TransferSupport support) {
                if (!canImport(support)) {
                  return false;
                }
                
                Transferable transferable = support.getTransferable();
                ColorPreview data;
                try {
                    transferable.getTransferData(FLAVOR);
                    data = (ColorPreview) transferable.getTransferData(FLAVOR);
                }catch (Exception e) {
                    return false;
                }
                JList.DropLocation dl = (JList.DropLocation) support.getDropLocation();
                int index = dl.getIndex();
                if (dl.isInsert()) {
                    listModel.add(index, data);
                } else {
                    listModel.set(index, data);
                }
                Rectangle r = colList.getCellBounds(index, index);
                colList.scrollRectToVisible(r);
                return true;
            }
        });
        
        colList.addListSelectionListener(new ListSelectionListener(){

            @Override
            public void valueChanged(ListSelectionEvent lse) {
                if(colList.getSelectedIndex() < 0 || colList.getSelectedIndex() >= listModel.size())
                    return;
                
                ColorPreview prev = (ColorPreview)listModel.getElementAt(colList.getSelectedIndex());
                txtMaxIter.setText(Integer.toString(prev.upperBound));
                colorChooser.setColor(prev.col);
            }
            
        });
        
        txtMaxIter.getDocument().addDocumentListener(new DocumentListener() {
            @Override
            public void changedUpdate(DocumentEvent e) {txtMaxIterActionPerformed(null);}

            @Override
            public void removeUpdate(DocumentEvent e) {txtMaxIterActionPerformed(null);}

            @Override
            public void insertUpdate(DocumentEvent e) {txtMaxIterActionPerformed(null);}
          });
        
        // load the colors into list
        for(ColorPalette.ColorTrait col : palette.getColors()){
            ColorPreview prev = new ColorPreview(col.col, col.maxIter);
            listModel.addElement(prev);
        }
    }

    public void updateColor(Color col){
        int val = -1;
        if(txtMaxIter.getText().length() > 0){
            try{
                val = Integer.parseInt(txtMaxIter.getText());
            }catch(NumberFormatException ex){
                
            }
        }
        ((ColorPreview)pnlPreview).setColorValue(col, val);
    }
    
    /**
     * This method is called from within the constructor to initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is always
     * regenerated by the Form Editor.
     */
    @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        jPanel2 = new javax.swing.JPanel();
        jScrollPane1 = new javax.swing.JScrollPane();
        colList = new javax.swing.JList();
        btnRemoveLast = new javax.swing.JButton();
        pnlPreview = new ColorPreview(Color.BLACK, -1);
        txtMaxIter = new javax.swing.JTextField();
        colorChooser = new javax.swing.JColorChooser();

        setDefaultCloseOperation(javax.swing.WindowConstants.DISPOSE_ON_CLOSE);
        setTitle("Palette Editor");
        addWindowListener(new java.awt.event.WindowAdapter() {
            public void windowClosing(java.awt.event.WindowEvent evt) {
                formWindowClosing(evt);
            }
        });
        getContentPane().setLayout(new javax.swing.BoxLayout(getContentPane(), javax.swing.BoxLayout.LINE_AXIS));

        jPanel2.setLayout(new javax.swing.BoxLayout(jPanel2, javax.swing.BoxLayout.PAGE_AXIS));

        colList.setModel(new javax.swing.AbstractListModel() {
            String[] strings = { "Item 1", "Item 2", "Item 3", "Item 4", "Item 5" };
            public int getSize() { return strings.length; }
            public Object getElementAt(int i) { return strings[i]; }
        });
        colList.setSelectionMode(javax.swing.ListSelectionModel.SINGLE_SELECTION);
        colList.setDropMode(javax.swing.DropMode.ON_OR_INSERT);
        jScrollPane1.setViewportView(colList);

        jPanel2.add(jScrollPane1);

        btnRemoveLast.setText("Remove Last Color");
        btnRemoveLast.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                btnRemoveLastActionPerformed(evt);
            }
        });
        jPanel2.add(btnRemoveLast);

        pnlPreview.setBackground(new java.awt.Color(179, 101, 101));

        javax.swing.GroupLayout pnlPreviewLayout = new javax.swing.GroupLayout(pnlPreview);
        pnlPreview.setLayout(pnlPreviewLayout);
        pnlPreviewLayout.setHorizontalGroup(
            pnlPreviewLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGap(0, 286, Short.MAX_VALUE)
        );
        pnlPreviewLayout.setVerticalGroup(
            pnlPreviewLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGap(0, 106, Short.MAX_VALUE)
        );

        jPanel2.add(pnlPreview);

        txtMaxIter.setText("-1");
        txtMaxIter.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                txtMaxIterActionPerformed(evt);
            }
        });
        jPanel2.add(txtMaxIter);

        getContentPane().add(jPanel2);
        getContentPane().add(colorChooser);

        pack();
    }// </editor-fold>//GEN-END:initComponents

    private void txtMaxIterActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_txtMaxIterActionPerformed
        updateColor(colorChooser.getColor());
    }//GEN-LAST:event_txtMaxIterActionPerformed

    private void btnRemoveLastActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_btnRemoveLastActionPerformed
        if(listModel.size() > 0){
            listModel.remove(listModel.size()-1);
        }
    }//GEN-LAST:event_btnRemoveLastActionPerformed

    private void formWindowClosing(java.awt.event.WindowEvent evt) {//GEN-FIRST:event_formWindowClosing
        // Write changes to palette
        palette.getColors().clear();
        for(Object prev : listModel.toArray()){
            ColorPreview col = (ColorPreview)prev;
            palette.addColorTrait(col.col,  col.upperBound);
        }
        palette.writeToParamaters();
    }//GEN-LAST:event_formWindowClosing

    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton btnRemoveLast;
    private javax.swing.JList colList;
    private javax.swing.JColorChooser colorChooser;
    private javax.swing.JPanel jPanel2;
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JPanel pnlPreview;
    private javax.swing.JTextField txtMaxIter;
    // End of variables declaration//GEN-END:variables

    private final DefaultListModel listModel = new DefaultListModel();
    private ColorSelectionModel colorModel;
    private DataFlavor FLAVOR;
    
    private ColorPalette palette;
    
    
    
}
