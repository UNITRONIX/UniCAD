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
#include <QMenu>
#include <QTimer>
#include <QDebug>
#include <QSizePolicy>

using namespace Gui;

namespace {
const char* kToolButtonStyle = R"(
QToolButton {
  background: transparent;
  border: 1px solid transparent;
  border-radius: 4px;
  padding: 3px;
  color: #CCCCCC;
  font-size: 9px;
  font-weight: bold;
}
QToolButton:hover {
  background: rgba(255,255,255,0.1);
  border: 1px solid rgba(255,255,255,0.12);
}
QToolButton:pressed, QToolButton:checked {
  background: #0696D7;
  border: 1px solid #0580B5;
}
QToolButton::menu-indicator {
  subcontrol-origin: padding;
  subcontrol-position: bottom right;
  width: 8px;
  height: 8px;
}
)";
} // namespace

// ---------------------------------------------------------------------------
// FusionTabToolbar - Fusion 360-style panel ribbon
// ---------------------------------------------------------------------------

FusionTabToolbar::ToolItem FusionTabToolbar::tool(const QString& primary,
                                                   const QStringList& variants)
{
    return {primary, variants};
}

FusionTabToolbar::FusionTabToolbar(QWidget* parent)
    : QWidget(parent)
    , m_workspaceBtn(nullptr)
    , m_tabBar(new QTabBar(this))
    , m_stack(new QStackedWidget(this))
    , m_finishSketchBtn(nullptr)
    , m_active(false)
    , m_inSketchMode(false)
{
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Workspace dropdown (DESIGN) — left of tabs, Fusion layout
    m_workspaceBtn = new QToolButton(this);
    m_workspaceBtn->setText(tr("DESIGN"));
    m_workspaceBtn->setPopupMode(QToolButton::InstantPopup);
    m_workspaceBtn->setToolButtonStyle(Qt::ToolButtonTextOnly);
    m_workspaceBtn->setMinimumHeight(64);
    m_workspaceBtn->setMinimumWidth(88);
    m_workspaceBtn->setStyleSheet(QStringLiteral(
        "QToolButton {"
        "  background: #3A3A3A;"
        "  color: #FFFFFF;"
        "  border: none;"
        "  border-right: 1px solid #1A1A1A;"
        "  padding: 8px 14px;"
        "  font-size: 12px;"
        "  font-weight: bold;"
        "}"
        "QToolButton:hover { background: #454545; }"
        "QToolButton::menu-indicator { subcontrol-position: right center; padding-right: 4px; }"
    ));
    auto* wsMenu = new QMenu(m_workspaceBtn);
    QAction* designAct = wsMenu->addAction(tr("Design"));
    designAct->setEnabled(false);
    designAct->setCheckable(true);
    designAct->setChecked(true);
    m_workspaceBtn->setMenu(wsMenu);
    mainLayout->addWidget(m_workspaceBtn);

    // Tabs + tool panels
    auto* tabsContainer = new QWidget();
    auto* tabsLayout = new QVBoxLayout(tabsContainer);
    tabsLayout->setContentsMargins(0, 0, 0, 0);
    tabsLayout->setSpacing(0);

    m_tabBar->setExpanding(false);
    m_tabBar->setDrawBase(false);
    m_tabBar->setDocumentMode(true);
    m_tabBar->setUsesScrollButtons(true);
    m_tabBar->setMinimumHeight(28);
    tabsLayout->addWidget(m_tabBar);

    m_stack->setMinimumHeight(56);
    m_stack->setMaximumHeight(64);
    tabsLayout->addWidget(m_stack, 1);

    mainLayout->addWidget(tabsContainer, 1);

    // Finish Sketch (contextual)
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
        "QPushButton:hover { background-color: #07A8F0; }"
        "QPushButton:pressed { background-color: #0580B5; }"
    ));
    connect(m_finishSketchBtn, &QPushButton::clicked, this, &FusionTabToolbar::onFinishSketchClicked);
    mainLayout->addWidget(m_finishSketchBtn);

    setMinimumHeight(88);
    setMaximumHeight(96);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    connect(m_tabBar, &QTabBar::currentChanged, this, &FusionTabToolbar::onTabChanged);

    setupStyle();
}

FusionTabToolbar::~FusionTabToolbar() = default;

