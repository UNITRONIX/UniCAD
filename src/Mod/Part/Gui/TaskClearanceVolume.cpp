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

#include "PreCompiled.h"

#include "TaskClearanceVolume.h"

#include <Mod/Part/App/FeatureClearanceVolume.h>

#include <App/Application.h>
#include <App/Document.h>
#include <App/DocumentObject.h>
#include <Gui/Application.h>
#include <Gui/Document.h>
#include <Gui/Selection/Selection.h>
#include <Gui/Command.h>
#include <Gui/TaskView/TaskView.h>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>

#include <BRepAlgoAPI_Cut.hxx>
#include <TopoDS_Shape.hxx>
#include <TopAbs_ShapeEnum.hxx>

using namespace PartGui;

// =============================================================================
// TaskClearanceVolumePanel
// =============================================================================

TaskClearanceVolumePanel::TaskClearanceVolumePanel(Part::ClearanceVolume* feature, QWidget* parent)
    : QWidget(parent)
    , SelectionObserver(true, Gui::ResolveMode::NoResolve)
    , m_feature(feature)
{
    // Define presets for different 3D printing technologies  
    m_presets.emplace_back(QStringLiteral("Custom"), 0.0, QStringLiteral("Set custom tolerance value"));
    m_presets.emplace_back(QStringLiteral("FDM - Standard (0.2mm)"), 0.2, QStringLiteral("Standard FDM printer tolerance"));
    m_presets.emplace_back(QStringLiteral("FDM - Tight (0.15mm)"), 0.15, QStringLiteral("High quality FDM with good calibration"));
    m_presets.emplace_back(QStringLiteral("FDM - Loose (0.3mm)"), 0.3, QStringLiteral("Larger tolerance for rough FDM prints"));
    m_presets.emplace_back(QStringLiteral("FDM - Interference (0.4mm)"), 0.4, QStringLiteral("For snap-fit and interference fits"));
    m_presets.emplace_back(QStringLiteral("SLA/DLP (0.1mm)"), 0.1, QStringLiteral("Resin printing tolerance"));
    m_presets.emplace_back(QStringLiteral("SLA/DLP - Tight (0.05mm)"), 0.05, QStringLiteral("High precision resin printing"));
    m_presets.emplace_back(QStringLiteral("SLS (0.15mm)"), 0.15, QStringLiteral("Selective Laser Sintering tolerance"));
    m_presets.emplace_back(QStringLiteral("CNC Machining (0.05mm)"), 0.05, QStringLiteral("CNC machined parts tolerance"));
    
    setupUi();
    loadFromFeature();
}

TaskClearanceVolumePanel::~TaskClearanceVolumePanel()
{
}

