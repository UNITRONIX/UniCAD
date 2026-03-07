// SPDX-License-Identifier: LGPL-2.1-or-later
// FusionCAD by UNITRONIX — ViewProvider for Offset Face feature.

#ifndef PARTGUI_ViewProviderOffsetFace_H
#define PARTGUI_ViewProviderOffsetFace_H

#include "ViewProviderDressUp.h"

namespace PartDesignGui
{

class PartDesignGuiExport ViewProviderOffsetFace : public ViewProviderDressUp
{
    Q_DECLARE_TR_FUNCTIONS(PartDesignGui::ViewProviderOffsetFace)
    PROPERTY_HEADER_WITH_OVERRIDE(PartDesignGui::ViewProviderOffsetFace);

public:
    ViewProviderOffsetFace()
    {
        sPixmap = "PartDesign_Thickness.svg";
        menuName = tr("Offset Face Parameters");
    }

    const std::string& featureName() const override;
    void setupContextMenu(QMenu*, QObject*, const char*) override;

protected:
    TaskDlgFeatureParameters* getEditDialog() override;
};

}  // namespace PartDesignGui

#endif  // PARTGUI_ViewProviderOffsetFace_H
