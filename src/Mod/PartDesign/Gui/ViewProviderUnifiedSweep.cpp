// SPDX-License-Identifier: LGPL-2.1-or-later
// FusionCAD by UNITRONIX — ViewProvider for unified Sweep.

#include <QMenu>

#include "TaskUnifiedSweepParameters.h"
#include "ViewProviderUnifiedSweep.h"

using namespace PartDesignGui;

PROPERTY_SOURCE(PartDesignGui::ViewProviderUnifiedSweep, PartDesignGui::ViewProviderPipe)

ViewProviderUnifiedSweep::ViewProviderUnifiedSweep()
{
    sPixmap = "PartDesign_AdditivePipe.svg";
}

void ViewProviderUnifiedSweep::setupContextMenu(QMenu* menu, QObject* receiver, const char* member)
{
    addDefaultAction(menu, QObject::tr("Edit Sweep"));
    ViewProviderPipe::setupContextMenu(menu, receiver, member);
}

TaskDlgFeatureParameters* ViewProviderUnifiedSweep::getEditDialog()
{
    return new TaskDlgUnifiedSweepParameters(this);
}