void TaskClearanceVolumePanel::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);
    
    // Port identification group
    auto* identGroup = new QGroupBox(tr("Port Identification"));
    auto* identLayout = new QFormLayout(identGroup);
    
    m_portNameEdit = new QLineEdit();
    m_portNameEdit->setPlaceholderText(tr("e.g., USB-C, HDMI, Power Jack"));
    identLayout->addRow(tr("Port Name:"), m_portNameEdit);
    connect(m_portNameEdit, &QLineEdit::textChanged, 
            this, &TaskClearanceVolumePanel::onPortNameChanged);
    
    mainLayout->addWidget(identGroup);
    
    // Face selection group
    auto* selectGroup = new QGroupBox(tr("Source Face"));
    auto* selectLayout = new QVBoxLayout(selectGroup);
    
    m_faceLabel = new QLabel(tr("No face selected"));
    m_faceLabel->setStyleSheet(QStringLiteral("color: gray; font-style: italic;"));
    selectLayout->addWidget(m_faceLabel);
    
    m_selectFaceBtn = new QPushButton(tr("Select Face..."));
    m_selectFaceBtn->setIcon(QIcon::fromTheme(QStringLiteral("edit-select")));
    selectLayout->addWidget(m_selectFaceBtn);
    connect(m_selectFaceBtn, &QPushButton::clicked,
            this, &TaskClearanceVolumePanel::onSelectFaceClicked);
    
    mainLayout->addWidget(selectGroup);
    
    // Tolerance group
    auto* toleranceGroup = new QGroupBox(tr("Tolerance / Offset"));
    auto* toleranceLayout = new QFormLayout(toleranceGroup);
    
    m_presetCombo = new QComboBox();
    for (const auto& preset : m_presets) {
        m_presetCombo->addItem(preset.name);
    }
    toleranceLayout->addRow(tr("Preset:"), m_presetCombo);
    connect(m_presetCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &TaskClearanceVolumePanel::onPresetChanged);
    
    m_offsetSpin = new QDoubleSpinBox();
    m_offsetSpin->setRange(0.0, 10.0);
    m_offsetSpin->setDecimals(3);
    m_offsetSpin->setSingleStep(0.05);
    m_offsetSpin->setSuffix(QStringLiteral(" mm"));
    m_offsetSpin->setToolTip(tr("Tolerance margin around the face outline (outward direction).\n"
                                 "This value is added to account for 3D printing accuracy."));
    toleranceLayout->addRow(tr("Offset (outward):"), m_offsetSpin);
    connect(m_offsetSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &TaskClearanceVolumePanel::onOffsetChanged);
    
    m_offsetReverseSpin = new QDoubleSpinBox();
    m_offsetReverseSpin->setRange(0.0, 10.0);
    m_offsetReverseSpin->setDecimals(3);
    m_offsetReverseSpin->setSingleStep(0.05);
    m_offsetReverseSpin->setSuffix(QStringLiteral(" mm"));
    m_offsetReverseSpin->setToolTip(tr("Tolerance margin for the inner part (into port).\n"
                                        "Set to 0 to use the same offset as outward direction."));
    toleranceLayout->addRow(tr("Offset (into port):"), m_offsetReverseSpin);
    connect(m_offsetReverseSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &TaskClearanceVolumePanel::onOffsetReverseChanged);
    
    mainLayout->addWidget(toleranceGroup);
    
    // Extrusion group
    auto* extrudeGroup = new QGroupBox(tr("Extrusion"));
    auto* extrudeLayout = new QFormLayout(extrudeGroup);
    
    m_depthSpin = new QDoubleSpinBox();
    m_depthSpin->setRange(0.0, 100.0);
    m_depthSpin->setDecimals(2);
    m_depthSpin->setSingleStep(1.0);
    m_depthSpin->setSuffix(QStringLiteral(" mm"));
    m_depthSpin->setToolTip(tr("How far the clearance volume extends outward from the face.\n"
                                "Should be at least the thickness of your enclosure wall."));
    extrudeLayout->addRow(tr("Depth (outward):"), m_depthSpin);
    connect(m_depthSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &TaskClearanceVolumePanel::onDepthChanged);
    
    m_depthReverseSpin = new QDoubleSpinBox();
    m_depthReverseSpin->setRange(0.0, 100.0);
    m_depthReverseSpin->setDecimals(2);
    m_depthReverseSpin->setSingleStep(1.0);
    m_depthReverseSpin->setSuffix(QStringLiteral(" mm"));
    m_depthReverseSpin->setToolTip(tr("How far the clearance extends INTO the connector.\n"
                                       "Use this to protect the entire port from wall collisions."));
    extrudeLayout->addRow(tr("Depth (into port):"), m_depthReverseSpin);
    connect(m_depthReverseSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &TaskClearanceVolumePanel::onDepthReverseChanged);
    
    m_symmetricCheck = new QCheckBox(tr("Symmetric (both directions)"));
    m_symmetricCheck->setToolTip(tr("Extrude the clearance volume in both directions from the face."));
    extrudeLayout->addRow(m_symmetricCheck);
    connect(m_symmetricCheck, &QCheckBox::toggled,
            this, &TaskClearanceVolumePanel::onSymmetricChanged);
    
    m_flipDirectionCheck = new QCheckBox(tr("Flip direction"));
    m_flipDirectionCheck->setToolTip(tr("Reverse the extrusion direction.\n"
                                         "Use this if the clearance volume goes the wrong way."));
    extrudeLayout->addRow(m_flipDirectionCheck);
    connect(m_flipDirectionCheck, &QCheckBox::toggled,
            this, &TaskClearanceVolumePanel::onFlipDirectionChanged);
    
    mainLayout->addWidget(extrudeGroup);
    
    // Behavior group  
    auto* behaviorGroup = new QGroupBox(tr("Cutting"));
    auto* behaviorLayout = new QFormLayout(behaviorGroup);
    
    // Target body selector
    m_targetBodyCombo = new QComboBox();
    m_targetBodyCombo->setToolTip(tr("Select the solid body to cut the clearance hole from"));
    behaviorLayout->addRow(tr("Target Body:"), m_targetBodyCombo);
    connect(m_targetBodyCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &TaskClearanceVolumePanel::onTargetBodyChanged);
    
    // Populate target body combo with available solids
    if (m_feature && m_feature->getDocument()) {
        m_targetBodyCombo->addItem(tr("(None)"), QVariant());
        auto* doc = m_feature->getDocument();
        for (auto* obj : doc->getObjects()) {
            if (obj == m_feature) continue;  // Skip self
            auto* partFeat = dynamic_cast<Part::Feature*>(obj);
            if (partFeat && !partFeat->Shape.getValue().IsNull()) {
                // Check if it's a solid
                TopAbs_ShapeEnum shapeType = partFeat->Shape.getValue().ShapeType();
                if (shapeType == TopAbs_SOLID || shapeType == TopAbs_COMPOUND || 
                    shapeType == TopAbs_COMPSOLID) {
                    QString label = QString::fromStdString(obj->Label.getValue());
                    m_targetBodyCombo->addItem(label, QVariant::fromValue(
                        reinterpret_cast<quintptr>(obj)));
                }
            }
        }
    }
    
    m_autoSubtractCheck = new QCheckBox(tr("Auto-subtract when exporting"));
    m_autoSubtractCheck->setToolTip(tr("Automatically cut this clearance volume when exporting to STL/STEP"));
    behaviorLayout->addRow(m_autoSubtractCheck);
    connect(m_autoSubtractCheck, &QCheckBox::toggled,
            this, &TaskClearanceVolumePanel::onAutoSubtractChanged);
    
    // Apply Cut button
    m_applyCutBtn = new QPushButton(tr("Apply Cut Now"));
    m_applyCutBtn->setToolTip(tr("Create a new object with the clearance volume cut from the target body"));
    m_applyCutBtn->setIcon(QIcon::fromTheme(QStringLiteral("edit-cut")));
    behaviorLayout->addRow(m_applyCutBtn);
    connect(m_applyCutBtn, &QPushButton::clicked,
            this, &TaskClearanceVolumePanel::onApplyCutClicked);
    
    mainLayout->addWidget(behaviorGroup);
    
    // Info label
    auto* infoLabel = new QLabel(tr(
        "<b>Tip:</b> Select a target body above and click 'Apply Cut Now' to create "
        "a new object with the clearance hole cut out, ready for 3D printing."));
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet(QStringLiteral("color: #666; font-size: 11px; padding: 8px;"));
    mainLayout->addWidget(infoLabel);
    
    mainLayout->addStretch();
}

