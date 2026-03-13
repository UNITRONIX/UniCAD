// SPDX-License-Identifier: LGPL-2.1-or-later

/***************************************************************************
 *   Copyright (c) 2008 Werner Mayer <wmayer[at]users.sourceforge.net>     *
 *                                                                         *
 *   This file is part of the FreeCAD CAx development system.              *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; see the file COPYING.LIB. If not,    *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/


#include <App/Document.h>
#include <Gui/Application.h>
#include <Gui/Command.h>
#include <Gui/Control.h>
#include <Gui/MDIView.h>
#include <Mod/Sketcher/Gui/Workbench.h>
#include <Mod/PartDesign/App/Body.h>
#include <Mod/PartDesign/App/FeatureMultiTransform.h>

#include "Utils.h"
#include "Workbench.h"
#include "WorkflowManager.h"

using namespace PartDesignGui;
namespace sp = std::placeholders;

#if 0  // needed for Qt's lupdate utility
    qApp->translate("Workbench", "&Sketch");
    //
    qApp->translate("Workbench", "&Part Design");
    qApp->translate("Workbench", "Datums");
    qApp->translate("Workbench", "Additive Features");
    qApp->translate("Workbench", "Subtractive Features");
    qApp->translate("Workbench", "Dress-Up Features");
    qApp->translate("Workbench", "Transformation Features");
    qApp->translate("Workbench", "Sprocketâ€¦");
    qApp->translate("Workbench", "Involute Gear");

    qApp->translate("Workbench", "Shaft Design Wizard");
    qApp->translate("Gui::TaskView::TaskWatcherCommands", "Face Tools");
    qApp->translate("Gui::TaskView::TaskWatcherCommands", "Edge Tools");
    qApp->translate("Gui::TaskView::TaskWatcherCommands", "Boolean Tools");
    qApp->translate("Gui::TaskView::TaskWatcherCommands", "Helper Tools");
    qApp->translate("Gui::TaskView::TaskWatcherCommands", "Modeling Tools");
    qApp->translate("Gui::TaskView::TaskWatcherCommands", "Create Geometry");
    //
    qApp->translate("Workbench", "Measure");
    qApp->translate("Workbench", "Refresh");
    qApp->translate("Workbench", "Toggle 3D");
    qApp->translate("Workbench", "Part Design Helper");
    qApp->translate("Workbench", "Part Design Modeling");
#endif

/// @namespace PartDesignGui @class Workbench
TYPESYSTEM_SOURCE(PartDesignGui::Workbench, Gui::StdWorkbench)

Workbench::Workbench() = default;

Workbench::~Workbench()
{
    WorkflowManager::destruct();
}

void Workbench::setupContextMenu(const char* recipient, Gui::MenuItem* item) const
{
    auto selection = Gui::Selection().getSelection();
    // Add move Tip Command
    if (!selection.empty()) {
        App::DocumentObject* feature = selection.front().pObject;
        PartDesign::Body* body = nullptr;

        body = PartDesignGui::getBodyFor(feature, false, false, true);
        // lote of assertion so feature should be marked as a tip
        if (selection.size() == 1 && feature && body
            && (feature->isDerivedFrom<PartDesign::Feature>()
                || (feature->isDerivedFrom<Part::Feature>()
                    && body->BaseFeature.getValue() == feature))) {
            *item << "PartDesign_MoveTip";
        }

        if (strcmp(recipient, "Tree") == 0) {
            Gui::MDIView* activeView = Gui::Application::Instance->activeView();

            if (activeView) {
                if (feature && feature->isDerivedFrom<PartDesign::Body>()) {
                    *item << "Std_ToggleFreeze";
                }

                if (activeView->getAppDocument()->countObjectsOfType<PartDesign::Body>() > 0) {
                    bool addMoveFeature = true;
                    bool addMoveFeatureInTree = (body != nullptr);
                    for (auto sel : selection) {
                        // if at least one selected feature cannot be moved to a body
                        // disable the entry
                        if (addMoveFeature && !PartDesign::Body::isAllowed(sel.pObject)) {
                            addMoveFeature = false;
                        }
                        // if all at least one selected feature doesn't belong to the same body
                        // disable the menu entry
                        if (addMoveFeatureInTree && !body->hasObject(sel.pObject)) {
                            addMoveFeatureInTree = false;
                        }

                        if (!addMoveFeatureInTree && !addMoveFeature) {
                            break;
                        }
                    }
                    if (addMoveFeature) {
                        *item << "PartDesign_MoveFeature";
                    }
                    if (addMoveFeatureInTree) {
                        *item << "PartDesign_MoveFeatureInTree";
                    }
                }
            }
            if (Gui::Selection().countObjectsOfType<PartDesign::Transformed>()
                    - Gui::Selection().countObjectsOfType<PartDesign::MultiTransform>()
                == 1) {
                *item << "PartDesign_MultiTransform";
            }
        }
    }

    if (item->hasItems()) {
        *item << "Separator";
    }
    Gui::StdWorkbench::setupContextMenu(recipient, item);
}

void Workbench::activated()
{
    Gui::Workbench::activated();

    WorkflowManager::init();

    std::vector<Gui::TaskView::TaskWatcher*> Watcher;

    const char* Vertex[]
        = {"Part_DatumPoint", "Part_DatumLine", "Part_DatumPlane", "Part_CoordinateSystem", nullptr};
    Watcher.push_back(new Gui::TaskView::TaskWatcherCommands(
        "SELECT Part::Feature SUBELEMENT Vertex COUNT 1..",
        Vertex,
        "Vertex Tools",
        "PartDesign_Body"
    ));

    const char* Vertex1[] = {
        "PartDesign_Point",
        "PartDesign_Line",
        "PartDesign_Plane",
        "PartDesign_CoordinateSystem",
        nullptr
    };
    Watcher.push_back(new Gui::TaskView::TaskWatcherCommands(
        "SELECT Part::Feature SUBELEMENT Vertex COUNT 1..",
        Vertex1,
        "Datum objects",
        "PartDesign_CoordinateSystem"
    ));

    const char* Edge[] = {
        "PartDesign_Fillet",
        "PartDesign_Chamfer",
        "Part_DatumPoint",
        "Part_DatumLine",
        "Part_DatumPlane",
        "Part_CoordinateSystem",
        nullptr
    };
    Watcher.push_back(new Gui::TaskView::TaskWatcherCommands(
        "SELECT Part::Feature SUBELEMENT Edge COUNT 1..",
        Edge,
        "Edge Tools",
        "PartDesign_Body"
    ));

    const char* Edge1[] = {
        "PartDesign_Point",
        "PartDesign_Line",
        "PartDesign_Plane",
        "PartDesign_CoordinateSystem",
        nullptr
    };
    Watcher.push_back(new Gui::TaskView::TaskWatcherCommands(
        "SELECT Part::Feature SUBELEMENT Edge COUNT 1..",
        Edge1,
        "Datum objects",
        "PartDesign_CoordinateSystem"
    ));

    const char* Face[] = {
        "PartDesign_PressPull",
        "PartDesign_Extrude",
        "PartDesign_NewSketch",
        "PartDesign_Fillet",
        "PartDesign_Chamfer",
        "PartDesign_Draft",
        "PartDesign_Thickness",
        "Part_ClearanceVolume",
        "PartDesign_OffsetFace",
        "PartDesign_DeleteFace",
        "PartDesign_ReplaceFace",
        "PartDesign_SplitFace",
        "PartDesign_MoveFace",
        "Part_DatumPoint",
        "Part_DatumLine",
        "Part_DatumPlane",
        "Part_CoordinateSystem",
        nullptr
    };
    Watcher.push_back(new Gui::TaskView::TaskWatcherCommands(
        "SELECT Part::Feature SUBELEMENT Face COUNT 1",
        Face,
        "Face Tools",
        "PartDesign_Body"
    ));

    const char* Face1[] = {
        "PartDesign_Point",
        "PartDesign_Line",
        "PartDesign_Plane",
        "PartDesign_CoordinateSystem",
        nullptr
    };
    Watcher.push_back(new Gui::TaskView::TaskWatcherCommands(
        "SELECT Part::Feature SUBELEMENT Face COUNT 1",
        Face1,
        "Datum objects",
        "PartDesign_CoordinateSystem"
    ));

    const char* Body[] = {"PartDesign_NewSketch", nullptr};
    Watcher.push_back(new Gui::TaskView::TaskWatcherCommands(
        "SELECT PartDesign::Body COUNT 1",
        Body,
        "Helper Tools",
        "PartDesign_Body"
    ));

    const char* Body2[] = {"PartDesign_Boolean", nullptr};
    Watcher.push_back(new Gui::TaskView::TaskWatcherCommands(
        "SELECT PartDesign::Body COUNT 1..",
        Body2,
        "Boolean Tools",
        "PartDesign_Body"
    ));

    const char* Plane1[] = {
        "PartDesign_NewSketch",
        "Part_DatumPoint",
        "Part_DatumLine",
        "Part_DatumPlane",
        "Part_CoordinateSystem",
        nullptr
    };
    Watcher.push_back(new Gui::TaskView::TaskWatcherCommands(
        "SELECT App::Plane COUNT 1",
        Plane1,
        "Helper Tools",
        "PartDesign_Body"
    ));

    const char* Plane2[] = {
        "PartDesign_NewSketch",
        "Part_DatumPoint",
        "Part_DatumLine",
        "Part_DatumPlane",
        "Part_CoordinateSystem",
        nullptr
    };
    Watcher.push_back(new Gui::TaskView::TaskWatcherCommands(
        "SELECT PartDesign::Plane COUNT 1",
        Plane2,
        "Helper Tools",
        "PartDesign_Body"
    ));

    const char* Plane3[] = {
        "PartDesign_Point",
        "PartDesign_Line",
        "PartDesign_Plane",
        "PartDesign_CoordinateSystem",
        nullptr
    };
    Watcher.push_back(new Gui::TaskView::TaskWatcherCommands(
        "SELECT App::Plane COUNT 1",
        Plane3,
        "Datum objects",
        "PartDesign_CoordinateSystem"
    ));

    const char* Plane4[] = {
        "PartDesign_Point",
        "PartDesign_Line",
        "PartDesign_Plane",
        "PartDesign_CoordinateSystem",
        nullptr
    };
    Watcher.push_back(new Gui::TaskView::TaskWatcherCommands(
        "SELECT PartDesign::Plane COUNT 1",
        Plane4,
        "Datum objects",
        "PartDesign_CoordinateSystem"
    ));

    const char* Line[] = {"Part_DatumPoint", "Part_DatumLine", "Part_DatumPlane", nullptr};
    Watcher.push_back(new Gui::TaskView::TaskWatcherCommands(
        "SELECT PartDesign::Line COUNT 1",
        Line,
        "Helper Tools",
        "PartDesign_Body"
    ));

    const char* Line1[] = {"PartDesign_Point", "PartDesign_Line", "PartDesign_Plane", nullptr};
    Watcher.push_back(new Gui::TaskView::TaskWatcherCommands(
        "SELECT PartDesign::Line COUNT 1",
        Line1,
        "Datum objects",
        "PartDesign_CoordinateSystem"
    ));

    const char* Point[]
        = {"Part_DatumPoint", "Part_DatumLine", "Part_DatumPlane", "Part_CoordinateSystem", nullptr};
    Watcher.push_back(new Gui::TaskView::TaskWatcherCommands(
        "SELECT PartDesign::Point COUNT 1",
        Point,
        "Helper Tools",
        "PartDesign_Body"
    ));

    const char* Point1[] = {
        "PartDesign_Point",
        "PartDesign_Line",
        "PartDesign_Plane",
        "PartDesign_CoordinateSystem",
        nullptr
    };
    Watcher.push_back(new Gui::TaskView::TaskWatcherCommands(
        "SELECT PartDesign::Point COUNT 1",
        Point1,
        "Datum objects",
        "PartDesign_CoordinateSystem"
    ));

    const char* NoSel[] = {"PartDesign_Body", nullptr};
    Watcher.push_back(
        new Gui::TaskView::TaskWatcherCommandsEmptySelection(NoSel, "Start Part", "Part_Box_Parametric")
    );

    const char* Faces[] = {
        "PartDesign_PressPull",
        "PartDesign_Extrude",
        "PartDesign_Fillet",
        "PartDesign_Chamfer",
        "PartDesign_Draft",
        "PartDesign_Thickness",
        "Part_ClearanceVolume",
        "PartDesign_OffsetFace",
        "PartDesign_DeleteFace",
        "PartDesign_ReplaceFace",
        "PartDesign_SplitFace",
        "PartDesign_MoveFace",
        nullptr
    };
    Watcher.push_back(new Gui::TaskView::TaskWatcherCommands(
        "SELECT Part::Feature SUBELEMENT Face COUNT 2..",
        Faces,
        "Face Tools",
        "PartDesign_Body"
    ));

    const char* Sketch[] = {
        "PartDesign_NewSketch",
        "PartDesign_PressPull",
        "PartDesign_Extrude",
        "PartDesign_Revolve",
        "PartDesign_Sweep",
        "PartDesign_Loft",
        "PartDesign_Hole",
        nullptr
    };
    Watcher.push_back(new Gui::TaskView::TaskWatcherCommands(
        "SELECT Sketcher::SketchObject COUNT 1",
        Sketch,
        "Modeling Tools",
        "PartDesign_Body"
    ));

    const char* Sketches[] = {
        "PartDesign_Loft",
        "PartDesign_Sweep",
        nullptr
    };
    Watcher.push_back(new Gui::TaskView::TaskWatcherCommands(
        "SELECT Sketcher::SketchObject COUNT 2..",
        Sketches,
        "Modeling tools",
        "PartDesign_Body"
    ));

    const char* ShapeBinder[] = {
        "PartDesign_PressPull",
        "PartDesign_Extrude",
        "PartDesign_Revolve",
        "PartDesign_Sweep",
        "PartDesign_Loft",
        "PartDesign_Hole",
        nullptr
    };
    Watcher.push_back(new Gui::TaskView::TaskWatcherCommands(
        "SELECT PartDesign::ShapeBinder COUNT 1",
        ShapeBinder,
        "Modeling tools",
        "PartDesign_Body"
    ));

    const char* SubShapeBinder[] = {
        "PartDesign_PressPull",
        "PartDesign_Extrude",
        "PartDesign_Revolve",
        "PartDesign_Sweep",
        "PartDesign_Loft",
        "PartDesign_Hole",
        nullptr
    };
    Watcher.push_back(new Gui::TaskView::TaskWatcherCommands(
        "SELECT PartDesign::SubShapeBinder COUNT 1",
        SubShapeBinder,
        "Modeling tools",
        "PartDesign_Body"
    ));

    const char* Transformed[] = {
        "PartDesign_Mirrored",
        "PartDesign_LinearPattern",
        "PartDesign_PolarPattern",
        "PartDesign_MultiTransform",
        nullptr
    };
    Watcher.push_back(new Gui::TaskView::TaskWatcherCommands(
        "SELECT PartDesign::SketchBased",
        Transformed,
        "Transformation Tools",
        "PartDesign_MultiTransform"
    ));

    addTaskWatcher(Watcher);
    if (App::GetApplication()
            .GetUserParameter()
            .GetGroup("BaseApp")
            ->GetGroup("Preferences")
            ->GetGroup("Mod/PartDesign")
            ->GetBool("SwitchToTask", true)) {
        Gui::Control().showTaskView();
    }
}

void Workbench::deactivated()
{
    removeTaskWatcher();
    // reset the active Body
    Gui::Command::doCommand(Gui::Command::Doc, "import PartDesignGui");

    Gui::Workbench::deactivated();
}

Gui::MenuItem* Workbench::setupMenuBar() const
{
    Gui::MenuItem* root = StdWorkbench::setupMenuBar();
    Gui::MenuItem* item = root->findItem("&Windows");

    // add another top level menu left besides the Part Design menu for the Sketcher commands
    Gui::MenuItem* sketch = new Gui::MenuItem;
    root->insertItem(item, sketch);
    sketch->setCommand("&Sketch");

    // UniCAD: Simplified Sketch menu â€” only essential commands
    *sketch << "PartDesign_NewSketch"
            << "Sketcher_EditSketch"
            << "Sketcher_ValidateSketch"
            << "Sketcher_MirrorSketch";

    Gui::MenuItem* part = new Gui::MenuItem;
    root->insertItem(item, part);
    part->setCommand("&Part Design");

    // UniCAD: Simplified menu â€” only unified commands, no legacy additive/subtractive split

    // Modify submenu
    Gui::MenuItem* modify = new Gui::MenuItem;
    modify->setCommand("Modify");
    *modify << "PartDesign_Fillet"
            << "PartDesign_Chamfer"
            << "PartDesign_Draft"
            << "PartDesign_Thickness"
            << "Part_ClearanceVolume"
            << "Separator"
            << "PartDesign_OffsetFace"
            << "PartDesign_DeleteFace"
            << "PartDesign_ReplaceFace"
            << "PartDesign_SplitFace"
            << "PartDesign_SplitBody"
            << "PartDesign_MoveFace";

    // Pattern submenu
    Gui::MenuItem* pattern = new Gui::MenuItem;
    pattern->setCommand("Pattern");
    *pattern << "PartDesign_Mirrored"
             << "PartDesign_LinearPattern"
             << "PartDesign_PolarPattern"
             << "PartDesign_MultiTransform";

    *part << "PartDesign_Body"
          << "PartDesign_SubShapeBinder"
          << "Separator"
          << "PartDesign_NewSketch"
          << "Separator"
          << "PartDesign_PressPull"
          << "PartDesign_Extrude"
          << "PartDesign_Revolve"
          << "PartDesign_Sweep"
          << "PartDesign_Loft"
          << "PartDesign_Hole"
          << "PartDesign_Emboss"
          << "Separator"
          << "PartDesign_CompPrimitiveAdditive"
          << "Separator" << modify
          << "Separator" << pattern
          << "Separator"
          << "PartDesign_Boolean"
          << "PartDesign_MoveBody"
          << "PartDesign_CopyBody"
          << "Separator"
          << "Materials_InspectAppearance"
          << "Part_CheckGeometry";

    Gui::MenuItem* view = root->findItem("&View");
    if (view) {
        Gui::MenuItem* appr = view->findItem("Std_RandomColor");
        appr = view->afterItem(appr);
        Gui::MenuItem* face = new Gui::MenuItem();
        face->setCommand("Part_ColorPerFace");
        view->insertItem(appr, face);
    }

    // Replace the "Duplicate selection" menu item with a replacement that is compatible with Body
    item = root->findItem("&Edit");
    Gui::MenuItem* dup = item->findItem("Std_DuplicateSelection");
    dup->setCommand("PartDesign_DuplicateSelection");

    return root;
}

Gui::ToolBarItem* Workbench::setupToolBars() const
{
    Gui::ToolBarItem* root = StdWorkbench::setupToolBars();
    Gui::ToolBarItem* part = new Gui::ToolBarItem(root);
    part->setCommand("Part Design");

    // UniCAD: Simplified helper toolbar â€” only essential items
    *part << "PartDesign_Body"
          << "PartDesign_NewSketch"
          << "PartDesign_SubShapeBinder";

    // UniCAD: Always show Sketcher toolbars for faster workflow (Fusion 360 style)
    Gui::ToolBarItem* sketchGeom = new Gui::ToolBarItem(root);
    sketchGeom->setCommand("Sketch Geometries");
    SketcherGui::addSketcherWorkbenchGeometries(*sketchGeom);

    Gui::ToolBarItem* sketchCons = new Gui::ToolBarItem(root);
    sketchCons->setCommand("Sketch Constraints");
    SketcherGui::addSketcherWorkbenchConstraints(*sketchCons);

    Gui::ToolBarItem* sketchTools = new Gui::ToolBarItem(root);
    sketchTools->setCommand("Sketch Tools");
    SketcherGui::addSketcherWorkbenchTools(*sketchTools);

    part = new Gui::ToolBarItem(root);
    part->setCommand("Create");

    // UniCAD: Unified commands only (Fusion 360 style workflow)
    *part << "PartDesign_PressPull"
          << "PartDesign_Extrude"
          << "PartDesign_Revolve"
          << "PartDesign_Sweep"
          << "PartDesign_Loft"
          << "Separator"
          << "PartDesign_Hole"
          << "PartDesign_Emboss"
          << "Separator"
          << "PartDesign_Boolean";

    part = new Gui::ToolBarItem(root);
    part->setCommand("Modify");
    *part << "PartDesign_Fillet"
          << "PartDesign_Chamfer"
          << "PartDesign_Draft"
          << "PartDesign_Thickness"
          << "Separator"
          << "PartDesign_OffsetFace"
          << "PartDesign_DeleteFace"
          << "PartDesign_ReplaceFace"
          << "PartDesign_SplitFace"
          << "PartDesign_SplitBody"
          << "PartDesign_MoveFace";

    part = new Gui::ToolBarItem(root);
    part->setCommand("Pattern");
    *part << "PartDesign_Mirrored"
          << "PartDesign_LinearPattern"
          << "PartDesign_PolarPattern"
          << "PartDesign_MultiTransform"
          << "Separator"
          << "PartDesign_MoveBody"
          << "PartDesign_CopyBody";

    // UniCAD: Selection Priority toolbar (Fusion 360 style)
    part = new Gui::ToolBarItem(root);
    part->setCommand("Selection Priority");
    *part << "PartDesign_SelectFacePriority"
          << "PartDesign_SelectEdgePriority"
          << "PartDesign_SelectBodyPriority"
          << "Separator"
          << "PartDesign_SelectThrough";

    return root;
}
