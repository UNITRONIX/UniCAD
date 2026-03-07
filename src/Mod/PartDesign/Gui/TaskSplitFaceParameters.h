// SPDX-License-Identifier: LGPL-2.1-or-later
// FusionCAD by UNITRONIX — Task panel for Split Face feature.

#ifndef GUI_TASKVIEW_TaskSplitFaceParameters_H
#define GUI_TASKVIEW_TaskSplitFaceParameters_H

#include "TaskDressUpParameters.h"
#include "ViewProviderSplitFace.h"

class QPushButton;
class QLabel;

namespace PartDesignGui
{

class TaskSplitFaceParameters : public TaskDressUpParameters
{
    Q_OBJECT

public:
    explicit TaskSplitFaceParameters(ViewProviderDressUp* DressUpView, QWidget* parent = nullptr);
    ~TaskSplitFaceParameters() override;

    void apply() override;

private Q_SLOTS:
    void onRefDeleted() override;
    void onSelectTool();

protected:
    void changeEvent(QEvent* e) override;
    void onSelectionChanged(const Gui::SelectionChanges& msg) override;
    void setButtons(const selectionModes mode) override;

private:
    bool selectingTool = false;
    QPushButton* btnSelectTool = nullptr;
    QLabel* lblTool = nullptr;
};

class TaskDlgSplitFaceParameters : public TaskDlgDressUpParameters
{
    Q_OBJECT

public:
    explicit TaskDlgSplitFaceParameters(ViewProviderSplitFace* DressUpView);
    ~TaskDlgSplitFaceParameters() override;

    bool accept() override;
};

}  // namespace PartDesignGui

#endif  // GUI_TASKVIEW_TaskSplitFaceParameters_H
