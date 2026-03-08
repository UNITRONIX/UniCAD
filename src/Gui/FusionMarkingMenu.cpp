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

#include "PreCompiled.h"

#include "FusionMarkingMenu.h"
#include "Application.h"
#include "Command.h"
#include "BitmapFactory.h"

#include <QApplication>
#include <QScreen>
#include <QtMath>
#include <QPainterPath>

using namespace Gui;

namespace {
    constexpr int MENU_RADIUS = 110;
    constexpr int INNER_RADIUS = 30;
    constexpr int ITEM_RADIUS = 26;
    constexpr int TOTAL_SIZE = (MENU_RADIUS + ITEM_RADIUS + 20) * 2;
    constexpr double PI = 3.14159265358979323846;
}

// ---------------------------------------------------------------------------
// FusionMarkingMenu
// ---------------------------------------------------------------------------

FusionMarkingMenu::FusionMarkingMenu(QWidget* parent)
    : QWidget(parent, Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint)
    , m_hoveredIndex(-1)
    , m_radius(MENU_RADIUS)
    , m_innerRadius(INNER_RADIUS)
    , m_overflowMenu(nullptr)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose, false);
    setMouseTracking(true);
    setFixedSize(TOTAL_SIZE, TOTAL_SIZE);
    m_center = QPoint(TOTAL_SIZE / 2, TOTAL_SIZE / 2);
}

FusionMarkingMenu::~FusionMarkingMenu()
{
    delete m_overflowMenu;
}

void FusionMarkingMenu::setRadialItems(const QList<MenuItem>& items)
{
    m_radialItems = items;
    // Pad to 8 items max
    while (m_radialItems.size() > 8) {
        m_radialItems.removeLast();
    }
}

void FusionMarkingMenu::setOverflowItems(const QList<MenuItem>& items)
{
    m_overflowItems = items;
}

void FusionMarkingMenu::showAt(const QPoint& globalPos)
{
    // Position so the center is at the cursor
    QPoint topLeft = globalPos - QPoint(TOTAL_SIZE / 2, TOTAL_SIZE / 2);

    // Clamp to screen bounds
    if (auto* screen = QApplication::screenAt(globalPos)) {
        QRect screenRect = screen->availableGeometry();
        if (topLeft.x() < screenRect.left()) {topLeft.setX(screenRect.left());}
        if (topLeft.y() < screenRect.top()) {topLeft.setY(screenRect.top());}
        if (topLeft.x() + TOTAL_SIZE > screenRect.right()) {topLeft.setX(screenRect.right() - TOTAL_SIZE);}
        if (topLeft.y() + TOTAL_SIZE > screenRect.bottom()) {topLeft.setY(screenRect.bottom() - TOTAL_SIZE);}
    }

    move(topLeft);
    m_center = globalPos - topLeft;
    m_hoveredIndex = -1;
    show();
    raise();
    setFocus();
}

// Slot positions arranged as: N(0), NE(1), E(2), SE(3), S(4), SW(5), W(6), NW(7)
// Angles starting from top (N = -90Â°), going clockwise
static double sliceAngle(int index)
{
    // Each slice is 45Â°, starting from North (-90Â° or 270Â°)
    return -90.0 + index * 45.0;
}

QPointF FusionMarkingMenu::sliceCenter(int index) const
{
    double angle = sliceAngle(index) * PI / 180.0;
    double x = m_center.x() + m_radius * qCos(angle);
    double y = m_center.y() + m_radius * qSin(angle);
    return QPointF(x, y);
}

int FusionMarkingMenu::hitTest(const QPoint& localPos) const
{
    double dx = localPos.x() - m_center.x();
    double dy = localPos.y() - m_center.y();
    double dist = qSqrt(dx * dx + dy * dy);

    // Inside inner dead zone
    if (dist < m_innerRadius) {
        return -1;
    }

    // Calculate angle from center, convert to slice
    double angle = qAtan2(dy, dx) * 180.0 / PI; // -180 to 180
    // Normalize to 0..360 from North going clockwise
    angle = angle + 90.0; // Shift so N=0
    if (angle < 0) {angle += 360.0;}
    if (angle >= 360.0) {angle -= 360.0;}

    int slice = static_cast<int>((angle + 22.5) / 45.0) % 8;
    if (slice >= 0 && slice < m_radialItems.size()) {
        return slice;
    }
    return -1;
}

