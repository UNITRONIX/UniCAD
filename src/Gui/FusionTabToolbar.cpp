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

#include "PreCompiled.h"

#include "FusionTabToolbar.h"
#include "Application.h"
#include "Command.h"
#include "MainWindow.h"
#include "BitmapFactory.h"
#include "Document.h"
#include "WorkbenchManager.h"

#include <Base/Interpreter.h>

#include <QApplication>
#include <QScrollArea>
#include <QLabel>
#include <QFrame>
#include <QPushButton>
#include <QTimer>
#include <QDebug>

using namespace Gui;

// ---------------------------------------------------------------------------
// FusionTabToolbar - Fusion 360-style unified tab toolbar
// ---------------------------------------------------------------------------
// Based on UniCAD design document PROPOZYCJA_ZMIAN_UI_FUNKCJE.md:
// - Fixed tabs: SKETCH, SOLID, SURFACE, SHEET METAL, MESH, INSPECT, TOOLS
// - Tabs organize commands from multiple workbenches into logical groups
// - Contextual highlighting when in sketch mode
// ---------------------------------------------------------------------------

FusionTabToolbar::FusionTabToolbar(QWidget* parent)
    : QWidget(parent)
    , m_tabBar(new QTabBar(this))
    , m_stack(new QStackedWidget(this))
    , m_finishSketchBtn(nullptr)
    , m_active(false)
    , m_inSketchMode(false)
{
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Left side: tabs and content
    auto* tabsContainer = new QWidget();
    auto* tabsLayout = new QVBoxLayout(tabsContainer);
    tabsLayout->setContentsMargins(0, 0, 0, 0);
    tabsLayout->setSpacing(0);

    // Tab bar setup - Fusion 360 style
    m_tabBar->setExpanding(false);
    m_tabBar->setDrawBase(false);
    m_tabBar->setDocumentMode(true);
    m_tabBar->setUsesScrollButtons(true);
    m_tabBar->setMinimumHeight(32);
    tabsLayout->addWidget(m_tabBar);

    // Stacked widget for tool pages
    m_stack->setMinimumHeight(40);
    m_stack->setMaximumHeight(48);
    tabsLayout->addWidget(m_stack, 1);

    mainLayout->addWidget(tabsContainer, 1);

    // Right side: Finish Sketch button (contextual)
    m_finishSketchBtn = new QPushButton(tr("Finish Sketch"));
    m_finishSketchBtn->setMinimumWidth(100);
    m_finishSketchBtn->setMinimumHeight(28);
    m_finishSketchBtn->setVisible(false);
    m_finishSketchBtn->setStyleSheet(QStringLiteral(
        "QPushButton {"
        "  background-color: #0696D7;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 4px;"
        "  padding: 6px 16px;"
        "  font-weight: bold;"
        "  margin: 4px 8px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #07A8F0;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #0580B5;"
        "}"
    ));
    connect(m_finishSketchBtn, &QPushButton::clicked, this, &FusionTabToolbar::onFinishSketchClicked);
    mainLayout->addWidget(m_finishSketchBtn);

    // Set minimum size for the whole toolbar
    setMinimumHeight(72);
    setMaximumHeight(80);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    connect(m_tabBar, &QTabBar::currentChanged, this, &FusionTabToolbar::onTabChanged);

    setupStyle();
    // Don't build tabs here - commands may not be loaded yet
    // Tabs will be built in setActive() or onWorkbenchActivated()
}

FusionTabToolbar::~FusionTabToolbar() = default;

