// SPDX-License-Identifier: LGPL-2.1-or-later
// UniCAD by UNITRONIX â€” ViewProvider for Offset Face feature.

#include "TaskOffsetFaceParameters.h"
#include "ViewProviderOffsetFace.h"

using namespace PartDesignGui;

PROPERTY_SOURCE(PartDesignGui::ViewProviderOffsetFace, PartDesignGui::ViewProviderDressUp)

const std::string& ViewProviderOffsetFace::featureName() const
{
    static const std::string name = "OffsetFace";
    return name;
}

void ViewProviderOffsetFace::setupContextMenu(QMenu* menu, QObject* receiver, const char* member)
{
    addDefaultAction(menu, QObject::tr("Edit Offset Face"));
    PartDesignGui::ViewProvider::setupContextMenu(menu, receiver, member);
}

TaskDlgFeatureParameters* ViewProviderOffsetFace::getEditDialog()
{
    return new TaskDlgOffsetFaceParameters(this);
}
