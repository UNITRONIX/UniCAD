// SPDX-License-Identifier: LGPL-2.1-or-later
// FusionCAD by UNITRONIX — Task panel for unified Sweep feature.

#ifndef GUI_TASKVIEW_TaskUnifiedSweepParameters_H
#define GUI_TASKVIEW_TaskUnifiedSweepParameters_H

#include "TaskPipeParameters.h"
#include "ViewProviderUnifiedSweep.h"

class QComboBox;

namespace PartDesignGui
{

class TaskDlgUnifiedSweepParameters : public TaskDlgPipeParameters
{
    Q_OBJECT

public:
    explicit TaskDlgUnifiedSweepParameters(ViewProviderUnifiedSweep* SweepView);

    bool accept() override;

private Q_SLOTS:
    void onOperationChanged(int index);

private:
    QComboBox* operationCombo;
};

}  // namespace PartDesignGui

#endif
