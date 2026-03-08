/***************************************************************************
 *   Copyright (c) 2025 UNITRONIX                                         *
 *   UniCAD - A fork of FreeCAD                                        *
 *                                                                         *
 *   This file is part of UniCAD.                                       *
 *                                                                         *
 *   UniCAD is free software; you can redistribute it and/or modify     *
 *   it under the terms of the GNU Lesser General Public License (LGPL)    *
 *   as published by the Free Software Foundation; either version 2.1 of   *
 *   the License, or (at your option) any later version.                   *
 ***************************************************************************/

#ifndef GUI_FUSIONMARKINGMENU_H
#define GUI_FUSIONMARKINGMENU_H

#include <QWidget>
#include <QMenu>
#include <QPainter>
#include <QMouseEvent>
#include <QAction>
#include <QList>
#include <QTimer>

#include <FCGlobal.h>

namespace Gui {

/**
 * FusionMarkingMenu provides a Fusion 360-style radial/pie context menu.
 *
 * When the user right-clicks in the 3D view, instead of a standard
 * linear context menu, a radial menu appears with commonly used
 * actions arranged in a circle around the cursor.
 *
 * The menu has:
 * - 8 positions (N, NE, E, SE, S, SW, W, NW) for quick-access commands
 * - A "More" section that opens a standard menu for less common actions
 * - Visual highlighting as the mouse moves toward a slice
 *
 * Inspired by Fusion 360's marking menu which provides fast gestural access.
 */
class GuiExport FusionMarkingMenu : public QWidget
{
    Q_OBJECT

public:
    explicit FusionMarkingMenu(QWidget* parent = nullptr);
    ~FusionMarkingMenu() override;

    struct MenuItem {
        QString text;
        QString commandName;   ///< FreeCAD command system name
        QIcon icon;
        bool enabled = true;
    };

    /// Set the items for the 8 radial positions (N, NE, E, SE, S, SW, W, NW)
    void setRadialItems(const QList<MenuItem>& items);

    /// Set overflow items shown in a standard submenu
    void setOverflowItems(const QList<MenuItem>& items);

    /// Show the marking menu centered at the given global position
    void showAt(const QPoint& globalPos);

    /// Static helper: build default marking menu for 3D view context
    static QList<MenuItem> defaultViewItems();

Q_SIGNALS:
    void commandTriggered(const QString& commandName);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void hideEvent(QHideEvent* event) override;

private:
    int hitTest(const QPoint& localPos) const;
    QPointF sliceCenter(int index) const;
    void executeItem(int index);

    QList<MenuItem> m_radialItems;
    QList<MenuItem> m_overflowItems;
    int m_hoveredIndex;
    int m_radius;
    int m_innerRadius;
    QPoint m_center;
    QMenu* m_overflowMenu;
};

} // namespace Gui

#endif // GUI_FUSIONMARKINGMENU_H
