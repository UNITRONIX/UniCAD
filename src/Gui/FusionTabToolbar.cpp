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

#include "PreCompiled.h"

#include "FusionTabToolbar.h"
#include "Application.h"
#include "Command.h"
#include "MainWindow.h"
#include "BitmapFactory.h"
#include "ToolBarManager.h"

#include <QApplication>
#include <QScrollArea>
#include <QLabel>
#include <QFrame>

using namespace Gui;

// ---------------------------------------------------------------------------
// FusionTabToolbar
// ---------------------------------------------------------------------------

FusionTabToolbar::FusionTabToolbar(QWidget* parent)
    : QWidget(parent)
    , m_tabBar(new QTabBar(this))
    , m_stack(new QStackedWidget(this))
    , m_active(false)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // Tab bar setup
    m_tabBar->setExpanding(false);
    m_tabBar->setDrawBase(false);
    m_tabBar->setDocumentMode(true);
    m_tabBar->setUsesScrollButtons(true);
    layout->addWidget(m_tabBar);

    // Stacked widget for tool pages
    m_stack->setMinimumHeight(44);
    m_stack->setMaximumHeight(54);
    layout->addWidget(m_stack);

    connect(m_tabBar, &QTabBar::currentChanged, this, &FusionTabToolbar::onTabChanged);

    setupStyle();
    setVisible(false);
}

FusionTabToolbar::~FusionTabToolbar() = default;

void FusionTabToolbar::setupStyle()
{
    setObjectName(QStringLiteral("FusionTabToolbar"));

    // Fusion 360-like styling: dark tabs with blue accent
    setStyleSheet(QStringLiteral(
        "QWidget#FusionTabToolbar {"
        "  background-color: #2D2D2D;"
        "}"
        "QTabBar::tab {"
        "  background: #333333;"
        "  color: #CCCCCC;"
        "  padding: 6px 16px;"
        "  margin-right: 1px;"
        "  border: none;"
        "  font-size: 11px;"
        "  font-weight: bold;"
        "  text-transform: uppercase;"
        "}"
        "QTabBar::tab:selected {"
        "  background: #2D2D2D;"
        "  color: #0696D7;"
        "  border-bottom: 2px solid #0696D7;"
        "}"
        "QTabBar::tab:hover {"
        "  background: #3D3D3D;"
        "  color: #FFFFFF;"
        "}"
        "QStackedWidget {"
        "  background-color: #2D2D2D;"
        "}"
    ));
}

void FusionTabToolbar::setActive(bool active)
{
    m_active = active;
    setVisible(active);
}

void FusionTabToolbar::onWorkbenchActivated(const QString& workbenchName)
{
    clearTabs();

    if (workbenchName.contains(QStringLiteral("PartDesign"), Qt::CaseInsensitive)) {
        m_tabs = buildPartDesignTabs();
    }
    else if (workbenchName.contains(QStringLiteral("Part"), Qt::CaseInsensitive)
          && !workbenchName.contains(QStringLiteral("PartDesign"), Qt::CaseInsensitive)) {
        m_tabs = buildPartTabs();
    }
    else {
        m_tabs = buildGenericTabs();
    }

    // Add tabs
    for (int i = 0; i < m_tabs.size(); ++i) {
        m_tabBar->addTab(m_tabs[i].name);

        // Create scrollable page for each tab
        auto* scrollArea = new QScrollArea();
        scrollArea->setWidgetResizable(true);
        scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scrollArea->setFrameShape(QFrame::NoFrame);
        scrollArea->setStyleSheet(QStringLiteral("background: transparent;"));

        auto* page = new QWidget();
        auto* hLayout = new QHBoxLayout(page);
        hLayout->setContentsMargins(4, 2, 4, 2);
        hLayout->setSpacing(2);

        // Populate with command buttons
        for (const auto& cmd : m_tabs[i].commands) {
            if (cmd == QStringLiteral("Separator")) {
                addSeparator(hLayout);
            }
            else {
                addCommandButton(hLayout, cmd.toLatin1().constData());
            }
        }

        // Add stretch at end to left-align buttons
        hLayout->addStretch();

        scrollArea->setWidget(page);
        m_stack->addWidget(scrollArea);
    }

    if (m_tabBar->count() > 0) {
        m_tabBar->setCurrentIndex(0);
    }
}

