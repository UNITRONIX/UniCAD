// SPDX-License-Identifier: LGPL-2.1-or-later
// FusionCAD by UNITRONIX — Task panel for unified Revolve feature.

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>

#include <App/Document.h>
#include <Gui/Command.h>
#include <Mod/PartDesign/App/FeatureUnifiedRevolve.h>

#include "TaskUnifiedRevolveParameters.h"

using namespace PartDesignGui;
using namespace Gui;

/* TRANSLATOR PartDesignGui::TaskDlgUnifiedRevolveParameters */

TaskDlgUnifiedRevolveParameters::TaskDlgUnifiedRevolveParameters(
    ViewProviderUnifiedRevolve* RevolveView)
    : TaskDlgSketchBasedParameters(RevolveView)
{
    assert(RevolveView);

    // Create the standard revolution parameters panel
    revolutionParams = new TaskRevolutionParameters(
        RevolveView, "PartDesign_Revolution", tr("Revolve"));

    // Add Operation combo box at the top of the revolution panel
    auto* opLayout = new QHBoxLayout();
    auto* opLabel = new QLabel(tr("Operation"));
    operationCombo = new QComboBox();
    operationCombo->addItem(tr("Join"));
    operationCombo->addItem(tr("Cut"));
    operationCombo->addItem(tr("Intersect"));
    operationCombo->addItem(tr("New Body"));
    opLayout->addWidget(opLabel);
    opLayout->addWidget(operationCombo);

    // Insert at top of the revolution panel's layout
    auto* mainLayout = qobject_cast<QVBoxLayout*>(revolutionParams->groupLayout());
    if (mainLayout) {
        // Create a widget to hold the operation layout
        auto* opWidget = new QWidget();
        opWidget->setLayout(opLayout);
        mainLayout->insertWidget(0, opWidget);
    }

    // Set initial value from feature
    auto* revolve = getObject<PartDesign::UnifiedRevolve>();
    if (revolve) {
        operationCombo->setCurrentIndex(revolve->Operation.getValue());
    }

    connect(operationCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &TaskDlgUnifiedRevolveParameters::onOperationChanged);

    Content.push_back(revolutionParams);
    Content.push_back(preview);
}

void TaskDlgUnifiedRevolveParameters::onOperationChanged(int index)
{
    auto* revolve = getObject<PartDesign::UnifiedRevolve>();
    if (!revolve) {
        return;
    }
    revolve->Operation.setValue(index);
    revolve->recomputeFeature();
}

bool TaskDlgUnifiedRevolveParameters::accept()
{
    // Apply the revolution parameters
    if (!TaskDlgSketchBasedParameters::accept()) {
        return false;
    }

    // Also apply Operation
    auto* revolve = getObject<PartDesign::UnifiedRevolve>();
    if (revolve) {
        FCMD_OBJ_CMD(revolve, "Operation = " << revolve->Operation.getValue());
    }
    return true;
}

#include "moc_TaskUnifiedRevolveParameters.cpp"
