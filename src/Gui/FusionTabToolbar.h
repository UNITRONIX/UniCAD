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

#include <FCGlobal.h>

namespace Gui {

class CommandManager;

/**
 * FusionTabToolbar provides a Fusion 360-style tabbed toolbar interface.
 *
 * Instead of showing traditional floating toolbars, this widget presents
 * tool categories as tabs (SOLID, SURFACE, MESH, INSPECT, TOOLS, etc.)
 * with horizontally arranged tool buttons below the active tab.
 *
 * It hooks into the workbench activation system to reorganize existing
 * toolbar commands into logical tab categories.
 */
class GuiExport FusionTabToolbar : public QWidget
{
    Q_OBJECT

public:
    explicit FusionTabToolbar(QWidget* parent = nullptr);
    ~FusionTabToolbar() override;

    /// Called when a workbench is activated to reorganize tabs
    void onWorkbenchActivated(const QString& workbenchName);

    /// Set whether this widget is the active toolbar mode
    void setActive(bool active);
    bool isActive() const { return m_active; }

    /// Returns the mapping: tab name → list of command names
    struct TabDefinition {
        QString name;
        QStringList commands;
    };

Q_SIGNALS:
    void tabChanged(int index);

private Q_SLOTS:
    void onTabChanged(int index);

private:
    void setupStyle();
    void clearTabs();
    void populateTab(int index);
    void addCommandButton(QLayout* layout, const char* cmdName);
    void addSeparator(QLayout* layout);

    /// Build tab definitions for PartDesign workbench
    QList<TabDefinition> buildPartDesignTabs() const;
    /// Build tab definitions for Part workbench
    QList<TabDefinition> buildPartTabs() const;
    /// Build generic/fallback tabs from current toolbars
    QList<TabDefinition> buildGenericTabs() const;

    QTabBar* m_tabBar;
    QStackedWidget* m_stack;
    QList<TabDefinition> m_tabs;
    bool m_active;
};

} // namespace Gui

#endif // GUI_FUSIONTABTOOLBAR_H