void FusionTabToolbar::setupStyle()
{
    setObjectName(QStringLiteral("FusionTabToolbar"));

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
        "  padding: 6px 16px;"
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
        "QLabel#FusionPanelLabel {"
        "  color: #888888;"
        "  font-size: 9px;"
        "  font-weight: bold;"
        "}"
    ));
}

void FusionTabToolbar::setActive(bool active)
{
    m_active = active;

    if (active) {
        QTimer::singleShot(200, this, [this]() {
            buildUnifiedTabs();
            setVisible(true);
        });
        QTimer::singleShot(1000, this, [this]() {
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

    if (inSketch && m_tabBar->count() > 0) {
        m_tabBar->setCurrentIndex(0);
        m_tabBar->setStyleSheet(m_tabBar->styleSheet() + QStringLiteral(
            "QTabBar::tab:first-child {"
            "  background: rgba(6, 150, 215, 0.2);"
            "  color: #0696D7;"
            "}"
        ));
    }
    else {
        setupStyle();
    }
}

void FusionTabToolbar::onFinishSketchClicked()
{
    invokeCommand(QStringLiteral("Sketcher_LeaveSketch"));
}

void FusionTabToolbar::onWorkbenchActivated(const QString& workbenchName)
{
    Q_UNUSED(workbenchName)
    int currentTab = m_tabBar->currentIndex();
    if (currentTab < 0) {
        currentTab = 1;
    }

    buildUnifiedTabs();

    if (currentTab >= 0 && currentTab < m_tabBar->count()) {
        m_tabBar->setCurrentIndex(currentTab);
    }
}

void FusionTabToolbar::buildUnifiedTabs()
{
    clearTabs();

    try {
        Base::Interpreter().runString("import PartDesignGui");
        Base::Interpreter().runString("import SketcherGui");
        Base::Interpreter().runString("import PartGui");
        Base::Interpreter().runString("import MeshGui");
    }
    catch (...) {
    }

    m_tabs = {
        buildSketchTab(),
        buildSolidTab(),
        buildModifyTab(),
        buildSurfaceTab(),
        buildSheetMetalTab(),
        buildMeshTab(),
        buildInspectTab(),
        buildToolsTab()
    };

    for (int i = 0; i < m_tabs.size(); ++i) {
        m_tabBar->addTab(m_tabs[i].name);

        auto* scrollArea = new QScrollArea();
        scrollArea->setWidgetResizable(true);
        scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scrollArea->setFrameShape(QFrame::NoFrame);
        scrollArea->setStyleSheet(QStringLiteral("background: transparent; border: none;"));

        auto* page = new QWidget();
        auto* hLayout = new QHBoxLayout(page);
        hLayout->setContentsMargins(4, 2, 4, 2);
        hLayout->setSpacing(0);
        hLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

        populatePage(hLayout, m_tabs[i]);

        hLayout->addStretch();
        scrollArea->setWidget(page);
        m_stack->addWidget(scrollArea);
    }

    if (m_tabBar->count() > 0) {
        m_tabBar->setCurrentIndex(1); // Default SOLID
    }
}

void FusionTabToolbar::populatePage(QLayout* layout, const TabDefinition& tab)
{
    bool first = true;
    for (const auto& panel : tab.panels) {
        if (!first) {
            addSeparator(layout);
        }
        addPanel(layout, panel);
        first = false;
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

bool FusionTabToolbar::commandExists(const char* cmdName) const
{
    if (!cmdName || !cmdName[0]) {
        return false;
    }
    return Application::Instance->commandManager().getCommandByName(cmdName) != nullptr;
}

QIcon FusionTabToolbar::iconForCommand(const char* cmdName) const
{
    Command* cmd = Application::Instance->commandManager().getCommandByName(cmdName);
    if (!cmd) {
        return {};
    }
    const char* pixmap = cmd->getPixmap();
    if (!pixmap || !pixmap[0]) {
        return {};
    }
    QIcon icon = BitmapFactory().iconFromTheme(pixmap);
    if (!icon.isNull()) {
        return icon;
    }
    QPixmap pm = BitmapFactory().pixmap(pixmap);
    if (!pm.isNull()) {
        return QIcon(pm);
    }
    return {};
}

QString FusionTabToolbar::tooltipForCommand(const char* cmdName) const
{
    Command* cmd = Application::Instance->commandManager().getCommandByName(cmdName);
    if (!cmd) {
        return QString::fromLatin1(cmdName);
    }
    QString tooltip = QString::fromUtf8(cmd->getMenuText());
    tooltip.remove(QLatin1Char('&'));
    QString accel = QString::fromUtf8(cmd->getAccel());
    if (!accel.isEmpty()) {
        tooltip += QStringLiteral(" (") + accel + QStringLiteral(")");
    }
    return tooltip;
}

void FusionTabToolbar::invokeCommand(const QString& cmdName)
{
    Command* c = Application::Instance->commandManager().getCommandByName(
        cmdName.toLatin1().constData());
    if (c) {
        c->invoke(0);
    }
}

QToolButton* FusionTabToolbar::createCommandButton(const char* cmdName)
{
    if (!commandExists(cmdName)) {
        return nullptr;
    }

    auto* btn = new QToolButton();
    btn->setToolButtonStyle(Qt::ToolButtonIconOnly);
    btn->setAutoRaise(true);
    btn->setIconSize(QSize(22, 22));
    btn->setFixedSize(34, 34);
    btn->setStyleSheet(QString::fromUtf8(kToolButtonStyle));
    btn->setToolTip(tooltipForCommand(cmdName));

    QIcon icon = iconForCommand(cmdName);
    if (!icon.isNull()) {
        btn->setIcon(icon);
    }
    else {
        QString cmdStr = QString::fromLatin1(cmdName);
        int idx = cmdStr.lastIndexOf(QLatin1Char('_'));
        QString shortName = (idx >= 0) ? cmdStr.mid(idx + 1, 2) : cmdStr.left(2);
        btn->setText(shortName.toUpper());
        btn->setToolButtonStyle(Qt::ToolButtonTextOnly);
    }

    QString cmdNameStr = QString::fromLatin1(cmdName);
    connect(btn, &QToolButton::clicked, this, [this, cmdNameStr]() {
        invokeCommand(cmdNameStr);
    });
    return btn;
}

QToolButton* FusionTabToolbar::createFlyoutButton(const ToolItem& toolItem)
{
    if (!commandExists(toolItem.primary.toLatin1().constData())) {
        // Try first available variant as primary
        for (const auto& v : toolItem.variants) {
            if (commandExists(v.toLatin1().constData())) {
                ToolItem alt = toolItem;
                alt.primary = v;
                alt.variants.removeAll(v);
                return createFlyoutButton(alt);
            }
        }
        return nullptr;
    }

    // Collect available variants (excluding primary)
    QStringList availableVariants;
    for (const auto& v : toolItem.variants) {
        if (v != toolItem.primary && commandExists(v.toLatin1().constData())) {
            availableVariants << v;
        }
    }

    if (availableVariants.isEmpty()) {
        return createCommandButton(toolItem.primary.toLatin1().constData());
    }

    auto* btn = new QToolButton();
    btn->setToolButtonStyle(Qt::ToolButtonIconOnly);
    btn->setAutoRaise(true);
    btn->setIconSize(QSize(22, 22));
    btn->setFixedSize(34, 34);
    btn->setPopupMode(QToolButton::MenuButtonPopup);
    btn->setStyleSheet(QString::fromUtf8(kToolButtonStyle));
    btn->setToolTip(tooltipForCommand(toolItem.primary.toLatin1().constData()));

    QIcon icon = iconForCommand(toolItem.primary.toLatin1().constData());
    if (!icon.isNull()) {
        btn->setIcon(icon);
    }
    else {
        btn->setText(toolItem.primary.section(QLatin1Char('_'), -1).left(2).toUpper());
        btn->setToolButtonStyle(Qt::ToolButtonTextOnly);
    }

    auto* menu = new QMenu(btn);
    QString primaryName = toolItem.primary;
    QAction* primaryAct = menu->addAction(tooltipForCommand(primaryName.toLatin1().constData()));
    primaryAct->setData(primaryName);
    QIcon primaryIcon = iconForCommand(primaryName.toLatin1().constData());
    if (!primaryIcon.isNull()) {
        primaryAct->setIcon(primaryIcon);
    }
    menu->addSeparator();

    for (const auto& v : availableVariants) {
        QAction* act = menu->addAction(tooltipForCommand(v.toLatin1().constData()));
        act->setData(v);
        QIcon vIcon = iconForCommand(v.toLatin1().constData());
        if (!vIcon.isNull()) {
            act->setIcon(vIcon);
        }
    }
    btn->setMenu(menu);

    // Remember last-used command on the button (Fusion-style flyout)
    btn->setProperty("fusionCmd", primaryName);

    connect(btn, &QToolButton::clicked, this, [this, btn]() {
        invokeCommand(btn->property("fusionCmd").toString());
    });
    connect(menu, &QMenu::triggered, this, [this, btn](QAction* act) {
        QString cmd = act->data().toString();
        if (cmd.isEmpty()) {
            return;
        }
        invokeCommand(cmd);
        btn->setProperty("fusionCmd", cmd);
        QIcon ic = iconForCommand(cmd.toLatin1().constData());
        if (!ic.isNull()) {
            btn->setIcon(ic);
        }
        btn->setToolTip(tooltipForCommand(cmd.toLatin1().constData()));
    });

    return btn;
}

void FusionTabToolbar::addPanel(QLayout* layout, const PanelDefinition& panel)
{
    auto* panelWidget = new QWidget();
    auto* panelLayout = new QVBoxLayout(panelWidget);
    panelLayout->setContentsMargins(6, 2, 6, 2);
    panelLayout->setSpacing(1);

    auto* toolsRow = new QWidget();
    auto* toolsLayout = new QHBoxLayout(toolsRow);
    toolsLayout->setContentsMargins(0, 0, 0, 0);
    toolsLayout->setSpacing(2);
    toolsLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    int toolsAdded = 0;
    for (const auto& t : panel.tools) {
        QToolButton* btn = t.variants.isEmpty()
            ? createCommandButton(t.primary.toLatin1().constData())
            : createFlyoutButton(t);
        if (btn) {
            toolsLayout->addWidget(btn);
            ++toolsAdded;
        }
    }

    // Overflow dropdown for remaining commands
    QStringList availableOverflow;
    for (const auto& cmd : panel.overflow) {
        if (commandExists(cmd.toLatin1().constData())) {
            availableOverflow << cmd;
        }
    }

    if (!availableOverflow.isEmpty()) {
        auto* moreBtn = new QToolButton();
        moreBtn->setText(QStringLiteral("▾"));
        moreBtn->setToolButtonStyle(Qt::ToolButtonTextOnly);
        moreBtn->setPopupMode(QToolButton::InstantPopup);
        moreBtn->setAutoRaise(true);
        moreBtn->setFixedSize(22, 34);
        moreBtn->setToolTip(tr("More %1 tools").arg(panel.name));
        moreBtn->setStyleSheet(QString::fromUtf8(kToolButtonStyle));

        auto* menu = new QMenu(moreBtn);
        for (const auto& cmd : availableOverflow) {
            QAction* act = menu->addAction(tooltipForCommand(cmd.toLatin1().constData()));
            act->setData(cmd);
            QIcon ic = iconForCommand(cmd.toLatin1().constData());
            if (!ic.isNull()) {
                act->setIcon(ic);
            }
        }
        moreBtn->setMenu(menu);
        connect(menu, &QMenu::triggered, this, [this](QAction* act) {
            invokeCommand(act->data().toString());
        });
        toolsLayout->addWidget(moreBtn);
        ++toolsAdded;
    }

    if (toolsAdded == 0) {
        delete panelWidget;
        return;
    }

    panelLayout->addWidget(toolsRow, 0, Qt::AlignHCenter);

    auto* labelRow = new QWidget();
    auto* labelLayout = new QHBoxLayout(labelRow);
    labelLayout->setContentsMargins(0, 0, 0, 0);
    labelLayout->setSpacing(2);
    labelLayout->setAlignment(Qt::AlignHCenter);

    auto* lbl = new QLabel(panel.name);
    lbl->setObjectName(QStringLiteral("FusionPanelLabel"));
    lbl->setAlignment(Qt::AlignCenter);
    labelLayout->addWidget(lbl);
    panelLayout->addWidget(labelRow);

    layout->addWidget(panelWidget);
}

void FusionTabToolbar::addSeparator(QLayout* layout)
{
    auto* line = new QFrame();
    line->setFrameShape(QFrame::VLine);
    line->setFrameShadow(QFrame::Plain);
    line->setFixedWidth(1);
    line->setFixedHeight(48);
    line->setStyleSheet(QStringLiteral("background: #444444;"));
    layout->addWidget(line);
}

// ---------------------------------------------------------------------------
// Tab / panel definitions — one function per visible icon; variants in flyouts
// ---------------------------------------------------------------------------

FusionTabToolbar::TabDefinition FusionTabToolbar::buildSketchTab() const
{
    PanelDefinition create{
        tr("CREATE"),
        {
            tool(QStringLiteral("PartDesign_NewSketch")),
            tool(QStringLiteral("Sketcher_CreateLine")),
            tool(QStringLiteral("Sketcher_CreateRectangle"),
                 {QStringLiteral("Sketcher_CreateOblong")}),
            tool(QStringLiteral("Sketcher_CreateCircle"),
                 {QStringLiteral("Sketcher_Create3PointCircle")}),
            tool(QStringLiteral("Sketcher_CreateArc"),
                 {QStringLiteral("Sketcher_Create3PointArc")}),
            tool(QStringLiteral("Sketcher_CreatePolyline")),
        },
        {
            QStringLiteral("Sketcher_CreatePoint"),
            QStringLiteral("Sketcher_CreateEllipseByCenter"),
            QStringLiteral("Sketcher_CreateSlot"),
            QStringLiteral("Sketcher_CreateRegularPolygon"),
            QStringLiteral("Sketcher_CreateBSpline"),
        }
    };

    PanelDefinition constrain{
        tr("CONSTRAIN"),
        {
            tool(QStringLiteral("Sketcher_ConstrainCoincident")),
            tool(QStringLiteral("Sketcher_ConstrainHorizontal")),
            tool(QStringLiteral("Sketcher_ConstrainVertical")),
            tool(QStringLiteral("Sketcher_ConstrainParallel")),
            tool(QStringLiteral("Sketcher_ConstrainPerpendicular")),
            tool(QStringLiteral("Sketcher_ConstrainDistance"),
                 {QStringLiteral("Sketcher_ConstrainDistanceX"),
                  QStringLiteral("Sketcher_ConstrainDistanceY")}),
        },
        {
            QStringLiteral("Sketcher_ConstrainPointOnObject"),
            QStringLiteral("Sketcher_ConstrainTangent"),
            QStringLiteral("Sketcher_ConstrainEqual"),
            QStringLiteral("Sketcher_ConstrainSymmetric"),
            QStringLiteral("Sketcher_ConstrainLock"),
            QStringLiteral("Sketcher_ConstrainRadius"),
            QStringLiteral("Sketcher_ConstrainDiameter"),
            QStringLiteral("Sketcher_ConstrainAngle"),
        }
    };

    PanelDefinition modify{
        tr("MODIFY"),
        {
            tool(QStringLiteral("Sketcher_Trimming")),
            tool(QStringLiteral("Sketcher_Extend")),
            tool(QStringLiteral("Sketcher_Offset")),
            tool(QStringLiteral("Sketcher_Move")),
        },
        {
            QStringLiteral("Sketcher_Split"),
            QStringLiteral("Sketcher_External"),
            QStringLiteral("Sketcher_CarbonCopy"),
            QStringLiteral("Sketcher_RectangularArray"),
        }
    };

    return {tr("SKETCH"), {create, constrain, modify}};
}

FusionTabToolbar::TabDefinition FusionTabToolbar::buildSolidTab() const
{
    PanelDefinition create{
        tr("CREATE"),
        {
            tool(QStringLiteral("PartDesign_NewSketch")),
            tool(QStringLiteral("PartDesign_Extrude")),
            tool(QStringLiteral("PartDesign_Revolve")),
            tool(QStringLiteral("PartDesign_Hole")),
            tool(QStringLiteral("PartDesign_LinearPattern"),
                 {QStringLiteral("PartDesign_PolarPattern"),
                  QStringLiteral("PartDesign_Mirrored"),
                  QStringLiteral("PartDesign_MultiTransform")}),
        },
        {
            QStringLiteral("PartDesign_Body"),
            QStringLiteral("PartDesign_Sweep"),
            QStringLiteral("PartDesign_Loft"),
            QStringLiteral("PartDesign_AdditiveHelix"),
            QStringLiteral("PartDesign_Emboss"),
            QStringLiteral("PartDesign_AdditiveBox"),
            QStringLiteral("PartDesign_AdditiveCylinder"),
            QStringLiteral("PartDesign_AdditiveSphere"),
        }
    };

    PanelDefinition modify{
        tr("MODIFY"),
        {
            tool(QStringLiteral("PartDesign_Fillet")),
            tool(QStringLiteral("PartDesign_Chamfer")),
            tool(QStringLiteral("PartDesign_Thickness")),
            tool(QStringLiteral("PartDesign_Boolean")),
            tool(QStringLiteral("Std_TransformManip")),
            tool(QStringLiteral("PartDesign_OffsetFace")),
        },
        {
            QStringLiteral("PartDesign_Draft"),
            QStringLiteral("PartDesign_DeleteFace"),
            QStringLiteral("PartDesign_MoveFace"),
            QStringLiteral("PartDesign_SplitFace"),
            QStringLiteral("PartDesign_ReplaceFace"),
        }
    };

    PanelDefinition construct{
        tr("CONSTRUCT"),
        {
            tool(QStringLiteral("PartDesign_Plane"),
                 {QStringLiteral("PartDesign_Line"),
                  QStringLiteral("PartDesign_Point"),
                  QStringLiteral("PartDesign_CoordinateSystem")}),
        },
        {}
    };

    PanelDefinition inspect{
        tr("INSPECT"),
        {
            tool(QStringLiteral("Std_MeasureDistance"),
                 {QStringLiteral("Part_Measure_Linear"),
                  QStringLiteral("Part_Measure_Angular"),
                  QStringLiteral("Part_CheckGeometry")}),
        },
        {}
    };

    PanelDefinition select{
        tr("SELECT"),
        {
            tool(QStringLiteral("Std_BoxSelection"),
                 {QStringLiteral("Std_BoxElementSelection"),
                  QStringLiteral("Std_SelectAll"),
                  QStringLiteral("Std_SelectVisibleObjects")}),
        },
        {}
    };

    return {tr("SOLID"), {create, modify, construct, inspect, select}};
}

FusionTabToolbar::TabDefinition FusionTabToolbar::buildModifyTab() const
{
    // Transform / organize only — no Fillet/Chamfer (those live on SOLID)
    PanelDefinition transform{
        tr("TRANSFORM"),
        {
            tool(QStringLiteral("Std_TransformManip")),
            tool(QStringLiteral("Std_Placement")),
            tool(QStringLiteral("Std_Alignment")),
        },
        {
            QStringLiteral("Std_Transform"),
        }
    };

    PanelDefinition organize{
        tr("ORGANIZE"),
        {
            tool(QStringLiteral("Std_Part")),
            tool(QStringLiteral("Std_Group")),
            tool(QStringLiteral("Std_LinkMake")),
        },
        {
            QStringLiteral("Std_MergeToContainer"),
            QStringLiteral("Std_Copy"),
            QStringLiteral("Std_Paste"),
            QStringLiteral("Std_DuplicateSelection"),
        }
    };

    PanelDefinition scale{
        tr("SCALE"),
        {
            tool(QStringLiteral("Part_Scale")),
            tool(QStringLiteral("Part_Mirror")),
        },
        {
            QStringLiteral("Part_Offset"),
            QStringLiteral("Part_Offset3D"),
            QStringLiteral("Part_Fuse"),
            QStringLiteral("Part_Cut"),
            QStringLiteral("Part_Common"),
        }
    };

    return {tr("MODIFY"), {transform, organize, scale}};
}

FusionTabToolbar::TabDefinition FusionTabToolbar::buildSurfaceTab() const
{
    // Surface tools only — no PartDesign Extrude duplication
    PanelDefinition create{
        tr("CREATE"),
        {
            tool(QStringLiteral("Surface_Filling")),
            tool(QStringLiteral("Surface_GeomFillSurface")),
            tool(QStringLiteral("Surface_Sections")),
            tool(QStringLiteral("Part_RuledSurface")),
        },
        {
            QStringLiteral("Surface_ExtendFace"),
            QStringLiteral("Part_Loft"),
            QStringLiteral("Part_Sweep"),
            QStringLiteral("Part_Revolve"),
        }
    };

    PanelDefinition modify{
        tr("MODIFY"),
        {
            tool(QStringLiteral("Part_Offset")),
            tool(QStringLiteral("Part_Thickness")),
        },
        {
            QStringLiteral("Surface_CurveOnMesh"),
        }
    };

    return {tr("SURFACE"), {create, modify}};
}

FusionTabToolbar::TabDefinition FusionTabToolbar::buildSheetMetalTab() const
{
    PanelDefinition create{
        tr("CREATE"),
        {
            tool(QStringLiteral("SheetMetal_AddBase")),
            tool(QStringLiteral("SheetMetal_AddWall")),
            tool(QStringLiteral("SheetMetal_AddFoldWall")),
            tool(QStringLiteral("SheetMetal_Unfold")),
        },
        {
            QStringLiteral("SheetMetal_AddCornerRelief"),
            QStringLiteral("SheetMetal_AddRelief"),
            QStringLiteral("SheetMetal_AddJunction"),
            QStringLiteral("SheetMetal_AddBend"),
            QStringLiteral("SheetMetal_SketchOnSheet"),
            QStringLiteral("SheetMetal_Forming"),
        }
    };

    return {tr("SHEET METAL"), {create}};
}

FusionTabToolbar::TabDefinition FusionTabToolbar::buildMeshTab() const
{
    PanelDefinition create{
        tr("CREATE"),
        {
            tool(QStringLiteral("Mesh_Import")),
            tool(QStringLiteral("Mesh_FromPartShape")),
            tool(QStringLiteral("Mesh_Export")),
        },
        {
            QStringLiteral("Mesh_RemeshGmsh"),
        }
    };

    PanelDefinition modify{
        tr("MODIFY"),
        {
            tool(QStringLiteral("Mesh_Smoothing")),
            tool(QStringLiteral("Mesh_Decimating")),
            tool(QStringLiteral("Mesh_FillupHoles")),
            tool(QStringLiteral("Mesh_HarmonizeNormals")),
        },
        {
            QStringLiteral("Mesh_FlipNormals"),
            QStringLiteral("Mesh_FillInteractiveHole"),
            QStringLiteral("Mesh_RemoveComponents"),
            QStringLiteral("Mesh_VertexCurvature"),
            QStringLiteral("Mesh_BoundingBox"),
            QStringLiteral("Mesh_Evaluation"),
        }
    };

    return {tr("MESH"), {create, modify}};
}

FusionTabToolbar::TabDefinition FusionTabToolbar::buildInspectTab() const
{
    PanelDefinition measure{
        tr("MEASURE"),
        {
            tool(QStringLiteral("Std_MeasureDistance")),
            tool(QStringLiteral("Part_Measure_Linear")),
            tool(QStringLiteral("Part_Measure_Angular")),
        },
        {
            QStringLiteral("Std_MeasureToggleAll"),
            QStringLiteral("Part_Measure_Refresh"),
            QStringLiteral("Part_Measure_Clear_All"),
        }
    };

    PanelDefinition analyze{
        tr("ANALYZE"),
        {
            tool(QStringLiteral("Part_CheckGeometry")),
            tool(QStringLiteral("Sketcher_ValidateSketch")),
            tool(QStringLiteral("Part_SectionCut")),
        },
        {}
    };

    return {tr("INSPECT"), {measure, analyze}};
}

FusionTabToolbar::TabDefinition FusionTabToolbar::buildToolsTab() const
{
    PanelDefinition file{
        tr("FILE"),
        {
            tool(QStringLiteral("Std_New")),
            tool(QStringLiteral("Std_Open")),
            tool(QStringLiteral("Std_Save")),
        },
        {
            QStringLiteral("Std_SaveAs"),
            QStringLiteral("Std_Import"),
            QStringLiteral("Std_Export"),
        }
    };

    PanelDefinition edit{
        tr("EDIT"),
        {
            tool(QStringLiteral("Std_Undo")),
            tool(QStringLiteral("Std_Redo")),
        },
        {}
    };

    PanelDefinition settings{
        tr("SETTINGS"),
        {
            tool(QStringLiteral("Std_DlgPreferences")),
            tool(QStringLiteral("Std_DlgMacroExecute")),
        },
        {
            QStringLiteral("Std_DlgParameter"),
            QStringLiteral("Std_DlgMacroRecord"),
            QStringLiteral("Std_DlgMacroExecuteDirect"),
            QStringLiteral("Std_ViewFitAll"),
            QStringLiteral("Std_ViewHome"),
        }
    };

    return {tr("TOOLS"), {file, edit, settings}};
}

#include "moc_FusionTabToolbar.cpp"
