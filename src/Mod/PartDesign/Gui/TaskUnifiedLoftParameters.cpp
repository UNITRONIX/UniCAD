// SPDX-License-Identifier: LGPL-2.1-or-later
// UniCAD by UNITRONIX â€” Task panel for unified Loft feature.

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>

#include <App/Document.h>
#include <Gui/Command.h>
#include <Mod/PartDesign/App/FeatureUnifiedLoft.h>

#include "TaskUnifiedLoftParameters.h"

using namespace PartDesignGui;
using namespace Gui;

/* TRANSLATOR PartDesignGui::TaskDlgUnifiedLoftParameters */

TaskDlgUnifiedLoftParameters::TaskDlgUnifiedLoftParameters(
    ViewProviderUnifiedLoft* LoftView)
    : TaskDlgLoftParameters(LoftView)
{
    // Add Operation combo box at the top of the loft parameter panel
    auto* opLayout = new QHBoxLayout();
    auto* opLabel = new QLabel(tr("Operation"));
    operationCombo = new QComboBox();
    operationCombo->addItem(tr("Join"));
    operationCombo->addItem(tr("Cut"));
    operationCombo->addItem(tr("Intersect"));
    operationCombo->addItem(tr("New Body"));
    opLayout->addWidget(opLabel);
    opLayout->addWidget(operationCombo);

    // Insert at top of the loft parameter panel's layout
    if (parameter) {
        auto* mainLayout = qobject_cast<QVBoxLayout*>(parameter->groupLayout());
        if (mainLayout) {
            auto* opWidget = new QWidget();
            opWidget->setLayout(opLayout);
            mainLayout->insertWidget(0, opWidget);
        }
    }

    // Set initial value from feature
    auto* loft = getObject<PartDesign::UnifiedLoft>();
    if (loft) {
        operationCombo->setCurrentIndex(loft->Operation.getValue());
    }

    connect(operationCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &TaskDlgUnifiedLoftParameters::onOperationChanged);
}

void TaskDlgUnifiedLoftParameters::onOperationChanged(int index)
{
    auto* loft = getObject<PartDesign::UnifiedLoft>();
    if (!loft) {
        return;
    }
    loft->Operation.setValue(index);
    loft->recomputeFeature();
}

bool TaskDlgUnifiedLoftParameters::accept()
{
    // Apply Operation before the standard accept
    auto* loft = getObject<PartDesign::UnifiedLoft>();
    if (loft) {
        FCMD_OBJ_CMD(loft, "Operation = " << loft->Operation.getValue());
    }
    return TaskDlgLoftParameters::accept();
}

#include "moc_TaskUnifiedLoftParameters.cpp"
