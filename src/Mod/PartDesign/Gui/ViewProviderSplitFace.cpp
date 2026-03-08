// SPDX-License-Identifier: LGPL-2.1-or-later
// UniCAD by UNITRONIX â€” ViewProvider for Split Face feature.

#include "TaskSplitFaceParameters.h"
#include "ViewProviderSplitFace.h"

using namespace PartDesignGui;

PROPERTY_SOURCE(PartDesignGui::ViewProviderSplitFace, PartDesignGui::ViewProviderDressUp)

const std::string& ViewProviderSplitFace::featureName() const
{
    static const std::string name = "SplitFace";
    return name;
}

void ViewProviderSplitFace::setupContextMenu(QMenu* menu, QObject* receiver, const char* member)
{
    addDefaultAction(menu, QObject::tr("Edit Split Face"));
    PartDesignGui::ViewProvider::setupContextMenu(menu, receiver, member);
}

TaskDlgFeatureParameters* ViewProviderSplitFace::getEditDialog()
{
    return new TaskDlgSplitFaceParameters(this);
}
