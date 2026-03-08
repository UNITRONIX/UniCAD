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

#ifndef GUI_FUSIONTIMELINE_H
#define GUI_FUSIONTIMELINE_H

#include <QDockWidget>
#include <QScrollArea>
#include <QHBoxLayout>
#include <QToolButton>
#include <QLabel>
#include <QList>
#include <QFrame>

#include <FCGlobal.h>

namespace App {
class Document;
class DocumentObject;
}

namespace Gui {

/**
 * FusionTimeline provides a Fusion 360-style parametric feature timeline.
 *
 * Displays the modeling history of the active PartDesign::Body as a
 * horizontal strip of feature "chips" at the bottom of the window.
 * Each chip shows the feature icon and name, and can be clicked to
 * select/edit that feature. A rollback marker can be dragged to
 * roll the model back to any point in history.
 */
class GuiExport FusionTimeline : public QDockWidget
{
    Q_OBJECT

public:
    explicit FusionTimeline(QWidget* parent = nullptr);
    ~FusionTimeline() override;

    /// Refresh the timeline from the active document/body
    void refresh();

Q_SIGNALS:
    void featureSelected(const QString& objectName);
    void rollbackRequested(int featureIndex);

private Q_SLOTS:
    void onFeatureClicked();

private:
    void clearTimeline();
    void addFeatureChip(App::DocumentObject* obj, int index, bool isCurrent);
    void setupStyle();

    QScrollArea* m_scrollArea;
    QWidget* m_container;
    QHBoxLayout* m_layout;

    struct FeatureChip {
        QToolButton* button;
        QString objectName;
        int index;
    };
    QList<FeatureChip> m_chips;
};

} // namespace Gui

#endif // GUI_FUSIONTIMELINE_H
