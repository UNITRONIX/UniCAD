// SPDX-License-Identifier: LGPL-2.1-or-later
// UniCAD by UNITRONIX â€” Task panel for Delete Face feature.

#ifndef GUI_TASKVIEW_TaskDeleteFaceParameters_H
#define GUI_TASKVIEW_TaskDeleteFaceParameters_H

#include "TaskDressUpParameters.h"
#include "ViewProviderDeleteFace.h"

namespace PartDesignGui
{

class TaskDeleteFaceParameters : public TaskDressUpParameters
{
    Q_OBJECT

public:
    explicit TaskDeleteFaceParameters(ViewProviderDressUp* DressUpView, QWidget* parent = nullptr);
    ~TaskDeleteFaceParameters() override;

    void apply() override;

private Q_SLOTS:
    void onRefDeleted() override;

protected:
    void changeEvent(QEvent* e) override;
    void onSelectionChanged(const Gui::SelectionChanges& msg) override;
    void setButtons(const selectionModes mode) override;
};

class TaskDlgDeleteFaceParameters : public TaskDlgDressUpParameters
{
    Q_OBJECT

public:
    explicit TaskDlgDeleteFaceParameters(ViewProviderDeleteFace* DressUpView);
    ~TaskDlgDeleteFaceParameters() override;

    bool accept() override;
};

}  // namespace PartDesignGui

#endif  // GUI_TASKVIEW_TaskDeleteFaceParameters_H
