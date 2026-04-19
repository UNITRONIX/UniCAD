# -*- coding: utf-8 -*-
"""
Quick Hole Tool - Creates a hole at the center of a selected face

Usage:
1. Select a planar face on a body
2. Run this script
3. Adjust hole parameters in the dialog
4. Click OK to create the hole

This is a simplified hole creation tool that doesn't require a sketch.
"""

import FreeCAD
import FreeCADGui
import Part
import PartDesign
from PySide import QtCore, QtGui

# Standard metric screw hole sizes (clearance holes - medium fit)
# Format: "Name": (through_hole_diameter, counterbore_diameter, counterbore_depth, head_type)
SCREW_SIZES = {
    "M2": (2.4, 4.4, 2.0, "Socket Head"),
    "M2.5": (2.9, 5.4, 2.5, "Socket Head"),
    "M3": (3.4, 6.5, 3.0, "Socket Head"),
    "M4": (4.5, 8.0, 4.0, "Socket Head"),
    "M5": (5.5, 9.5, 5.0, "Socket Head"),
    "M6": (6.6, 11.0, 6.0, "Socket Head"),
    "M8": (9.0, 14.5, 8.0, "Socket Head"),
    "M10": (11.0, 18.0, 10.0, "Socket Head"),
    "M12": (13.5, 20.0, 12.0, "Socket Head"),
    # Self-tapping screws (smaller holes)
    "M2 (tap)": (1.6, 4.4, 2.0, "Self-tap"),
    "M2.5 (tap)": (2.05, 5.4, 2.5, "Self-tap"),
    "M3 (tap)": (2.5, 6.5, 3.0, "Self-tap"),
    "M4 (tap)": (3.3, 8.0, 4.0, "Self-tap"),
    "M5 (tap)": (4.2, 9.5, 5.0, "Self-tap"),
    # Countersunk screws
    "M3 CSK": (3.4, 6.3, 1.7, "Countersunk"),
    "M4 CSK": (4.5, 8.4, 2.3, "Countersunk"),
    "M5 CSK": (5.5, 10.4, 2.8, "Countersunk"),
    "M6 CSK": (6.6, 12.6, 3.3, "Countersunk"),
    # Custom
    "Custom": (3.0, 6.0, 3.0, "Custom"),
}


