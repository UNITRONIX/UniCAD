// SPDX-License-Identifier: LGPL-2.1-or-later
// FusionCAD by UNITRONIX — ViewProvider for unified Loft.

#ifndef PARTDESIGNGUI_ViewProviderUnifiedLoft_H
#define PARTDESIGNGUI_ViewProviderUnifiedLoft_H

#include "ViewProviderLoft.h"

namespace PartDesignGui
{

class PartDesignGuiExport ViewProviderUnifiedLoft : public ViewProviderLoft
{
    PROPERTY_HEADER_WITH_OVERRIDE(PartDesignGui::ViewProviderUnifiedLoft);

public:
    ViewProviderUnifiedLoft();
    ~ViewProviderUnifiedLoft() override = default;

    void setupContextMenu(QMenu*, QObject*, const char*) override;

protected:
    TaskDlgFeatureParameters* getEditDialog() override;
};

}  // namespace PartDesignGui

#endif