void FusionTabToolbar::setupStyle()
{
    setObjectName(QStringLiteral("FusionTabToolbar"));

    // Fusion 360-like styling: dark tabs with blue accent
    setStyleSheet(QStringLiteral(
        "QWidget#FusionTabToolbar {"
        "  background-color: #2D2D2D;"
        "  border-bottom: 1px solid #1A1A1A;"
        "}"
        "QTabBar {"
        "  background: #2D2D2D;"
        "}"
        "QTabBar::tab {"
        "  background: transparent;"
        "  color: #A0A0A0;"
        "  padding: 8px 20px;"
        "  margin-right: 0px;"
        "  border: none;"
        "  font-size: 11px;"
        "  font-weight: bold;"
        "}"
        "QTabBar::tab:selected {"
        "  color: #FFFFFF;"
        "  border-bottom: 2px solid #0696D7;"
        "}"
        "QTabBar::tab:hover:!selected {"
        "  color: #D0D0D0;"
        "  background: rgba(255,255,255,0.05);"
        "}"
        "QStackedWidget {"
        "  background-color: #2D2D2D;"
        "}"
    ));
}

void FusionTabToolbar::setActive(bool active)
{
    m_active = active;
    
    if (active) {
        // Use deferred build to ensure workbenches and commands are loaded
        // Multiple retries to catch late-loading commands
        QTimer::singleShot(200, this, [this]() {
            buildUnifiedTabs();
            setVisible(true);
        });
        QTimer::singleShot(1000, this, [this]() {
            // Rebuild again after 1 second to catch more commands
            int currentTab = m_tabBar->currentIndex();
            buildUnifiedTabs();
            if (currentTab >= 0 && currentTab < m_tabBar->count()) {
                m_tabBar->setCurrentIndex(currentTab);
            }
        });
    }
    else {
        setVisible(false);
    }
}

void FusionTabToolbar::setSketchMode(bool inSketch)
{
    m_inSketchMode = inSketch;
    m_finishSketchBtn->setVisible(inSketch);
    
    // Highlight SKETCH tab when in sketch mode
    if (inSketch && m_tabBar->count() > 0) {
        // Switch to SKETCH tab and highlight it
        m_tabBar->setCurrentIndex(0);
        m_tabBar->setStyleSheet(m_tabBar->styleSheet() + QStringLiteral(
            "QTabBar::tab:first-child {"
            "  background: rgba(6, 150, 215, 0.2);"
            "  color: #0696D7;"
            "}"
        ));
    }
    else {
        // Reset style
        setupStyle();
    }
}

void FusionTabToolbar::onFinishSketchClicked()
{
    // Execute Sketcher_LeaveSketch command
    auto& mgr = Application::Instance->commandManager();
    Command* cmd = mgr.getCommandByName("Sketcher_LeaveSketch");
    if (cmd) {
        cmd->invoke(0);
    }
}

void FusionTabToolbar::onWorkbenchActivated(const QString& workbenchName)
{
    Q_UNUSED(workbenchName)
    // Rebuild tabs to pick up newly available commands from activated workbench
    // Save current tab index
    int currentTab = m_tabBar->currentIndex();
    if (currentTab < 0) currentTab = 1; // Default to SOLID
    
    buildUnifiedTabs();
    
    // Restore tab selection
    if (currentTab >= 0 && currentTab < m_tabBar->count()) {
        m_tabBar->setCurrentIndex(currentTab);
    }
}

