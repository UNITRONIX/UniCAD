# -*- coding: utf-8 -*-
"""
Quick Port Cutout - Simplified tool to cut holes in enclosure walls for ports

Usage:
1. Select a face on the port/connector you want to create clearance for
2. Run this tool
3. System automatically detects enclosure wall in front of the port
4. Adjust offset/tolerance if needed
5. Click 'Cut Hole' - Done!

This is a simplified alternative to ClearanceVolume that doesn't require
manual target body selection - it uses ray-casting to auto-detect walls.
"""

import FreeCAD
import FreeCADGui
import Part
from PySide import QtCore, QtGui

# Preset tolerances for different port types
PRESETS = {
    "Tight fit (0.1mm)": 0.1,
    "Standard (0.2mm)": 0.2,
    "Loose fit (0.5mm)": 0.5,
    "3D Print (0.3mm)": 0.3,
    "CNC (0.15mm)": 0.15,
}


def find_target_wall(source_face, source_obj, max_distance=200.0):
    """
    Use ray-casting to find the nearest wall in front of the source face.
    Returns (target_obj, target_face, hit_point, distance) or None.
    """
    # Get face center and normal
    center = source_face.CenterOfGravity
    
    try:
        uv = source_face.Surface.parameter(center)
        normal = source_face.normalAt(uv[0], uv[1])
    except:
        # Fallback - try to get normal from first vertex
        normal = FreeCAD.Vector(0, 0, 1)
    
    # Create ray from center in normal direction
    ray_start = center
    ray_end = center + normal * max_distance
    ray_line = Part.makeLine(ray_start, ray_end)
    
    best_hit = None
    best_distance = float('inf')
    best_obj = None
    best_face = None
    
    # Check all objects in document
    for obj in FreeCAD.ActiveDocument.Objects:
        # Skip source object
        if obj == source_obj:
            continue
        # Skip objects without shape
        if not hasattr(obj, 'Shape') or obj.Shape.isNull():
            continue
        # Skip ClearanceVolume/preview objects
        if 'ClearanceVolume' in str(getattr(obj, 'TypeId', '')) or 'Preview' in obj.Label:
            continue
        if 'Clearance' in obj.Label or 'Cutout' in obj.Label:
            continue
            
        try:
            shape = obj.Shape
            
            # Check each face of the object
            for i, face in enumerate(shape.Faces):
                # Check if ray intersects this face
                try:
                    # Use distToShape for intersection
                    dist_info = ray_line.distToShape(face)
                    if dist_info[0] < 0.1:  # Very close = intersection
                        hit_point = dist_info[1][0][0]
                        distance = (hit_point - center).Length
                        
                        # Must be in front (positive direction along normal)
                        direction = hit_point - center
                        if direction.dot(normal) > 0.1:  # In front
                            if distance < best_distance and distance > 0.5:  # Not too close
                                best_distance = distance
                                best_hit = hit_point
                                best_obj = obj
                                best_face = face
                except:
                    pass
                    
            # Alternative: use BRepExtrema for more reliable intersection
            try:
                from OCC.Core.BRepExtrema import BRepExtrema_DistShapeShape
            except:
                pass
                
        except Exception as e:
            continue
    
    if best_obj:
        return (best_obj, best_face, best_hit, best_distance)
    return None


