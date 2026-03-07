/***************************************************************************
 *   Copyright (c) 2025 UNITRONIX                                         *
 *   FusionCAD - A fork of FreeCAD                                        *
 *                                                                         *
 *   This file is part of FusionCAD.                                       *
 *                                                                         *
 *   FusionCAD is free software; you can redistribute it and/or modify     *
 *   it under the terms of the GNU Lesser General Public License (LGPL)    *
 *   as published by the Free Software Foundation; either version 2.1 of   *
 *   the License, or (at your option) any later version.                   *
 ***************************************************************************/

#ifndef GUI_FUSIONNAVIGATIONBAR_H
#define GUI_FUSIONNAVIGATIONBAR_H

#include <QToolBar>
#include <QToolButton>

#include <FCGlobal.h>

namespace Gui {

/**
 * FusionNavigationBar provides a Fusion 360-style navigation toolbar.
 *
 * Positioned at the bottom of the 3D viewport, it offers quick access
 * to navigation controls: Orbit, Pan, Zoom, Fit All, and view presets.
 * This mirrors the bottom navigation bar in Fusion 360.
 */
class GuiExport FusionNavigationBar : public QToolBar
{
    Q_OBJECT

public:
    explicit FusionNavigationBar(QWidget* parent = nullptr);
    ~FusionNavigationBar() override;

private:
    void setupActions();
    void addNavButton(const char* cmdName, const QString& label, const QString& iconTheme);
    void setupStyle();
};

} // namespace Gui

#endif // GUI_FUSIONNAVIGATIONBAR_H
