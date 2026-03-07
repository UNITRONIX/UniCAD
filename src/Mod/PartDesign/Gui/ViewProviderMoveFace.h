// SPDX-License-Identifier: LGPL-2.1-or-later
// FusionCAD by UNITRONIX — ViewProvider for Move Face feature.

#ifndef PARTGUI_ViewProviderMoveFace_H
#define PARTGUI_ViewProviderMoveFace_H

#include "ViewProviderDressUp.h"

namespace PartDesignGui
{

class PartDesignGuiExport ViewProviderMoveFace : public ViewProviderDressUp
{
    Q_DECLARE_TR_FUNCTIONS(PartDesignGui::ViewProviderMoveFace)
    PROPERTY_HEADER_WITH_OVERRIDE(PartDesignGui::ViewProviderMoveFace);

public:
    ViewProviderMoveFace()
    {
        sPixmap = "PartDesign_Thickness.svg";
        menuName = tr("Move Face Parameters");
    }

    const std::string& featureName() const override;
    void setupContextMenu(QMenu*, QObject*, const char*) override;

protected:
    TaskDlgFeatureParameters* getEditDialog() override;
};

}  // namespace PartDesignGui

#endif  // PARTGUI_ViewProviderMoveFace_H
