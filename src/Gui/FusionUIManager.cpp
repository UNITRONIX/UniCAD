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

#include "FusionUIManager.h"
#include "FusionTabToolbar.h"
#include "FusionTimeline.h"
#include "FusionNavigationBar.h"
#include "FusionMarkingMenu.h"
#include "FusionSelectionBar.h"
#include "Application.h"
#include "MainWindow.h"
#include "MDIView.h"
#include "NaviCube.h"
#include "ToolBarManager.h"
#include "View3DInventor.h"
#include "View3DInventorViewer.h"
#include "ViewProviderDocumentObject.h"
#include "Inventor/SoFCUniversalGrid.h"

#include "WorkbenchManager.h"

#include <App/Application.h>

#include <Inventor/SbColor.h>
#include <Inventor/SoRenderManager.h>

#include <QToolBar>
#include <QMenuBar>
#include <QMainWindow>
#include <QTimer>

#include <cstring>

using namespace Gui;

// ---------------------------------------------------------------------------
// FusionUIManager - Singleton
// ---------------------------------------------------------------------------

FusionUIManager* FusionUIManager::s_instance = nullptr;

FusionUIManager* FusionUIManager::instance()
{
    if (!s_instance) {
        s_instance = new FusionUIManager();
    }
    return s_instance;
}

void FusionUIManager::destroy()
{
    delete s_instance;
    s_instance = nullptr;
}

FusionUIManager::FusionUIManager()
    : QObject()
    , m_enabled(false)
    , m_mainWindow(nullptr)
    , m_tabToolbarWrapper(nullptr)
    , m_tabToolbar(nullptr)
    , m_timeline(nullptr)
    , m_navBar(nullptr)
    , m_markingMenu(nullptr)
    , m_selectionBar(nullptr)
{
}

FusionUIManager::~FusionUIManager()
{
    // Widgets are owned by MainWindow and will be deleted with it
    delete m_markingMenu;
    s_instance = nullptr;
}

void FusionUIManager::initialize(MainWindow* mainWindow)
{
    if (!mainWindow) {
        return;
    }
    m_mainWindow = mainWindow;

    // Check preference - enabled by default for UniCAD Fusion 360-style experience
    auto hGrp = App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/View"
    );
    m_enabled = hGrp->GetBool("FusionUIEnabled", true);

    // Create Fusion Tab Toolbar - inserted above the central widget
    m_tabToolbar = new FusionTabToolbar(mainWindow);
    // Insert between menu bar and central area using addToolBar at top
    m_tabToolbarWrapper = new QToolBar(QStringLiteral("FusionTabs"), mainWindow);
    m_tabToolbarWrapper->setObjectName(QStringLiteral("FusionTabsToolbar"));
    m_tabToolbarWrapper->setMovable(false);
    m_tabToolbarWrapper->setFloatable(false);
    m_tabToolbarWrapper->setMinimumHeight(72);
    m_tabToolbarWrapper->addWidget(m_tabToolbar);
    m_tabToolbarWrapper->setStyleSheet(QStringLiteral("QToolBar#FusionTabsToolbar { border: none; padding: 0; background: #2D2D2D; }"));
    mainWindow->addToolBar(Qt::TopToolBarArea, m_tabToolbarWrapper);
    // Force toolbar to take full width
    mainWindow->addToolBarBreak(Qt::TopToolBarArea);

    // Create Fusion Timeline - docked at bottom
    m_timeline = new FusionTimeline(mainWindow);
    mainWindow->addDockWidget(Qt::BottomDockWidgetArea, m_timeline);

    // Create Fusion Navigation Bar - at bottom of window
    m_navBar = new FusionNavigationBar(mainWindow);
    mainWindow->addToolBar(Qt::BottomToolBarArea, m_navBar);

    // Create Marking Menu (not parented to main window - it's a popup)
    m_markingMenu = new FusionMarkingMenu();

    // Create Selection Bar (docked at bottom, above navigation bar)
    m_selectionBar = new FusionSelectionBar(mainWindow);
    m_selectionBar->setObjectName(QStringLiteral("FusionSelectionBar"));
    m_selectionBar->setMovable(false);
    m_selectionBar->setFloatable(false);
    mainWindow->addToolBar(Qt::BottomToolBarArea, m_selectionBar);

    // Connect to workbench activation signal
    Application::Instance->signalActivateWorkbench.connect(
        [this](const char* name) {
            this->onWorkbenchActivated(name);
        }
    );

    // Connect to document change signal
    Application::Instance->signalActiveDocument.connect(
        [this](const Gui::Document&) {
            this->onActiveDocumentChanged();
        }
    );

    // Connect to view activation signal - configure Blueprint style for new views
    Application::Instance->signalActivateView.connect(
        [this](const MDIView* view) {
            if (m_enabled && view) {
                // Delay configuration to ensure view is fully initialized
                QTimer::singleShot(100, this, [this]() {
                    configureBlueprintStyle();
                });
            }
        }
    );

    // Connect to edit mode signals for Sketch Palette
    Application::Instance->signalInEdit.connect(
        [this](const ViewProviderDocumentObject& vp) {
            this->onInEdit(vp);
        }
    );

    Application::Instance->signalResetEdit.connect(
        [this](const ViewProviderDocumentObject& vp) {
            this->onResetEdit(vp);
        }
    );

    // Apply Fusion mode
    setEnabled(m_enabled);
}

