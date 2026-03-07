// SPDX-License-Identifier: LGPL-2.1-or-later
// FusionCAD by UNITRONIX — Split Face feature (direct edit).
// Splits selected faces on a solid using BRepFeat_SplitShape
// with intersection edges computed via BRepAlgoAPI_Section.

#include "PreCompiled.h"
#ifndef _PreComp_
#include <BRepAlgoAPI_Section.hxx>
#include <BRepFeat_SplitShape.hxx>
#include <Precision.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#endif

#include "FeatureSplitFace.h"

using namespace PartDesign;

PROPERTY_SOURCE(PartDesign::SplitFace, PartDesign::DressUp)

SplitFace::SplitFace()
{
    ADD_PROPERTY_TYPE(SplittingTool, (nullptr), "SplitFace", App::Prop_None,
        "The tool shape (sketch, plane, or face) used to split");
}

short SplitFace::mustExecute() const
{
    if (SplittingTool.isTouched()) {
        return 1;
    }
    return DressUp::mustExecute();
}

App::DocumentObjectExecReturn* SplitFace::execute()
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

    const std::vector<std::string>& subStrings = Base.getSubValues(true);

    if (subStrings.empty()) {
        this->positionByBaseFeature();
        this->Shape.setValue(baseShape);
        return App::DocumentObject::StdReturn;
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

        // Collect faces to split
        std::vector<TopoDS_Face> facesToSplit;
        for (const auto& sub : subStrings) {
            TopoDS_Shape face;
            try {
                face = baseShape.getSubShape(sub.c_str());
            }
            catch (...) {
            }
            if (face.IsNull() || face.ShapeType() != TopAbs_FACE) {
                return new App::DocumentObjectExecReturn(
                    QT_TRANSLATE_NOOP("Exception", "Invalid face reference"));
            }
            facesToSplit.push_back(TopoDS::Face(face));
        }

        // Use BRepFeat_SplitShape on the base solid
        BRepFeat_SplitShape splitter(baseShape.getShape());

        bool hasAnySplit = false;
        for (const auto& face : facesToSplit) {
            // Compute intersection of tool with this face
            BRepAlgoAPI_Section section(face, toolShape, Standard_False);
            section.Approximation(Standard_True);
            section.Build();

            if (!section.IsDone()) {
                continue;
            }

            TopoDS_Shape sectionResult = section.Shape();
            if (sectionResult.IsNull()) {
                continue;
            }

            // Add each intersection edge to the splitter
            for (TopExp_Explorer edgeExp(sectionResult, TopAbs_EDGE); edgeExp.More();
                 edgeExp.Next()) {
                splitter.Add(TopoDS::Edge(edgeExp.Current()), face);
                hasAnySplit = true;
            }
        }

        if (!hasAnySplit) {
            return new App::DocumentObjectExecReturn(QT_TRANSLATE_NOOP(
                "Exception",
                "Splitting tool does not intersect any selected face"));
        }

        splitter.Build();
        if (!splitter.IsDone()) {
            return new App::DocumentObjectExecReturn(
                QT_TRANSLATE_NOOP("Exception", "Split face operation failed"));
        }

        TopoDS_Shape result = splitter.Shape();
        if (result.IsNull()) {
            return new App::DocumentObjectExecReturn(
                QT_TRANSLATE_NOOP("Exception", "Split face result is null"));
        }

        Part::TopoShape resultShape(result);
        auto solid = getSolid(resultShape);
        if (solid.isNull()) {
            return new App::DocumentObjectExecReturn(
                QT_TRANSLATE_NOOP("Exception", "Resulting shape is not a solid"));
        }

        if (!isSingleSolidRuleSatisfied(resultShape.getShape())) {
            return new App::DocumentObjectExecReturn(QT_TRANSLATE_NOOP(
                "Exception",
                "Result has multiple solids: enable 'Allow Compound' in the active body."));
        }

        this->Shape.setValue(getSolid(resultShape));
        return App::DocumentObject::StdReturn;
    }
    catch (Standard_Failure& e) {
        return new App::DocumentObjectExecReturn(e.GetMessageString());
    }
    catch (Base::Exception& e) {
        return new App::DocumentObjectExecReturn(e.what());
    }
}
