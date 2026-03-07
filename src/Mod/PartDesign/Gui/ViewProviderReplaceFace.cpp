// SPDX-License-Identifier: LGPL-2.1-or-later
// FusionCAD by UNITRONIX — ViewProvider for Replace Face feature.

#include "TaskReplaceFaceParameters.h"
#include "ViewProviderReplaceFace.h"

using namespace PartDesignGui;

PROPERTY_SOURCE(PartDesignGui::ViewProviderReplaceFace, PartDesignGui::ViewProviderDressUp)

const std::string& ViewProviderReplaceFace::featureName() const
{
    static const std::string name = "ReplaceFace";
    return name;
}

void ViewProviderReplaceFace::setupContextMenu(QMenu* menu, QObject* receiver, const char* member)
{
    addDefaultAction(menu, QObject::tr("Edit Replace Face"));
    PartDesignGui::ViewProvider::setupContextMenu(menu, receiver, member);
}

TaskDlgFeatureParameters* ViewProviderReplaceFace::getEditDialog()
{
    return new TaskDlgReplaceFaceParameters(this);
}
