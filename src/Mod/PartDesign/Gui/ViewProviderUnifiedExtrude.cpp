// SPDX-License-Identifier: LGPL-2.1-or-later
// UniCAD by UNITRONIX — ViewProvider for unified Extrude feature.

#include <QMenu>
#include <cstring>

#include <Base/ServiceProvider.h>
#include <Mod/PartDesign/App/FeatureUnifiedExtrude.h>

#include "StyleParameters.h"
#include "TaskUnifiedExtrudeParameters.h"
#include "ViewProviderUnifiedExtrude.h"

using namespace PartDesignGui;

PROPERTY_SOURCE(PartDesignGui::ViewProviderUnifiedExtrude, PartDesignGui::ViewProviderExtrude)

ViewProviderUnifiedExtrude::ViewProviderUnifiedExtrude()
{
    sPixmap = "PartDesign_Pad.svg";
}

void ViewProviderUnifiedExtrude::attach(App::DocumentObject* pcObject)
{
    ViewProviderExtrude::attach(pcObject);
    updatePreviewColorFromOperation();
}

void ViewProviderUnifiedExtrude::setupContextMenu(QMenu* menu, QObject* receiver, const char* member)
{
    addDefaultAction(menu, QObject::tr("Edit Extrude"));
    PartDesignGui::ViewProviderSketchBased::setupContextMenu(menu, receiver, member);
}

TaskDlgFeatureParameters* ViewProviderUnifiedExtrude::getEditDialog()
{
    return new TaskDlgUnifiedExtrudeParameters(this);
}

void ViewProviderUnifiedExtrude::updatePreviewColorFromOperation()
{
    auto* extrude = getObject<PartDesign::Extrude>();
    if (!extrude) {
        return;
    }

    auto* styleParameterManager = Base::provideService<Gui::StyleParameters::ParameterManager>();
    if (!styleParameterManager) {
        return;
    }

    const char* op = extrude->Operation.getValueAsString();
    if (strcmp(op, "Cut") == 0) {
        PreviewColor.setValue(
            styleParameterManager->resolve(StyleParameters::PreviewSubtractiveColor));
    }
    else if (strcmp(op, "Intersect") == 0) {
        PreviewColor.setValue(
            styleParameterManager->resolve(StyleParameters::PreviewCommonColor));
    }
    else {
        // Join / NewBody — Fusion additive blue
        PreviewColor.setValue(
            styleParameterManager->resolve(StyleParameters::PreviewAdditiveColor));
    }
}

void ViewProviderUnifiedExtrude::updateData(const App::Property* prop)
{
    auto* extrude = getObject<PartDesign::Extrude>();
    if (extrude && prop == &extrude->Operation) {
        updatePreviewColorFromOperation();
    }

    ViewProviderExtrude::updateData(prop);
}

void ViewProviderUnifiedExtrude::updatePreview()
{
    ViewProviderExtrude::updatePreview();
    // Apply after base so Intersect (orange) is not overwritten by Additive default
    updatePreviewColorFromOperation();
}
