// SPDX-License-Identifier: LGPL-2.1-or-later
// FusionCAD by UNITRONIX — Task panel for unified Extrude feature.

#ifndef GUI_TASKVIEW_TaskUnifiedExtrudeParameters_H
#define GUI_TASKVIEW_TaskUnifiedExtrudeParameters_H

#include "TaskExtrudeParameters.h"
#include "ViewProviderUnifiedExtrude.h"

class QComboBox;

namespace PartDesignGui
{

class TaskUnifiedExtrudeParameters : public TaskExtrudeParameters
{
    Q_OBJECT

public:
    explicit TaskUnifiedExtrudeParameters(
        ViewProviderUnifiedExtrude* ExtrudeView,
        QWidget* parent = nullptr,
        bool newObj = false
    );
    ~TaskUnifiedExtrudeParameters() override;

    void apply() override;

private Q_SLOTS:
    void onOperationChanged(int index);

private:
    void onModeChanged(int index, Side side) override;
    void translateModeList(QComboBox* box, int index) override;
    void updateUI(Side side) override;
    void onOperationAutoSwitched(int newOpIndex) override;

    QComboBox* operationCombo = nullptr;
};

/// Task dialog for unified Extrude
class TaskDlgUnifiedExtrudeParameters : public TaskDlgExtrudeParameters
{
    Q_OBJECT

public:
    explicit TaskDlgUnifiedExtrudeParameters(
        ViewProviderUnifiedExtrude* ExtrudeView,
        bool newObj = false
    );

protected:
    TaskExtrudeParameters* getTaskParameters() override
    {
        return parameters;
    }

private:
    TaskUnifiedExtrudeParameters* parameters;
};

}  // namespace PartDesignGui

#endif  // GUI_TASKVIEW_TaskUnifiedExtrudeParameters_H
