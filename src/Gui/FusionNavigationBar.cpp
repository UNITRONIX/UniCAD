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

#include "FusionNavigationBar.h"
#include "Application.h"
#include "Command.h"
#include "BitmapFactory.h"

#include <QLabel>
#include <QFrame>

using namespace Gui;

// ---------------------------------------------------------------------------
// FusionNavigationBar
// ---------------------------------------------------------------------------

FusionNavigationBar::FusionNavigationBar(QWidget* parent)
    : QToolBar(tr("Navigation Bar"), parent)
{
    setObjectName(QStringLiteral("FusionNavigationBar"));
    setMovable(false);
    setFloatable(false);
    setIconSize(QSize(20, 20));

    setupStyle();
    setupActions();
}

FusionNavigationBar::~FusionNavigationBar() = default;

void FusionNavigationBar::setupStyle()
{
    setStyleSheet(QStringLiteral(
        "QToolBar {"
        "  background: #1E1E1E;"
        "  border: none;"
        "  padding: 2px 8px;"
        "  spacing: 4px;"
        "}"
        "QToolButton {"
        "  background: transparent;"
        "  color: #AAAAAA;"
        "  border: none;"
        "  border-radius: 3px;"
        "  padding: 4px 8px;"
        "  font-size: 10px;"
        "}"
        "QToolButton:hover {"
        "  background: #3D3D3D;"
        "  color: #FFFFFF;"
        "}"
        "QToolButton:pressed {"
        "  background: #0696D7;"
        "  color: #FFFFFF;"
        "}"
        "QLabel {"
        "  color: #666666;"
        "  font-size: 10px;"
        "  padding: 0px 4px;"
        "}"
    ));
}

void FusionNavigationBar::setupActions()
{
    // View presets section
    addNavButton("Std_ViewFitAll",    tr("Fit"),       QStringLiteral("view-refresh"));
    addNavButton("Std_ViewHome",      tr("Home"),      QStringLiteral("go-home"));

    addSeparator();

    // Standard views
    addNavButton("Std_ViewFront",     tr("Front"),     QStringLiteral("view-front"));
    addNavButton("Std_ViewTop",       tr("Top"),       QStringLiteral("view-top"));
    addNavButton("Std_ViewRight",     tr("Right"),     QStringLiteral("view-right"));
    addNavButton("Std_ViewIsometric", tr("ISO"),       QStringLiteral("view-axonometric"));

    addSeparator();

    // Display style
    addNavButton("Std_DrawStyleAsIs",       tr("Shaded"), QStringLiteral("DrawStyleAsIs"));
    addNavButton("Std_DrawStyleWireFrame",  tr("Wire"),   QStringLiteral("DrawStyleWireFrame"));

    addSeparator();

    // Toggle tools
    addNavButton("Std_ToggleVisibility", tr("Show/Hide"), QStringLiteral("Std_ToggleVisibility"));
    addNavButton("Std_PerspectiveCamera", tr("Persp."),   QStringLiteral("view-perspective"));

    // Right-side spacer + label
    auto* spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    addWidget(spacer);

    // FusionCAD branding label on right side
    auto* brandLabel = new QLabel(QStringLiteral("FusionCAD"));
    brandLabel->setStyleSheet(QStringLiteral(
        "color: #444444; font-size: 9px; font-weight: bold; padding-right: 8px;"
    ));
    addWidget(brandLabel);
}

void FusionNavigationBar::addNavButton(const char* cmdName, const QString& label,
                                        const QString& /*iconTheme*/)
{
    auto& mgr = Application::Instance->commandManager();
    Command* cmd = mgr.getCommandByName(cmdName);

    auto* btn = new QToolButton();
    btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btn->setAutoRaise(true);
    btn->setText(label);
    btn->setIconSize(QSize(18, 18));

    if (cmd) {
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

        connect(btn, &QToolButton::clicked, [cmdName, &mgr]() {
            Command* c = mgr.getCommandByName(cmdName);
            if (c) {
                c->invoke(0);
            }
        });
    }
    else {
        btn->setToolTip(label);
        btn->setEnabled(false);
    }

    addWidget(btn);
}

#include "moc_FusionNavigationBar.cpp"
