/***************************************************************************
 *   Copyright (c) 2025 UNITRONIX                                         *
 *   UniCAD - A fork of FreeCAD with Fusion 360-style UI                  *
 *                                                                         *
 *   This file is part of UniCAD.                                         *
 *                                                                         *
 *   UniCAD is free software; you can redistribute it and/or modify       *
 *   it under the terms of the GNU Lesser General Public License (LGPL)   *
 *   as published by the Free Software Foundation; either version 2.1 of  *
 *   the License, or (at your option) any later version.                  *
 ***************************************************************************/

#ifndef GUI_FUSIONTABTOOLBAR_H
#define GUI_FUSIONTABTOOLBAR_H

#include <QWidget>
#include <QTabBar>
#include <QToolBar>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QToolButton>
#include <QScrollArea>
#include <QMap>
#include <QStringList>
#include <QPushButton>

#include <FCGlobal.h>

namespace Gui {

class CommandManager;

/**
 * FusionTabToolbar provides a Fusion 360-style unified tabbed toolbar.
 *
 * Based on UniCAD design documents, this widget presents unified tabs:
 * SKETCH, SOLID, SURFACE, SHEET METAL, MESH, INSPECT, TOOLS
 *
 * These tabs aggregate commands from multiple workbenches into logical
 * groups, following the Fusion 360 "grammar of work" philosophy.
 *
 * Features:
 * - Unified tabs that don't change with workbench
 * - Contextual "Finish Sketch" button when in sketch mode
 * - SKETCH tab highlighting during sketch editing
 */
class GuiExport FusionTabToolbar : public QWidget
{
    Q_OBJECT

public:
    explicit FusionTabToolbar(QWidget* parent = nullptr);
    ~FusionTabToolbar() override;

    /// Called when a workbench is activated
    void onWorkbenchActivated(const QString& workbenchName);

    /// Set whether this widget is the active toolbar mode
    void setActive(bool active);
    bool isActive() const { return m_active; }

    /// Set sketch mode (shows Finish Sketch button, highlights SKETCH tab)
    void setSketchMode(bool inSketch);
    bool isInSketchMode() const { return m_inSketchMode; }

    /// Tab definition structure
    struct TabDefinition {
        QString name;
        QStringList commands;
    };

Q_SIGNALS:
    void tabChanged(int index);

private Q_SLOTS:
    void onTabChanged(int index);
    void onFinishSketchClicked();

private:
    void setupStyle();
    void clearTabs();
    void populateTab(int index);
    bool addCommandButton(QLayout* layout, const char* cmdName);
    void addSeparator(QLayout* layout);
    void addGroupLabel(QLayout* layout, const QString& label);

    /// Build unified tabs (Fusion 360-style)
    void buildUnifiedTabs();

    /// Tab builders for each category
    TabDefinition buildSketchTab() const;
    TabDefinition buildSolidTab() const;
    TabDefinition buildModifyTab() const;
    TabDefinition buildSurfaceTab() const;
    TabDefinition buildSheetMetalTab() const;
    TabDefinition buildMeshTab() const;
    TabDefinition buildInspectTab() const;
    TabDefinition buildToolsTab() const;

    /// Legacy methods for compatibility
    QList<TabDefinition> buildPartDesignTabs() const;
    QList<TabDefinition> buildPartTabs() const;
    QList<TabDefinition> buildGenericTabs() const;

    QTabBar* m_tabBar;
    QStackedWidget* m_stack;
    QList<TabDefinition> m_tabs;
    QPushButton* m_finishSketchBtn;
    bool m_active;
    bool m_inSketchMode;
};

} // namespace Gui

#endif // GUI_FUSIONTABTOOLBAR_H
