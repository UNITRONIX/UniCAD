/***************************************************************************
 *   Copyright (c) 2026 UniCAD Project                                     *
 *                                                                         *
 *   This file is part of UniCAD.                                          *
 *                                                                         *
 *   UniCAD is free software: you can redistribute it and/or modify it     *
 *   under the terms of the GNU Lesser General Public License as           *
 *   published by the Free Software Foundation, either version 2.1 of the  *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   UniCAD is distributed in the hope that it will be useful, but         *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU      *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with UniCAD. If not, see                                *
 *   <https://www.gnu.org/licenses/>.                                      *
 ***************************************************************************/

#ifndef PART_FEATURE_CLEARANCE_VOLUME_H
#define PART_FEATURE_CLEARANCE_VOLUME_H

#include <App/PropertyLinks.h>
#include <App/PropertyStandard.h>
#include <App/PropertyUnits.h>

#include "PartFeature.h"

namespace Part
{

/**
 * @brief ClearanceVolume - Virtual shape representing required clearance space
 * 
 * This feature creates a "ghost" volume around selected faces with configurable
 * offset (tolerance margin for 3D printing) and extrusion depth. The volume
 * can be used to automatically cut holes in enclosure walls.
 * 
 * Workflow:
 * 1. Select face(s) on a component (e.g., USB-C port)
 * 2. Set Offset (typical: 0.1-0.4mm for FDM printing)
 * 3. Set Depth (how far the clearance extends)
 * 4. The clearance volume becomes a "keep-out zone"
 * 5. When creating enclosure walls, these volumes auto-subtract
 * 
 * Properties:
 * - SourceFaces: Link to face(s) defining the clearance shape
 * - Offset: Tolerance margin around the face outline (mm)
 * - Depth: Extrusion depth of the clearance volume (mm)
 * - Direction: Extrusion direction (Normal, Custom, or Bidirectional)
 * - AutoSubtract: If true, automatically subtracts from intersecting solids
 * 
 * Example use case:
 * Creating enclosure for PCB with USB-C port:
 * - Select USB-C connector face
 * - Set Offset = 0.2mm (FDM tolerance)
 * - Set Depth = 5mm (wall thickness + margin)
 * - Create enclosure wall → hole is automatically cut
 */
class PartExport ClearanceVolume : public Part::Feature
{
    PROPERTY_HEADER_WITH_OVERRIDE(Part::ClearanceVolume);

public:
    ClearanceVolume();
    ~ClearanceVolume() override = default;

    // -------------------------------------------------------------------------
    // Properties
    // -------------------------------------------------------------------------
    
    /// Source face(s) that define the clearance shape outline
    App::PropertyLinkSub SourceFaces;
    
    /// Offset/tolerance margin around the face outline (mm)
    /// Typical values: 0.1-0.4mm for FDM, 0.05-0.1mm for SLA
    App::PropertyLength Offset;
    
    /// Offset for the inner part (into port) - if 0, uses main Offset
    App::PropertyLength OffsetReverse;
    
    /// Extrusion depth of the clearance volume (mm)
    App::PropertyLength Depth;
    
    /// Reverse extrusion depth - extends into the port/connector (mm)
    /// Use this to protect the entire connector depth from wall collisions
    App::PropertyLength DepthReverse;
    
    /// Whether to extrude in both directions from the face
    App::PropertyBool Symmetric;
    
    /// Custom extrusion direction (if not using face normal)
    App::PropertyVector Direction;
    
    /// Use face normal as direction (if true, Direction is ignored)
    App::PropertyBool UseNormal;
    
    /// Flip/reverse the extrusion direction (useful when normal points wrong way)
    App::PropertyBool FlipDirection;
    
    /// If true, this volume will auto-subtract from solids during operations
    App::PropertyBool AutoSubtract;
    
    /// Target body to automatically subtract clearance from
    App::PropertyLink TargetBody;
    
    /// Name/label for identification (e.g., "USB-C Port", "HDMI")
    App::PropertyString PortName;
    
    /// Color for visualization (RGBA)
    App::PropertyColor DisplayColor;

    // -------------------------------------------------------------------------
    // Feature interface
    // -------------------------------------------------------------------------
    
    /// Recompute the clearance shape
    App::DocumentObjectExecReturn* execute() override;
    
    /// Short description
    const char* getViewProviderName() const override;
    
    /// Check if clearance volume intersects with given shape
    bool intersectsWith(const TopoDS_Shape& shape) const;
    
    /// Get the clearance shape for boolean operations
    TopoDS_Shape getClearanceShape() const;
    
    /// Get all clearance volumes in the document
    static std::vector<ClearanceVolume*> getAllInDocument(App::Document* doc);

protected:
    /// Build the clearance shape from source faces
    TopoDS_Shape buildClearanceShape();
    
    /// Offset a face outline by the tolerance margin
    TopoDS_Shape offsetFaceOutline(const TopoDS_Face& face, double offset);
    
    /// Extrude the offset outline to create the volume
    TopoDS_Shape extrudeOutline(const TopoDS_Shape& outline, 
                                 const gp_Vec& direction, 
                                 double depth);
    
    /// Get the normal direction for a face
    gp_Vec getFaceNormal(const TopoDS_Face& face) const;
};

} // namespace Part

#endif // PART_FEATURE_CLEARANCE_VOLUME_H