void FusionTabToolbar::buildUnifiedTabs()
{
    clearTabs();

    // Ensure workbench modules are loaded to register their commands
    // This imports the Python modules which register C++ commands
    try {
        Base::Interpreter().runString("import PartDesignGui");
        Base::Interpreter().runString("import SketcherGui");
        Base::Interpreter().runString("import PartGui");
        Base::Interpreter().runString("import MeshGui");
    }
    catch (...) {
        // Ignore errors - modules may already be loaded or not available
    }

    // Build unified tabs according to Fusion 360 design document
    // These tabs aggregate commands from multiple workbenches
    m_tabs = {
        buildSketchTab(),
        buildSolidTab(),
        buildSurfaceTab(),
        buildSheetMetalTab(),
        buildMeshTab(),
        buildInspectTab(),
        buildToolsTab()
    };

    // Add tabs to tab bar
    for (int i = 0; i < m_tabs.size(); ++i) {
        m_tabBar->addTab(m_tabs[i].name);
        
        // Create scrollable page for each tab
        auto* scrollArea = new QScrollArea();
        scrollArea->setWidgetResizable(true);
        scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scrollArea->setFrameShape(QFrame::NoFrame);
        scrollArea->setStyleSheet(QStringLiteral("background: transparent; border: none;"));

        auto* page = new QWidget();
        auto* hLayout = new QHBoxLayout(page);
        hLayout->setContentsMargins(8, 4, 8, 4);
        hLayout->setSpacing(4);

        // Populate with command buttons - track last added type
        bool lastWasSeparator = true;  // Start true to skip leading separators
        int buttonsAdded = 0;
        
        for (const auto& cmd : m_tabs[i].commands) {
            if (cmd == QStringLiteral("Separator")) {
                // Only add separator if we have buttons before and it's not consecutive
                if (!lastWasSeparator && buttonsAdded > 0) {
                    addSeparator(hLayout);
                    lastWasSeparator = true;
                }
            }
            else if (cmd.startsWith(QStringLiteral("Group:"))) {
                // Group label
                QString groupName = cmd.mid(6);
                addGroupLabel(hLayout, groupName);
                lastWasSeparator = false;
            }
            else {
                if (addCommandButton(hLayout, cmd.toLatin1().constData())) {
                    buttonsAdded++;
                    lastWasSeparator = false;
                }
            }
        }

        hLayout->addStretch();
        scrollArea->setWidget(page);
        m_stack->addWidget(scrollArea);
    }

    if (m_tabBar->count() > 0) {
        m_tabBar->setCurrentIndex(1); // Default to SOLID tab
    }
}

void FusionTabToolbar::clearTabs()
{
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

bool FusionTabToolbar::addCommandButton(QLayout* layout, const char* cmdName)
{
    auto& mgr = Application::Instance->commandManager();
    Command* cmd = mgr.getCommandByName(cmdName);
    if (!cmd) {
        // Command not available in current context - skip silently
        return false;
    }

    auto* btn = new QToolButton();
    btn->setToolButtonStyle(Qt::ToolButtonIconOnly);
    btn->setAutoRaise(true);
    btn->setIconSize(QSize(22, 22));
    btn->setMinimumSize(32, 32);
    btn->setMaximumSize(36, 36);

    // Get icon - try multiple methods
    bool hasIcon = false;
    const char* pixmap = cmd->getPixmap();
    if (pixmap && pixmap[0]) {
        // Try theme icon first
        QIcon icon = BitmapFactory().iconFromTheme(pixmap);
        if (!icon.isNull()) {
            btn->setIcon(icon);
            hasIcon = true;
        }
        else {
            // Try pixmap directly
            QPixmap pm = BitmapFactory().pixmap(pixmap);
            if (!pm.isNull()) {
                btn->setIcon(QIcon(pm));
                hasIcon = true;
            }
        }
    }
    
    // Fallback: show first 2 letters of command name
    if (!hasIcon) {
        QString cmdStr = QString::fromLatin1(cmdName);
        // Extract short name (e.g., "Pad" from "PartDesign_Pad")
        int idx = cmdStr.lastIndexOf(QLatin1Char('_'));
        QString shortName = (idx >= 0) ? cmdStr.mid(idx + 1, 2) : cmdStr.left(2);
        btn->setText(shortName.toUpper());
        btn->setToolButtonStyle(Qt::ToolButtonTextOnly);
    }

    // Build tooltip: Name (Shortcut)
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
        "  padding: 4px;"
        "  color: #CCCCCC;"
        "  font-size: 9px;"
        "  font-weight: bold;"
        "}"
        "QToolButton:hover {"
        "  background: rgba(255,255,255,0.1);"
        "}"
        "QToolButton:pressed {"
        "  background: #0696D7;"
        "}"
        "QToolButton:checked {"
        "  background: #0696D7;"
        "}"
    ));

    // Connect to command execution - capture command name as QString (by value)
    QString cmdNameStr = QString::fromLatin1(cmdName);
    connect(btn, &QToolButton::clicked, this, [cmdNameStr]() {
        auto& cmdMgr = Application::Instance->commandManager();
        Command* c = cmdMgr.getCommandByName(cmdNameStr.toLatin1().constData());
        if (c) {
            c->invoke(0);
        }
    });

    layout->addWidget(btn);
    return true;
}

