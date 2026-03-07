// SPDX-License-Identifier: LGPL-2.1-or-later
// FusionCAD by UNITRONIX — ViewProvider for unified Sweep.

#ifndef PARTDESIGNGUI_ViewProviderUnifiedSweep_H
#define PARTDESIGNGUI_ViewProviderUnifiedSweep_H

#include "ViewProviderPipe.h"

namespace PartDesignGui
{

class PartDesignGuiExport ViewProviderUnifiedSweep : public ViewProviderPipe
{
    PROPERTY_HEADER_WITH_OVERRIDE(PartDesignGui::ViewProviderUnifiedSweep);

public:
    ViewProviderUnifiedSweep();
    ~ViewProviderUnifiedSweep() override = default;

    void setupContextMenu(QMenu*, QObject*, const char*) override;

protected:
    TaskDlgFeatureParameters* getEditDialog() override;
};

}  // namespace PartDesignGui

#endif
