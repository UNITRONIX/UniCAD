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
#include <QList>

#include <FCGlobal.h>

namespace Gui {

class CommandManager;

/**
 * FusionTabToolbar provides a Fusion 360-style unified tabbed toolbar.
 *
 * Tabs (SKETCH, SOLID, …) organize commands into labeled panels with a
 * small set of primary icons and overflow / variant flyout menus —
 * matching Fusion's CREATE / MODIFY / CONSTRUCT density.
 */
class GuiExport FusionTabToolbar : public QWidget
{
    Q_OBJECT

public:
    /// One visible tool; optional variants appear in a flyout menu.
    struct ToolItem {
        QString primary;
        QStringList variants;
    };

    /// Named panel with primary tools and overflow menu commands.
    struct PanelDefinition {
        QString name;
        QList<ToolItem> tools;
        QStringList overflow;
    };

    /// Tab containing Fusion-style panels.
    struct TabDefinition {
        QString name;
        QList<PanelDefinition> panels;
    };

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

Q_SIGNALS:
    void tabChanged(int index);

private Q_SLOTS:
    void onTabChanged(int index);
    void onFinishSketchClicked();

private:
    void setupStyle();
    void clearTabs();
    void buildUnifiedTabs();
    void populatePage(QLayout* layout, const TabDefinition& tab);

    QIcon iconForCommand(const char* cmdName) const;
    QString tooltipForCommand(const char* cmdName) const;
    void invokeCommand(const QString& cmdName);
    bool commandExists(const char* cmdName) const;

    QToolButton* createCommandButton(const char* cmdName);
    QToolButton* createFlyoutButton(const ToolItem& tool);
    void addPanel(QLayout* layout, const PanelDefinition& panel);
    void addSeparator(QLayout* layout);

    TabDefinition buildSketchTab() const;
    TabDefinition buildSolidTab() const;
    TabDefinition buildModifyTab() const;
    TabDefinition buildSurfaceTab() const;
    TabDefinition buildSheetMetalTab() const;
    TabDefinition buildMeshTab() const;
    TabDefinition buildInspectTab() const;
    TabDefinition buildRenderTab() const;
    TabDefinition buildToolsTab() const;

    static ToolItem tool(const QString& primary,
                         const QStringList& variants = QStringList());

    QToolButton* m_workspaceBtn;
    QTabBar* m_tabBar;
    QStackedWidget* m_stack;
    QList<TabDefinition> m_tabs;
    QPushButton* m_finishSketchBtn;
    bool m_active;
    bool m_inSketchMode;
};

} // namespace Gui

#endif // GUI_FUSIONTABTOOLBAR_H