void FusionTabToolbar::clearTabs()
{
    // Disconnect to prevent signal during cleanup
    m_tabBar->blockSignals(true);
    while (m_tabBar->count() > 0) {
        m_tabBar->removeTab(0);
    }
    while (m_stack->count() > 0) {
        QWidget* w = m_stack->widget(0);
        m_stack->removeWidget(w);
        delete w;
    }
    m_tabs.clear();
    m_tabBar->blockSignals(false);
}

void FusionTabToolbar::onTabChanged(int index)
{
    if (index >= 0 && index < m_stack->count()) {
        m_stack->setCurrentIndex(index);
        Q_EMIT tabChanged(index);
    }
}

void FusionTabToolbar::populateTab(int index)
{
    if (index < 0 || index >= m_tabs.size()) {
        return;
    }

    auto* page = m_stack->widget(index);
    if (!page) {
        return;
    }

    auto* layout = page->layout();
    if (!layout) {
        return;
    }

    for (const auto& cmd : m_tabs[index].commands) {
        if (cmd == QStringLiteral("Separator")) {
            addSeparator(layout);
        }
        else {
            addCommandButton(layout, cmd.toLatin1().constData());
        }
    }
}

void FusionTabToolbar::addCommandButton(QLayout* layout, const char* cmdName)
{
    auto& mgr = Application::Instance->commandManager();
    Command* cmd = mgr.getCommandByName(cmdName);
    if (!cmd) {
        return;
    }

    auto* btn = new QToolButton();
    btn->setToolButtonStyle(Qt::ToolButtonIconOnly);
    btn->setAutoRaise(true);
    btn->setIconSize(QSize(28, 28));
    btn->setMinimumSize(36, 36);
    btn->setMaximumSize(44, 44);

    // Get icon and tooltip
    const char* pixmap = cmd->getPixmap();
    if (pixmap && pixmap[0]) {
        QIcon icon = BitmapFactory().iconFromTheme(pixmap);
        if (!icon.isNull()) {
            btn->setIcon(icon);
        }
    }

    QString tooltip = QString::fromUtf8(cmd->getMenuText());
    QString accel = QString::fromUtf8(cmd->getAccel());
    if (!accel.isEmpty()) {
        tooltip += QStringLiteral(" (") + accel + QStringLiteral(")");
    }
    btn->setToolTip(tooltip);

    btn->setStyleSheet(QStringLiteral(
        "QToolButton {"
        "  background: transparent;"
        "  border: none;"
        "  border-radius: 4px;"
        "  padding: 3px;"
        "}"
        "QToolButton:hover {"
        "  background: #3D3D3D;"
        "}"
        "QToolButton:pressed {"
        "  background: #0696D7;"
        "}"
    ));

    // Connect to command execution
    connect(btn, &QToolButton::clicked, [cmdName, &mgr]() {
        Command* c = mgr.getCommandByName(cmdName);
        if (c) {
            c->invoke(0);
        }
    });

    layout->addWidget(btn);
}

void FusionTabToolbar::addSeparator(QLayout* layout)
{
    auto* line = new QFrame();
    line->setFrameShape(QFrame::VLine);
    line->setFrameShadow(QFrame::Plain);
    line->setFixedWidth(1);
    line->setFixedHeight(32);
    line->setStyleSheet(QStringLiteral("color: #555555;"));
    layout->addWidget(line);
}

// ---------------------------------------------------------------------------
// Tab definitions for PartDesign workbench
// ---------------------------------------------------------------------------

