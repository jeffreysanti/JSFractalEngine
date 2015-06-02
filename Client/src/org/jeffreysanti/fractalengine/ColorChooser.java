/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package org.jeffreysanti.fractalengine;

import java.awt.Color;
import java.awt.Cursor;
import java.awt.datatransfer.DataFlavor;
import java.awt.datatransfer.Transferable;
import java.awt.datatransfer.UnsupportedFlavorException;
import java.awt.dnd.DnDConstants;
import java.awt.dnd.DragGestureEvent;
import java.awt.dnd.DragGestureListener;
import java.awt.dnd.DragSource;
import java.awt.dnd.DropTarget;
import java.awt.dnd.DropTargetAdapter;
import java.awt.dnd.DropTargetDropEvent;
import java.awt.dnd.DropTargetListener;
import java.io.IOException;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.swing.JComponent;
import javax.swing.JPanel;
import javax.swing.TransferHandler;
import javax.swing.colorchooser.ColorSelectionModel;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

/**
 *
 * @author jeffrey
 */
public class ColorChooser extends javax.swing.JFrame {

    /**
     * Creates new form ColorChooser
     */
    
    public static class ColorTransfer extends Color implements Transferable{
        public ColorTransfer(Color c){
            super(c.getRed(), c.getGreen(), c.getBlue());
        }
        
        @Override
        public DataFlavor[] getTransferDataFlavors() {
            try {
                return new DataFlavor[]{(DataFlavor)FLAVOR.clone()};
            } catch (CloneNotSupportedException ex) {
                Logger.getLogger(ColorChooser.class.getName()).log(Level.SEVERE, null, ex);
            }
            return new DataFlavor[]{};
        }

        @Override
        public boolean isDataFlavorSupported(DataFlavor flavor) {
            return true;
        }

        @Override
        public Object getTransferData(DataFlavor flavor) throws UnsupportedFlavorException, IOException {
            return this;
        }
        
        public static DataFlavor FLAVOR;
        static{
            try {
                FLAVOR = new DataFlavor(DataFlavor.javaJVMLocalObjectMimeType + ";class=" + ColorChooser.ColorTransfer.class.getName());
            } catch (ClassNotFoundException ex) {
                Logger.getLogger(ColorChooser.class.getName()).log(Level.SEVERE, null, ex);
            }
        }
    }
    
    static class DragGestureList implements DragGestureListener {
        @Override
        public void dragGestureRecognized(DragGestureEvent event) {
            Cursor cursor = null;
            JPanel pnl = (JPanel)event.getComponent();

            if (event.getDragAction() == DnDConstants.ACTION_COPY) {
                cursor = DragSource.DefaultCopyDrop;
            }
            event.startDrag(cursor, new ColorTransfer(pnl.getBackground()));
        }
    }
    
    static class DropTargetList extends DropTargetAdapter implements DropTargetListener {
        private DropTarget dropTarget;
        private JPanel panel;

        public DropTargetList(JPanel panel) {
            this.panel = panel;
            dropTarget = new DropTarget(panel, DnDConstants.ACTION_COPY, this, true, null);
        }

        public void handleDroppedColor(Color c){
            // to be overriden
        }
        
        @Override
        public void drop(DropTargetDropEvent event) {
            try {
                Transferable tr = event.getTransferable();
                ColorTransfer tran = (ColorTransfer)tr.getTransferData(ColorTransfer.FLAVOR);

                if (event.isDataFlavorSupported(ColorTransfer.FLAVOR)) {
                        event.acceptDrop(DnDConstants.ACTION_COPY);
                        event.dropComplete(true);
                        handleDroppedColor(tran);
                        return;
                }
                event.rejectDrop();
            } catch (Exception e) {
                e.printStackTrace();
                event.rejectDrop();
            }
        }
    }
    
    public ColorChooser() {
        initComponents();
        
        this.setDefaultCloseOperation(DISPOSE_ON_CLOSE);
        
        ColorSelectionModel colorModel = chooser.getSelectionModel();
        colorModel.addChangeListener(new ChangeListener() {
            @Override
            public void stateChanged(ChangeEvent evt) {
                ColorSelectionModel model = (ColorSelectionModel)evt.getSource();
                col.setBackground(model.getSelectedColor());
            }
        });
        
        col.setTransferHandler(new TransferHandler(){
            @Override
            public Transferable createTransferable(JComponent c){
                return new ColorTransfer(col.getBackground());
            }
        });
        
        ds = new DragSource();
        ds.createDefaultDragGestureRecognizer(col, DnDConstants.ACTION_COPY, new DragGestureList());
    }
    
    public void setColor(Color c){
        chooser.getSelectionModel().setSelectedColor(c);
        col.setBackground(c);
    }

    /**
     * This method is called from within the constructor to initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is always
     * regenerated by the Form Editor.
     */
    @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        col = new javax.swing.JPanel();
        chooser = new javax.swing.JColorChooser();

        setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);
        getContentPane().setLayout(new javax.swing.BoxLayout(getContentPane(), javax.swing.BoxLayout.LINE_AXIS));

        col.setMinimumSize(new java.awt.Dimension(100, 100));

        javax.swing.GroupLayout colLayout = new javax.swing.GroupLayout(col);
        col.setLayout(colLayout);
        colLayout.setHorizontalGroup(
            colLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGap(0, 100, Short.MAX_VALUE)
        );
        colLayout.setVerticalGroup(
            colLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGap(0, 629, Short.MAX_VALUE)
        );

        getContentPane().add(col);
        getContentPane().add(chooser);

        pack();
    }// </editor-fold>//GEN-END:initComponents

    /**
     * @param args the command line arguments
     */
    public static void main(String args[]) {
        /* Set the Nimbus look and feel */
        //<editor-fold defaultstate="collapsed" desc=" Look and feel setting code (optional) ">
        /* If Nimbus (introduced in Java SE 6) is not available, stay with the default look and feel.
         * For details see http://download.oracle.com/javase/tutorial/uiswing/lookandfeel/plaf.html 
         */
        try {
            for (javax.swing.UIManager.LookAndFeelInfo info : javax.swing.UIManager.getInstalledLookAndFeels()) {
                if ("Nimbus".equals(info.getName())) {
                    javax.swing.UIManager.setLookAndFeel(info.getClassName());
                    break;
                }
            }
        } catch (ClassNotFoundException ex) {
            java.util.logging.Logger.getLogger(ColorChooser.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (InstantiationException ex) {
            java.util.logging.Logger.getLogger(ColorChooser.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (IllegalAccessException ex) {
            java.util.logging.Logger.getLogger(ColorChooser.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (javax.swing.UnsupportedLookAndFeelException ex) {
            java.util.logging.Logger.getLogger(ColorChooser.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        }
        //</editor-fold>

        /* Create and display the form */
        java.awt.EventQueue.invokeLater(new Runnable() {
            public void run() {
                new ColorChooser().setVisible(true);
            }
        });
    }

    private DragSource ds;
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JColorChooser chooser;
    private javax.swing.JPanel col;
    // End of variables declaration//GEN-END:variables
}
