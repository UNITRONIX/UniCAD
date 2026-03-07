// SPDX-License-Identifier: LGPL-2.1-or-later
// FusionCAD by UNITRONIX — ViewProvider for unified Extrude feature.

#ifndef PARTGUI_ViewProviderUnifiedExtrude_H
#define PARTGUI_ViewProviderUnifiedExtrude_H

#include "ViewProviderExtrude.h"

namespace PartDesignGui
{

class PartDesignGuiExport ViewProviderUnifiedExtrude : public ViewProviderExtrude
{
    PROPERTY_HEADER_WITH_OVERRIDE(PartDesignGui::ViewProviderUnifiedExtrude);

public:
    ViewProviderUnifiedExtrude();
    ~ViewProviderUnifiedExtrude() override = default;

    void setupContextMenu(QMenu*, QObject*, const char*) override;

protected:
    TaskDlgFeatureParameters* getEditDialog() override;
};

}  // namespace PartDesignGui

#endif  // PARTGUI_ViewProviderUnifiedExtrude_H
