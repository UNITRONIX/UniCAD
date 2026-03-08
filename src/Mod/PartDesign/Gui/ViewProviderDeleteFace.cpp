// SPDX-License-Identifier: LGPL-2.1-or-later
// UniCAD by UNITRONIX â€” ViewProvider for Delete Face feature.

#include "TaskDeleteFaceParameters.h"
#include "ViewProviderDeleteFace.h"

using namespace PartDesignGui;

PROPERTY_SOURCE(PartDesignGui::ViewProviderDeleteFace, PartDesignGui::ViewProviderDressUp)

const std::string& ViewProviderDeleteFace::featureName() const
{
    static const std::string name = "DeleteFace";
    return name;
}

void ViewProviderDeleteFace::setupContextMenu(QMenu* menu, QObject* receiver, const char* member)
{
    addDefaultAction(menu, QObject::tr("Edit Delete Face"));
    PartDesignGui::ViewProvider::setupContextMenu(menu, receiver, member);
}

TaskDlgFeatureParameters* ViewProviderDeleteFace::getEditDialog()
{
    return new TaskDlgDeleteFaceParameters(this);
}