void FusionMarkingMenu::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Semi-transparent dark background circle
    QPainterPath bgPath;
    bgPath.addEllipse(m_center, m_radius + ITEM_RADIUS + 10, m_radius + ITEM_RADIUS + 10);
    QPainterPath innerPath;
    innerPath.addEllipse(m_center, m_innerRadius, m_innerRadius);
    QPainterPath ring = bgPath.subtracted(innerPath);

    painter.fillPath(ring, QColor(30, 30, 30, 200));

    // Draw center dot
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(100, 100, 100, 180));
    painter.drawEllipse(m_center, 4, 4);

    // Draw slice separators
    painter.setPen(QPen(QColor(60, 60, 60, 150), 1));
    for (int i = 0; i < 8; ++i) {
        double angle1 = (sliceAngle(i) - 22.5) * PI / 180.0;
        QPointF outerPt(m_center.x() + (m_radius + ITEM_RADIUS + 10) * qCos(angle1),
                       m_center.y() + (m_radius + ITEM_RADIUS + 10) * qSin(angle1));
        QPointF innerPt(m_center.x() + m_innerRadius * qCos(angle1),
                       m_center.y() + m_innerRadius * qSin(angle1));
        painter.drawLine(innerPt, outerPt);
    }

    // Draw items
    for (int i = 0; i < m_radialItems.size(); ++i) {
        QPointF center = sliceCenter(i);
        bool isHovered = (i == m_hoveredIndex);

        // Highlight circle
        if (isHovered) {
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(6, 150, 215, 180)); // Fusion 360 blue
            painter.drawEllipse(center, ITEM_RADIUS + 4, ITEM_RADIUS + 4);
        }

        // Icon background
        painter.setPen(Qt::NoPen);
        painter.setBrush(isHovered ? QColor(6, 150, 215) : QColor(60, 60, 60, 220));
        painter.drawEllipse(center, ITEM_RADIUS, ITEM_RADIUS);

        // Icon
        if (!m_radialItems[i].icon.isNull()) {
            QRect iconRect(static_cast<int>(center.x()) - 12,
                          static_cast<int>(center.y()) - 12, 24, 24);
            m_radialItems[i].icon.paint(&painter, iconRect,
                                        Qt::AlignCenter,
                                        m_radialItems[i].enabled ? QIcon::Normal : QIcon::Disabled);
        }

        // Text label below the icon circle
        QFont font = painter.font();
        font.setPixelSize(10);
        font.setBold(isHovered);
        painter.setFont(font);
        painter.setPen(isHovered ? QColor(255, 255, 255) : QColor(200, 200, 200));
        QRectF textRect(center.x() - 50, center.y() + ITEM_RADIUS + 2, 100, 16);
        painter.drawText(textRect, Qt::AlignHCenter | Qt::AlignTop, m_radialItems[i].text);
    }
}

void FusionMarkingMenu::mouseMoveEvent(QMouseEvent* event)
{
    int newHovered = hitTest(event->pos());
    if (newHovered != m_hoveredIndex) {
        m_hoveredIndex = newHovered;
        update();
    }
}

void FusionMarkingMenu::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton || event->button() == Qt::LeftButton) {
        int index = hitTest(event->pos());
        if (index >= 0) {
            executeItem(index);
        }
        hide();
    }
}

void FusionMarkingMenu::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) {
        hide();
    }
}

void FusionMarkingMenu::hideEvent(QHideEvent* event)
{
    m_hoveredIndex = -1;
    QWidget::hideEvent(event);
}

void FusionMarkingMenu::executeItem(int index)
{
    if (index < 0 || index >= m_radialItems.size()) {
        return;
    }
    if (!m_radialItems[index].enabled) {
        return;
    }

    const QString& cmdName = m_radialItems[index].commandName;
    if (cmdName.isEmpty()) {
        return;
    }

    Q_EMIT commandTriggered(cmdName);

    // Execute the FreeCAD command
    auto& mgr = Application::Instance->commandManager();
    Command* cmd = mgr.getCommandByName(cmdName.toLatin1().constData());
    if (cmd) {
        cmd->invoke(0);
    }
}

// ---------------------------------------------------------------------------
// Default marking menu items for 3D view
// ---------------------------------------------------------------------------

QList<FusionMarkingMenu::MenuItem> FusionMarkingMenu::defaultViewItems()
{
    QList<MenuItem> items;
    auto& mgr = Application::Instance->commandManager();

    auto makeItem = [&](const char* cmdName, const QString& label) -> MenuItem {
        MenuItem item;
        item.commandName = QString::fromLatin1(cmdName);
        item.text = label;

        Command* cmd = mgr.getCommandByName(cmdName);
        if (cmd) {
            const char* pixmap = cmd->getPixmap();
            if (pixmap && pixmap[0]) {
                item.icon = BitmapFactory().iconFromTheme(pixmap);
            }
        }
        return item;
    };

    // 8 positions: N, NE, E, SE, S, SW, W, NW
    // Arranged for quick gestural access (most common = N, E, S, W)
    items.append(makeItem("Std_ViewFitAll",      tr("Fit All")));     // N  - top
    items.append(makeItem("Std_SelForward",       tr("Redo Sel")));    // NE
    items.append(makeItem("Std_ViewHome",         tr("Home")));        // E  - right
    items.append(makeItem("Std_ToggleVisibility", tr("Visibility")));  // SE
    items.append(makeItem("Std_Delete",           tr("Delete")));      // S  - bottom
    items.append(makeItem("Std_Undo",             tr("Undo")));        // SW
    items.append(makeItem("Std_ViewIsometric",    tr("ISO View")));    // W  - left
    items.append(makeItem("Std_SelBack",          tr("Undo Sel")));    // NW

    return items;
}

#include "moc_FusionMarkingMenu.cpp"
