/***************************************************************************
 *   Copyright (c) 2025 UNITRONIX                                         *
 *   UniCAD - A fork of FreeCAD                                           *
 *                                                                         *
 *   This file is part of UniCAD.                                          *
 *                                                                         *
 *   UniCAD is free software; you can redistribute it and/or modify        *
 *   it under the terms of the GNU Lesser General Public License (LGPL)    *
 *   as published by the Free Software Foundation; either version 2.1 of   *
 *   the License, or (at your option) any later version.                   *
 ***************************************************************************/

#include "PreCompiled.h"

#include "FusionSelectionBar.h"
#include "BitmapFactory.h"
#include "Selection.h"

#include <QHBoxLayout>
#include <QAction>
#include <cstring>

using namespace Gui;

// ---------------------------------------------------------------------------
// FusionSelectionGate - Custom selection gate for geometry type filtering
// ---------------------------------------------------------------------------

class FusionSelectionGate : public SelectionGate
{
public:
    explicit FusionSelectionGate(FusionSelectionBar::FilterTypes filters)
        : m_filters(filters)
    {
    }

    bool allow(App::Document* /*doc*/, App::DocumentObject* /*obj*/, const char* subName) override
    {
        // If no filters are set, allow everything
        if (m_filters == FusionSelectionBar::None || m_filters == FusionSelectionBar::All) {
            return true;
        }

        // If no subname, this is a whole object selection
        if (!subName || subName[0] == '\0') {
            // Allow if Bodies or Features filter is active
            return m_filters.testFlag(FusionSelectionBar::Bodies) 
                || m_filters.testFlag(FusionSelectionBar::Features);
        }

        // Check subelement type based on prefix
        // FreeCAD uses prefixes like "Vertex", "Edge", "Face" for subelements
        if (strncmp(subName, "Vertex", 6) == 0) {
            if (!m_filters.testFlag(FusionSelectionBar::Vertices)) {
                notAllowedReason = QT_TR_NOOP("Vertex selection is filtered out");
                return false;
            }
        }
        else if (strncmp(subName, "Edge", 4) == 0) {
            if (!m_filters.testFlag(FusionSelectionBar::Edges)) {
                notAllowedReason = QT_TR_NOOP("Edge selection is filtered out");
                return false;
            }
        }
        else if (strncmp(subName, "Face", 4) == 0) {
            if (!m_filters.testFlag(FusionSelectionBar::Faces)) {
                notAllowedReason = QT_TR_NOOP("Face selection is filtered out");
                return false;
            }
        }
        else {
            // Unknown subelement type - check Bodies/Features filter
            if (!m_filters.testFlag(FusionSelectionBar::Bodies) 
                && !m_filters.testFlag(FusionSelectionBar::Features)) {
                notAllowedReason = QT_TR_NOOP("Object selection is filtered out");
                return false;
            }
        }

        return true;
    }

private:
    FusionSelectionBar::FilterTypes m_filters;
};

// ---------------------------------------------------------------------------
// FusionSelectionBar
// ---------------------------------------------------------------------------

FusionSelectionBar::FusionSelectionBar(QWidget* parent)
    : QToolBar(parent)
    , m_activeFilters(None)
    , m_filterGroup(nullptr)
    , m_verticesBtn(nullptr)
    , m_edgesBtn(nullptr)
    , m_facesBtn(nullptr)
    , m_bodiesBtn(nullptr)
    , m_featuresBtn(nullptr)
    , m_clearBtn(nullptr)
    , m_selectionLabel(nullptr)
{
    setObjectName(QStringLiteral("FusionSelectionBar"));
    setWindowTitle(tr("Selection Filters"));
    
    setupUI();
}

FusionSelectionBar::~FusionSelectionBar() = default;

void FusionSelectionBar::setupUI()
{
    // Dark Fusion 360-style appearance
    setStyleSheet(QStringLiteral(
        "QToolBar#FusionSelectionBar {"
        "    background: #2D2D2D;"
        "    border: none;"
        "    spacing: 2px;"
        "    padding: 2px 8px;"
        "}"
        "QToolButton {"
        "    background: transparent;"
        "    border: 1px solid transparent;"
        "    border-radius: 3px;"
        "    padding: 4px;"
        "    margin: 1px;"
        "}"
        "QToolButton:hover {"
        "    background: #3D3D3D;"
        "    border: 1px solid #505050;"
        "}"
        "QToolButton:checked {"
        "    background: #0696D7;"
        "    border: 1px solid #0696D7;"
        "}"
        "QToolButton:pressed {"
        "    background: #0580B8;"
        "}"
        "QLabel {"
        "    color: #B0B0B0;"
        "    font-size: 11px;"
        "    padding: 0 8px;"
        "}"
    ));

    // Selection label
    m_selectionLabel = new QLabel(tr("Selection Filter:"), this);
    addWidget(m_selectionLabel);
    
    addSeparator();

    // Filter button group (allows multiple selection)
    m_filterGroup = new QButtonGroup(this);
    m_filterGroup->setExclusive(false);
    
    // Create filter buttons
    m_verticesBtn = createFilterButton(QStringLiteral("Constraint_PointOnPoint"),
                                        tr("Vertices (Points)"), Vertices);
    m_edgesBtn = createFilterButton(QStringLiteral("Part_Line_Edge"),
                                     tr("Edges (Lines, Curves)"), Edges);
    m_facesBtn = createFilterButton(QStringLiteral("Draft_SelectPlane"),
                                     tr("Faces (Surfaces)"), Faces);
    m_bodiesBtn = createFilterButton(QStringLiteral("Part_Box"),
                                      tr("Bodies (Solids)"), Bodies);
    m_featuresBtn = createFilterButton(QStringLiteral("Tree_Part"),
                                        tr("Features"), Features);
    
    // Add buttons to toolbar and group
    addWidget(m_verticesBtn);
    m_filterGroup->addButton(m_verticesBtn, static_cast<int>(Vertices));
    
    addWidget(m_edgesBtn);
    m_filterGroup->addButton(m_edgesBtn, static_cast<int>(Edges));
    
    addWidget(m_facesBtn);
    m_filterGroup->addButton(m_facesBtn, static_cast<int>(Faces));
    
    addWidget(m_bodiesBtn);
    m_filterGroup->addButton(m_bodiesBtn, static_cast<int>(Bodies));
    
    addWidget(m_featuresBtn);
    m_filterGroup->addButton(m_featuresBtn, static_cast<int>(Features));
    
    addSeparator();
    
    // Clear filters button
    m_clearBtn = new QToolButton(this);
    m_clearBtn->setIcon(BitmapFactory().iconFromTheme("edit-clear"));
    m_clearBtn->setToolTip(tr("Clear All Filters"));
    m_clearBtn->setAutoRaise(true);
    addWidget(m_clearBtn);
    
    // Connect signals
    connect(m_filterGroup, &QButtonGroup::idToggled,
            this, &FusionSelectionBar::onFilterButtonToggled);
    connect(m_clearBtn, &QToolButton::clicked,
            this, &FusionSelectionBar::onClearFilters);
}

