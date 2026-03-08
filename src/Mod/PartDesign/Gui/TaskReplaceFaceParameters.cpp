// SPDX-License-Identifier: LGPL-2.1-or-later
// UniCAD by UNITRONIX â€” Task panel for Replace Face feature.

#include <QAction>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

#include <App/Document.h>
#include <App/DocumentObject.h>
#include <Base/Interpreter.h>
#include <Gui/Command.h>
#include <Gui/Selection/Selection.h>
#include <Gui/ViewProvider.h>
#include <Mod/PartDesign/App/FeatureReplaceFace.h>

#include "TaskReplaceFaceParameters.h"

using namespace PartDesignGui;
using namespace Gui;

/* TRANSLATOR PartDesignGui::TaskReplaceFaceParameters */

TaskReplaceFaceParameters::TaskReplaceFaceParameters(ViewProviderDressUp* DressUpView,
                                                     QWidget* parent)
    : TaskDressUpParameters(DressUpView, false, true, parent)
{
    auto* proxy = new QWidget(this);
    auto* layout = new QVBoxLayout(proxy);

    auto* label = new QLabel(tr("Select faces to replace on the body,\n"
                                "then click the button below to pick the\n"
                                "replacement surface."),
                             proxy);
    label->setWordWrap(true);
    layout->addWidget(label);

    btnSelectTarget = new QPushButton(tr("Select Target Face"), proxy);
    layout->addWidget(btnSelectTarget);

    lblTarget = new QLabel(tr("Target: (none)"), proxy);
    layout->addWidget(lblTarget);

    this->groupLayout()->addWidget(proxy);

    connect(btnSelectTarget, &QPushButton::clicked, this, &TaskReplaceFaceParameters::onSelectTarget);
}

TaskReplaceFaceParameters::~TaskReplaceFaceParameters() = default;

void TaskReplaceFaceParameters::onSelectTarget()
{
    selectingTarget = !selectingTarget;
    if (selectingTarget) {
        btnSelectTarget->setText(tr("Picking target â€¦ (click again to cancel)"));
    }
    else {
        btnSelectTarget->setText(tr("Select Target Face"));
    }
}

void TaskReplaceFaceParameters::onRefDeleted()
{
    // handled by base class
}

void TaskReplaceFaceParameters::setButtons(const selectionModes mode)
{
    Q_UNUSED(mode);
}

void TaskReplaceFaceParameters::changeEvent(QEvent* e)
{
    TaskBox::changeEvent(e);
}

void TaskReplaceFaceParameters::onSelectionChanged(const Gui::SelectionChanges& msg)
{
    if (msg.Type != Gui::SelectionChanges::AddSelection) {
        return;
    }
    if (!msg.pSubName) {
        return;
    }

    std::string subName(msg.pSubName);
    if (subName.rfind("Face", 0) != 0) {
        return;
    }

    if (selectingTarget) {
        // Store target face reference in the feature's TargetFace property
        auto feat = dynamic_cast<PartDesign::ReplaceFace*>(getDressUpView()->getObject());
        if (feat) {
            App::DocumentObject* selObj =
                feat->getDocument()->getObject(msg.pObjectName);
            if (selObj) {
                std::vector<std::string> subs;
                subs.push_back(subName);
                feat->TargetFace.setValue(selObj, subs);
                lblTarget->setText(
                    tr("Target: %1.%2")
                        .arg(QString::fromLatin1(msg.pObjectName))
                        .arg(QString::fromStdString(subName)));
            }
        }
        selectingTarget = false;
        btnSelectTarget->setText(tr("Select Target Face"));
        getDressUpView()->getObject()->recomputeFeature();
    }
    else {
        // Standard source face selection
        referenceSelected(msg, this->findChild<QListWidget*>());
    }
}

void TaskReplaceFaceParameters::apply()
{
    // TargetFace is set directly on the feature during selection
}

//**************************************************************************
// TaskDialog
//**************************************************************************

TaskDlgReplaceFaceParameters::TaskDlgReplaceFaceParameters(ViewProviderReplaceFace* DressUpView)
    : TaskDlgDressUpParameters(DressUpView)
{
    parameter = new TaskReplaceFaceParameters(DressUpView);

    Content.push_back(parameter);
    Content.push_back(preview);
}

TaskDlgReplaceFaceParameters::~TaskDlgReplaceFaceParameters() = default;

bool TaskDlgReplaceFaceParameters::accept()
{
    auto obj = getObject();
    if (!obj->isError()) {
        getViewObject()->showPreviousFeature(false);
    }

    parameter->apply();

    return TaskDlgDressUpParameters::accept();
}

#include "moc_TaskReplaceFaceParameters.cpp"
