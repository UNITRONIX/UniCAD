/***************************************************************************
 *   Copyright (c) 2026 UNITRONIX                                         *
 *   UniCAD - A fork of FreeCAD with Fusion 360-style UI                  *
 *                                                                         *
 *   This file is part of UniCAD.                                         *
 *                                                                         *
 *   UniCAD is free software; you can redistribute it and/or modify       *
 *   it under the terms of the GNU Lesser General Public License (LGPL)   *
 *   as published by the Free Software Foundation; either version 2.1 of  *
 *   the License, or (at your option) any later version.                  *
 ***************************************************************************/

#ifndef GUI_FUSIONSKETCHPALETTE_H
#define GUI_FUSIONSKETCHPALETTE_H

#include <QWidget>
#include <QToolButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include <QList>

#include <FCGlobal.h>

namespace Gui {

/**
 * FusionSketchPalette provides a Fusion 360-style floating sketch toolbar.
 *
 * This compact floating panel appears when entering sketch edit mode and
 * provides quick access to the most commonly used sketch tools:
 * - Line, Rectangle, Circle, Arc
 * - Constraints (Horizontal, Vertical, Coincident, etc.)
 * - Trim, Offset, Mirror
 *
 * Features:
 * - Draggable positioning
 * - Collapsible sections
 * - Semi-transparent dark theme matching UniCAD style
 * - Smooth show/hide animations
 *
 * The palette follows the cursor initially but can be pinned to a location.
 */
class GuiExport FusionSketchPalette : public QWidget
{
    Q_OBJECT

public:
    explicit FusionSketchPalette(QWidget* parent = nullptr);
    ~FusionSketchPalette() override;

    /// Show the palette with animation
    void showPalette(const QPoint& position = QPoint());
    
    /// Hide the palette with animation
    void hidePalette();
    
    /// Check if palette is currently visible
    bool isPaletteVisible() const { return m_visible; }
    
    /// Pin/unpin the palette (pinned = stays in place)
    void setPinned(bool pinned);
    bool isPinned() const { return m_pinned; }
    
    /// Update position to follow a point (if not pinned)
    void updatePosition(const QPoint& cursorPos);
    
    /// Update DOF (degrees of freedom) display
    void updateDOF(const QString& state, const QString& msg, const QString& link, const QString& linkMsg);

Q_SIGNALS:
    void paletteHidden();
    void palettePinned(bool pinned);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    void setupUI();
    void setupStyle();
    void addSection(const QString& title, const QStringList& commands);
    QToolButton* createToolButton(const QString& cmdName);
    void addSeparatorLine();

    // UI elements
    QVBoxLayout* m_mainLayout;
    QWidget* m_titleBar;
    QLabel* m_titleLabel;
    QToolButton* m_pinButton;
    QToolButton* m_closeButton;
    QLabel* m_dofLabel;
    QWidget* m_dofWidget;
    
    // State
    bool m_visible;
    bool m_pinned;
    bool m_dragging;
    QPoint m_dragOffset;
    
    // Animation
    QPropertyAnimation* m_fadeAnimation;
};

} // namespace Gui

#endif // GUI_FUSIONSKETCHPALETTE_H
