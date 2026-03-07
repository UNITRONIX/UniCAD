// SPDX-License-Identifier: LGPL-2.1-or-later
// FusionCAD by UNITRONIX — Delete Face + Heal feature (direct edit).
// Removes selected faces from a solid and heals the resulting gaps.

#ifndef PARTDESIGN_FEATURE_DELETEFACE_H
#define PARTDESIGN_FEATURE_DELETEFACE_H

#include "FeatureDressUp.h"

namespace PartDesign
{

class PartDesignExport DeleteFace : public DressUp
{
    PROPERTY_HEADER_WITH_OVERRIDE(PartDesign::DeleteFace);

public:
    DeleteFace();

    App::DocumentObjectExecReturn* execute() override;
    short mustExecute() const override;

    const char* getViewProviderName() const override
    {
        return "PartDesignGui::ViewProviderDeleteFace";
    }
};

}  // namespace PartDesign

#endif  // PARTDESIGN_FEATURE_DELETEFACE_H
