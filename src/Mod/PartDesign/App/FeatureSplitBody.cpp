// SPDX-License-Identifier: LGPL-2.1-or-later
// UniCAD by UNITRONIX â€” Split Body feature.
// Splits a solid body into multiple solids using BRepAlgoAPI_Cut / BRepAlgoAPI_Section.

#include "PreCompiled.h"
#ifndef _PreComp_
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <BRepAlgoAPI_Section.hxx>
#include <BRepBuilderAPI_MakeSolid.hxx>
#include <Precision.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Solid.hxx>
#include <TopoDS_Compound.hxx>
#include <BRep_Builder.hxx>
#endif

#include "FeatureSplitBody.h"

using namespace PartDesign;

PROPERTY_SOURCE(PartDesign::SplitBody, PartDesign::DressUp)

SplitBody::SplitBody()
{
    ADD_PROPERTY_TYPE(SplittingTool, (nullptr), "SplitBody", App::Prop_None,
        "The tool shape (sketch, datum plane, face, or body) used to split the body");
}

short SplitBody::mustExecute() const
{
    if (SplittingTool.isTouched()) {
        return 1;
    }
    return DressUp::mustExecute();
}

App::DocumentObjectExecReturn* SplitBody::execute()
{
    if (onlyHaveRefined()) {
        return App::DocumentObject::StdReturn;
    }

    Part::TopoShape baseShape;
    try {
        baseShape = getBaseTopoShape();
    }
    catch (Base::Exception& e) {
        return new App::DocumentObjectExecReturn(e.what());
    }

    // Resolve splitting tool
    App::DocumentObject* toolObj = SplittingTool.getValue();
    const std::vector<std::string>& toolSubs = SplittingTool.getSubValues(true);

    if (!toolObj) {
        return new App::DocumentObjectExecReturn(
            QT_TRANSLATE_NOOP("Exception", "No splitting tool selected"));
    }

    this->Placement.setValue(Base::Placement());

    try {
        // Get the tool shape
        auto toolProp = toolObj->getPropertyByName("Shape");
        if (!toolProp
            || !toolProp->isDerivedFrom(App::PropertyComplexGeoData::getClassTypeId())) {
            return new App::DocumentObjectExecReturn(
                QT_TRANSLATE_NOOP("Exception", "Splitting tool has no valid shape"));
        }
        auto* toolShapeProp = static_cast<Part::PropertyPartShape*>(toolProp);
        Part::TopoShape toolTopoShape = toolShapeProp->getShape();

        TopoDS_Shape toolShape;
        if (!toolSubs.empty()) {
            try {
                toolShape = toolTopoShape.getSubShape(toolSubs.front().c_str());
            }
            catch (...) {
            }
        }
        if (toolShape.IsNull()) {
            toolShape = toolTopoShape.getShape();
        }
        if (toolShape.IsNull()) {
            return new App::DocumentObjectExecReturn(
                QT_TRANSLATE_NOOP("Exception", "Splitting tool shape is null"));
        }

        // For a planar / surface tool, we use BRepAlgoAPI_Cut to get the "first half"
        // The result keeps the first solid (the part on one side of the tool).
        // In a full implementation, both halves would be separate bodies.
        // For now: the feature result is the first solid after cutting with the tool.
        BRepAlgoAPI_Cut cutter(baseShape.getShape(), toolShape);
        cutter.Build();

        if (!cutter.IsDone()) {
            return new App::DocumentObjectExecReturn(
                QT_TRANSLATE_NOOP("Exception", "Split body operation failed"));
        }

        TopoDS_Shape result = cutter.Shape();
        if (result.IsNull()) {
            return new App::DocumentObjectExecReturn(
                QT_TRANSLATE_NOOP("Exception", "Split body result is null"));
        }

        // Collect all resulting solids
        std::vector<TopoDS_Solid> solids;
        for (TopExp_Explorer ex(result, TopAbs_SOLID); ex.More(); ex.Next()) {
            solids.push_back(TopoDS::Solid(ex.Current()));
        }

        if (solids.empty()) {
            return new App::DocumentObjectExecReturn(
                QT_TRANSLATE_NOOP("Exception", "No solids in split result"));
        }

        if (solids.size() == 1) {
            // Single solid â€” just use it
            Part::TopoShape resultShape(solids[0]);
            this->Shape.setValue(getSolid(resultShape));
        }
        else {
            // Multiple solids â€” create a compound of all
            BRep_Builder builder;
            TopoDS_Compound compound;
            builder.MakeCompound(compound);
            for (const auto& solid : solids) {
                builder.Add(compound, solid);
            }
            Part::TopoShape resultShape(compound);
            this->Shape.setValue(resultShape);
        }

        return App::DocumentObject::StdReturn;
    }
    catch (Standard_Failure& e) {
        return new App::DocumentObjectExecReturn(e.GetMessageString());
    }
    catch (Base::Exception& e) {
        return new App::DocumentObjectExecReturn(e.what());
    }
}
