// SPDX-License-Identifier: LGPL-2.1-or-later
// UniCAD by UNITRONIX â€” Task panel for Move Face feature.

#ifndef GUI_TASKVIEW_TaskMoveFaceParameters_H
#define GUI_TASKVIEW_TaskMoveFaceParameters_H

#include "TaskDressUpParameters.h"
#include "ViewProviderMoveFace.h"

class QDoubleSpinBox;

namespace PartDesignGui
{

class TaskMoveFaceParameters : public TaskDressUpParameters
{
    Q_OBJECT

public:
    explicit TaskMoveFaceParameters(ViewProviderDressUp* DressUpView, QWidget* parent = nullptr);
    ~TaskMoveFaceParameters() override;

    void apply() override;

    double getDistance() const;

private Q_SLOTS:
    void onDistanceChanged(double val);
    void onRefDeleted() override;

protected:
    void changeEvent(QEvent* e) override;
    void onSelectionChanged(const Gui::SelectionChanges& msg) override;
    void setButtons(const selectionModes mode) override;

private:
    QDoubleSpinBox* distanceSpinBox = nullptr;
};

class TaskDlgMoveFaceParameters : public TaskDlgDressUpParameters
{
    Q_OBJECT

public:
    explicit TaskDlgMoveFaceParameters(ViewProviderMoveFace* DressUpView);
    ~TaskDlgMoveFaceParameters() override;

    bool accept() override;
};

}  // namespace PartDesignGui

#endif  // GUI_TASKVIEW_TaskMoveFaceParameters_H
