// SPDX-License-Identifier: LGPL-2.1-or-later
// UniCAD by UNITRONIX â€” Split Face feature (direct edit).
// Splits selected faces on a solid using a tool shape (sketch, plane, face).

#ifndef PARTDESIGN_FEATURE_SPLITFACE_H
#define PARTDESIGN_FEATURE_SPLITFACE_H

#include <App/PropertyLinks.h>
#include "FeatureDressUp.h"

namespace PartDesign
{

class PartDesignExport SplitFace : public DressUp
{
    PROPERTY_HEADER_WITH_OVERRIDE(PartDesign::SplitFace);

public:
    SplitFace();

    App::PropertyLinkSub SplittingTool;

    App::DocumentObjectExecReturn* execute() override;
    short mustExecute() const override;

    const char* getViewProviderName() const override
    {
        return "PartDesignGui::ViewProviderSplitFace";
    }
};

}  // namespace PartDesign

#endif  // PARTDESIGN_FEATURE_SPLITFACE_H
