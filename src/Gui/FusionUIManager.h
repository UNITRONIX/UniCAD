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

#ifndef GUI_FUSIONUIMANAGER_H
#define GUI_FUSIONUIMANAGER_H

#include <QObject>
#include <QPointer>
#include <QToolBar>

#include <fastsignals/connection.h>

#include <FCGlobal.h>

namespace Gui {

class FusionTabToolbar;
class FusionTimeline;
class FusionNavigationBar;
class FusionMarkingMenu;
class FusionSelectionBar;
class MainWindow;
class ViewProviderDocumentObject;

/**
 * FusionUIManager orchestrates all Fusion 360-style UI components.
 *
 * This singleton manager:
 * - Creates and registers the FusionTabToolbar, FusionTimeline,
 *   FusionNavigationBar and FusionMarkingMenu
 * - Hooks into workbench activation to update the tab toolbar
 * - Hooks into document changes to update the timeline
 * - Configures the NaviCube with Fusion 360 styling
 * - Hides traditional toolbars when Fusion mode is active
 *
 * Enable/disable the Fusion UI via the preference:
 *   User parameter:BaseApp/Preferences/View/FusionUIEnabled (bool, default true)
 */
class GuiExport FusionUIManager : public QObject
{
    Q_OBJECT

public:
    static FusionUIManager* instance();
    static void destroy();

    /// Initialize and attach to MainWindow (called once after main window is fully created)
    void initialize(MainWindow* mainWindow);

    /// Is the Fusion 360 UI mode active?
    bool isEnabled() const { return m_enabled; }

    /// Toggle the Fusion 360 UI mode
    void setEnabled(bool enabled);

    /// Access Fusion marking menu for the 3D view context menu override
    FusionMarkingMenu* markingMenu() const { return m_markingMenu; }

    /// Access Fusion selection bar
    FusionSelectionBar* selectionBar() const { return m_selectionBar; }

public Q_SLOTS:
    /// Called when a workbench is activated
    void onWorkbenchActivated(const char* name);

    /// Called when the active document changes
    void onActiveDocumentChanged();

    /// Refresh the timeline (call after feature creation/deletion)
    void refreshTimeline();

    /// Configure the NaviCube with Fusion 360 style
    void configureNaviCube();
    
    /// Configure Blueprint-style background and grid
    void configureBlueprintStyle();
    
    /// Restore original background style
    void restoreOriginalStyle();

public Q_SLOTS:
    /// Called when entering edit mode (e.g., Sketcher)
    void onInEdit(const ViewProviderDocumentObject& vp);
    
    /// Called when exiting edit mode
    void onResetEdit(const ViewProviderDocumentObject& vp);

private:
    FusionUIManager();
    ~FusionUIManager() override;

    void hideTraditionalToolbars();
    void showTraditionalToolbars();

    static FusionUIManager* s_instance;

    bool m_enabled;
    QPointer<MainWindow> m_mainWindow;
    QToolBar* m_tabToolbarWrapper;
    FusionTabToolbar* m_tabToolbar;
    FusionTimeline* m_timeline;
    FusionNavigationBar* m_navBar;
    FusionMarkingMenu* m_markingMenu;
    FusionSelectionBar* m_selectionBar;
};

} // namespace Gui

#endif // GUI_FUSIONUIMANAGER_H