void FusionTabToolbar::addSeparator(QLayout* layout)
{
    auto* line = new QFrame();
    line->setFrameShape(QFrame::VLine);
    line->setFrameShadow(QFrame::Plain);
    line->setFixedWidth(1);
    line->setFixedHeight(28);
    line->setStyleSheet(QStringLiteral("background: #444444;"));
    layout->addWidget(line);
}

void FusionTabToolbar::addGroupLabel(QLayout* layout, const QString& label)
{
    auto* lbl = new QLabel(label);
    lbl->setStyleSheet(QStringLiteral(
        "QLabel {"
        "  color: #888888;"
        "  font-size: 9px;"
        "  font-weight: bold;"
        "  padding: 0 4px;"
        "}"
    ));
    layout->addWidget(lbl);
}

// ---------------------------------------------------------------------------
// Tab Definitions - Fusion 360-style unified tabs
// Commands are aggregated from multiple workbenches
// ---------------------------------------------------------------------------

FusionTabToolbar::TabDefinition FusionTabToolbar::buildSketchTab() const
{
    return {
        tr("SKETCH"),
        {
            // Create sketch
            QStringLiteral("PartDesign_NewSketch"),
            QStringLiteral("Sketcher_NewSketch"),
            QStringLiteral("Separator"),
            
            // Draw geometry
            QStringLiteral("Sketcher_CreateLine"),
            QStringLiteral("Sketcher_CreateRectangle"),
            QStringLiteral("Sketcher_CreateOblong"),
            QStringLiteral("Sketcher_CreateCircle"),
            QStringLiteral("Sketcher_Create3PointCircle"),
            QStringLiteral("Sketcher_CreateArc"),
            QStringLiteral("Sketcher_Create3PointArc"),
            QStringLiteral("Sketcher_CreateEllipseByCenter"),
            QStringLiteral("Sketcher_CreatePolyline"),
            QStringLiteral("Sketcher_CreateBSpline"),
            QStringLiteral("Sketcher_CreateSlot"),
            QStringLiteral("Sketcher_CreateRegularPolygon"),
            QStringLiteral("Sketcher_CreatePoint"),
            QStringLiteral("Separator"),
            
            // Constraints - Geometric
            QStringLiteral("Sketcher_ConstrainCoincident"),
            QStringLiteral("Sketcher_ConstrainPointOnObject"),
            QStringLiteral("Sketcher_ConstrainHorizontal"),
            QStringLiteral("Sketcher_ConstrainVertical"),
            QStringLiteral("Sketcher_ConstrainParallel"),
            QStringLiteral("Sketcher_ConstrainPerpendicular"),
            QStringLiteral("Sketcher_ConstrainTangent"),
            QStringLiteral("Sketcher_ConstrainEqual"),
            QStringLiteral("Sketcher_ConstrainSymmetric"),
            QStringLiteral("Separator"),
            
            // Constraints - Dimensional
            QStringLiteral("Sketcher_ConstrainLock"),
            QStringLiteral("Sketcher_ConstrainDistanceX"),
            QStringLiteral("Sketcher_ConstrainDistanceY"),
            QStringLiteral("Sketcher_ConstrainDistance"),
            QStringLiteral("Sketcher_ConstrainRadius"),
            QStringLiteral("Sketcher_ConstrainDiameter"),
            QStringLiteral("Sketcher_ConstrainAngle"),
            QStringLiteral("Separator"),
            
            // Edit tools
            QStringLiteral("Sketcher_Trimming"),
            QStringLiteral("Sketcher_Extend"),
            QStringLiteral("Sketcher_Split"),
            QStringLiteral("Sketcher_External"),
            QStringLiteral("Sketcher_CarbonCopy"),
            QStringLiteral("Sketcher_Offset"),
            QStringLiteral("Sketcher_Move"),
            QStringLiteral("Sketcher_RectangularArray"),
        }
    };
}