class QuickPortCutoutDialog(QtGui.QDialog):
    """Simple dialog for Quick Port Cutout with auto-detection"""
    
    def __init__(self, source_face, source_obj, auto_target=None, parent=None):
        super(QuickPortCutoutDialog, self).__init__(parent)
        self.source_face = source_face
        self.source_obj = source_obj
        self.target_obj = auto_target[0] if auto_target else None
        self.target_face = auto_target[1] if auto_target else None
        self.hit_distance = auto_target[3] if auto_target else None
        self.preview_obj = None
        self.cutting_shape = None
        self.normal = self.get_face_normal()
        self.setup_ui()
        self.update_preview()
        
    def get_face_normal(self):
        """Get normal vector of source face"""
        center = self.source_face.CenterOfGravity
        try:
            uv = self.source_face.Surface.parameter(center)
            return self.source_face.normalAt(uv[0], uv[1])
        except:
            return FreeCAD.Vector(0, 0, 1)
        
    def setup_ui(self):
        self.setWindowTitle("Quick Port Cutout")
        self.setMinimumWidth(400)
        self.setWindowFlags(self.windowFlags() | QtCore.Qt.WindowStaysOnTopHint)
        
        layout = QtGui.QVBoxLayout(self)
        
        # Source info
        info_group = QtGui.QGroupBox("Source Port")
        info_layout = QtGui.QFormLayout(info_group)
        
        source_label = QtGui.QLabel(f"{self.source_obj.Label}")
        source_label.setStyleSheet("color: #4CAF50; font-weight: bold;")
        info_layout.addRow("Object:", source_label)
        
        # Calculate face size
        bbox = self.source_face.BoundBox
        size_label = QtGui.QLabel(f"{bbox.XLength:.1f} x {bbox.YLength:.1f} x {bbox.ZLength:.1f} mm")
        info_layout.addRow("Port Size:", size_label)
        
        layout.addWidget(info_group)
        
        # Auto-detected target
        target_group = QtGui.QGroupBox("Auto-Detected Wall")
        target_layout = QtGui.QFormLayout(target_group)
        
        if self.target_obj:
            target_label = QtGui.QLabel(f"{self.target_obj.Label}")
            target_label.setStyleSheet("color: #4CAF50; font-weight: bold;")
            target_layout.addRow("Target:", target_label)
            
            dist_label = QtGui.QLabel(f"{self.hit_distance:.1f} mm")
            target_layout.addRow("Distance:", dist_label)
            
            # Status
            status_label = QtGui.QLabel("✓ Wall detected automatically!")
            status_label.setStyleSheet("color: #4CAF50;")
            target_layout.addRow("", status_label)
        else:
            no_target = QtGui.QLabel("No wall found in front of port")
            no_target.setStyleSheet("color: #FF5722;")
            target_layout.addRow("Status:", no_target)
            
            # Manual selection fallback
            self.target_combo = QtGui.QComboBox()
            self.populate_targets()
            self.target_combo.currentTextChanged.connect(self.on_target_combo_changed)
            target_layout.addRow("Manual:", self.target_combo)
        
        layout.addWidget(target_group)
        
        # Tolerance settings
        tol_group = QtGui.QGroupBox("Clearance Settings")
        tol_layout = QtGui.QFormLayout(tol_group)
        
        self.preset_combo = QtGui.QComboBox()
        self.preset_combo.addItems(list(PRESETS.keys()))
        self.preset_combo.setCurrentText("Standard (0.2mm)")
        self.preset_combo.currentTextChanged.connect(self.on_preset_changed)
        tol_layout.addRow("Preset:", self.preset_combo)
        
        self.offset_spin = QtGui.QDoubleSpinBox()
        self.offset_spin.setRange(0.0, 10.0)
        self.offset_spin.setDecimals(2)
        self.offset_spin.setSuffix(" mm")
        self.offset_spin.setValue(0.2)
        self.offset_spin.valueChanged.connect(self.update_preview)
        tol_layout.addRow("Offset (all sides):", self.offset_spin)
        
        self.depth_spin = QtGui.QDoubleSpinBox()
        self.depth_spin.setRange(0.1, 100.0)
        self.depth_spin.setDecimals(1)
        self.depth_spin.setSuffix(" mm")
        # Auto-set depth based on detected distance
        default_depth = min(self.hit_distance + 2, 20.0) if self.hit_distance else 5.0
        self.depth_spin.setValue(default_depth)
        self.depth_spin.valueChanged.connect(self.update_preview)
        tol_layout.addRow("Cut depth:", self.depth_spin)
        
        self.symmetric_check = QtGui.QCheckBox("Symmetric (both directions)")
        self.symmetric_check.setChecked(True)
        self.symmetric_check.setToolTip("Cut in both directions from the face - ensures the hole goes all the way through the wall")
        self.symmetric_check.toggled.connect(self.update_preview)
        tol_layout.addRow("", self.symmetric_check)
        
        self.flip_check = QtGui.QCheckBox("Flip direction")
        self.flip_check.setToolTip("Reverse the cut direction")
        self.flip_check.toggled.connect(self.update_preview)
        tol_layout.addRow("", self.flip_check)
        
        layout.addWidget(tol_group)
        
        # Buttons
        button_layout = QtGui.QHBoxLayout()
        
        self.cut_btn = QtGui.QPushButton("✓ Cut Hole")
        self.cut_btn.clicked.connect(self.do_cut)
        self.cut_btn.setEnabled(self.target_obj is not None)
        self.cut_btn.setStyleSheet(
            "background-color: #4CAF50; color: white; font-weight: bold; padding: 10px; font-size: 14px;"
            if self.target_obj else
            "background-color: #9E9E9E; color: white; padding: 10px;"
        )
        
        cancel_btn = QtGui.QPushButton("Cancel")
        cancel_btn.clicked.connect(self.reject)
        
        button_layout.addWidget(self.cut_btn)
        button_layout.addWidget(cancel_btn)
        layout.addLayout(button_layout)
        
    def populate_targets(self):
        """Fill combo with potential target objects (fallback)"""
        self.target_combo.clear()
        self.target_combo.addItem("(Select target object)")
        
        for obj in FreeCAD.ActiveDocument.Objects:
            if obj == self.source_obj:
                continue
            if not hasattr(obj, 'Shape') or obj.Shape.isNull():
                continue
            if 'ClearanceVolume' in str(getattr(obj, 'TypeId', '')) or 'Clearance' in obj.Label:
                continue
            self.target_combo.addItem(obj.Label, obj.Name)
            
    def on_target_combo_changed(self, text):
        """Handle manual target selection"""
        if text == "(Select target object)":
            self.target_obj = None
            self.cut_btn.setEnabled(False)
            return
            
        idx = self.target_combo.currentIndex()
        obj_name = self.target_combo.itemData(idx)
        if obj_name:
            self.target_obj = FreeCAD.ActiveDocument.getObject(obj_name)
            self.cut_btn.setEnabled(True)
            self.cut_btn.setStyleSheet(
                "background-color: #4CAF50; color: white; font-weight: bold; padding: 10px; font-size: 14px;"
            )
            self.update_preview()
            
    def on_preset_changed(self, preset_name):
        """Update offset from preset"""
        if preset_name in PRESETS:
            self.offset_spin.setValue(PRESETS[preset_name])
        
    def _make_cutting_rect(self, normal, offset):
        """Build a flat rectangle that envelops the source face projection + offset."""
        face = self.source_face
        center = face.CenterOfGravity

        # Build a local 2-axis frame on the face plane
        if abs(normal.z) < 0.9:
            ref = FreeCAD.Vector(0, 0, 1)
        else:
            ref = FreeCAD.Vector(1, 0, 0)
        u_axis = normal.cross(ref).normalize()
        v_axis = normal.cross(u_axis).normalize()

        # Project every vertex of the face onto u / v axes
        u_vals = []
        v_vals = []
        for vertex in face.Vertexes:
            d = vertex.Point - center
            u_vals.append(d.dot(u_axis))
            v_vals.append(d.dot(v_axis))

        u_min = min(u_vals) - offset
        u_max = max(u_vals) + offset
        v_min = min(v_vals) - offset
        v_max = max(v_vals) + offset

        # Four corners of the rectangle
        c1 = center + u_axis * u_min + v_axis * v_min
        c2 = center + u_axis * u_max + v_axis * v_min
        c3 = center + u_axis * u_max + v_axis * v_max
        c4 = center + u_axis * u_min + v_axis * v_max

        wire = Part.makePolygon([c1, c2, c3, c4, c1])
        return Part.Face(wire)

    def update_preview(self):
        """Update the preview cutting shape – clean rectangle extruded along normal."""
        try:
            self.remove_preview()

            normal = FreeCAD.Vector(self.normal)
            offset = self.offset_spin.value()
            depth = self.depth_spin.value()
            symmetric = self.symmetric_check.isChecked()
            flip = self.flip_check.isChecked()

            if flip:
                normal = FreeCAD.Vector(-normal.x, -normal.y, -normal.z)

            # Simple rectangle encompassing the port face
            cutting_face = self._make_cutting_rect(normal, offset)

            # Extrude along normal
            if symmetric:
                cutting_face.translate(normal * (-depth))
                self.cutting_shape = cutting_face.extrude(normal * (depth * 2))
            else:
                self.cutting_shape = cutting_face.extrude(normal * depth)

            # Show transparent orange preview
            self.preview_obj = FreeCAD.ActiveDocument.addObject(
                "Part::Feature", "CutoutPreview")
            self.preview_obj.Shape = self.cutting_shape
            self.preview_obj.ViewObject.ShapeColor = (1.0, 0.5, 0.0)
            self.preview_obj.ViewObject.Transparency = 60
            self.preview_obj.ViewObject.DisplayMode = "Shaded"

        except Exception as e:
            FreeCAD.Console.PrintError(f"Preview error: {e}\n")
            
    def remove_preview(self):
        """Remove preview object"""
        if self.preview_obj:
            try:
                FreeCAD.ActiveDocument.removeObject(self.preview_obj.Name)
            except:
                pass
            self.preview_obj = None
            
    @staticmethod
    def _find_body(obj):
        """Return the PartDesign::Body that owns *obj*, or None."""
        for parent in obj.InList:
            if getattr(parent, 'TypeId', '') == 'PartDesign::Body':
                return parent
        return None

    def do_cut(self):
        """Perform a parametric Boolean cut on the target wall.

        * If the target lives inside a PartDesign::Body the cut is done with
          PartDesign::Boolean so the result stays inside the Body and can be
          further edited with PartDesign tools (Pad, Pocket, …).
        * Otherwise falls back to Part::Cut (works for pure-Part workflow).
        """
        if not self.target_obj or not self.cutting_shape:
            QtGui.QMessageBox.warning(self, "Error",
                                      "Please select a target object first.")
            return

        try:
            FreeCAD.ActiveDocument.openTransaction("Quick Port Cutout")
            doc = FreeCAD.ActiveDocument

            # --- cutting-tool object (always needed) ---------------------
            tool_obj = doc.addObject("Part::Feature", "PortCutoutTool")
            tool_obj.Shape = self.cutting_shape
            tool_obj.Label = f"{self.source_obj.Label}_CutTool"
            tool_obj.ViewObject.Visibility = False

            target_body = self._find_body(self.target_obj)

            if target_body:
                # ---- PartDesign path: PartDesign::Boolean ---------------
                # Wrap the tool in its own Body so Boolean accepts it
                tool_body = doc.addObject("PartDesign::Body", "PortCutoutToolBody")
                tool_body.BaseFeature = tool_obj
                tool_body.ViewObject.Visibility = False

                bool_feat = doc.addObject("PartDesign::Boolean", "PortCutout")
                target_body.addObject(bool_feat)
                bool_feat.Type = 1          # 0=Fuse, 1=Cut, 2=Common
                bool_feat.setObjects([tool_body])
                bool_feat.Label = f"{target_body.Label}_PortCut"

                # Copy visual properties
                try:
                    bool_feat.ViewObject.ShapeAppearance = (
                        self.target_obj.ViewObject.ShapeAppearance)
                except Exception:
                    pass

                self.remove_preview()
                doc.commitTransaction()
                doc.recompute()

                FreeCADGui.Selection.clearSelection()
                FreeCADGui.Selection.addSelection(bool_feat)

                QtGui.QMessageBox.information(
                    self, "Success",
                    f"Port cutout created inside Body!\n\n"
                    f"PartDesign Boolean: '{bool_feat.Label}'\n"
                    f"Body: '{target_body.Label}'\n\n"
                    f"You can Pad/Pocket faces of the result normally.\n"
                    f"To undo: Edit → Undo (Ctrl+Z)")
            else:
                # ---- Part path: Part::Cut -------------------------------
                cut_obj = doc.addObject("Part::Cut", "PortCutout")
                cut_obj.Base = self.target_obj
                cut_obj.Tool = tool_obj
                cut_obj.Label = f"{self.target_obj.Label}_PortCut"

                try:
                    cut_obj.ViewObject.ShapeAppearance = (
                        self.target_obj.ViewObject.ShapeAppearance)
                except Exception:
                    pass

                self.target_obj.ViewObject.Visibility = False

                self.remove_preview()
                doc.commitTransaction()
                doc.recompute()

                FreeCADGui.Selection.clearSelection()
                FreeCADGui.Selection.addSelection(cut_obj)

                QtGui.QMessageBox.information(
                    self, "Success",
                    f"Port cutout created!\n\n"
                    f"Boolean Cut: '{cut_obj.Label}'\n\n"
                    f"To undo: Edit → Undo (Ctrl+Z)")

            self.accept()

        except Exception as e:
            FreeCAD.ActiveDocument.abortTransaction()
            self.remove_preview()
            QtGui.QMessageBox.critical(self, "Error",
                                       f"Cut failed:\n{str(e)}")
            
    def reject(self):
        """Cancel and cleanup"""
        self.remove_preview()
        super().reject()
        
    def closeEvent(self, event):
        """Cleanup on close"""
        self.remove_preview()
        super().closeEvent(event)


