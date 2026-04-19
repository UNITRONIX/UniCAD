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

#include <Gui/Application.h>
#include <Gui/Command.h>
#include <Gui/Document.h>
#include <Gui/MainWindow.h>

#include <Base/Interpreter.h>


//===========================================================================
// Part_QuickPortCutout
//===========================================================================

DEF_STD_CMD_A(CmdPartQuickPortCutout)

CmdPartQuickPortCutout::CmdPartQuickPortCutout()
    : Command("Part_QuickPortCutout")
{
    sAppModule = "Part";
    sGroup = "Part";
    sMenuText = QT_TR_NOOP("Quick Port Cutout");
    sToolTipText = QT_TR_NOOP("Automatically detect and cut a hole in enclosure wall for the selected port face.\n\n"
                              "1. Select a face on the port/connector\n"
                              "2. Click this command - wall is auto-detected via ray-casting\n"
                              "3. Adjust tolerance and click 'Cut Hole'");
    sWhatsThis = "Part_QuickPortCutout";
    sStatusTip = sToolTipText;
    sPixmap = "Part_QuickPortCutout";
    sAccel = "Shift+C";
}

void CmdPartQuickPortCutout::activated(int /*iMsg*/)
{
    try {
        Base::Interpreter().runString(
            "try:\n"
            "    import sys, os, importlib.util\n"
            "    _found = False\n"
            "    for _p in sys.path:\n"
            "        _f = os.path.join(_p, 'Scripts', 'QuickPortCutout.py')\n"
            "        if os.path.exists(_f):\n"
            "            _spec = importlib.util.spec_from_file_location('QuickPortCutout', _f)\n"
            "            _mod = importlib.util.module_from_spec(_spec)\n"
            "            _spec.loader.exec_module(_mod)\n"
            "            _mod.quick_port_cutout()\n"
            "            _found = True\n"
            "            break\n"
            "    if not _found:\n"
            "        from PySide import QtGui\n"
            "        QtGui.QMessageBox.critical(None, 'Quick Port Cutout', 'Script not found in sys.path')\n"
            "except Exception as e:\n"
            "    import FreeCAD\n"
            "    FreeCAD.Console.PrintError(f'QuickPortCutout error: {e}\\n')\n"
            "    from PySide import QtGui\n"
            "    QtGui.QMessageBox.critical(None, 'Quick Port Cutout', f'Error: {e}')\n"
        );
    }
    catch (Base::PyException& e) {
        e.reportException();
    }
}

bool CmdPartQuickPortCutout::isActive()
{
    return hasActiveDocument();
}

//===========================================================================
// Registration
//===========================================================================

void CreateQuickPortCutoutCommands()
{
    Gui::CommandManager& rcCmdMgr = Gui::Application::Instance->commandManager();
    rcCmdMgr.addCommand(new CmdPartQuickPortCutout());
}