QList<FusionTabToolbar::TabDefinition> FusionTabToolbar::buildPartDesignTabs() const
{
    QList<TabDefinition> tabs;

    // SOLID tab - modeling features (like Fusion 360 SOLID design mode)
    tabs.append({
        tr("SOLID"),
        {
            QStringLiteral("PartDesign_Body"),
            QStringLiteral("PartDesign_CompSketches"),
            QStringLiteral("Separator"),
            QStringLiteral("PartDesign_Pad"),
            QStringLiteral("PartDesign_Revolution"),
            QStringLiteral("PartDesign_AdditiveLoft"),
            QStringLiteral("PartDesign_AdditivePipe"),
            QStringLiteral("PartDesign_AdditiveHelix"),
            QStringLiteral("Separator"),
            QStringLiteral("PartDesign_Pocket"),
            QStringLiteral("PartDesign_Hole"),
            QStringLiteral("PartDesign_Groove"),
            QStringLiteral("PartDesign_SubtractiveLoft"),
            QStringLiteral("PartDesign_SubtractivePipe"),
            QStringLiteral("PartDesign_SubtractiveHelix"),
            QStringLiteral("Separator"),
            QStringLiteral("PartDesign_Boolean"),
        }
    });

    // MODIFY tab - dress-up + transform features
    tabs.append({
        tr("MODIFY"),
        {
            QStringLiteral("PartDesign_Fillet"),
            QStringLiteral("PartDesign_Chamfer"),
            QStringLiteral("PartDesign_Draft"),
            QStringLiteral("PartDesign_Thickness"),
            QStringLiteral("Separator"),
            QStringLiteral("PartDesign_Mirrored"),
            QStringLiteral("PartDesign_LinearPattern"),
            QStringLiteral("PartDesign_PolarPattern"),
            QStringLiteral("PartDesign_MultiTransform"),
        }
    });

    // CONSTRUCT tab - datum features + reference geometry
    tabs.append({
        tr("CONSTRUCT"),
        {
            QStringLiteral("PartDesign_Plane"),
            QStringLiteral("PartDesign_Line"),
            QStringLiteral("PartDesign_Point"),
            QStringLiteral("PartDesign_CoordinateSystem"),
            QStringLiteral("Separator"),
            QStringLiteral("PartDesign_SubShapeBinder"),
            QStringLiteral("PartDesign_Clone"),
        }
    });

    // INSPECT tab - measurement and analysis tools
    tabs.append({
        tr("INSPECT"),
        {
            QStringLiteral("Part_CheckGeometry"),
            QStringLiteral("Sketcher_ValidateSketch"),
            QStringLiteral("Separator"),
            QStringLiteral("Std_MeasureDistance"),
        }
    });

    // TOOLS tab - general/utility tools
    tabs.append({
        tr("TOOLS"),
        {
            QStringLiteral("Std_DlgParameter"),
            QStringLiteral("Std_DlgPreferences"),
            QStringLiteral("Separator"),
            QStringLiteral("Std_DlgMacroRecord"),
            QStringLiteral("Std_DlgMacroExecute"),
        }
    });

    return tabs;
}

// ---------------------------------------------------------------------------
// Tab definitions for Part workbench
// ---------------------------------------------------------------------------

QList<FusionTabToolbar::TabDefinition> FusionTabToolbar::buildPartTabs() const
{
    QList<TabDefinition> tabs;

    tabs.append({
        tr("SOLID"),
        {
            QStringLiteral("Part_Box"),
            QStringLiteral("Part_Cylinder"),
            QStringLiteral("Part_Sphere"),
            QStringLiteral("Part_Cone"),
            QStringLiteral("Part_Torus"),
            QStringLiteral("Separator"),
            QStringLiteral("Part_Extrude"),
            QStringLiteral("Part_Revolve"),
            QStringLiteral("Part_Loft"),
            QStringLiteral("Part_Sweep"),
        }
    });

    tabs.append({
        tr("MODIFY"),
        {
            QStringLiteral("Part_Fillet"),
            QStringLiteral("Part_Chamfer"),
            QStringLiteral("Separator"),
            QStringLiteral("Part_Cut"),
            QStringLiteral("Part_Fuse"),
            QStringLiteral("Part_Common"),
            QStringLiteral("Part_Section"),
        }
    });

    tabs.append({
        tr("TOOLS"),
        {
            QStringLiteral("Part_CheckGeometry"),
            QStringLiteral("Separator"),
            QStringLiteral("Std_DlgParameter"),
            QStringLiteral("Std_DlgPreferences"),
        }
    });

    return tabs;
}

// ---------------------------------------------------------------------------
// Generic fallback tabs for other workbenches
// ---------------------------------------------------------------------------

QList<FusionTabToolbar::TabDefinition> FusionTabToolbar::buildGenericTabs() const
{
    QList<TabDefinition> tabs;

    // CREATE tab with common creation tools
    tabs.append({
        tr("CREATE"),
        {
            QStringLiteral("Std_New"),
            QStringLiteral("Std_Open"),
            QStringLiteral("Std_Save"),
            QStringLiteral("Separator"),
            QStringLiteral("Std_Import"),
            QStringLiteral("Std_Export"),
        }
    });

    // TOOLS tab
    tabs.append({
        tr("TOOLS"),
        {
            QStringLiteral("Std_DlgParameter"),
            QStringLiteral("Std_DlgPreferences"),
            QStringLiteral("Separator"),
            QStringLiteral("Std_DlgMacroRecord"),
            QStringLiteral("Std_DlgMacroExecute"),
        }
    });

    return tabs;
}

#include "moc_FusionTabToolbar.cpp"