class QuickHoleDialog(QtGui.QDialog):
    """Dialog for Quick Hole parameters"""
    
    def __init__(self, face_center, face_normal, parent=None):
        super(QuickHoleDialog, self).__init__(parent)
        self.face_center = face_center
        self.face_normal = face_normal
        self.result_params = None
        self.setup_ui()
        
    def setup_ui(self):
        self.setWindowTitle("Quick Hole")
        self.setMinimumWidth(300)
        
        layout = QtGui.QVBoxLayout(self)
        
        # Screw size selection
        size_group = QtGui.QGroupBox("Screw Size")
        size_layout = QtGui.QFormLayout(size_group)
        
        self.size_combo = QtGui.QComboBox()
        self.size_combo.addItems(list(SCREW_SIZES.keys()))
        self.size_combo.setCurrentText("M3")
        self.size_combo.currentTextChanged.connect(self.on_size_changed)
        size_layout.addRow("Size:", self.size_combo)
        
        layout.addWidget(size_group)
        
        # Hole parameters
        params_group = QtGui.QGroupBox("Hole Parameters")
        params_layout = QtGui.QFormLayout(params_group)
        
        self.diameter_spin = QtGui.QDoubleSpinBox()
        self.diameter_spin.setRange(0.1, 100.0)
        self.diameter_spin.setDecimals(2)
        self.diameter_spin.setSuffix(" mm")
        self.diameter_spin.setValue(3.4)
        params_layout.addRow("Diameter:", self.diameter_spin)
        
        self.depth_spin = QtGui.QDoubleSpinBox()
        self.depth_spin.setRange(0.1, 500.0)
        self.depth_spin.setDecimals(2)
        self.depth_spin.setSuffix(" mm")
        self.depth_spin.setValue(10.0)
        params_layout.addRow("Depth:", self.depth_spin)
        
        self.through_check = QtGui.QCheckBox("Through All")
        self.through_check.setChecked(True)
        self.through_check.toggled.connect(self.on_through_changed)
        params_layout.addRow("", self.through_check)
        
        layout.addWidget(params_group)
        
        # Counterbore options
        cb_group = QtGui.QGroupBox("Counterbore / Countersink")
        cb_layout = QtGui.QFormLayout(cb_group)
        
        self.cb_check = QtGui.QCheckBox("Add counterbore")
        self.cb_check.toggled.connect(self.on_cb_changed)
        cb_layout.addRow("", self.cb_check)
        
        self.cb_diameter_spin = QtGui.QDoubleSpinBox()
        self.cb_diameter_spin.setRange(0.1, 100.0)
        self.cb_diameter_spin.setDecimals(2)
        self.cb_diameter_spin.setSuffix(" mm")
        self.cb_diameter_spin.setValue(6.5)
        self.cb_diameter_spin.setEnabled(False)
        cb_layout.addRow("CB Diameter:", self.cb_diameter_spin)
        
        self.cb_depth_spin = QtGui.QDoubleSpinBox()
        self.cb_depth_spin.setRange(0.1, 100.0)
        self.cb_depth_spin.setDecimals(2)
        self.cb_depth_spin.setSuffix(" mm")
        self.cb_depth_spin.setValue(3.0)
        self.cb_depth_spin.setEnabled(False)
        cb_layout.addRow("CB Depth:", self.cb_depth_spin)
        
        layout.addWidget(cb_group)
        
        # Position info
        pos_group = QtGui.QGroupBox("Position (Face Center)")
        pos_layout = QtGui.QFormLayout(pos_group)
        
        pos_label = QtGui.QLabel(f"X: {self.face_center.x:.2f}, Y: {self.face_center.y:.2f}, Z: {self.face_center.z:.2f}")
        pos_layout.addRow("Center:", pos_label)
        
        layout.addWidget(pos_group)
        
        # Buttons
        button_layout = QtGui.QHBoxLayout()
        
        ok_btn = QtGui.QPushButton("Create Hole")
        ok_btn.clicked.connect(self.accept)
        ok_btn.setDefault(True)
        
        cancel_btn = QtGui.QPushButton("Cancel")
        cancel_btn.clicked.connect(self.reject)
        
        button_layout.addWidget(ok_btn)
        button_layout.addWidget(cancel_btn)
        layout.addLayout(button_layout)
        
        # Initialize with M3
        self.on_size_changed("M3")
        
    def on_size_changed(self, size_name):
        """Update parameters when screw size changes"""
        if size_name in SCREW_SIZES:
            diameter, cb_dia, cb_depth, _ = SCREW_SIZES[size_name]
            self.diameter_spin.setValue(diameter)
            self.cb_diameter_spin.setValue(cb_dia)
            self.cb_depth_spin.setValue(cb_depth)
            
            # Enable custom editing only for Custom
            is_custom = (size_name == "Custom")
            self.diameter_spin.setEnabled(is_custom)
            
    def on_through_changed(self, checked):
        """Toggle depth field based on through-all option"""
        self.depth_spin.setEnabled(not checked)
        
    def on_cb_changed(self, checked):
        """Toggle counterbore fields"""
        self.cb_diameter_spin.setEnabled(checked)
        self.cb_depth_spin.setEnabled(checked)
        
    def get_parameters(self):
        """Return hole parameters"""
        return {
            'diameter': self.diameter_spin.value(),
            'depth': self.depth_spin.value() if not self.through_check.isChecked() else None,
            'through_all': self.through_check.isChecked(),
            'counterbore': self.cb_check.isChecked(),
            'cb_diameter': self.cb_diameter_spin.value(),
            'cb_depth': self.cb_depth_spin.value(),
            'center': self.face_center,
            'normal': self.face_normal,
        }


def get_face_center_and_normal(face):
    """Calculate the center point and normal of a face"""
    # Get center of mass of the face
    props = face.GProp()
    center = face.CenterOfGravity
    
    # Get surface normal at center
    # Project center onto face to get UV parameters
    surface = face.Surface
    try:
        uv = surface.parameter(center)
        normal = face.normalAt(uv[0], uv[1])
    except:
        # Fallback: use first point normal
        normal = face.normalAt(0, 0)
    
    return center, normal


