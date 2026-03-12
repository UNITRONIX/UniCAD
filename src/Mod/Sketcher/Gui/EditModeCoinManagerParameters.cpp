// SPDX-License-Identifier: LGPL-2.1-or-later

/***************************************************************************
 *   Copyright (c) 2021 Abdullah Tahiri <abdullah.tahiri.yo@gmail.com>     *
 *                                                                         *
 *   This file is part of the FreeCAD CAx development system.              *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; see the file COPYING.LIB. If not,    *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/

#include <FCConfig.h>

#include "Mod/Sketcher/App/ExternalGeometryFacade.h"

#include <Base/Color.h>
#include <Gui/ViewParams.h>

#include "EditModeCoinManagerParameters.h"


using namespace SketcherGui;

int GeometryLayerParameters::getSubLayerIndex(const int geoId, const Sketcher::GeometryFacade* geom) const
{
    bool isConstruction = geom->getConstruction();
    bool isInternal = geom->isInternalAligned();
    bool isExternal = geoId <= Sketcher::GeoEnum::RefExt;
    if (isExternal) {
        auto egf = Sketcher::ExternalGeometryFacade::getFacade(geom->clone());
        if (egf->testFlag(Sketcher::ExternalGeometryExtension::Defining)) {
            return static_cast<int>(SubLayer::ExternalDefining);
        }
    }

    return static_cast<int>(
        isExternal           ? SubLayer::External
            : isInternal     ? SubLayer::Internal
            : isConstruction ? SubLayer::Construction
                             : SubLayer::Normal
    );
}

SbColor DrawingParameters::InformationColor(0.0f, 1.0f, 0.0f);  // #00FF00 -> (  0,255,  0)

namespace
{  // Anonymous namespace to avoid making those variables global
unsigned long HColorLong = Gui::ViewParams::instance()->getAxisXColor();
Base::Color Hcolor = Base::Color(static_cast<uint32_t>(HColorLong));

unsigned long VColorLong = Gui::ViewParams::instance()->getAxisYColor();
Base::Color Vcolor = Base::Color(static_cast<uint32_t>(VColorLong));
}  // namespace
SbColor DrawingParameters::CrossColorH(Hcolor.r, Hcolor.g, Hcolor.b);
SbColor DrawingParameters::CrossColorV(Vcolor.r, Vcolor.g, Vcolor.b);

// UniCAD Fusion 360-style colors
SbColor DrawingParameters::InvalidSketchColor(1.0f, 0.42f, 0.0f);    // #FF6D00 -> (255,109,  0)
SbColor DrawingParameters::FullyConstrainedColor(0.0f, 0.90f, 0.46f);  // #00E676 -> (  0,230,118) - Fusion green
SbColor DrawingParameters::FullyConstraintInternalAlignmentColor(
    0.75f,
    0.85f,
    0.75f
);                                                                     // #BFD9BF -> (191,217,191)
SbColor DrawingParameters::InternalAlignedGeoColor(0.6f, 0.7f, 0.6f);  // #99B399 -> (153,179,153)
SbColor DrawingParameters::FullyConstraintElementColor(
    0.0f,
    0.83f,
    0.53f
);                                                                       // #00D488 -> (  0,212,136) - Fusion constrained
SbColor DrawingParameters::CurveColor(1.0f, 1.0f, 1.0f);                 // #FFFFFF -> (255,255,255) - white edges
SbColor DrawingParameters::PreselectColor(0.0f, 0.90f, 1.0f);            // #00E5FF -> (  0,229,255) - Fusion cyan preselect
SbColor DrawingParameters::SelectColor(1.0f, 0.08f, 0.58f);              // #FF1493 -> (255, 20,147) - Fusion magenta select
SbColor DrawingParameters::PreselectSelectedColor(1.0f, 0.41f, 0.71f);   // #FF69B4 -> (255,105,180) - hot pink
SbColor DrawingParameters::CurveExternalColor(0.0f, 0.83f, 1.0f);        // #00D4FF -> (  0,212,255) - Fusion cyan external
SbColor DrawingParameters::CurveExternalDefiningColor(0.0f, 0.75f, 0.95f);  // #00BFF2 -> (  0,191,242)
SbColor DrawingParameters::CurveDraftColor(1.0f, 0.55f, 0.0f);  // #FF8C00 -> (255,140,  0) - Fusion orange construction
SbColor DrawingParameters::FullyConstraintConstructionElementColor(
    1.0f,
    0.70f,
    0.30f
);  // #FFB34D -> (255,179, 77) - Fusion orange constrained construction

SbColor DrawingParameters::ConstrDimColor(1.0f, 0.149f, 0.0f);  // #FF2600 -> (255, 38,  0)
SbColor DrawingParameters::ConstrIcoColor(1.0f, 0.149f, 0.0f);  // #FF2600 -> (255, 38,  0)
SbColor DrawingParameters::NonDrivingConstrDimColor(0.0f, 0.149f, 1.0f);  // #0026FF -> (  0, 38,255)
SbColor DrawingParameters::ExprBasedConstrDimColor(1.0f, 0.5f, 0.149f);  // #FF7F26 -> (255, 127,38)
SbColor DrawingParameters::DeactivatedConstrDimColor(0.5f, 0.5f, 0.5f);  // ##7f7f7f -> (127,127,127)
SbColor DrawingParameters::CursorTextColor(0.0f, 0.0f, 1.0f);            // #0000FF -> (0,0,255)

const MultiFieldId MultiFieldId::Invalid = MultiFieldId();
