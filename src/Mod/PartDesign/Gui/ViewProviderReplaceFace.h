// SPDX-License-Identifier: LGPL-2.1-or-later
// FusionCAD by UNITRONIX — ViewProvider for Replace Face feature.

#ifndef PARTGUI_ViewProviderReplaceFace_H
#define PARTGUI_ViewProviderReplaceFace_H

#include "ViewProviderDressUp.h"

namespace PartDesignGui
{

class PartDesignGuiExport ViewProviderReplaceFace : public ViewProviderDressUp
{
    Q_DECLARE_TR_FUNCTIONS(PartDesignGui::ViewProviderReplaceFace)
    PROPERTY_HEADER_WITH_OVERRIDE(PartDesignGui::ViewProviderReplaceFace);

public:
    ViewProviderReplaceFace()
    {
        sPixmap = "PartDesign_Thickness.svg";
        menuName = tr("Replace Face Parameters");
    }

    const std::string& featureName() const override;
    void setupContextMenu(QMenu*, QObject*, const char*) override;

protected:
    TaskDlgFeatureParameters* getEditDialog() override;
};

}  // namespace PartDesignGui

#endif  // PARTGUI_ViewProviderReplaceFace_H
