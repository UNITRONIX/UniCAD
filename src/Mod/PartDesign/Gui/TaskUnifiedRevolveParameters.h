// SPDX-License-Identifier: LGPL-2.1-or-later
// UniCAD by UNITRONIX â€” Task panel for unified Revolve feature.

#ifndef GUI_TASKVIEW_TaskUnifiedRevolveParameters_H
#define GUI_TASKVIEW_TaskUnifiedRevolveParameters_H

#include "TaskRevolutionParameters.h"
#include "ViewProviderUnifiedRevolve.h"

class QComboBox;

namespace PartDesignGui
{

class TaskDlgUnifiedRevolveParameters : public TaskDlgSketchBasedParameters
{
    Q_OBJECT

public:
    explicit TaskDlgUnifiedRevolveParameters(ViewProviderUnifiedRevolve* RevolveView);

    bool accept() override;

private Q_SLOTS:
    void onOperationChanged(int index);

private:
    TaskRevolutionParameters* revolutionParams;
    QComboBox* operationCombo;
};

}  // namespace PartDesignGui

#endif