def quick_port_cutout():
    """Main function for Quick Port Cutout with auto-detection"""
    
    # Check selection
    sel = FreeCADGui.Selection.getSelectionEx()
    if not sel:
        QtGui.QMessageBox.warning(None, "Quick Port Cutout",
            "Please select a face on the port/connector first.\n\n"
            "1. Select a face on the port you want clearance for\n"
            "2. Run this command\n"
            "3. Wall is auto-detected, just click 'Cut Hole'")
        return
    
    # Get selected face
    sub_objects = sel[0].SubObjects
    if not sub_objects:
        QtGui.QMessageBox.warning(None, "Quick Port Cutout",
            "Please select a face, not the whole object.")
        return
    
    face = None
    for sub in sub_objects:
        if isinstance(sub, Part.Face):
            face = sub
            break
    
    if not face:
        QtGui.QMessageBox.warning(None, "Quick Port Cutout",
            "Please select a face on the port.")
        return
    
    source_obj = sel[0].Object
    
    # Auto-detect wall in front of port
    FreeCAD.Console.PrintMessage("Quick Port Cutout: Detecting wall...\n")
    auto_target = find_target_wall(face, source_obj)
    
    if auto_target:
        FreeCAD.Console.PrintMessage(f"Quick Port Cutout: Found wall '{auto_target[0].Label}' at {auto_target[3]:.1f}mm\n")
    else:
        FreeCAD.Console.PrintWarning("Quick Port Cutout: No wall detected, manual selection required\n")
    
    # Show dialog with auto-detected target
    dialog = QuickPortCutoutDialog(face, source_obj, auto_target, FreeCADGui.getMainWindow())
    dialog.exec_()


if __name__ == "__main__":
    quick_port_cutout()
