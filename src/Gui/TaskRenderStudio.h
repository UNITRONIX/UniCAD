/***************************************************************************
 *   Copyright (c) 2026 UNITRONIX                                          *
 *   UniCAD - A fork of FreeCAD                                            *
 ***************************************************************************/

#ifndef GUI_TASKRENDERSTUDIO_H
#define GUI_TASKRENDERSTUDIO_H

#include <Gui/TaskView/TaskDialog.h>
#include <Gui/TaskView/TaskView.h>

class QDoubleSpinBox;
class QSpinBox;
class QCheckBox;

namespace Gui
{

class TaskRenderStudioBox: public Gui::TaskView::TaskBox
{
    Q_OBJECT

public:
    explicit TaskRenderStudioBox(QWidget* parent = nullptr);
    ~TaskRenderStudioBox() override = default;

private Q_SLOTS:
    void onLightingChanged();
    void onGroundToggled(bool checked);
    void onFillToggled(bool checked);
    void onBackToggled(bool checked);
    void onAmbientChanged(int value);

private:
    void loadFromPreferences();
    void applyLightingToPreferences();

    QDoubleSpinBox* m_azimuth = nullptr;
    QDoubleSpinBox* m_elevation = nullptr;
    QSpinBox* m_intensity = nullptr;
    QSpinBox* m_ambient = nullptr;
    QCheckBox* m_ground = nullptr;
    QCheckBox* m_fill = nullptr;
    QCheckBox* m_back = nullptr;
};

class GuiExport TaskRenderStudio: public Gui::TaskView::TaskDialog
{
    Q_OBJECT

public:
    TaskRenderStudio();
    ~TaskRenderStudio() override = default;

    bool isAllowedAlterDocument() const override
    {
        return true;
    }

    QDialogButtonBox::StandardButtons getStandardButtons() const override
    {
        return QDialogButtonBox::Close;
    }

private:
    TaskRenderStudioBox* m_box = nullptr;
};

}  // namespace Gui

#endif  // GUI_TASKRENDERSTUDIO_H
