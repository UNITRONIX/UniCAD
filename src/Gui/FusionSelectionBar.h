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

#ifndef GUI_FUSIONSELECTIONBAR_H
#define GUI_FUSIONSELECTIONBAR_H

#include <QToolBar>
#include <QToolButton>
#include <QButtonGroup>
#include <QLabel>
#include <FCGlobal.h>

namespace Gui {

/**
 * FusionSelectionBar provides Fusion 360-style selection filters.
 * 
 * This toolbar allows users to filter what types of geometry can be selected:
 * - Vertices (points)
 * - Edges (lines, arcs, curves)
 * - Faces (surfaces)
 * - Bodies (solid objects)
 * - Features (parametric features)
 * 
 * Multiple filters can be active simultaneously (OR logic).
 * When no filters are active, all geometry types are selectable.
 */
class GuiExport FusionSelectionBar : public QToolBar
{
    Q_OBJECT

public:
    enum FilterType {
        None        = 0,
        Vertices    = 1 << 0,
        Edges       = 1 << 1,
        Faces       = 1 << 2,
        Bodies      = 1 << 3,
        Features    = 1 << 4,
        All         = Vertices | Edges | Faces | Bodies | Features
    };
    Q_DECLARE_FLAGS(FilterTypes, FilterType)

    explicit FusionSelectionBar(QWidget* parent = nullptr);
    ~FusionSelectionBar() override;

    /// Get currently active filters
    FilterTypes activeFilters() const { return m_activeFilters; }

    /// Set active filters programmatically
    void setActiveFilters(FilterTypes filters);

    /// Check if a specific filter is active
    bool isFilterActive(FilterType filter) const;

    /// Toggle a filter on/off
    void toggleFilter(FilterType filter);

    /// Clear all filters (allow all selections)
    void clearFilters();

Q_SIGNALS:
    /// Emitted when filter selection changes
    void filtersChanged(FilterTypes filters);

private Q_SLOTS:
    void onFilterButtonToggled(int id, bool checked);
    void onClearFilters();

private:
    void setupUI();
    QToolButton* createFilterButton(const QString& iconName, const QString& tooltip, FilterType filter);
    void updateFilterState();
    void applySelectionFilter();

    FilterTypes m_activeFilters;
    QButtonGroup* m_filterGroup;
    
    QToolButton* m_verticesBtn;
    QToolButton* m_edgesBtn;
    QToolButton* m_facesBtn;
    QToolButton* m_bodiesBtn;
    QToolButton* m_featuresBtn;
    QToolButton* m_clearBtn;
    QLabel* m_selectionLabel;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(FusionSelectionBar::FilterTypes)

} // namespace Gui

#endif // GUI_FUSIONSELECTIONBAR_H