void FusionUIManager::setEnabled(bool enabled)
{
    m_enabled = enabled;

    if (m_tabToolbarWrapper) {
        m_tabToolbarWrapper->setVisible(enabled);
    }
    if (m_tabToolbar) {
        m_tabToolbar->setActive(enabled);
    }
    if (m_timeline) {
        m_timeline->setVisible(enabled);
    }
    if (m_navBar) {
        m_navBar->setVisible(enabled);
    }
    if (m_selectionBar) {
        m_selectionBar->setVisible(enabled);
    }

    if (enabled) {
        // First trigger workbench update to populate tabs BEFORE hiding traditional toolbars
        if (Application::Instance) {
            std::string wb = WorkbenchManager::instance()->activeName();
            if (!wb.empty()) {
                onWorkbenchActivated(wb.c_str());
            }
        }

        // Now hide traditional toolbars (FusionTabToolbar is already populated)
        hideTraditionalToolbars();
        configureNaviCube();
        
        // Apply Blueprint-style background and grid
        configureBlueprintStyle();

        // Refresh timeline
        refreshTimeline();
    }
    else {
        showTraditionalToolbars();
        
        // Restore original background style
        restoreOriginalStyle();
    }

    // Save preference
    auto hGrp = App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/View"
    );
    hGrp->SetBool("FusionUIEnabled", enabled);
}

void FusionUIManager::onWorkbenchActivated(const char* name)
{
    if (!m_enabled || !m_tabToolbar) {
        return;
    }

    QString wbName = QString::fromLatin1(name);
    m_tabToolbar->onWorkbenchActivated(wbName);

    // Hide traditional toolbars when in Fusion mode  
    hideTraditionalToolbars();
}

void FusionUIManager::onActiveDocumentChanged()
{
    if (m_enabled) {
        refreshTimeline();
    }
}

void FusionUIManager::refreshTimeline()
{
    if (m_timeline) {
        m_timeline->refresh();
    }
}

void FusionUIManager::configureNaviCube()
{
    if (!m_mainWindow) {
        return;
    }

    // Find all 3D viewers and configure their NaviCubes
    auto views = m_mainWindow->findChildren<View3DInventor*>();
    for (auto* view3d : views) {
        if (!view3d) {
            continue;
        }
        auto* viewer = view3d->getViewer();
        if (!viewer) {
            continue;
        }

        NaviCube* naviCube = viewer->getNaviCube();
        if (!naviCube) {
            continue;
        }

        // Fusion 360-style: top-right corner, Fusion blue color scheme
        naviCube->setCorner(NaviCube::TopRightCorner);
        naviCube->setSize(120);

        // Fusion 360-like colors (dark with blue accent)
        naviCube->setBaseColor(QColor(50, 50, 50));
        naviCube->setEmphaseColor(QColor(6, 150, 215));    // Fusion blue
        naviCube->setHiliteColor(QColor(80, 180, 230));     // Light blue on hover
        naviCube->setBorderWidth(1.0);
    }
}

void FusionUIManager::hideTraditionalToolbars()
{
    if (!m_mainWindow) {
        return;
    }

    // Hide all traditional toolbars except our Fusion ones
    QList<QToolBar*> toolbars = m_mainWindow->findChildren<QToolBar*>();
    for (auto* toolbar : toolbars) {
        QString name = toolbar->objectName();
        // Keep Fusion UI toolbars visible
        if (name == QStringLiteral("FusionTabsToolbar")
            || name == QStringLiteral("FusionNavigationBar")
            || name == QStringLiteral("FusionSelectionBar")) {
            continue;
        }
        // Hide standard toolbars
        if (toolbar->parentWidget() == m_mainWindow) {
            toolbar->setVisible(false);
        }
    }
}

