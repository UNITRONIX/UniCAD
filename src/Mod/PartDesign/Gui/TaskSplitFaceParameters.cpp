// SPDX-License-Identifier: LGPL-2.1-or-later
// FusionCAD by UNITRONIX — Task panel for Split Face feature.

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
#include <Mod/PartDesign/App/FeatureSplitFace.h>

#include "TaskSplitFaceParameters.h"

using namespace PartDesignGui;
using namespace Gui;

/* TRANSLATOR PartDesignGui::TaskSplitFaceParameters */

TaskSplitFaceParameters::TaskSplitFaceParameters(ViewProviderDressUp* DressUpView,
                                                  QWidget* parent)
    : TaskDressUpParameters(DressUpView, false, true, parent)
{
    auto* proxy = new QWidget(this);
    auto* layout = new QVBoxLayout(proxy);

    auto* label = new QLabel(tr("Select faces to split, then click the button\n"
                                "below to pick the splitting tool\n"
                                "(sketch, datum plane, or face)."),
                             proxy);
    label->setWordWrap(true);
    layout->addWidget(label);

    btnSelectTool = new QPushButton(tr("Select Splitting Tool"), proxy);
    layout->addWidget(btnSelectTool);

    lblTool = new QLabel(tr("Tool: (none)"), proxy);
    layout->addWidget(lblTool);

    this->groupLayout()->addWidget(proxy);

    connect(btnSelectTool, &QPushButton::clicked, this, &TaskSplitFaceParameters::onSelectTool);
}

TaskSplitFaceParameters::~TaskSplitFaceParameters() = default;

void TaskSplitFaceParameters::onSelectTool()
{
    selectingTool = !selectingTool;
    if (selectingTool) {
        btnSelectTool->setText(tr("Picking tool … (click again to cancel)"));
    }
    else {
        btnSelectTool->setText(tr("Select Splitting Tool"));
    }
}

void TaskSplitFaceParameters::onRefDeleted()
{
    // handled by base class
}

void TaskSplitFaceParameters::setButtons(const selectionModes mode)
{
    Q_UNUSED(mode);
}

void TaskSplitFaceParameters::changeEvent(QEvent* e)
{
    TaskBox::changeEvent(e);
}

void TaskSplitFaceParameters::onSelectionChanged(const Gui::SelectionChanges& msg)
{
    if (msg.Type != Gui::SelectionChanges::AddSelection) {
        return;
    }
    if (!msg.pObjectName) {
        return;
    }

    if (selectingTool) {
        // Store splitting tool reference
        auto feat = dynamic_cast<PartDesign::SplitFace*>(getDressUpView()->getObject());
        if (feat) {
            App::DocumentObject* selObj =
                feat->getDocument()->getObject(msg.pObjectName);
            if (selObj) {
                std::vector<std::string> subs;
                if (msg.pSubName && std::string(msg.pSubName).length() > 0) {
                    subs.push_back(std::string(msg.pSubName));
                }
                feat->SplittingTool.setValue(selObj, subs);
                lblTool->setText(
                    tr("Tool: %1").arg(QString::fromLatin1(msg.pObjectName)));
            }
        }
        selectingTool = false;
        btnSelectTool->setText(tr("Select Splitting Tool"));
        getDressUpView()->getObject()->recomputeFeature();
    }
    else {
        // Standard source face selection
        if (msg.pSubName && std::string(msg.pSubName).rfind("Face", 0) == 0) {
            referenceSelected(msg, this->findChild<QListWidget*>());
        }
    }
}

void TaskSplitFaceParameters::apply()
{
    // SplittingTool is set directly on the feature during selection
}

//**************************************************************************
// TaskDialog
//**************************************************************************

TaskDlgSplitFaceParameters::TaskDlgSplitFaceParameters(ViewProviderSplitFace* DressUpView)
    : TaskDlgDressUpParameters(DressUpView)
{
    parameter = new TaskSplitFaceParameters(DressUpView);

    Content.push_back(parameter);
    Content.push_back(preview);
}

TaskDlgSplitFaceParameters::~TaskDlgSplitFaceParameters() = default;

bool TaskDlgSplitFaceParameters::accept()
{
    auto obj = getObject();
    if (!obj->isError()) {
        getViewObject()->showPreviousFeature(false);
    }

    parameter->apply();

    return TaskDlgDressUpParameters::accept();
}

#include "moc_TaskSplitFaceParameters.cpp"
