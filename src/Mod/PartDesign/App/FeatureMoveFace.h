// SPDX-License-Identifier: LGPL-2.1-or-later
// UniCAD by UNITRONIX â€” Move Face feature (direct edit).
// Translates selected faces on a solid by a displacement vector.

#ifndef PARTDESIGN_FEATURE_MOVEFACE_H
#define PARTDESIGN_FEATURE_MOVEFACE_H

#include <App/PropertyUnits.h>
#include "FeatureDressUp.h"

namespace PartDesign
{

class PartDesignExport MoveFace : public DressUp
{
    PROPERTY_HEADER_WITH_OVERRIDE(PartDesign::MoveFace);

public:
    MoveFace();

    App::PropertyLength Distance;
    App::PropertyVector Direction;
    App::PropertyBool UseCustomDirection;

    App::DocumentObjectExecReturn* execute() override;
    short mustExecute() const override;

    const char* getViewProviderName() const override
    {
        return "PartDesignGui::ViewProviderMoveFace";
    }
};

}  // namespace PartDesign

#endif  // PARTDESIGN_FEATURE_MOVEFACE_H
