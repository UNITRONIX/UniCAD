// SPDX-License-Identifier: LGPL-2.1-or-later
// UniCAD by UNITRONIX â€” Task panel for Split Body feature.

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
#include <Mod/PartDesign/App/FeatureSplitBody.h>

#include "TaskSplitBodyParameters.h"

using namespace PartDesignGui;
using namespace Gui;

/* TRANSLATOR PartDesignGui::TaskSplitBodyParameters */

TaskSplitBodyParameters::TaskSplitBodyParameters(ViewProviderDressUp* DressUpView,
                                                  QWidget* parent)
    : TaskDressUpParameters(DressUpView, false, true, parent)
{
    auto* proxy = new QWidget(this);
    auto* layout = new QVBoxLayout(proxy);

    auto* label = new QLabel(tr("Select the splitting tool\n"
                                "(sketch, datum plane, face, or body)\n"
                                "to divide the body into separate solids."),
                             proxy);
    label->setWordWrap(true);
    layout->addWidget(label);

    btnSelectTool = new QPushButton(tr("Select Splitting Tool"), proxy);
    layout->addWidget(btnSelectTool);

    lblTool = new QLabel(tr("Tool: (none)"), proxy);
    layout->addWidget(lblTool);

    this->groupLayout()->addWidget(proxy);

    connect(btnSelectTool, &QPushButton::clicked, this, &TaskSplitBodyParameters::onSelectTool);
}

TaskSplitBodyParameters::~TaskSplitBodyParameters() = default;

void TaskSplitBodyParameters::onSelectTool()
{
    selectingTool = !selectingTool;
    if (selectingTool) {
        btnSelectTool->setText(tr("Picking tool â€¦ (click again to cancel)"));
    }
    else {
        btnSelectTool->setText(tr("Select Splitting Tool"));
    }
}

void TaskSplitBodyParameters::onRefDeleted()
{
    // handled by base class
}

void TaskSplitBodyParameters::setButtons(const selectionModes mode)
{
    Q_UNUSED(mode);
}

void TaskSplitBodyParameters::changeEvent(QEvent* e)
{
    TaskBox::changeEvent(e);
}

void TaskSplitBodyParameters::onSelectionChanged(const Gui::SelectionChanges& msg)
{
    if (msg.Type != Gui::SelectionChanges::AddSelection) {
        return;
    }
    if (!msg.pObjectName) {
        return;
    }

    if (selectingTool) {
        auto feat = dynamic_cast<PartDesign::SplitBody*>(getDressUpView()->getObject());
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
        if (msg.pSubName && std::string(msg.pSubName).rfind("Face", 0) == 0) {
            referenceSelected(msg, this->findChild<QListWidget*>());
        }
    }
}

void TaskSplitBodyParameters::apply()
{
    // SplittingTool is set directly on the feature during selection
}

//**************************************************************************
// TaskDialog
//**************************************************************************

TaskDlgSplitBodyParameters::TaskDlgSplitBodyParameters(ViewProviderSplitBody* DressUpView)
    : TaskDlgDressUpParameters(DressUpView)
{
    parameter = new TaskSplitBodyParameters(DressUpView);

    Content.push_back(parameter);
    Content.push_back(preview);
}

TaskDlgSplitBodyParameters::~TaskDlgSplitBodyParameters() = default;

bool TaskDlgSplitBodyParameters::accept()
{
    auto obj = getObject();
    if (!obj->isError()) {
        getViewObject()->showPreviousFeature(false);
    }

    parameter->apply();

    return TaskDlgDressUpParameters::accept();
}

#include "moc_TaskSplitBodyParameters.cpp"