void FusionUIManager::showTraditionalToolbars()
{
    if (!m_mainWindow) {
        return;
    }

    QList<QToolBar*> toolbars = m_mainWindow->findChildren<QToolBar*>();
    for (auto* toolbar : toolbars) {
        QString name = toolbar->objectName();
        // Skip Fusion toolbars
        if (name == QStringLiteral("FusionTabsToolbar")
            || name == QStringLiteral("FusionNavigationBar")
            || name == QStringLiteral("FusionSelectionBar")) {
            toolbar->setVisible(false);
            continue;
        }
        // Show standard toolbars
        if (toolbar->parentWidget() == m_mainWindow) {
            toolbar->setVisible(true);
        }
    }
}

void FusionUIManager::configureBlueprintStyle()
{
    if (!m_mainWindow) {
        return;
    }

    // Find all 3D viewers and configure Blueprint-style background + grid
    auto views = m_mainWindow->findChildren<View3DInventor*>();
    for (auto* view3d : views) {
        if (!view3d) {
            continue;
        }
        auto* viewer = view3d->getViewer();
        if (!viewer) {
            continue;
        }

        // Universal Grid disabled for now - needs more work on camera/projection alignment
        // TODO: Fix grid rendering to properly align with viewport
        viewer->setUniversalGridVisible(false);
        viewer->setUniversalGridOriginVisible(false);

        // Set Blueprint-style gradient background
        // Fusion 360 uses a blue gradient that looks like a blueprint
        // Top: darker blue (#1a3a52) -> Bottom: slightly lighter (#2d5a78)
        SbColor topColor(0.102f, 0.227f, 0.322f);      // #1a3a52
        SbColor bottomColor(0.176f, 0.353f, 0.471f);   // #2d5a78
        SbColor midColor(0.133f, 0.282f, 0.388f);      // #224862
        
        viewer->setGradientBackground(View3DInventorViewer::Background::LinearGradient);
        viewer->setGradientBackgroundColor(topColor, bottomColor, midColor);
        
        // Force redraw to apply changes
        viewer->getSoRenderManager()->scheduleRedraw();
    }
}

void FusionUIManager::restoreOriginalStyle()
{
    if (!m_mainWindow) {
        return;
    }

    // Find all 3D viewers and restore original style
    auto views = m_mainWindow->findChildren<View3DInventor*>();
    for (auto* view3d : views) {
        if (!view3d) {
            continue;
        }
        auto* viewer = view3d->getViewer();
        if (!viewer) {
            continue;
        }

        // Disable Universal Grid
        viewer->setUniversalGridVisible(false);
        viewer->setUniversalGridOriginVisible(false);

        // Restore original FreeCAD gradient (gray tones)
        SbColor topColor(0.098f, 0.098f, 0.098f);      // Dark gray
        SbColor bottomColor(0.333f, 0.333f, 0.333f);   // Medium gray
        
        viewer->setGradientBackground(View3DInventorViewer::Background::LinearGradient);
        viewer->setGradientBackgroundColor(topColor, bottomColor);
        
        // Force redraw to apply changes
        viewer->getSoRenderManager()->scheduleRedraw();
    }
}

void FusionUIManager::onInEdit(const ViewProviderDocumentObject& vp)
{
    if (!m_enabled || !m_mainWindow) {
        return;
    }

    // Check if this is a Sketcher ViewProvider
    const char* typeName = vp.getTypeId().getName();
    if (typeName && strstr(typeName, "Sketch") != nullptr) {
        // Highlight SKETCH tab in toolbar
        if (m_tabToolbar) {
            m_tabToolbar->setSketchMode(true);
        }
    }
}

void FusionUIManager::onResetEdit(const ViewProviderDocumentObject& vp)
{
    if (!m_enabled || !m_mainWindow) {
        return;
    }

    // Check if this was a Sketcher ViewProvider
    const char* typeName = vp.getTypeId().getName();
    if (typeName && strstr(typeName, "Sketch") != nullptr) {
        // Remove SKETCH tab highlight
        if (m_tabToolbar) {
            m_tabToolbar->setSketchMode(false);
        }
    }
}

#include "moc_FusionUIManager.cpp"
