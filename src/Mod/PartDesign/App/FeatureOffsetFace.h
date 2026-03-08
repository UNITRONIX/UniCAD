// SPDX-License-Identifier: LGPL-2.1-or-later
// UniCAD by UNITRONIX â€” Offset Face feature (direct edit).
// Offsets selected faces on a solid along their normals.

#ifndef PARTDESIGN_FEATURE_OFFSETFACE_H
#define PARTDESIGN_FEATURE_OFFSETFACE_H

#include <App/PropertyUnits.h>
#include "FeatureDressUp.h"

namespace PartDesign
{

class PartDesignExport OffsetFace : public DressUp
{
    PROPERTY_HEADER_WITH_OVERRIDE(PartDesign::OffsetFace);

public:
    OffsetFace();

    App::PropertyLength Offset;

    App::DocumentObjectExecReturn* execute() override;
    short mustExecute() const override;

    const char* getViewProviderName() const override
    {
        return "PartDesignGui::ViewProviderOffsetFace";
    }
};

}  // namespace PartDesign

#endif  // PARTDESIGN_FEATURE_OFFSETFACE_H