QToolButton* FusionSelectionBar::createFilterButton(const QString& iconName,
                                                     const QString& tooltip,
                                                     FilterType filter)
{
    Q_UNUSED(filter)
    
    auto* btn = new QToolButton(this);
    btn->setCheckable(true);
    btn->setAutoRaise(true);
    btn->setToolTip(tooltip);
    
    // Try to load icon
    QIcon icon = BitmapFactory().iconFromTheme(iconName.toLatin1().constData());
    if (!icon.isNull()) {
        btn->setIcon(icon);
        btn->setIconSize(QSize(20, 20));
    }
    else {
        // Fallback to text if icon not found
        btn->setText(tooltip.left(1));
    }
    
    return btn;
}

void FusionSelectionBar::onFilterButtonToggled(int id, bool checked)
{
    auto filter = static_cast<FilterType>(id);
    
    if (checked) {
        m_activeFilters |= filter;
    }
    else {
        m_activeFilters &= ~filter;
    }
    
    updateFilterState();
    applySelectionFilter();
    
    Q_EMIT filtersChanged(m_activeFilters);
}

void FusionSelectionBar::onClearFilters()
{
    clearFilters();
}

void FusionSelectionBar::setActiveFilters(FilterTypes filters)
{
    if (m_activeFilters == filters) {
        return;
    }
    
    m_activeFilters = filters;
    
    // Update button states
    m_verticesBtn->setChecked(filters.testFlag(Vertices));
    m_edgesBtn->setChecked(filters.testFlag(Edges));
    m_facesBtn->setChecked(filters.testFlag(Faces));
    m_bodiesBtn->setChecked(filters.testFlag(Bodies));
    m_featuresBtn->setChecked(filters.testFlag(Features));
    
    updateFilterState();
    applySelectionFilter();
    
    Q_EMIT filtersChanged(m_activeFilters);
}

bool FusionSelectionBar::isFilterActive(FilterType filter) const
{
    return m_activeFilters.testFlag(filter);
}

void FusionSelectionBar::toggleFilter(FilterType filter)
{
    if (m_activeFilters.testFlag(filter)) {
        m_activeFilters &= ~filter;
    }
    else {
        m_activeFilters |= filter;
    }
    
    // Update corresponding button
    switch (filter) {
        case Vertices: m_verticesBtn->setChecked(m_activeFilters.testFlag(Vertices)); break;
        case Edges: m_edgesBtn->setChecked(m_activeFilters.testFlag(Edges)); break;
        case Faces: m_facesBtn->setChecked(m_activeFilters.testFlag(Faces)); break;
        case Bodies: m_bodiesBtn->setChecked(m_activeFilters.testFlag(Bodies)); break;
        case Features: m_featuresBtn->setChecked(m_activeFilters.testFlag(Features)); break;
        default: break;
    }
    
    updateFilterState();
    applySelectionFilter();
    
    Q_EMIT filtersChanged(m_activeFilters);
}

void FusionSelectionBar::clearFilters()
{
    m_activeFilters = None;
    
    // Uncheck all buttons
    m_verticesBtn->setChecked(false);
    m_edgesBtn->setChecked(false);
    m_facesBtn->setChecked(false);
    m_bodiesBtn->setChecked(false);
    m_featuresBtn->setChecked(false);
    
    updateFilterState();
    applySelectionFilter();
    
    Q_EMIT filtersChanged(m_activeFilters);
}

void FusionSelectionBar::updateFilterState()
{
    // Update label to show active filter count
    int count = 0;
    if (m_activeFilters.testFlag(Vertices)) count++;
    if (m_activeFilters.testFlag(Edges)) count++;
    if (m_activeFilters.testFlag(Faces)) count++;
    if (m_activeFilters.testFlag(Bodies)) count++;
    if (m_activeFilters.testFlag(Features)) count++;
    
    if (count == 0) {
        m_selectionLabel->setText(tr("Selection Filter:"));
    }
    else {
        m_selectionLabel->setText(tr("Selection Filter (%1):").arg(count));
    }
}

void FusionSelectionBar::applySelectionFilter()
{
    // Remove any existing selection gate
    Selection().rmvSelectionGate();
    
    // If filters are active, add our custom gate
    if (m_activeFilters != None && m_activeFilters != All) {
        Selection().addSelectionGate(new FusionSelectionGate(m_activeFilters));
    }
}

#include "moc_FusionSelectionBar.cpp"
