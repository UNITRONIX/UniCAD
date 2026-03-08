// SPDX-License-Identifier: LGPL-2.1-or-later
// UniCAD by UNITRONIX â€” ViewProvider for Move Face feature.

#include "TaskMoveFaceParameters.h"
#include "ViewProviderMoveFace.h"

using namespace PartDesignGui;

PROPERTY_SOURCE(PartDesignGui::ViewProviderMoveFace, PartDesignGui::ViewProviderDressUp)

const std::string& ViewProviderMoveFace::featureName() const
{
    static const std::string name = "MoveFace";
    return name;
}

void ViewProviderMoveFace::setupContextMenu(QMenu* menu, QObject* receiver, const char* member)
{
    addDefaultAction(menu, QObject::tr("Edit Move Face"));
    PartDesignGui::ViewProvider::setupContextMenu(menu, receiver, member);
}

TaskDlgFeatureParameters* ViewProviderMoveFace::getEditDialog()
{
    return new TaskDlgMoveFaceParameters(this);
}
