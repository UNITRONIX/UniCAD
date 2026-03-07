// SPDX-License-Identifier: LGPL-2.1-or-later
// FusionCAD by UNITRONIX — Task panel for Replace Face feature.

#ifndef GUI_TASKVIEW_TaskReplaceFaceParameters_H
#define GUI_TASKVIEW_TaskReplaceFaceParameters_H

#include "TaskDressUpParameters.h"
#include "ViewProviderReplaceFace.h"

class QPushButton;
class QLabel;

namespace PartDesignGui
{

class TaskReplaceFaceParameters : public TaskDressUpParameters
{
    Q_OBJECT

public:
    explicit TaskReplaceFaceParameters(ViewProviderDressUp* DressUpView, QWidget* parent = nullptr);
    ~TaskReplaceFaceParameters() override;

    void apply() override;

private Q_SLOTS:
    void onRefDeleted() override;
    void onSelectTarget();

protected:
    void changeEvent(QEvent* e) override;
    void onSelectionChanged(const Gui::SelectionChanges& msg) override;
    void setButtons(const selectionModes mode) override;

private:
    bool selectingTarget = false;
    QPushButton* btnSelectTarget = nullptr;
    QLabel* lblTarget = nullptr;
};

class TaskDlgReplaceFaceParameters : public TaskDlgDressUpParameters
{
    Q_OBJECT

public:
    explicit TaskDlgReplaceFaceParameters(ViewProviderReplaceFace* DressUpView);
    ~TaskDlgReplaceFaceParameters() override;

    bool accept() override;
};

}  // namespace PartDesignGui

#endif  // GUI_TASKVIEW_TaskReplaceFaceParameters_H
