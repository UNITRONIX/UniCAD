// SPDX-License-Identifier: LGPL-2.1-or-later
// FusionCAD by UNITRONIX — Task panel for Move Face feature.

#include <QAction>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>

#include <App/Document.h>
#include <App/DocumentObject.h>
#include <Base/Interpreter.h>
#include <Gui/Command.h>
#include <Gui/Selection/Selection.h>
#include <Gui/ViewProvider.h>
#include <Mod/PartDesign/App/FeatureMoveFace.h>

#include "TaskMoveFaceParameters.h"

using namespace PartDesignGui;
using namespace Gui;

/* TRANSLATOR PartDesignGui::TaskMoveFaceParameters */

TaskMoveFaceParameters::TaskMoveFaceParameters(ViewProviderDressUp* DressUpView, QWidget* parent)
    : TaskDressUpParameters(DressUpView, false, true, parent)
{
    // Build UI programmatically
    auto* proxy = new QWidget(this);
    auto* layout = new QVBoxLayout(proxy);

    auto* formLayout = new QFormLayout;

    distanceSpinBox = new QDoubleSpinBox(proxy);
    distanceSpinBox->setMinimum(-1000.0);
    distanceSpinBox->setMaximum(1000.0);
    distanceSpinBox->setSingleStep(0.1);
    distanceSpinBox->setDecimals(3);
    distanceSpinBox->setSuffix(QString::fromLatin1(" mm"));

    auto* pcMoveFace = getObject<PartDesign::MoveFace>();
    if (pcMoveFace) {
        distanceSpinBox->setValue(pcMoveFace->Distance.getValue());
    }

    formLayout->addRow(tr("Distance:"), distanceSpinBox);
    layout->addLayout(formLayout);

    this->groupLayout()->addWidget(proxy);

    connect(distanceSpinBox,
            qOverload<double>(&QDoubleSpinBox::valueChanged),
            this,
            &TaskMoveFaceParameters::onDistanceChanged);
}

TaskMoveFaceParameters::~TaskMoveFaceParameters() = default;

void TaskMoveFaceParameters::onDistanceChanged(double val)
{
    setButtons(none);
    auto* pcMoveFace = getObject<PartDesign::MoveFace>();
    if (pcMoveFace) {
        setupTransaction();
        pcMoveFace->Distance.setValue(val);
        pcMoveFace->recomputeFeature();
    }
}

double TaskMoveFaceParameters::getDistance() const
{
    return distanceSpinBox ? distanceSpinBox->value() : 0.0;
}

void TaskMoveFaceParameters::onRefDeleted()
{
    // handled by base class
}

void TaskMoveFaceParameters::setButtons(const selectionModes mode)
{
    Q_UNUSED(mode);
}

void TaskMoveFaceParameters::changeEvent(QEvent* e)
{
    TaskBox::changeEvent(e);
}

void TaskMoveFaceParameters::onSelectionChanged(const Gui::SelectionChanges& msg)
{
    if (msg.Type == Gui::SelectionChanges::AddSelection) {
        if (msg.pSubName && std::string(msg.pSubName).rfind("Face", 0) == 0) {
            referenceSelected(msg, this->findChild<QListWidget*>());
        }
    }
}

void TaskMoveFaceParameters::apply()
{
}

//**************************************************************************
// TaskDialog
//**************************************************************************

TaskDlgMoveFaceParameters::TaskDlgMoveFaceParameters(ViewProviderMoveFace* DressUpView)
    : TaskDlgDressUpParameters(DressUpView)
{
    parameter = new TaskMoveFaceParameters(DressUpView);

    Content.push_back(parameter);
    Content.push_back(preview);
}

TaskDlgMoveFaceParameters::~TaskDlgMoveFaceParameters() = default;

bool TaskDlgMoveFaceParameters::accept()
{
    auto obj = getObject();
    if (!obj->isError()) {
        getViewObject()->showPreviousFeature(false);
    }

    parameter->apply();

    auto* param = dynamic_cast<TaskMoveFaceParameters*>(parameter);
    FCMD_OBJ_CMD(obj, "Distance = " << param->getDistance());

    return TaskDlgDressUpParameters::accept();
}

#include "moc_TaskMoveFaceParameters.cpp"
