// SPDX-License-Identifier: LGPL-2.1-or-later
// UniCAD by UNITRONIX â€” Fusion 360-style unified Sweep feature.

#ifndef PARTDESIGN_UNIFIED_SWEEP_H
#define PARTDESIGN_UNIFIED_SWEEP_H

#include "FeaturePipe.h"

namespace PartDesign
{

class PartDesignExport UnifiedSweep : public Pipe
{
    PROPERTY_HEADER_WITH_OVERRIDE(PartDesign::UnifiedSweep);

public:
    UnifiedSweep();

    App::PropertyEnumeration Operation;

    App::DocumentObjectExecReturn* execute() override;
    short mustExecute() const override;

    const char* getViewProviderName() const override
    {
        return "PartDesignGui::ViewProviderUnifiedSweep";
    }

private:
    static const char* OperationEnums[];
};

}  // namespace PartDesign

#endif  // PARTDESIGN_UNIFIED_SWEEP_H
