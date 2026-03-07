// SPDX-License-Identifier: LGPL-2.1-or-later
// FusionCAD by UNITRONIX — Task panel for unified Extrude feature.

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSignalBlocker>

#include <App/Document.h>
#include <Gui/Command.h>
#include <Mod/PartDesign/App/FeatureUnifiedExtrude.h>

#include "ui_TaskPadPocketParameters.h"
#include "TaskUnifiedExtrudeParameters.h"

using namespace PartDesignGui;
using namespace Gui;

/* TRANSLATOR PartDesignGui::TaskUnifiedExtrudeParameters */

TaskUnifiedExtrudeParameters::TaskUnifiedExtrudeParameters(
    ViewProviderUnifiedExtrude* ExtrudeView,
    QWidget* parent,
    bool newObj)
    : TaskExtrudeParameters(ExtrudeView, parent, "PartDesign_Pad", tr("Extrude"))
{
    // FusionCAD: Add Operation combo box at top of the panel
    auto* opLayout = new QHBoxLayout();
    auto* opLabel = new QLabel(tr("Operation"), proxy);
    operationCombo = new QComboBox(proxy);
    operationCombo->addItem(tr("Join"));
    operationCombo->addItem(tr("Cut"));
    operationCombo->addItem(tr("Intersect"));
    operationCombo->addItem(tr("New Body"));
    opLayout->addWidget(opLabel);
    opLayout->addWidget(operationCombo);

    // Insert Operation at top of the form layout (before sidesMode)
    auto* mainLayout = qobject_cast<QVBoxLayout*>(proxy->layout());
    if (mainLayout) {
        mainLayout->insertLayout(0, opLayout);
    }

    // Set initial value from feature
    auto* extrude = getObject<PartDesign::Extrude>();
    if (extrude) {
        operationCombo->setCurrentIndex(extrude->Operation.getValue());
    }

    connect(operationCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &TaskUnifiedExtrudeParameters::onOperationChanged);

    // Tooltips
    ui->offsetEdit->setToolTip(tr("Offset from face where extrusion will end on side 1"));
    ui->offsetEdit2->setToolTip(tr("Offset from face where extrusion will end on side 2"));
    ui->checkBoxReversed->setToolTip(tr("Reverses extrusion direction"));

    // History paths
    ui->lengthEdit->setEntryName(QByteArray("Length"));
    ui->lengthEdit->setParamGrpPath(QByteArray("User parameter:BaseApp/History/ExtrudeLength"));
    ui->lengthEdit2->setEntryName(QByteArray("Length2"));
    ui->lengthEdit2->setParamGrpPath(QByteArray("User parameter:BaseApp/History/ExtrudeLength2"));
    ui->offsetEdit->setEntryName(QByteArray("Offset"));
    ui->offsetEdit->setParamGrpPath(QByteArray("User parameter:BaseApp/History/ExtrudeOffset"));
    ui->offsetEdit2->setEntryName(QByteArray("Offset2"));
    ui->offsetEdit2->setParamGrpPath(QByteArray("User parameter:BaseApp/History/ExtrudeOffset2"));
    ui->taperEdit->setEntryName(QByteArray("TaperAngle"));
    ui->taperEdit->setParamGrpPath(QByteArray("User parameter:BaseApp/History/ExtrudeTaperAngle"));
    ui->taperEdit2->setEntryName(QByteArray("TaperAngle2"));
    ui->taperEdit2->setParamGrpPath(QByteArray("User parameter:BaseApp/History/ExtrudeTaperAngle2"));

    setupDialog();

    if (newObj) {
        readValuesFromHistory();
    }
}

TaskUnifiedExtrudeParameters::~TaskUnifiedExtrudeParameters() = default;

void TaskUnifiedExtrudeParameters::onOperationChanged(int index)
{
    auto* extrude = getObject<PartDesign::Extrude>();
    if (!extrude) {
        return;
    }
    extrude->Operation.setValue(index);
    recomputeFeature();
}

void TaskUnifiedExtrudeParameters::onOperationAutoSwitched(int newOpIndex)
{
    // FusionCAD: Sync combo box when base class auto-switches Operation
    if (operationCombo && operationCombo->currentIndex() != newOpIndex) {
        QSignalBlocker blocker(operationCombo);
        operationCombo->setCurrentIndex(newOpIndex);
    }
}

void TaskUnifiedExtrudeParameters::translateModeList(QComboBox* box, int index)
{
    box->clear();
    box->addItem(tr("Dimension"));
    box->addItem(tr("To last"));
    box->addItem(tr("To first"));
    box->addItem(tr("Up to face"));
    box->addItem(tr("Up to shape"));
    box->setCurrentIndex(index);
}

void TaskUnifiedExtrudeParameters::updateUI(Side side)
{
    fillDirectionCombo();
    updateWholeUI(Type::Pad, side);
}

void TaskUnifiedExtrudeParameters::onModeChanged(int index, Side side)
{
    auto& sideCtrl = getSideController(side);

    switch (static_cast<Mode>(index)) {
        case Mode::Dimension:
            sideCtrl.Type->setValue("Length");
            break;
        case Mode::ToLast:
            sideCtrl.Type->setValue("UpToLast");
            break;
        case Mode::ToFirst:
            sideCtrl.Type->setValue("UpToFirst");
            break;
        case Mode::ToFace:
            sideCtrl.Type->setValue("UpToFace");
            if (sideCtrl.lineFaceName->text().isEmpty()) {
                sideCtrl.buttonFace->setChecked(true);
                handleLineFaceNameClick(sideCtrl.lineFaceName);
            }
            break;
        case Mode::ToShape:
            sideCtrl.Type->setValue("UpToShape");
            break;
    }

    updateUI(side);
    recomputeFeature();
}

void TaskUnifiedExtrudeParameters::apply()
{
    applyParameters();

    // Also apply Operation
    auto* extrude = getObject<PartDesign::Extrude>();
    if (extrude) {
        FCMD_OBJ_CMD(extrude, "Operation = " << extrude->Operation.getValue());
    }
}

//**************************************************************************
// TaskDialog
//**************************************************************************

TaskDlgUnifiedExtrudeParameters::TaskDlgUnifiedExtrudeParameters(
    ViewProviderUnifiedExtrude* ExtrudeView,
    bool /*newObj*/)
    : TaskDlgExtrudeParameters(ExtrudeView)
    , parameters(new TaskUnifiedExtrudeParameters(ExtrudeView))
{
    Content.push_back(parameters);
    Content.push_back(preview);
}

#include "moc_TaskUnifiedExtrudeParameters.cpp"
