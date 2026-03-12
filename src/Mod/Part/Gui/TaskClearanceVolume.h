/***************************************************************************
 *   Copyright (c) 2026 UniCAD Project                                     *
 *                                                                         *
 *   This file is part of UniCAD.                                          *
 *                                                                         *
 *   UniCAD is free software: you can redistribute it and/or modify it     *
 *   under the terms of the GNU Lesser General Public License as           *
 *   published by the Free Software Foundation, either version 2.1 of the  *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   UniCAD is distributed in the hope that it will be useful, but         *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU      *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with UniCAD. If not, see                                *
 *   <https://www.gnu.org/licenses/>.                                      *
 ***************************************************************************/

#ifndef PARTGUI_TASK_CLEARANCE_VOLUME_H
#define PARTGUI_TASK_CLEARANCE_VOLUME_H

#include <Gui/TaskView/TaskDialog.h>
#include <Gui/TaskView/TaskView.h>
#include <Gui/Selection/Selection.h>

#include <QWidget>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>

namespace Part {
class ClearanceVolume;
}

namespace PartGui {

/**
 * @brief Panel widget for ClearanceVolume settings
 */
class TaskClearanceVolumePanel : public QWidget, public Gui::SelectionObserver
{
    Q_OBJECT

public:
    TaskClearanceVolumePanel(Part::ClearanceVolume* feature, QWidget* parent = nullptr);
    ~TaskClearanceVolumePanel() override;

    /// Apply changes to feature
    bool accept();
    
    /// Reject changes
    bool reject();

protected:
    /// Selection changed callback
    void onSelectionChanged(const Gui::SelectionChanges& msg) override;

private Q_SLOTS:
    void onPresetChanged(int index);
    void onSelectFaceClicked();
    void onOffsetChanged(double value);
    void onOffsetReverseChanged(double value);
    void onDepthChanged(double value);
    void onDepthReverseChanged(double value);
    void onSymmetricChanged(bool checked);
    void onAutoSubtractChanged(bool checked);
    void onPortNameChanged(const QString& text);

private:
    void setupUi();
    void loadFromFeature();
    void updatePreview();
    
    Part::ClearanceVolume* m_feature;
    
    // UI elements
    QComboBox* m_presetCombo;
    QDoubleSpinBox* m_offsetSpin;
    QDoubleSpinBox* m_offsetReverseSpin;
    QDoubleSpinBox* m_depthSpin;
    QDoubleSpinBox* m_depthReverseSpin;
    QCheckBox* m_symmetricCheck;
    QCheckBox* m_autoSubtractCheck;
    QLineEdit* m_portNameEdit;
    QPushButton* m_selectFaceBtn;
    QLabel* m_faceLabel;
    
    // Preset values for different printing technologies
    struct PrintingPreset {
        QString name;
        double offset;
        QString description;
        
        PrintingPreset(const QString& n, double o, const QString& d)
            : name(n), offset(o), description(d) {}
    };
    std::vector<PrintingPreset> m_presets;
};

/**
 * @brief Task dialog for creating/editing ClearanceVolume
 */
class TaskClearanceVolume : public Gui::TaskView::TaskDialog
{
    Q_OBJECT

public:
    TaskClearanceVolume(Part::ClearanceVolume* feature);
    ~TaskClearanceVolume() override;

    void open() override;
    bool accept() override;
    bool reject() override;
    
    QDialogButtonBox::StandardButtons getStandardButtons() const override;

private:
    TaskClearanceVolumePanel* m_panel;
    Part::ClearanceVolume* m_feature;
};

} // namespace PartGui

#endif // PARTGUI_TASK_CLEARANCE_VOLUME_H
