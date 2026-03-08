// SPDX-License-Identifier: LGPL-2.1-or-later
// UniCAD by UNITRONIX â€” ViewProvider for unified Loft.

#include <QMenu>

#include "TaskUnifiedLoftParameters.h"
#include "ViewProviderUnifiedLoft.h"

using namespace PartDesignGui;

PROPERTY_SOURCE(PartDesignGui::ViewProviderUnifiedLoft, PartDesignGui::ViewProviderLoft)

ViewProviderUnifiedLoft::ViewProviderUnifiedLoft()
{
    sPixmap = "PartDesign_AdditiveLoft.svg";
}

void ViewProviderUnifiedLoft::setupContextMenu(QMenu* menu, QObject* receiver, const char* member)
{
    addDefaultAction(menu, QObject::tr("Edit Loft"));
    ViewProviderLoft::setupContextMenu(menu, receiver, member);
}

TaskDlgFeatureParameters* ViewProviderUnifiedLoft::getEditDialog()
{
    return new TaskDlgUnifiedLoftParameters(this);
}
