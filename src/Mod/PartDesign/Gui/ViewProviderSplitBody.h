// SPDX-License-Identifier: LGPL-2.1-or-later
// UniCAD by UNITRONIX â€” ViewProvider for Split Body feature.

#ifndef PARTGUI_ViewProviderSplitBody_H
#define PARTGUI_ViewProviderSplitBody_H

#include "ViewProviderDressUp.h"

namespace PartDesignGui
{

class PartDesignGuiExport ViewProviderSplitBody : public ViewProviderDressUp
{
    Q_DECLARE_TR_FUNCTIONS(PartDesignGui::ViewProviderSplitBody)
    PROPERTY_HEADER_WITH_OVERRIDE(PartDesignGui::ViewProviderSplitBody);

public:
    ViewProviderSplitBody()
    {
        sPixmap = "PartDesign_Thickness.svg";
        menuName = tr("Split Body Parameters");
    }

    const std::string& featureName() const override;
    void setupContextMenu(QMenu*, QObject*, const char*) override;

protected:
    TaskDlgFeatureParameters* getEditDialog() override;
};

}  // namespace PartDesignGui

#endif
