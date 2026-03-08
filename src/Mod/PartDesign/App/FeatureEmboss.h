// SPDX-License-Identifier: LGPL-2.1-or-later
// UniCAD: Emboss/Deboss feature â€” project sketch onto face and extrude

#ifndef PARTDESIGN_FEATURE_EMBOSS_H
#define PARTDESIGN_FEATURE_EMBOSS_H

#include <App/PropertyStandard.h>
#include <App/PropertyUnits.h>
#include "FeatureSketchBased.h"

namespace PartDesign
{

class PartDesignExport Emboss : public ProfileBased
{
    PROPERTY_HEADER_WITH_OVERRIDE(PartDesign::Emboss);

public:
    Emboss();

    App::PropertyLength Depth;
    App::PropertyBool Reversed;  // true = deboss (cut), false = emboss (fuse)
    App::PropertyLinkSub TargetFace;

    App::DocumentObjectExecReturn* execute() override;
    short mustExecute() const override;

    const char* getViewProviderName() const override
    {
        return "PartDesignGui::ViewProviderEmboss";
    }
};

}  // namespace PartDesign

#endif  // PARTDESIGN_FEATURE_EMBOSS_H
