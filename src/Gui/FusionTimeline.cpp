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

#include "FusionTimeline.h"
#include "Application.h"
#include "Document.h"
#include "BitmapFactory.h"
#include "ViewProvider.h"
#include "Selection/Selection.h"

#include <App/Document.h>
#include <App/DocumentObject.h>

#include <QFrame>
#include <QPainter>
#include <QMouseEvent>

using namespace Gui;

// ---------------------------------------------------------------------------
// FusionTimeline
// ---------------------------------------------------------------------------

FusionTimeline::FusionTimeline(QWidget* parent)
    : QDockWidget(tr("Timeline"), parent)
    , m_scrollArea(new QScrollArea(this))
    , m_container(new QWidget())
    , m_layout(new QHBoxLayout(m_container))
{
    setObjectName(QStringLiteral("FusionTimeline"));
    setAllowedAreas(Qt::BottomDockWidgetArea);
    setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);

    // Setup scroll area
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setWidget(m_container);

    // Layout
    m_layout->setContentsMargins(8, 2, 8, 2);
    m_layout->setSpacing(4);
    m_layout->addStretch();

    setWidget(m_scrollArea);
    setupStyle();

    setMinimumHeight(48);
    setMaximumHeight(56);
}

FusionTimeline::~FusionTimeline() = default;

void FusionTimeline::setupStyle()
{
    setStyleSheet(QStringLiteral(
        "QDockWidget {"
        "  background-color: #252525;"
        "  color: #CCCCCC;"
        "  font-size: 11px;"
        "  border: none;"
        "}"
        "QDockWidget::title {"
        "  background: #1E1E1E;"
        "  padding: 4px;"
        "  font-weight: bold;"
        "  color: #888888;"
        "  text-align: left;"
        "}"
        "QScrollArea {"
        "  background: #252525;"
        "  border: none;"
        "}"
    ));
}

void FusionTimeline::clearTimeline()
{
    // Remove all chip buttons
    for (auto& chip : m_chips) {
        m_layout->removeWidget(chip.button);
        delete chip.button;
    }
    m_chips.clear();

    // Remove the stretch
    QLayoutItem* item;
    while ((item = m_layout->takeAt(0)) != nullptr) {
        delete item;
    }
}

void FusionTimeline::refresh()
{
    clearTimeline();

    Gui::Document* guiDoc = Application::Instance->activeDocument();
    if (!guiDoc) {
        m_layout->addStretch();
        return;
    }

    App::Document* appDoc = guiDoc->getDocument();
    if (!appDoc) {
        m_layout->addStretch();
        return;
    }

    // Show all document objects in the timeline
    // Simple flat list of all objects
    auto objects = appDoc->getObjects();
    int index = 0;
    int totalCount = static_cast<int>(objects.size());
    
    for (auto* obj : objects) {
        if (!obj || !obj->getNameInDocument()) {
            continue;
        }
        
        // The last object is considered the "tip" (current feature)
        bool isTip = (index == totalCount - 1);
        addFeatureChip(obj, index, isTip);
        ++index;
    }

    m_layout->addStretch();
}

void FusionTimeline::addFeatureChip(App::DocumentObject* obj, int index, bool isCurrent)
{
    auto* btn = new QToolButton();
    btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    btn->setAutoRaise(true);
    btn->setIconSize(QSize(20, 20));

    // Get the viewprovider icon
    Gui::Document* guiDoc = Application::Instance->activeDocument();
    if (guiDoc) {
        Gui::ViewProvider* vp = guiDoc->getViewProvider(obj);
        if (vp) {
            btn->setIcon(vp->getIcon());
        }
    }

    // Feature label  
    QString label = QString::fromUtf8(obj->Label.getValue());
    if (label.length() > 16) {
        label = label.left(14) + QStringLiteral("..");
    }
    btn->setText(label);
    btn->setToolTip(QString::fromUtf8(obj->Label.getValue())
                    + QStringLiteral(" [")
                    + QString::fromLatin1(obj->getNameInDocument())
                    + QStringLiteral("]"));

    // Style for feature chips
    QString chipStyle;
    if (isCurrent) {
        // Current/tip feature: blue highlight
        chipStyle = QStringLiteral(
            "QToolButton {"
            "  background: #0696D7;"
            "  color: #FFFFFF;"
            "  border: none;"
            "  border-radius: 4px;"
            "  padding: 2px 6px;"
            "  font-size: 10px;"
            "  font-weight: bold;"
            "}"
            "QToolButton:hover {"
            "  background: #0AA8EE;"
            "}"
        );
    }
    else {
        chipStyle = QStringLiteral(
            "QToolButton {"
            "  background: #3A3A3A;"
            "  color: #BBBBBB;"
            "  border: none;"
            "  border-radius: 4px;"
            "  padding: 2px 6px;"
            "  font-size: 10px;"
            "}"
            "QToolButton:hover {"
            "  background: #4A4A4A;"
            "  color: #FFFFFF;"
            "}"
        );
    }
    btn->setStyleSheet(chipStyle);

    // Store chip info
    FeatureChip chip;
    chip.button = btn;
    chip.objectName = QString::fromLatin1(obj->getNameInDocument());
    chip.index = index;
    m_chips.append(chip);

    // Connect click
    connect(btn, &QToolButton::clicked, this, &FusionTimeline::onFeatureClicked);

    // Add connector line between chips (visual timeline line)
    if (m_layout->count() > 0) {
        auto* connector = new QFrame();
        connector->setFrameShape(QFrame::HLine);
        connector->setFixedWidth(12);
        connector->setFixedHeight(2);
        connector->setStyleSheet(QStringLiteral("background: #555555;"));
        m_layout->addWidget(connector);
    }

    m_layout->addWidget(btn);
}

void FusionTimeline::onFeatureClicked()
{
    auto* btn = qobject_cast<QToolButton*>(sender());
    if (!btn) {
        return;
    }

    // Find the chip
    for (const auto& chip : m_chips) {
        if (chip.button == btn) {
            Q_EMIT featureSelected(chip.objectName);

            // Select the feature in the document
            Gui::Document* guiDoc = Application::Instance->activeDocument();
            if (guiDoc) {
                App::Document* appDoc = guiDoc->getDocument();
                if (appDoc) {
                    Selection().clearSelection();
                    Selection().addSelection(appDoc->getName(),
                                             chip.objectName.toLatin1().constData());
                }
            }
            break;
        }
    }
}

#include "moc_FusionTimeline.cpp"
