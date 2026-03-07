// SPDX-License-Identifier: LGPL-2.1-or-later
// FusionCAD by UNITRONIX — Task panel for Offset Face feature.

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
#include <Mod/PartDesign/App/FeatureOffsetFace.h>

#include "TaskOffsetFaceParameters.h"

using namespace PartDesignGui;
using namespace Gui;

/* TRANSLATOR PartDesignGui::TaskOffsetFaceParameters */

TaskOffsetFaceParameters::TaskOffsetFaceParameters(ViewProviderDressUp* DressUpView,
                                                   QWidget* parent)
    : TaskDressUpParameters(DressUpView, false, true, parent)
{
    // Build UI programmatically
    auto* proxy = new QWidget(this);
    auto* layout = new QVBoxLayout(proxy);

    auto* formLayout = new QFormLayout;

    offsetSpinBox = new QDoubleSpinBox(proxy);
    offsetSpinBox->setMinimum(-1000.0);
    offsetSpinBox->setMaximum(1000.0);
    offsetSpinBox->setSingleStep(0.1);
    offsetSpinBox->setDecimals(3);
    offsetSpinBox->setSuffix(QString::fromLatin1(" mm"));

    auto* pcOffsetFace = getObject<PartDesign::OffsetFace>();
    if (pcOffsetFace) {
        offsetSpinBox->setValue(pcOffsetFace->Offset.getValue());
    }

    formLayout->addRow(tr("Offset:"), offsetSpinBox);
    layout->addLayout(formLayout);

    this->groupLayout()->addWidget(proxy);

    connect(offsetSpinBox,
            qOverload<double>(&QDoubleSpinBox::valueChanged),
            this,
            &TaskOffsetFaceParameters::onOffsetChanged);
}

TaskOffsetFaceParameters::~TaskOffsetFaceParameters() = default;

void TaskOffsetFaceParameters::onOffsetChanged(double val)
{
    setButtons(none);
    auto* pcOffsetFace = getObject<PartDesign::OffsetFace>();
    if (pcOffsetFace) {
        setupTransaction();
        pcOffsetFace->Offset.setValue(val);
        pcOffsetFace->recomputeFeature();
    }
}

double TaskOffsetFaceParameters::getOffset() const
{
    return offsetSpinBox ? offsetSpinBox->value() : 0.0;
}

void TaskOffsetFaceParameters::onRefDeleted()
{
    // handled by base class
}

void TaskOffsetFaceParameters::setButtons(const selectionModes mode)
{
    Q_UNUSED(mode);
}

void TaskOffsetFaceParameters::changeEvent(QEvent* e)
{
    TaskBox::changeEvent(e);
}

void TaskOffsetFaceParameters::onSelectionChanged(const Gui::SelectionChanges& msg)
{
    if (msg.Type == Gui::SelectionChanges::AddSelection) {
        // Accept faces only
        if (msg.pSubName && std::string(msg.pSubName).rfind("Face", 0) == 0) {
            referenceSelected(msg, this->findChild<QListWidget*>());
        }
    }
}

void TaskOffsetFaceParameters::apply()
{
}

//**************************************************************************
// TaskDialog
//**************************************************************************

TaskDlgOffsetFaceParameters::TaskDlgOffsetFaceParameters(ViewProviderOffsetFace* DressUpView)
    : TaskDlgDressUpParameters(DressUpView)
{
    parameter = new TaskOffsetFaceParameters(DressUpView);

    Content.push_back(parameter);
    Content.push_back(preview);
}

TaskDlgOffsetFaceParameters::~TaskDlgOffsetFaceParameters() = default;

bool TaskDlgOffsetFaceParameters::accept()
{
    auto obj = getObject();
    if (!obj->isError()) {
        getViewObject()->showPreviousFeature(false);
    }

    parameter->apply();

    auto* param = dynamic_cast<TaskOffsetFaceParameters*>(parameter);
    FCMD_OBJ_CMD(obj, "Offset = " << param->getOffset());

    return TaskDlgDressUpParameters::accept();
}

#include "moc_TaskOffsetFaceParameters.cpp"