FusionTabToolbar::TabDefinition FusionTabToolbar::buildSolidTab() const
{
    return {
        tr("SOLID"),
        {
            // Body management
            QStringLiteral("PartDesign_Body"),
            QStringLiteral("PartDesign_NewSketch"),
            QStringLiteral("Separator"),
            
            // Additive features (Extrude/Join)
            QStringLiteral("PartDesign_Pad"),
            QStringLiteral("PartDesign_Revolution"),
            QStringLiteral("PartDesign_AdditiveLoft"),
            QStringLiteral("PartDesign_AdditivePipe"),
            QStringLiteral("PartDesign_AdditiveHelix"),
            QStringLiteral("PartDesign_AdditiveBox"),
            QStringLiteral("PartDesign_AdditiveCylinder"),
            QStringLiteral("PartDesign_AdditiveSphere"),
            QStringLiteral("Separator"),
            
            // Subtractive features (Cut)
            QStringLiteral("PartDesign_Pocket"),
            QStringLiteral("PartDesign_Hole"),
            QStringLiteral("PartDesign_Groove"),
            QStringLiteral("PartDesign_SubtractiveLoft"),
            QStringLiteral("PartDesign_SubtractivePipe"),
            QStringLiteral("PartDesign_SubtractiveHelix"),
            QStringLiteral("PartDesign_SubtractiveBox"),
            QStringLiteral("Separator"),
            
            // Dress-up features
            QStringLiteral("PartDesign_Fillet"),
            QStringLiteral("PartDesign_Chamfer"),
            QStringLiteral("PartDesign_Draft"),
            QStringLiteral("PartDesign_Thickness"),
            QStringLiteral("Separator"),
            
            // Patterns
            QStringLiteral("PartDesign_Mirrored"),
            QStringLiteral("PartDesign_LinearPattern"),
            QStringLiteral("PartDesign_PolarPattern"),
            QStringLiteral("PartDesign_MultiTransform"),
            QStringLiteral("Separator"),
            
            // Boolean
            QStringLiteral("PartDesign_Boolean"),
            
            // Reference geometry
            QStringLiteral("Separator"),
            QStringLiteral("PartDesign_Plane"),
            QStringLiteral("PartDesign_Line"),
            QStringLiteral("PartDesign_Point"),
            QStringLiteral("PartDesign_CoordinateSystem"),
        }
    };
}

FusionTabToolbar::TabDefinition FusionTabToolbar::buildSurfaceTab() const
{
    return {
        tr("SURFACE"),
        {
            // Surface creation (from Part/Surface workbench)
            QStringLiteral("Surface_Filling"),
            QStringLiteral("Surface_GeomFillSurface"),
            QStringLiteral("Surface_Sections"),
            QStringLiteral("Surface_ExtendFace"),
            QStringLiteral("Separator"),
            
            // Part surfaces
            QStringLiteral("Part_Extrude"),
            QStringLiteral("Part_Revolve"),
            QStringLiteral("Part_Loft"),
            QStringLiteral("Part_Sweep"),
            QStringLiteral("Part_RuledSurface"),
            QStringLiteral("Separator"),
            
            // Modification
            QStringLiteral("Part_Offset"),
            QStringLiteral("Part_Thickness"),
            QStringLiteral("Surface_CurveOnMesh"),
        }
    };
}

FusionTabToolbar::TabDefinition FusionTabToolbar::buildSheetMetalTab() const
{
    return {
        tr("SHEET METAL"),
        {
            // Sheet metal commands (if SheetMetal workbench is available)
            QStringLiteral("SheetMetal_AddBase"),
            QStringLiteral("SheetMetal_AddWall"),
            QStringLiteral("SheetMetal_AddFoldWall"),
            QStringLiteral("SheetMetal_Unfold"),
            QStringLiteral("SheetMetal_AddCornerRelief"),
            QStringLiteral("SheetMetal_AddRelief"),
            QStringLiteral("SheetMetal_AddJunction"),
            QStringLiteral("SheetMetal_AddBend"),
            QStringLiteral("SheetMetal_SketchOnSheet"),
            QStringLiteral("SheetMetal_Forming"),
        }
    };
}

