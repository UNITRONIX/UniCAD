// SPDX-License-Identifier: LGPL-2.1-or-later
/****************************************************************************
 *                                                                          *
 *   Copyright (c) 2025 Kacper Donat <kacper@kadet.net>                     *
 *                                                                          *
 *   This file is part of FreeCAD.                                          *
 *                                                                          *
 *   FreeCAD is free software: you can redistribute it and/or modify it     *
 *   under the terms of the GNU Lesser General Public License as            *
 *   published by the Free Software Foundation, either version 2.1 of the   *
 *   License, or (at your option) any later version.                        *
 *                                                                          *
 *   FreeCAD is distributed in the hope that it will be useful, but         *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of             *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU       *
 *   Lesser General Public License for more details.                        *
 *                                                                          *
 *   You should have received a copy of the GNU Lesser General Public       *
 *   License along with FreeCAD. If not, see                                *
 *   <https://www.gnu.org/licenses/>.                                       *
 *                                                                          *
 ***************************************************************************/

#ifndef STYLEPARAMETERS_H
#define STYLEPARAMETERS_H

#include <Gui/StyleParameters/ParameterManager.h>

namespace PartDesignGui::StyleParameters
{
// UniCAD: Fusion 360-style preview colors
DEFINE_STYLE_PARAMETER(PreviewAdditiveColor, Base::Color(0.024F, 0.588F, 0.843F));     // Fusion blue #0696D7
DEFINE_STYLE_PARAMETER(PreviewSubtractiveColor, Base::Color(1.0F, 0.2F, 0.2F));        // Red #FF3333
DEFINE_STYLE_PARAMETER(PreviewCommonColor, Base::Color(1.0F, 0.75F, 0.0F));            // Orange #FFBF00
DEFINE_STYLE_PARAMETER(PreviewDressUpColor, Base::Color(0.0F, 0.83F, 1.0F));           // Cyan #00D4FF

DEFINE_STYLE_PARAMETER(PreviewProfileLineWidth, Gui::StyleParameters::Numeric(3));     // Thinner profile lines
DEFINE_STYLE_PARAMETER(PreviewProfileOpacity, Gui::StyleParameters::Numeric(0.0));

DEFINE_STYLE_PARAMETER(PreviewErrorColor, Base::Color(1.0F, 0.267F, 0.267F));          // Soft red #FF4444
DEFINE_STYLE_PARAMETER(PreviewErrorOpacity, Gui::StyleParameters::Numeric(0.1));

DEFINE_STYLE_PARAMETER(PreviewToolOpacity, Gui::StyleParameters::Numeric(0.08));
DEFINE_STYLE_PARAMETER(PreviewShapeOpacity, Gui::StyleParameters::Numeric(0.25));      // More visible preview

DEFINE_STYLE_PARAMETER(PreviewLineWidth, Gui::StyleParameters::Numeric(2));
}  // namespace PartDesignGui::StyleParameters

#endif  // STYLEPARAMETERS_H
