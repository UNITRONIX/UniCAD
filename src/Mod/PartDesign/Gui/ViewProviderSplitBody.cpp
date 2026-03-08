// SPDX-License-Identifier: LGPL-2.1-or-later
// UniCAD by UNITRONIX â€” ViewProvider for Split Body feature.

#include "TaskSplitBodyParameters.h"
#include "ViewProviderSplitBody.h"

using namespace PartDesignGui;

PROPERTY_SOURCE(PartDesignGui::ViewProviderSplitBody, PartDesignGui::ViewProviderDressUp)

const std::string& ViewProviderSplitBody::featureName() const
{
    static const std::string name = "SplitBody";
    return name;
}

void ViewProviderSplitBody::setupContextMenu(QMenu* menu, QObject* receiver, const char* member)
{
    addDefaultAction(menu, QObject::tr("Edit Split Body"));
    PartDesignGui::ViewProvider::setupContextMenu(menu, receiver, member);
}

TaskDlgFeatureParameters* ViewProviderSplitBody::getEditDialog()
{
    return new TaskDlgSplitBodyParameters(this);
}