def create_hole_feature(doc, body, params):
    """Create the hole using PartDesign Pocket with a circular sketch"""
    import Sketcher
    
    center = params['center']
    normal = params['normal']
    diameter = params['diameter']
    
    # Find the face to attach sketch to
    # We need to create a sketch on the selected face
    
    # Create a datum plane at the face location
    # Or use the face directly as sketch support
    
    # For simplicity, we'll use Part operations instead of PartDesign
    # This works on any Part::Feature, not just PartDesign bodies
    
    # Create cylinder for the hole
    hole_radius = diameter / 2.0
    
    if params['through_all']:
        # Use a large depth for through-all
        hole_depth = 1000.0  # Large value
    else:
        hole_depth = params['depth']
    
    # Direction is opposite to face normal (cut into the material)
    direction = FreeCAD.Vector(-normal.x, -normal.y, -normal.z)
    
    # Create the main hole cylinder
    hole_cylinder = Part.makeCylinder(
        hole_radius,
        hole_depth,
        center,
        direction
    )
    
    # Create counterbore if requested
    if params['counterbore']:
        cb_radius = params['cb_diameter'] / 2.0
        cb_depth = params['cb_depth']
        
        cb_cylinder = Part.makeCylinder(
            cb_radius,
            cb_depth,
            center,
            direction
        )
        
        # Fuse hole and counterbore
        hole_shape = hole_cylinder.fuse(cb_cylinder)
    else:
        hole_shape = hole_cylinder
    
    return hole_shape


def quick_hole():
    """Main function to create a quick hole"""
    
    # Check selection
    sel = FreeCADGui.Selection.getSelectionEx()
    if not sel:
        QtGui.QMessageBox.warning(None, "Quick Hole", 
            "Please select a face first.")
        return
    
    # Get selected face
    sub_objects = sel[0].SubObjects
    if not sub_objects:
        QtGui.QMessageBox.warning(None, "Quick Hole",
            "Please select a face, not the whole object.")
        return
    
    face = None
    for sub in sub_objects:
        if isinstance(sub, Part.Face):
            face = sub
            break
    
    if not face:
        QtGui.QMessageBox.warning(None, "Quick Hole",
            "Please select a face.")
        return
    
    # Check if face is planar
    if face.Surface.TypeId != "Part::GeomPlane":
        QtGui.QMessageBox.warning(None, "Quick Hole",
            "Please select a planar face.\n\n"
            "Curved faces are not supported yet.")
        return
    
    # Get face center and normal
    center, normal = get_face_center_and_normal(face)
    
    # Get the parent object
    parent_obj = sel[0].Object
    
    # Show dialog
    dialog = QuickHoleDialog(center, normal, FreeCADGui.getMainWindow())
    
    if dialog.exec_() == QtGui.QDialog.Accepted:
        params = dialog.get_parameters()
        
        # Create hole shape
        hole_shape = create_hole_feature(FreeCAD.ActiveDocument, None, params)
        
        # Cut from parent object
        try:
            result_shape = parent_obj.Shape.cut(hole_shape)
            
            # Update the object or create new one
            FreeCAD.ActiveDocument.openTransaction("Quick Hole")
            
            # Create a new object with the result
            new_obj = FreeCAD.ActiveDocument.addObject("Part::Feature", "HoledPart")
            new_obj.Shape = result_shape
            new_obj.Label = parent_obj.Label + "_Holed"
            
            # Copy visual properties
            if hasattr(parent_obj, "ViewObject") and hasattr(new_obj, "ViewObject"):
                try:
                    new_obj.ViewObject.ShapeAppearance = parent_obj.ViewObject.ShapeAppearance
                except:
                    pass
            
            # Hide original
            parent_obj.ViewObject.Visibility = False
            
            FreeCAD.ActiveDocument.commitTransaction()
            FreeCAD.ActiveDocument.recompute()
            
            FreeCADGui.Selection.clearSelection()
            FreeCADGui.Selection.addSelection(new_obj)
            
        except Exception as e:
            QtGui.QMessageBox.critical(None, "Quick Hole Error",
                f"Failed to create hole:\n{str(e)}")
            FreeCAD.ActiveDocument.abortTransaction()


# Register as a command
class QuickHoleCommand:
    """FreeCAD command for Quick Hole"""
    
    def GetResources(self):
        return {
            'Pixmap': 'PartDesign_Hole',
            'MenuText': 'Quick Hole',
            'ToolTip': 'Create a hole at the center of the selected face',
            'Accel': 'Shift+H'
        }
    
    def IsActive(self):
        return FreeCAD.ActiveDocument is not None
    
    def Activated(self):
        quick_hole()


# Register command
FreeCADGui.addCommand('QuickHole', QuickHoleCommand())

# Run directly if executed as script
if __name__ == "__main__":
    quick_hole()
