// SPDX-License-Identifier: LGPL-2.1-or-later
// UniCAD by UNITRONIX â€” ViewProvider for unified Revolve.

#ifndef PARTDESIGNGUI_ViewProviderUnifiedRevolve_H
#define PARTDESIGNGUI_ViewProviderUnifiedRevolve_H

#include "ViewProviderSketchBased.h"

namespace PartDesignGui
{

class PartDesignGuiExport ViewProviderUnifiedRevolve : public ViewProviderSketchBased
{
    PROPERTY_HEADER_WITH_OVERRIDE(PartDesignGui::ViewProviderUnifiedRevolve);

public:
    ViewProviderUnifiedRevolve();
    ~ViewProviderUnifiedRevolve() override = default;

    void setupContextMenu(QMenu*, QObject*, const char*) override;

protected:
    TaskDlgFeatureParameters* getEditDialog() override;
};

}  // namespace PartDesignGui

#endif