void TaskClearanceVolumePanel::loadFromFeature()
{
    if (!m_feature) {
        return;
    }
    
    // Load values from feature
    m_portNameEdit->setText(QString::fromStdString(m_feature->PortName.getValue()));
    m_offsetSpin->setValue(m_feature->Offset.getValue());
    m_offsetReverseSpin->setValue(m_feature->OffsetReverse.getValue());
    m_depthSpin->setValue(m_feature->Depth.getValue());
    m_depthReverseSpin->setValue(m_feature->DepthReverse.getValue());
    m_symmetricCheck->setChecked(m_feature->Symmetric.getValue());
    m_flipDirectionCheck->setChecked(m_feature->FlipDirection.getValue());
    m_autoSubtractCheck->setChecked(m_feature->AutoSubtract.getValue());
    
    // Set target body combo
    App::DocumentObject* targetBody = m_feature->TargetBody.getValue();
    if (targetBody) {
        for (int i = 0; i < m_targetBodyCombo->count(); ++i) {
            QVariant data = m_targetBodyCombo->itemData(i);
            if (!data.isNull()) {
                auto* obj = reinterpret_cast<App::DocumentObject*>(data.value<quintptr>());
                if (obj == targetBody) {
                    m_targetBodyCombo->setCurrentIndex(i);
                    break;
                }
            }
        }
    }
    
    // Update face label
    App::DocumentObject* source = m_feature->SourceFaces.getValue();
    if (source) {
        const auto& subs = m_feature->SourceFaces.getSubValues();
        QString faceInfo = QString::fromStdString(source->Label.getValue());
        if (!subs.empty()) {
            faceInfo += QStringLiteral(" : ") + QString::fromStdString(subs[0]);
        }
        m_faceLabel->setText(faceInfo);
        m_faceLabel->setStyleSheet(QStringLiteral("color: green;"));
    }
    
    // Set preset combo to "Custom" since we loaded existing values
    m_presetCombo->setCurrentIndex(0);
}