FusionTabToolbar::TabDefinition FusionTabToolbar::buildMeshTab() const
{
    return {
        tr("MESH"),
        {
            // Mesh workbench commands
            QStringLiteral("Mesh_Import"),
            QStringLiteral("Mesh_Export"),
            QStringLiteral("Separator"),
            QStringLiteral("Mesh_FromPartShape"),
            QStringLiteral("Mesh_RemeshGmsh"),
            QStringLiteral("Separator"),
            QStringLiteral("Mesh_VertexCurvature"),
            QStringLiteral("Mesh_HarmonizeNormals"),
            QStringLiteral("Mesh_FlipNormals"),
            QStringLiteral("Separator"),
            QStringLiteral("Mesh_FillupHoles"),
            QStringLiteral("Mesh_FillInteractiveHole"),
            QStringLiteral("Mesh_RemoveComponents"),
            QStringLiteral("Mesh_Smoothing"),
            QStringLiteral("Mesh_Decimating"),
            QStringLiteral("Separator"),
            QStringLiteral("Mesh_BoundingBox"),
            QStringLiteral("Mesh_Evaluation"),
        }
    };
}

FusionTabToolbar::TabDefinition FusionTabToolbar::buildInspectTab() const
{
    return {
        tr("INSPECT"),
        {
            // Measurement
            QStringLiteral("Std_MeasureDistance"),
            QStringLiteral("Part_Measure_Linear"),
            QStringLiteral("Part_Measure_Angular"),
            QStringLiteral("Separator"),
            
            // Analysis
            QStringLiteral("Part_CheckGeometry"),
            QStringLiteral("Sketcher_ValidateSketch"),
            QStringLiteral("Part_SectionCut"),
            QStringLiteral("Separator"),
            
            // Info
            QStringLiteral("Std_MeasureToggleAll"),
            QStringLiteral("Part_Measure_Refresh"),
            QStringLiteral("Part_Measure_Clear_All"),
        }
    };
}

FusionTabToolbar::TabDefinition FusionTabToolbar::buildToolsTab() const
{
    return {
        tr("TOOLS"),
        {
            // File operations
            QStringLiteral("Std_New"),
            QStringLiteral("Std_Open"),
            QStringLiteral("Std_Save"),
            QStringLiteral("Std_SaveAs"),
            QStringLiteral("Separator"),
            
            // Import/Export
            QStringLiteral("Std_Import"),
            QStringLiteral("Std_Export"),
            QStringLiteral("Separator"),
            
            // Edit
            QStringLiteral("Std_Undo"),
            QStringLiteral("Std_Redo"),
            QStringLiteral("Separator"),
            
            // View
            QStringLiteral("Std_ViewFitAll"),
            QStringLiteral("Std_ViewHome"),
            QStringLiteral("Separator"),
            
            // Settings
            QStringLiteral("Std_DlgPreferences"),
            QStringLiteral("Std_DlgParameter"),
            QStringLiteral("Separator"),
            
            // Macros
            QStringLiteral("Std_DlgMacroRecord"),
            QStringLiteral("Std_DlgMacroExecute"),
            QStringLiteral("Std_DlgMacroExecuteDirect"),
        }
    };
}

// Legacy methods for compatibility
void FusionTabToolbar::populateTab(int index)
{
    Q_UNUSED(index)
    // Not needed with unified tabs
}

QList<FusionTabToolbar::TabDefinition> FusionTabToolbar::buildPartDesignTabs() const
{
    return { buildSolidTab() };
}

QList<FusionTabToolbar::TabDefinition> FusionTabToolbar::buildPartTabs() const
{
    return { buildSolidTab() };
}

QList<FusionTabToolbar::TabDefinition> FusionTabToolbar::buildGenericTabs() const
{
    return { buildToolsTab() };
}

#include "moc_FusionTabToolbar.cpp"
