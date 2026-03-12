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

#include <QMessageBox>

#include <Gui/Application.h>
#include <Gui/Command.h>
#include <Gui/Document.h>
#include <Gui/MainWindow.h>
#include <Gui/Selection/Selection.h>
#include <Gui/Control.h>

#include <App/Document.h>

#include <Mod/Part/App/FeatureClearanceVolume.h>
#include "TaskClearanceVolume.h"

//===========================================================================
// Part_ClearanceVolume
//===========================================================================

DEF_STD_CMD_A(CmdPartClearanceVolume)

CmdPartClearanceVolume::CmdPartClearanceVolume()
    : Command("Part_ClearanceVolume")
{
    sAppModule = "Part";
    sGroup = "Part";
    sMenuText = QT_TR_NOOP("Clearance Volume");
    sToolTipText = QT_TR_NOOP("Create a clearance/keep-out volume for 3D printing tolerance.\n\n"
                              "Select a face on a component (e.g., USB port) to define the clearance zone.\n"
                              "The volume will automatically cut holes in enclosure walls with the specified tolerance.");
    sWhatsThis = "Part_ClearanceVolume";
    sStatusTip = sToolTipText;
    sPixmap = "Part_Offset3D"; // TODO: Create dedicated icon
}

void CmdPartClearanceVolume::activated(int /*iMsg*/)
{
    App::Document* doc = getDocument();
    if (!doc) {
        return;
    }
    
    // Create the clearance volume feature
    openCommand("Create Clearance Volume");
    
    std::string objName = doc->getUniqueObjectName("ClearanceVolume");
    doCommand(Doc, "App.ActiveDocument.addObject('Part::ClearanceVolume', '%s')", objName.c_str());
    
    // Get the created object
    auto* clearance = dynamic_cast<Part::ClearanceVolume*>(doc->getObject(objName.c_str()));
    if (!clearance) {
        abortCommand();
        return;
    }
    
    // Check if a face is already selected
    std::vector<Gui::SelectionObject> selection = Gui::Selection().getSelectionEx();
    for (const auto& selObj : selection) {
        const std::vector<std::string>& subNames = selObj.getSubNames();
        for (const auto& sub : subNames) {
            if (sub.find("Face") == 0) {
                // Found a face selection - set it as source
                doCommand(Doc, "App.ActiveDocument.%s.SourceFaces = (App.ActiveDocument.%s, ['%s'])",
                          objName.c_str(), 
                          selObj.getObject()->getNameInDocument(),
                          sub.c_str());
                break;
            }
        }
    }
    
    // Set default label
    doCommand(Doc, "App.ActiveDocument.%s.Label = 'Clearance Volume'", objName.c_str());
    
    commitCommand();
    updateActive();
    
    // Open the task panel
    Gui::Control().showDialog(new PartGui::TaskClearanceVolume(clearance));
}

bool CmdPartClearanceVolume::isActive()
{
    return hasActiveDocument();
}

//===========================================================================
// Part_ApplyClearanceVolumes
//===========================================================================

DEF_STD_CMD_A(CmdPartApplyClearanceVolumes)

CmdPartApplyClearanceVolumes::CmdPartApplyClearanceVolumes()
    : Command("Part_ApplyClearanceVolumes")
{
    sAppModule = "Part";
    sGroup = "Part";
    sMenuText = QT_TR_NOOP("Apply Clearance Volumes");
    sToolTipText = QT_TR_NOOP("Subtract all clearance volumes from the selected solid.\n\n"
                              "This cuts holes for all defined clearance zones (ports, connectors) "
                              "with the configured 3D printing tolerance.");
    sWhatsThis = "Part_ApplyClearanceVolumes";
    sStatusTip = sToolTipText;
    sPixmap = "Part_Cut";
}

void CmdPartApplyClearanceVolumes::activated(int /*iMsg*/)
{
    App::Document* doc = getDocument();
    if (!doc) {
        return;
    }
    
    // Get selected solid
    std::vector<Gui::SelectionObject> selection = Gui::Selection().getSelectionEx();
    if (selection.empty()) {
        QMessageBox::warning(Gui::getMainWindow(), 
                            QObject::tr("No Selection"),
                            QObject::tr("Please select a solid to apply clearance volumes to."));
        return;
    }
    
    auto* targetObj = dynamic_cast<Part::Feature*>(selection[0].getObject());
    if (!targetObj) {
        QMessageBox::warning(Gui::getMainWindow(),
                            QObject::tr("Invalid Selection"),
                            QObject::tr("Selected object is not a Part feature."));
        return;
    }
    
    // Get all clearance volumes with AutoSubtract enabled
    auto clearanceVolumes = Part::ClearanceVolume::getAllInDocument(doc);
    
    std::vector<Part::ClearanceVolume*> toApply;
    for (auto* cv : clearanceVolumes) {
        if (cv->AutoSubtract.getValue() && cv->intersectsWith(targetObj->Shape.getValue())) {
            toApply.push_back(cv);
        }
    }
    
    if (toApply.empty()) {
        QMessageBox::information(Gui::getMainWindow(),
                                QObject::tr("No Clearances"),
                                QObject::tr("No clearance volumes intersect with the selected solid."));
        return;
    }
    
    // Create cuts for each clearance volume
    openCommand("Apply Clearance Volumes");
    
    std::string currentShape = targetObj->getNameInDocument();
    
    for (size_t i = 0; i < toApply.size(); ++i) {
        std::string cutName = doc->getUniqueObjectName("ClearanceCut");
        doCommand(Doc, "App.ActiveDocument.addObject('Part::Cut', '%s')", cutName.c_str());
        doCommand(Doc, "App.ActiveDocument.%s.Base = App.ActiveDocument.%s", 
                  cutName.c_str(), currentShape.c_str());
        doCommand(Doc, "App.ActiveDocument.%s.Tool = App.ActiveDocument.%s",
                  cutName.c_str(), toApply[i]->getNameInDocument());
        doCommand(Doc, "App.ActiveDocument.%s.Label = 'Enclosure with %s'",
                  cutName.c_str(), toApply[i]->PortName.getValue());
        
        currentShape = cutName;
    }
    
    commitCommand();
    updateActive();
    
    QMessageBox::information(Gui::getMainWindow(),
                            QObject::tr("Clearances Applied"),
                            QObject::tr("Applied %1 clearance volume(s) to create holes with tolerance.")
                                .arg(static_cast<int>(toApply.size())));
}

bool CmdPartApplyClearanceVolumes::isActive()
{
    return hasActiveDocument();
}

//===========================================================================
// Registration
//===========================================================================

void CreateClearanceVolumeCommands()
{
    Gui::CommandManager& rcCmdMgr = Gui::Application::Instance->commandManager();
    
    rcCmdMgr.addCommand(new CmdPartClearanceVolume());
    rcCmdMgr.addCommand(new CmdPartApplyClearanceVolumes());
}
