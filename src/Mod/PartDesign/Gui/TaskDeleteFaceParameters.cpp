// SPDX-License-Identifier: LGPL-2.1-or-later
// FusionCAD by UNITRONIX — Task panel for Delete Face feature.

#include <QAction>
#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>

#include <App/Document.h>
#include <App/DocumentObject.h>
#include <Base/Interpreter.h>
#include <Gui/Command.h>
#include <Gui/Selection/Selection.h>
#include <Gui/ViewProvider.h>
#include <Mod/PartDesign/App/FeatureDeleteFace.h>

#include "TaskDeleteFaceParameters.h"

using namespace PartDesignGui;
using namespace Gui;

/* TRANSLATOR PartDesignGui::TaskDeleteFaceParameters */

TaskDeleteFaceParameters::TaskDeleteFaceParameters(ViewProviderDressUp* DressUpView,
                                                   QWidget* parent)
    : TaskDressUpParameters(DressUpView, false, true, parent)
{
    auto* proxy = new QWidget(this);
    auto* layout = new QVBoxLayout(proxy);

    auto* label = new QLabel(tr("Select faces to delete and heal.\n"
                                "The solid will be rebuilt without the selected faces."),
                             proxy);
    label->setWordWrap(true);
    layout->addWidget(label);

    this->groupLayout()->addWidget(proxy);
}

TaskDeleteFaceParameters::~TaskDeleteFaceParameters() = default;

void TaskDeleteFaceParameters::onRefDeleted()
{
    // handled by base class
}

void TaskDeleteFaceParameters::setButtons(const selectionModes mode)
{
    Q_UNUSED(mode);
}

void TaskDeleteFaceParameters::changeEvent(QEvent* e)
{
    TaskBox::changeEvent(e);
}

void TaskDeleteFaceParameters::onSelectionChanged(const Gui::SelectionChanges& msg)
{
    if (msg.Type == Gui::SelectionChanges::AddSelection) {
        if (msg.pSubName && std::string(msg.pSubName).rfind("Face", 0) == 0) {
            referenceSelected(msg, this->findChild<QListWidget*>());
        }
    }
}

void TaskDeleteFaceParameters::apply()
{
}

//**************************************************************************
// TaskDialog
//**************************************************************************

TaskDlgDeleteFaceParameters::TaskDlgDeleteFaceParameters(ViewProviderDeleteFace* DressUpView)
    : TaskDlgDressUpParameters(DressUpView)
{
    parameter = new TaskDeleteFaceParameters(DressUpView);

    Content.push_back(parameter);
    Content.push_back(preview);
}

TaskDlgDeleteFaceParameters::~TaskDlgDeleteFaceParameters() = default;

bool TaskDlgDeleteFaceParameters::accept()
{
    auto obj = getObject();
    if (!obj->isError()) {
        getViewObject()->showPreviousFeature(false);
    }

    parameter->apply();

    return TaskDlgDressUpParameters::accept();
}

#include "moc_TaskDeleteFaceParameters.cpp"
