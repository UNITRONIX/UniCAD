// SPDX-License-Identifier: LGPL-2.1-or-later
// UniCAD by UNITRONIX â€” Split Body feature.
// Splits a solid body into two separate bodies using a tool (sketch, plane, face, or body).

#ifndef PARTDESIGN_FEATURE_SPLITBODY_H
#define PARTDESIGN_FEATURE_SPLITBODY_H

#include <App/PropertyLinks.h>
#include "FeatureDressUp.h"

namespace PartDesign
{

class PartDesignExport SplitBody : public DressUp
{
    PROPERTY_HEADER_WITH_OVERRIDE(PartDesign::SplitBody);

public:
    SplitBody();

    App::PropertyLinkSub SplittingTool;

    App::DocumentObjectExecReturn* execute() override;
    short mustExecute() const override;

    const char* getViewProviderName() const override
    {
        return "PartDesignGui::ViewProviderSplitBody";
    }
};

}  // namespace PartDesign

#endif  // PARTDESIGN_FEATURE_SPLITBODY_H
