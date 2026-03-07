# -*- coding: utf-8 -*-
# FusionCAD Sketch Helpers
# Provides Fusion 360-style automatic face boundary and center projection for sketches

"""
FusionCAD Sketch Helpers Module

This module provides automatic projection of face boundaries and center points
when creating sketches on faces, similar to Fusion 360 behavior.

Features:
- Project all edges of the attachment face into the sketch
- Project the face center (centroid) as a construction point
- Automatic boundary snapping support
"""

import FreeCAD as App
import FreeCADGui as Gui
import Part
from PySide import QtWidgets

def get_face_center(face):
    """
    Calculate the center (centroid) of a face.
    
    Args:
        face: TopoDS_Face or Part.Face object
        
    Returns:
        FreeCAD.Vector: The center point of the face
    """
    try:
        # Get center of mass (centroid) of the face
        props = face.GProp_CenterMass()
        return App.Vector(props)
    except Exception:
        # Fallback: use bounding box center
        bbox = face.BoundBox
        return App.Vector(
            (bbox.XMin + bbox.XMax) / 2,
            (bbox.YMin + bbox.YMax) / 2,
            (bbox.ZMin + bbox.ZMax) / 2
        )


def get_attachment_face(sketch):
    """
    Get the face that a sketch is attached to.
    
    Args:
        sketch: Sketcher.SketchObject
        
    Returns:
        Part.Face or None if not attached to a face
    """
    support = sketch.AttachmentSupport
    if not support:
        return None
    
    obj = support[0][0]  # First object in support
    sub_elements = support[0][1]  # Sub-element names
    
    if not sub_elements:
        return None
    
    sub_name = sub_elements[0] if isinstance(sub_elements, (list, tuple)) else sub_elements
    
    if not sub_name.startswith("Face"):
        return None
    
    try:
        shape = obj.Shape.getElement(sub_name)
        if shape.ShapeType == "Face":
            return shape
    except Exception:
        pass
    
    return None


def project_face_boundary(sketch, project_center=True):
    """
    Project the boundary edges and optionally center of the attachment face
    into the sketch as external geometry.
    
    Args:
        sketch: Sketcher.SketchObject - The sketch to add projections to
        project_center: bool - Whether to add a construction point at face center
        
    Returns:
        bool: True if successful, False otherwise
    """
    face = get_attachment_face(sketch)
    if face is None:
        App.Console.PrintMessage("FusionCAD: Sketch is not attached to a face.\n")
        return False
    
    support = sketch.AttachmentSupport
    obj = support[0][0]
    sub_elements = support[0][1]
    sub_name = sub_elements[0] if isinstance(sub_elements, (list, tuple)) else sub_elements
    
    # Get face index from name (e.g., "Face1" -> 1)
    face_index = int(sub_name.replace("Face", ""))
    
    # Project all edges of the face
    projected_count = 0
    for i, edge in enumerate(face.Edges):
        edge_name = f"Edge{obj.Shape.Edges.index(edge) + 1}"
        try:
            sketch.addExternal(obj.Name, edge_name)
            projected_count += 1
        except Exception as e:
            # Edge might already be projected or fail for other reasons
            App.Console.PrintLog(f"FusionCAD: Could not project edge {edge_name}: {e}\n")
    
    # Add construction point at face center if requested
    if project_center:
        try:
            center = get_face_center(face)
            
            # Transform center to sketch local coordinates
            sketch_placement = sketch.Placement
            inv_placement = sketch_placement.inverse()
            local_center = inv_placement.multVec(center)
            
            # Add a construction point at the center
            point_id = sketch.addGeometry(
                Part.Point(App.Vector(local_center.x, local_center.y, 0)),
                True  # Construction mode
            )
            
            # Constrain point to the center location using Block constraint
            sketch.addConstraint(Part.Sketcher.Constraint('Block', point_id))
            
            App.Console.PrintMessage(
                f"FusionCAD: Added face center point at ({local_center.x:.3f}, {local_center.y:.3f})\n"
            )
        except Exception as e:
            App.Console.PrintWarning(f"FusionCAD: Could not add face center point: {e}\n")
    
    if projected_count > 0:
        App.Console.PrintMessage(f"FusionCAD: Projected {projected_count} edges from face boundary.\n")
        return True
    
    return False


class CmdFusionProjectFaceBoundary:
    """Command to project face boundary and center into active sketch."""
    
    def GetResources(self):
        return {
            'Pixmap': 'Sketcher_Projection',
            'MenuText': 'Project Face Boundary',
            'ToolTip': 'Project the boundary edges and center of the attachment face into the sketch (Fusion 360 style)',
            'Accel': 'G, B'
        }
    
    def Activated(self):
        # Get active document and sketch
        doc = App.ActiveDocument
        if not doc:
            return
        
        # Check if we're editing a sketch
        if not hasattr(Gui, 'ActiveDocument') or not Gui.ActiveDocument:
            return
        
        # Get the object being edited
        edit_obj = Gui.ActiveDocument.getInEdit()
        if not edit_obj:
            QtWidgets.QMessageBox.warning(
                None,
                "FusionCAD",
                "Please enter sketch edit mode first.\n\nDouble-click on a sketch to edit it."
            )
            return
        
        obj = edit_obj.Object
        if not obj.isDerivedFrom("Sketcher::SketchObject"):
            QtWidgets.QMessageBox.warning(
                None,
                "FusionCAD",
                "Please enter sketch edit mode first."
            )
            return
        
        # Open undo transaction
        doc.openTransaction("Project Face Boundary")
        
        try:
            success = project_face_boundary(obj, project_center=True)
            if success:
                doc.recompute()
                doc.commitTransaction()
            else:
                doc.abortTransaction()
                QtWidgets.QMessageBox.information(
                    None,
                    "FusionCAD",
                    "This sketch is not attached to a face.\n\n"
                    "To use this feature, create a sketch on a face of an existing solid."
                )
        except Exception as e:
            doc.abortTransaction()
            App.Console.PrintError(f"FusionCAD: Error projecting face boundary: {e}\n")
    
    def IsActive(self):
        # Active when editing a sketch
        if not hasattr(Gui, 'ActiveDocument') or not Gui.ActiveDocument:
            return False
        edit_obj = Gui.ActiveDocument.getInEdit()
        if not edit_obj:
            return False
        return edit_obj.Object.isDerivedFrom("Sketcher::SketchObject")


def register_commands():
    """Register FusionCAD Sketch Helper commands."""
    try:
        Gui.addCommand('FusionCAD_ProjectFaceBoundary', CmdFusionProjectFaceBoundary())
        App.Console.PrintMessage("FusionCAD: Sketch helper commands registered.\n")
    except Exception as e:
        App.Console.PrintError(f"FusionCAD: Could not register commands: {e}\n")


# Auto-register when module is imported
if App.GuiUp:
    register_commands()
