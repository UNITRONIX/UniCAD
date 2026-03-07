// SPDX-License-Identifier: LGPL-2.1-or-later
// FusionCAD by UNITRONIX — Task panel for unified Loft feature.

#ifndef GUI_TASKVIEW_TaskUnifiedLoftParameters_H
#define GUI_TASKVIEW_TaskUnifiedLoftParameters_H

#include "TaskLoftParameters.h"
#include "ViewProviderUnifiedLoft.h"

class QComboBox;

namespace PartDesignGui
{

class TaskDlgUnifiedLoftParameters : public TaskDlgLoftParameters
{
    Q_OBJECT

public:
    explicit TaskDlgUnifiedLoftParameters(ViewProviderUnifiedLoft* LoftView);

    bool accept() override;

private Q_SLOTS:
    void onOperationChanged(int index);

private:
    QComboBox* operationCombo;
};

}  // namespace PartDesignGui

#endif
