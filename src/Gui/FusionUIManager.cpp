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

#include "FusionUIManager.h"
#include "FusionTabToolbar.h"
#include "FusionTimeline.h"
#include "FusionNavigationBar.h"
#include "FusionMarkingMenu.h"
#include "Application.h"
#include "MainWindow.h"
#include "NaviCube.h"
#include "ToolBarManager.h"
#include "View3DInventor.h"
#include "View3DInventorViewer.h"
#include "WorkbenchManager.h"

#include <App/Application.h>

#include <QToolBar>
#include <QMenuBar>
#include <QMainWindow>

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
    , m_tabToolbar(nullptr)
    , m_timeline(nullptr)
    , m_navBar(nullptr)
    , m_markingMenu(nullptr)
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

    // Check preference - disabled by default to preserve traditional FreeCAD UI
    auto hGrp = App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/View"
    );
    m_enabled = hGrp->GetBool("FusionUIEnabled", false);

    // Create Fusion Tab Toolbar - inserted above the central widget
    m_tabToolbar = new FusionTabToolbar(mainWindow);
    // Insert between menu bar and central area using addToolBar at top
    auto* tabToolbarWrapper = new QToolBar(QStringLiteral("FusionTabs"), mainWindow);
    tabToolbarWrapper->setObjectName(QStringLiteral("FusionTabsToolbar"));
    tabToolbarWrapper->setMovable(false);
    tabToolbarWrapper->setFloatable(false);
    tabToolbarWrapper->addWidget(m_tabToolbar);
    tabToolbarWrapper->setStyleSheet(QStringLiteral("QToolBar { border: none; padding: 0; }"));
    mainWindow->addToolBar(Qt::TopToolBarArea, tabToolbarWrapper);

    // Create Fusion Timeline - docked at bottom
    m_timeline = new FusionTimeline(mainWindow);
    mainWindow->addDockWidget(Qt::BottomDockWidgetArea, m_timeline);

    // Create Fusion Navigation Bar - at bottom of window
    m_navBar = new FusionNavigationBar(mainWindow);
    mainWindow->addToolBar(Qt::BottomToolBarArea, m_navBar);

    // Create Marking Menu (not parented to main window - it's a popup)
    m_markingMenu = new FusionMarkingMenu();

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

    // Apply Fusion mode
    setEnabled(m_enabled);
}

void FusionUIManager::setEnabled(bool enabled)
{
    m_enabled = enabled;

    if (m_tabToolbar) {
        m_tabToolbar->setActive(enabled);
    }
    if (m_timeline) {
        m_timeline->setVisible(enabled);
    }
    if (m_navBar) {
        m_navBar->setVisible(enabled);
    }

    if (enabled) {
        hideTraditionalToolbars();
        configureNaviCube();

        // Trigger initial workbench update
        if (Application::Instance) {
            std::string wb = WorkbenchManager::instance()->activeName();
            if (!wb.empty()) {
                onWorkbenchActivated(wb.c_str());
            }
        }

        // Refresh timeline
        refreshTimeline();
    }
    else {
        showTraditionalToolbars();
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
            || name == QStringLiteral("FusionNavigationBar")) {
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
            || name == QStringLiteral("FusionNavigationBar")) {
            toolbar->setVisible(false);
            continue;
        }
        // Show standard toolbars
        if (toolbar->parentWidget() == m_mainWindow) {
            toolbar->setVisible(true);
        }
    }
}

#include "moc_FusionUIManager.cpp"
