// SPDX-License-Identifier: LGPL-2.1-or-later
// UniCAD by UNITRONIX â€” Task panel for Split Body feature.

#ifndef GUI_TASKVIEW_TaskSplitBodyParameters_H
#define GUI_TASKVIEW_TaskSplitBodyParameters_H

#include "TaskDressUpParameters.h"
#include "ViewProviderSplitBody.h"

class QPushButton;
class QLabel;

namespace PartDesignGui
{

class TaskSplitBodyParameters : public TaskDressUpParameters
{
    Q_OBJECT

public:
    explicit TaskSplitBodyParameters(ViewProviderDressUp* DressUpView, QWidget* parent = nullptr);
    ~TaskSplitBodyParameters() override;

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

class TaskDlgSplitBodyParameters : public TaskDlgDressUpParameters
{
    Q_OBJECT

public:
    explicit TaskDlgSplitBodyParameters(ViewProviderSplitBody* DressUpView);
    ~TaskDlgSplitBodyParameters() override;

    bool accept() override;
};

}  // namespace PartDesignGui

#endif
