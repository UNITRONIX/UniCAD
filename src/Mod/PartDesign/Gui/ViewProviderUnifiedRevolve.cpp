// SPDX-License-Identifier: LGPL-2.1-or-later
// FusionCAD by UNITRONIX — ViewProvider for unified Revolve.

#include <QMenu>

#include "TaskUnifiedRevolveParameters.h"
#include "ViewProviderUnifiedRevolve.h"

using namespace PartDesignGui;

PROPERTY_SOURCE(PartDesignGui::ViewProviderUnifiedRevolve, PartDesignGui::ViewProviderSketchBased)

ViewProviderUnifiedRevolve::ViewProviderUnifiedRevolve()
{
    sPixmap = "PartDesign_Revolution.svg";
}

void ViewProviderUnifiedRevolve::setupContextMenu(QMenu* menu, QObject* receiver, const char* member)
{
    addDefaultAction(menu, QObject::tr("Edit Revolve"));
    ViewProviderSketchBased::setupContextMenu(menu, receiver, member);
}

TaskDlgFeatureParameters* ViewProviderUnifiedRevolve::getEditDialog()
{
    return new TaskDlgUnifiedRevolveParameters(this);
}
