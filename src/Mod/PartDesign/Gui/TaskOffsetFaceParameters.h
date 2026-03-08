// SPDX-License-Identifier: LGPL-2.1-or-later
// UniCAD by UNITRONIX â€” Task panel for Offset Face feature.

#ifndef GUI_TASKVIEW_TaskOffsetFaceParameters_H
#define GUI_TASKVIEW_TaskOffsetFaceParameters_H

#include "TaskDressUpParameters.h"
#include "ViewProviderOffsetFace.h"

class QDoubleSpinBox;

namespace PartDesignGui
{

class TaskOffsetFaceParameters : public TaskDressUpParameters
{
    Q_OBJECT

public:
    explicit TaskOffsetFaceParameters(ViewProviderDressUp* DressUpView, QWidget* parent = nullptr);
    ~TaskOffsetFaceParameters() override;

    void apply() override;

    double getOffset() const;

private Q_SLOTS:
    void onOffsetChanged(double val);
    void onRefDeleted() override;

protected:
    void changeEvent(QEvent* e) override;
    void onSelectionChanged(const Gui::SelectionChanges& msg) override;
    void setButtons(const selectionModes mode) override;

private:
    QDoubleSpinBox* offsetSpinBox = nullptr;
};

class TaskDlgOffsetFaceParameters : public TaskDlgDressUpParameters
{
    Q_OBJECT

public:
    explicit TaskDlgOffsetFaceParameters(ViewProviderOffsetFace* DressUpView);
    ~TaskDlgOffsetFaceParameters() override;

    bool accept() override;
};

}  // namespace PartDesignGui

#endif  // GUI_TASKVIEW_TaskOffsetFaceParameters_H