bool TaskClearanceVolumePanel::accept()
{
    if (!m_feature) {
        return false;
    }
    
    // Values are already applied via signals
    // Just recompute to ensure everything is up to date
    m_feature->recomputeFeature();
    
    return true;
}

bool TaskClearanceVolumePanel::reject()
{
    // Could restore original values here if needed
    return true;
}

void TaskClearanceVolumePanel::onSelectionChanged(const Gui::SelectionChanges& msg)
{
    if (msg.Type != Gui::SelectionChanges::AddSelection) {
        return;
    }
    
    // Check if a face was selected
    if (!msg.pSubName || strlen(msg.pSubName) == 0) {
        return;
    }
    
    std::string subName = msg.pSubName;
    if (subName.find("Face") != 0) {
        return; // Not a face
    }
    
    // Get the selected object
    App::Document* doc = App::GetApplication().getDocument(msg.pDocName);
    if (!doc) {
        return;
    }
    
    App::DocumentObject* obj = doc->getObject(msg.pObjectName);
    if (!obj) {
        return;
    }
    
    // Update the feature
    if (m_feature) {
        m_feature->SourceFaces.setValue(obj, {subName});
        
        // Update UI
        QString faceInfo = QString::fromStdString(obj->Label.getValue());
        faceInfo += QStringLiteral(" : ") + QString::fromStdString(subName);
        m_faceLabel->setText(faceInfo);
        m_faceLabel->setStyleSheet(QStringLiteral("color: green;"));
        
        updatePreview();
    }
}

void TaskClearanceVolumePanel::onPresetChanged(int index)
{
    if (index > 0 && index < static_cast<int>(m_presets.size())) {
        m_offsetSpin->setValue(m_presets[index].offset);
    }
}

void TaskClearanceVolumePanel::onSelectFaceClicked()
{
    // Clear current selection and prompt user
    Gui::Selection().clearSelection();
    m_faceLabel->setText(tr("Select a face on the model..."));
    m_faceLabel->setStyleSheet(QStringLiteral("color: blue; font-style: italic;"));
}

void TaskClearanceVolumePanel::onOffsetChanged(double value)
{
    if (m_feature) {
        m_feature->Offset.setValue(value);
        updatePreview();
    }
    
    // Reset preset to "Custom" if value doesn't match any preset
    bool matchesPreset = false;
    for (size_t i = 1; i < m_presets.size(); ++i) {
        if (qAbs(m_presets[i].offset - value) < 0.001) {
            m_presetCombo->blockSignals(true);
            m_presetCombo->setCurrentIndex(static_cast<int>(i));
            m_presetCombo->blockSignals(false);
            matchesPreset = true;
            break;
        }
    }
    if (!matchesPreset) {
        m_presetCombo->blockSignals(true);
        m_presetCombo->setCurrentIndex(0);
        m_presetCombo->blockSignals(false);
    }
}

void TaskClearanceVolumePanel::onOffsetReverseChanged(double value)
{
    if (m_feature) {
        m_feature->OffsetReverse.setValue(value);
        updatePreview();
    }
}

void TaskClearanceVolumePanel::onDepthChanged(double value)
{
    if (m_feature) {
        m_feature->Depth.setValue(value);
        updatePreview();
    }
}

void TaskClearanceVolumePanel::onDepthReverseChanged(double value)
{
    if (m_feature) {
        m_feature->DepthReverse.setValue(value);
        updatePreview();
    }
}

void TaskClearanceVolumePanel::onSymmetricChanged(bool checked)
{
    if (m_feature) {
        m_feature->Symmetric.setValue(checked);
        updatePreview();
    }
}

void TaskClearanceVolumePanel::onFlipDirectionChanged(bool checked)
{
    if (m_feature) {
        m_feature->FlipDirection.setValue(checked);
        updatePreview();
    }
}

void TaskClearanceVolumePanel::onAutoSubtractChanged(bool checked)
{
    if (m_feature) {
        m_feature->AutoSubtract.setValue(checked);
    }
}

