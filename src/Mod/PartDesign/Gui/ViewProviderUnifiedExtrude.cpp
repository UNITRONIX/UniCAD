// SPDX-License-Identifier: LGPL-2.1-or-later
// FusionCAD by UNITRONIX — ViewProvider for unified Extrude feature.

#include <QMenu>

#include "TaskUnifiedExtrudeParameters.h"
#include "ViewProviderUnifiedExtrude.h"

using namespace PartDesignGui;

PROPERTY_SOURCE(PartDesignGui::ViewProviderUnifiedExtrude, PartDesignGui::ViewProviderExtrude)

ViewProviderUnifiedExtrude::ViewProviderUnifiedExtrude()
{
    sPixmap = "PartDesign_Pad.svg";
}

void ViewProviderUnifiedExtrude::setupContextMenu(QMenu* menu, QObject* receiver, const char* member)
{
    addDefaultAction(menu, QObject::tr("Edit Extrude"));
    PartDesignGui::ViewProviderSketchBased::setupContextMenu(menu, receiver, member);
}

TaskDlgFeatureParameters* ViewProviderUnifiedExtrude::getEditDialog()
{
    return new TaskDlgUnifiedExtrudeParameters(this);
}
