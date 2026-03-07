// SPDX-License-Identifier: LGPL-2.1-or-later
// FusionCAD by UNITRONIX — ViewProvider for Delete Face feature.

#ifndef PARTGUI_ViewProviderDeleteFace_H
#define PARTGUI_ViewProviderDeleteFace_H

#include "ViewProviderDressUp.h"

namespace PartDesignGui
{

class PartDesignGuiExport ViewProviderDeleteFace : public ViewProviderDressUp
{
    Q_DECLARE_TR_FUNCTIONS(PartDesignGui::ViewProviderDeleteFace)
    PROPERTY_HEADER_WITH_OVERRIDE(PartDesignGui::ViewProviderDeleteFace);

public:
    ViewProviderDeleteFace()
    {
        sPixmap = "PartDesign_Thickness.svg";
        menuName = tr("Delete Face Parameters");
    }

    const std::string& featureName() const override;
    void setupContextMenu(QMenu*, QObject*, const char*) override;

protected:
    TaskDlgFeatureParameters* getEditDialog() override;
};

}  // namespace PartDesignGui

#endif  // PARTGUI_ViewProviderDeleteFace_H
