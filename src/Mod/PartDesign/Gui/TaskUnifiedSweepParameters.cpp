// SPDX-License-Identifier: LGPL-2.1-or-later
// FusionCAD by UNITRONIX — Task panel for unified Sweep feature.

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>

#include <App/Document.h>
#include <Gui/Command.h>
#include <Mod/PartDesign/App/FeatureUnifiedSweep.h>

#include "TaskUnifiedSweepParameters.h"

using namespace PartDesignGui;
using namespace Gui;

/* TRANSLATOR PartDesignGui::TaskDlgUnifiedSweepParameters */

TaskDlgUnifiedSweepParameters::TaskDlgUnifiedSweepParameters(
    ViewProviderUnifiedSweep* SweepView)
    : TaskDlgPipeParameters(SweepView)
{
    // Add Operation combo box at the top of the first parameter panel
    auto* opLayout = new QHBoxLayout();
    auto* opLabel = new QLabel(tr("Operation"));
    operationCombo = new QComboBox();
    operationCombo->addItem(tr("Join"));
    operationCombo->addItem(tr("Cut"));
    operationCombo->addItem(tr("Intersect"));
    operationCombo->addItem(tr("New Body"));
    opLayout->addWidget(opLabel);
    opLayout->addWidget(operationCombo);

    // Insert at top of the sweep parameter panel's layout
    if (parameter) {
        auto* mainLayout = qobject_cast<QVBoxLayout*>(parameter->groupLayout());
        if (mainLayout) {
            auto* opWidget = new QWidget();
            opWidget->setLayout(opLayout);
            mainLayout->insertWidget(0, opWidget);
        }
    }

    // Set initial value from feature
    auto* sweep = getObject<PartDesign::UnifiedSweep>();
    if (sweep) {
        operationCombo->setCurrentIndex(sweep->Operation.getValue());
    }

    connect(operationCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &TaskDlgUnifiedSweepParameters::onOperationChanged);
}

void TaskDlgUnifiedSweepParameters::onOperationChanged(int index)
{
    auto* sweep = getObject<PartDesign::UnifiedSweep>();
    if (!sweep) {
        return;
    }
    sweep->Operation.setValue(index);
    sweep->recomputeFeature();
}

bool TaskDlgUnifiedSweepParameters::accept()
{
    // Apply Operation before the standard accept
    auto* sweep = getObject<PartDesign::UnifiedSweep>();
    if (sweep) {
        FCMD_OBJ_CMD(sweep, "Operation = " << sweep->Operation.getValue());
    }
    return TaskDlgPipeParameters::accept();
}

#include "moc_TaskUnifiedSweepParameters.cpp"
