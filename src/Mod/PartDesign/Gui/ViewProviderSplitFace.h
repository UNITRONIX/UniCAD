// SPDX-License-Identifier: LGPL-2.1-or-later
// FusionCAD by UNITRONIX — ViewProvider for Split Face feature.

#ifndef PARTGUI_ViewProviderSplitFace_H
#define PARTGUI_ViewProviderSplitFace_H

#include "ViewProviderDressUp.h"

namespace PartDesignGui
{

class PartDesignGuiExport ViewProviderSplitFace : public ViewProviderDressUp
{
    Q_DECLARE_TR_FUNCTIONS(PartDesignGui::ViewProviderSplitFace)
    PROPERTY_HEADER_WITH_OVERRIDE(PartDesignGui::ViewProviderSplitFace);

public:
    ViewProviderSplitFace()
    {
        sPixmap = "PartDesign_Thickness.svg";
        menuName = tr("Split Face Parameters");
    }

    const std::string& featureName() const override;
    void setupContextMenu(QMenu*, QObject*, const char*) override;

protected:
    TaskDlgFeatureParameters* getEditDialog() override;
};

}  // namespace PartDesignGui

#endif  // PARTGUI_ViewProviderSplitFace_H