void TaskClearanceVolumePanel::onTargetBodyChanged(int index)
{
    if (!m_feature || index < 0) {
        return;
    }
    
    QVariant data = m_targetBodyCombo->itemData(index);
    if (data.isNull()) {
        m_feature->TargetBody.setValue(nullptr);
    }
    else {
        auto* obj = reinterpret_cast<App::DocumentObject*>(data.value<quintptr>());
        m_feature->TargetBody.setValue(obj);
    }
}

void TaskClearanceVolumePanel::onApplyCutClicked()
{
    if (!m_feature) {
        return;
    }
    
    // Get target body
    App::DocumentObject* targetObj = m_feature->TargetBody.getValue();
    if (!targetObj) {
        // Try to get from combo
        int index = m_targetBodyCombo->currentIndex();
        if (index > 0) {
            QVariant data = m_targetBodyCombo->itemData(index);
            if (!data.isNull()) {
                targetObj = reinterpret_cast<App::DocumentObject*>(data.value<quintptr>());
            }
        }
    }
    
    if (!targetObj) {
        QMessageBox::warning(this, tr("No Target"), 
            tr("Please select a target body to cut from."));
        return;
    }
    
    auto* targetPart = dynamic_cast<Part::Feature*>(targetObj);
    if (!targetPart) {
        QMessageBox::warning(this, tr("Invalid Target"), 
            tr("Target must be a Part feature with a solid shape."));
        return;
    }
    
    // Get shapes
    TopoDS_Shape targetShape = targetPart->Shape.getValue();
    TopoDS_Shape clearanceShape = m_feature->Shape.getValue();
    
    if (targetShape.IsNull() || clearanceShape.IsNull()) {
        QMessageBox::warning(this, tr("Invalid Shapes"), 
            tr("One or both shapes are invalid."));
        return;
    }
    
    // Perform Boolean Cut
    BRepAlgoAPI_Cut cutOp(targetShape, clearanceShape);
    cutOp.Build();
    
    if (!cutOp.IsDone()) {
        QMessageBox::warning(this, tr("Cut Failed"), 
            tr("Boolean cut operation failed."));
        return;
    }
    
    TopoDS_Shape resultShape = cutOp.Shape();
    
    // Create new Part::Feature with result
    App::Document* doc = m_feature->getDocument();
    auto* resultFeature = dynamic_cast<Part::Feature*>(
        doc->addObject("Part::Feature", "CutResult"));
    
    if (resultFeature) {
        QString label = QString::fromStdString(targetObj->Label.getValue()) + 
                        QStringLiteral("_Cut");
        resultFeature->Label.setValue(label.toStdString().c_str());
        resultFeature->Shape.setValue(resultShape);
        
        // Hide original and clearance volume
        targetObj->Visibility.setValue(false);
        m_feature->Visibility.setValue(false);
        
        doc->recompute();
        
        QMessageBox::information(this, tr("Cut Applied"), 
            tr("Created new object '%1' with clearance hole cut out.\n\n"
               "Original objects have been hidden.").arg(label));
    }
}

void TaskClearanceVolumePanel::onPortNameChanged(const QString& text)
{
    if (m_feature) {
        m_feature->PortName.setValue(text.toStdString().c_str());
        m_feature->Label.setValue(text.isEmpty() ? "ClearanceVolume" : text.toStdString().c_str());
    }
}

void TaskClearanceVolumePanel::updatePreview()
{
    if (m_feature) {
        m_feature->recomputeFeature();
    }
}

// =============================================================================
// TaskClearanceVolume
// =============================================================================

TaskClearanceVolume::TaskClearanceVolume(Part::ClearanceVolume* feature)
    : m_feature(feature)
{
    m_panel = new TaskClearanceVolumePanel(feature);
    
    Gui::TaskView::TaskBox* taskBox = new Gui::TaskView::TaskBox(
        QPixmap(), tr("Clearance Volume"), true, nullptr);
    taskBox->groupLayout()->addWidget(m_panel);
    Content.push_back(taskBox);
}

TaskClearanceVolume::~TaskClearanceVolume()
{
}

void TaskClearanceVolume::open()
{
}

bool TaskClearanceVolume::accept()
{
    return m_panel->accept();
}

bool TaskClearanceVolume::reject()
{
    return m_panel->reject();
}

QDialogButtonBox::StandardButtons TaskClearanceVolume::getStandardButtons() const
{
    return QDialogButtonBox::Ok | QDialogButtonBox::Cancel;
}
