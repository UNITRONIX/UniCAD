// SPDX-License-Identifier: LGPL-2.1-or-later
// UniCAD by UNITRONIX â€” Fusion 360-style unified Loft feature.

#ifndef PARTDESIGN_UNIFIED_LOFT_H
#define PARTDESIGN_UNIFIED_LOFT_H

#include "FeatureLoft.h"

namespace PartDesign
{

class PartDesignExport UnifiedLoft : public Loft
{
    PROPERTY_HEADER_WITH_OVERRIDE(PartDesign::UnifiedLoft);

public:
    UnifiedLoft();

    App::PropertyEnumeration Operation;

    App::DocumentObjectExecReturn* execute() override;
    short mustExecute() const override;

    const char* getViewProviderName() const override
    {
        return "PartDesignGui::ViewProviderUnifiedLoft";
    }

private:
    static const char* OperationEnums[];
};

}  // namespace PartDesign

#endif  // PARTDESIGN_UNIFIED_LOFT_H
