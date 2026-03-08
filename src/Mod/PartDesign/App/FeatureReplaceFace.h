// SPDX-License-Identifier: LGPL-2.1-or-later
// UniCAD by UNITRONIX â€” Replace Face feature (direct edit).
// Replaces selected faces on a solid with a target surface.

#ifndef PARTDESIGN_FEATURE_REPLACEFACE_H
#define PARTDESIGN_FEATURE_REPLACEFACE_H

#include <App/PropertyLinks.h>
#include "FeatureDressUp.h"

namespace PartDesign
{

class PartDesignExport ReplaceFace : public DressUp
{
    PROPERTY_HEADER_WITH_OVERRIDE(PartDesign::ReplaceFace);

public:
    ReplaceFace();

    App::PropertyLinkSub TargetFace;

    App::DocumentObjectExecReturn* execute() override;
    short mustExecute() const override;

    const char* getViewProviderName() const override
    {
        return "PartDesignGui::ViewProviderReplaceFace";
    }
};

}  // namespace PartDesign

#endif  // PARTDESIGN_FEATURE_REPLACEFACE_H
