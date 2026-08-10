/***************************************************************************
 *   Copyright (c) 2026 UNITRONIX                                          *
 *   UniCAD - A fork of FreeCAD                                            *
 *                                                                         *
 *   This file is part of UniCAD.                                          *
 *                                                                         *
 *   UniCAD is free software; you can redistribute it and/or modify        *
 *   it under the terms of the GNU Lesser General Public License (LGPL)    *
 *   as published by the Free Software Foundation; either version 2.1 of   *
 *   the License, or (at your option) any later version.                   *
 ***************************************************************************/

#include "PreCompiled.h"

#include "ViewportStyleManager.h"

#include "Application.h"
#include "Document.h"
#include "MainWindow.h"
#include "View3DInventor.h"
#include "View3DInventorViewer.h"
#include "View3DSettings.h"
#include "ViewParams.h"
#include "ViewProviderDocumentObject.h"
#include "Inventor/SoFCUniversalGrid.h"

#include <App/Application.h>
#include <App/Document.h>
#include <App/DocumentObject.h>
#include <App/PropertyStandard.h>
#include <Base/Color.h>
#include <Base/Console.h>

#include <Inventor/SbColor.h>
#include <Inventor/SoRenderManager.h>

using namespace Gui;

namespace
{

unsigned long packRGB(int r, int g, int b, int a = 0)
{
    return (static_cast<unsigned long>(r & 0xff) << 24)
        | (static_cast<unsigned long>(g & 0xff) << 16)
        | (static_cast<unsigned long>(b & 0xff) << 8)
        | static_cast<unsigned long>(a & 0xff);
}

unsigned long packRGBA(int r, int g, int b, int a = 255)
{
    return packRGB(r, g, b, a);
}

ParameterGrp::handle viewGroup()
{
    return App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/View"
    );
}

ParameterGrp::handle lightGroup()
{
    return viewGroup()->GetGroup("LightSources");
}

void applyEdgeStyleToOpenDocuments()
{
    auto hGrp = viewGroup();
    const int lineWidth = hGrp->GetInt("DefaultShapeLineWidth", 2);
    const unsigned long packed = hGrp->GetUnsigned("DefaultShapeLineColor", packRGB(18, 18, 20));
    Base::Color lineColor;
    lineColor.setPackedRGB(static_cast<uint32_t>(packed));

    for (App::Document* doc : App::GetApplication().getDocuments()) {
        Gui::Document* guiDoc = Application::Instance->getDocument(doc);
        if (!guiDoc) {
            continue;
        }
        for (App::DocumentObject* obj : doc->getObjects()) {
            auto* vp = dynamic_cast<ViewProviderDocumentObject*>(guiDoc->getViewProvider(obj));
            if (!vp) {
                continue;
            }
            if (auto* lw = dynamic_cast<App::PropertyFloat*>(vp->getPropertyByName("LineWidth"))) {
                lw->setValue(static_cast<double>(lineWidth));
            }
            if (auto* lc = dynamic_cast<App::PropertyColor*>(vp->getPropertyByName("LineColor"))) {
                lc->setValue(lineColor);
            }
            if (auto* mode = dynamic_cast<App::PropertyEnumeration*>(vp->getPropertyByName("DisplayMode"))) {
                try {
                    mode->setValue("Flat Lines");
                }
                catch (...) {
                    // Object does not support Flat Lines — ignore
                }
            }
        }
    }
}

}  // namespace

ViewportStyleManager& ViewportStyleManager::instance()
{
    static ViewportStyleManager mgr;
    return mgr;
}

std::vector<std::string> ViewportStyleManager::availablePresets()
{
    return {StyleShaprDark, StyleShaprLight, StyleStudio};
}

const char* ViewportStyleManager::presetLabel(const std::string& id)
{
    if (id == StyleShaprLight) {
        return "Shapr Light";
    }
    if (id == StyleStudio) {
        return "Render Studio";
    }
    return "Shapr Dark (recommended)";
}

std::string ViewportStyleManager::activePreset() const
{
    std::string id = viewGroup()->GetASCII("ViewportStyle", StyleShaprDark);
    if (id != StyleShaprDark && id != StyleShaprLight && id != StyleStudio) {
        return StyleShaprDark;
    }
    return id;
}

void ViewportStyleManager::ensureDefaultApplied()
{
    auto hGrp = viewGroup();
    if (!hGrp->GetBool("ViewportStyleInitialized", false)) {
        applyPreset(StyleShaprDark);
        hGrp->SetBool("ViewportStyleInitialized", true);
        hGrp->SetInt("ShaprEdgeStyleVersion", 2);
    }
    else if (hGrp->GetInt("ShaprEdgeStyleVersion", 0) < 2) {
        // Upgrade edge readability (thicker BREP lines + stronger outline)
        applyPreset(activePreset());
        hGrp->SetInt("ShaprEdgeStyleVersion", 2);
    }
    else {
        // Re-apply runtime viewer bits (grid / post-FX) without rewriting all prefs
        applyPreferencesToViewers();
    }
}

void ViewportStyleManager::applyPreset(const std::string& id)
{
    std::string preset = StyleShaprDark;
    if (id == StyleShaprLight) {
        preset = StyleShaprLight;
    }
    else if (id == StyleStudio) {
        preset = StyleStudio;
    }

    if (preset == StyleStudio) {
        writeStudioPreferences();
    }
    else {
        writePresetPreferences(preset);
        viewGroup()->SetBool("StudioModeActive", false);
        viewGroup()->SetBool("ShowRenderGroundPlane", false);
    }
    viewGroup()->SetASCII("ViewportStyle", preset);

    // Refresh cached ViewParams
    ViewParams::instance()->getHandle()->NotifyAll();

    applyPreferencesToViewers();
    applyEdgeStyleToOpenDocuments();
    Base::Console().log("Viewport style applied: %s\n", preset.c_str());
}

void ViewportStyleManager::writePresetPreferences(const std::string& id) const
{
    auto hGrp = viewGroup();
    auto hLight = lightGroup();
    const bool dark = (id != StyleShaprLight);

    if (dark) {
        // Charcoal background, subtle vertical gradient
        hGrp->SetBool("Gradient", true);
        hGrp->SetBool("RadialGradient", false);
        hGrp->SetBool("UseBackgroundColorMid", false);
        hGrp->SetUnsigned("BackgroundColor", packRGB(30, 30, 30));
        hGrp->SetUnsigned("BackgroundColor2", packRGB(26, 26, 26));   // top
        hGrp->SetUnsigned("BackgroundColor3", packRGB(42, 42, 44));   // bottom
        hGrp->SetUnsigned("BackgroundColor4", packRGB(34, 34, 36));

        // Matte clay defaults
        hGrp->SetUnsigned("DefaultShapeColor", packRGB(182, 184, 188));
        // High-contrast edges (Shapr-like readability)
        hGrp->SetUnsigned("DefaultShapeLineColor", packRGB(18, 18, 20));
        hGrp->SetUnsigned("DefaultShapeVertexColor", packRGB(18, 18, 20));
        hGrp->SetUnsigned("DefaultSpecularColor", packRGB(18, 18, 20));
        hGrp->SetUnsigned("DefaultAmbientColor", packRGB(55, 55, 58));
        hGrp->SetUnsigned("DefaultEmissiveColor", packRGB(0, 0, 0));
        hGrp->SetInt("DefaultShapeShininess", 8);
        hGrp->SetInt("DefaultShapeLineWidth", 2);
        hGrp->SetInt("DefaultShapePointSize", 2);
        hGrp->SetInt("DefaultShapeTransparency", 0);

        hLight->SetBool("EnableHeadlight", true);
        hLight->SetUnsigned("HeadlightColor", packRGBA(255, 255, 255));
        hLight->SetInt("HeadlightIntensity", 85);
        hLight->SetASCII("HeadlightDirection", View3DSettings::defaultHeadLightDirection);

        hLight->SetBool("EnableBacklight", true);
        hLight->SetUnsigned("BacklightColor", packRGBA(230, 230, 235));
        hLight->SetInt("BacklightIntensity", 45);
        hLight->SetASCII("BacklightDirection", View3DSettings::defaultBackLightDirection);

        hLight->SetBool("EnableFillLight", true);
        hLight->SetUnsigned("FillLightColor", packRGBA(220, 235, 245));
        hLight->SetInt("FillLightIntensity", 35);
        hLight->SetASCII("FillLightDirection", View3DSettings::defaultFillLightDirection);

        hLight->SetUnsigned("AmbientLightColor", packRGBA(255, 255, 255));
        hLight->SetInt("AmbientLightIntensity", 18);

        hGrp->SetBool("EnableSSAO", true);
        hGrp->SetBool("EnableEdgeOutline", true);
        hGrp->SetBool("ShowUniversalGrid", true);
        hGrp->SetBool("ShowRenderGroundPlane", false);
        hGrp->SetUnsigned("UniversalGridColor", packRGB(70, 70, 74));
        hGrp->SetUnsigned("UniversalSubGridColor", packRGB(50, 50, 54));
        hGrp->SetUnsigned("EdgeOutlineColor", packRGB(8, 8, 10));
        hGrp->SetFloat("EdgeOutlineThreshold", 0.018);
    }
    else {
        // Light / off-white workspace
        hGrp->SetBool("Gradient", true);
        hGrp->SetBool("RadialGradient", false);
        hGrp->SetBool("UseBackgroundColorMid", false);
        hGrp->SetUnsigned("BackgroundColor", packRGB(240, 240, 242));
        hGrp->SetUnsigned("BackgroundColor2", packRGB(248, 248, 250));  // top
        hGrp->SetUnsigned("BackgroundColor3", packRGB(228, 228, 232));  // bottom
        hGrp->SetUnsigned("BackgroundColor4", packRGB(238, 238, 242));

        hGrp->SetUnsigned("DefaultShapeColor", packRGB(170, 172, 176));
        hGrp->SetUnsigned("DefaultShapeLineColor", packRGB(35, 35, 38));
        hGrp->SetUnsigned("DefaultShapeVertexColor", packRGB(35, 35, 38));
        hGrp->SetUnsigned("DefaultSpecularColor", packRGB(22, 22, 24));
        hGrp->SetUnsigned("DefaultAmbientColor", packRGB(90, 90, 94));
        hGrp->SetUnsigned("DefaultEmissiveColor", packRGB(0, 0, 0));
        hGrp->SetInt("DefaultShapeShininess", 8);
        hGrp->SetInt("DefaultShapeLineWidth", 2);
        hGrp->SetInt("DefaultShapePointSize", 2);
        hGrp->SetInt("DefaultShapeTransparency", 0);

        hLight->SetBool("EnableHeadlight", true);
        hLight->SetUnsigned("HeadlightColor", packRGBA(255, 255, 255));
        hLight->SetInt("HeadlightIntensity", 80);
        hLight->SetASCII("HeadlightDirection", View3DSettings::defaultHeadLightDirection);

        hLight->SetBool("EnableBacklight", true);
        hLight->SetUnsigned("BacklightColor", packRGBA(245, 245, 240));
        hLight->SetInt("BacklightIntensity", 40);
        hLight->SetASCII("BacklightDirection", View3DSettings::defaultBackLightDirection);

        hLight->SetBool("EnableFillLight", true);
        hLight->SetUnsigned("FillLightColor", packRGBA(235, 245, 255));
        hLight->SetInt("FillLightIntensity", 30);
        hLight->SetASCII("FillLightDirection", View3DSettings::defaultFillLightDirection);

        hLight->SetUnsigned("AmbientLightColor", packRGBA(255, 255, 255));
        hLight->SetInt("AmbientLightIntensity", 32);

        hGrp->SetBool("EnableSSAO", true);
        hGrp->SetBool("EnableEdgeOutline", true);
        hGrp->SetBool("ShowUniversalGrid", true);
        hGrp->SetBool("ShowRenderGroundPlane", false);
        hGrp->SetUnsigned("UniversalGridColor", packRGB(185, 188, 195));
        hGrp->SetUnsigned("UniversalSubGridColor", packRGB(210, 212, 218));
        hGrp->SetUnsigned("EdgeOutlineColor", packRGB(25, 25, 28));
        hGrp->SetFloat("EdgeOutlineThreshold", 0.018);
    }
}

void ViewportStyleManager::writeStudioPreferences() const
{
    // Studio builds on Shapr Dark with stronger three-point lights + ground plane
    writePresetPreferences(StyleShaprDark);

    auto hGrp = viewGroup();
    auto hLight = lightGroup();

    hLight->SetBool("EnableHeadlight", true);
    hLight->SetInt("HeadlightIntensity", 95);
    hLight->SetBool("EnableBacklight", true);
    hLight->SetInt("BacklightIntensity", 55);
    hLight->SetBool("EnableFillLight", true);
    hLight->SetInt("FillLightIntensity", 45);
    hLight->SetInt("AmbientLightIntensity", 12);

    hGrp->SetBool("EnableSSAO", true);
    hGrp->SetBool("EnableEdgeOutline", true);
    hGrp->SetBool("ShowUniversalGrid", true);
    hGrp->SetBool("ShowRenderGroundPlane", true);
    hGrp->SetBool("StudioModeActive", true);
}

namespace
{

void copyBool(ParameterGrp::handle from, ParameterGrp::handle to, const char* key, bool def)
{
    to->SetBool(key, from->GetBool(key, def));
}

void copyInt(ParameterGrp::handle from, ParameterGrp::handle to, const char* key, int def)
{
    to->SetInt(key, from->GetInt(key, def));
}

void copyUnsigned(ParameterGrp::handle from, ParameterGrp::handle to, const char* key, unsigned long def)
{
    to->SetUnsigned(key, from->GetUnsigned(key, def));
}

void copyAscii(ParameterGrp::handle from, ParameterGrp::handle to, const char* key, const char* def)
{
    to->SetASCII(key, from->GetASCII(key, def).c_str());
}

void copyFloat(ParameterGrp::handle from, ParameterGrp::handle to, const char* key, double def)
{
    to->SetFloat(key, from->GetFloat(key, def));
}

void snapshotStudioRelevantPrefs(ParameterGrp::handle fromView, ParameterGrp::handle fromLight,
                                 ParameterGrp::handle toView, ParameterGrp::handle toLight)
{
    copyBool(fromView, toView, "EnableSSAO", true);
    copyBool(fromView, toView, "EnableEdgeOutline", true);
    copyBool(fromView, toView, "ShowUniversalGrid", true);
    copyBool(fromView, toView, "ShowRenderGroundPlane", false);
    copyUnsigned(fromView, toView, "UniversalGridColor", packRGB(70, 70, 74));
    copyUnsigned(fromView, toView, "UniversalSubGridColor", packRGB(50, 50, 54));
    copyUnsigned(fromView, toView, "EdgeOutlineColor", packRGB(8, 8, 10));
    copyFloat(fromView, toView, "EdgeOutlineThreshold", 0.018);
    copyAscii(fromView, toView, "ViewportStyle", ViewportStyleManager::StyleShaprDark);

    copyBool(fromLight, toLight, "EnableHeadlight", true);
    copyBool(fromLight, toLight, "EnableBacklight", true);
    copyBool(fromLight, toLight, "EnableFillLight", true);
    copyInt(fromLight, toLight, "HeadlightIntensity", 90);
    copyInt(fromLight, toLight, "BacklightIntensity", 45);
    copyInt(fromLight, toLight, "FillLightIntensity", 35);
    copyInt(fromLight, toLight, "AmbientLightIntensity", 20);
    copyUnsigned(fromLight, toLight, "HeadlightColor", packRGBA(255, 255, 255));
    copyUnsigned(fromLight, toLight, "BacklightColor", packRGBA(230, 230, 235));
    copyUnsigned(fromLight, toLight, "FillLightColor", packRGBA(220, 235, 245));
    copyUnsigned(fromLight, toLight, "AmbientLightColor", packRGBA(255, 255, 255));
    copyAscii(fromLight, toLight, "HeadlightDirection", View3DSettings::defaultHeadLightDirection);
    copyAscii(fromLight, toLight, "BacklightDirection", View3DSettings::defaultBackLightDirection);
    copyAscii(fromLight, toLight, "FillLightDirection", View3DSettings::defaultFillLightDirection);
}

}  // namespace

bool ViewportStyleManager::isStudioModeActive() const
{
    return viewGroup()->GetBool("StudioModeActive", false);
}

void ViewportStyleManager::saveStudioBackup() const
{
    auto hView = viewGroup();
    auto hLight = lightGroup();
    auto hBackupView = hView->GetGroup("StudioBackup");
    auto hBackupLight = hBackupView->GetGroup("LightSources");
    snapshotStudioRelevantPrefs(hView, hLight, hBackupView, hBackupLight);
}

void ViewportStyleManager::restoreStudioBackup() const
{
    auto hView = viewGroup();
    auto hLight = lightGroup();
    auto hBackupView = hView->GetGroup("StudioBackup");
    auto hBackupLight = hBackupView->GetGroup("LightSources");
    snapshotStudioRelevantPrefs(hBackupView, hBackupLight, hView, hLight);
    hView->SetBool("StudioModeActive", false);
}

void ViewportStyleManager::applyStudioMode(bool enable)
{
    auto hView = viewGroup();
    if (enable) {
        if (!isStudioModeActive()) {
            saveStudioBackup();
        }
        writeStudioPreferences();
        hView->SetASCII("ViewportStyle", StyleStudio);
        ViewParams::instance()->getHandle()->NotifyAll();
        applyPreferencesToViewers();
        Base::Console().log("Render Studio mode enabled\n");
    }
    else if (isStudioModeActive()) {
        restoreStudioBackup();
        ViewParams::instance()->getHandle()->NotifyAll();
        applyPreferencesToViewers();
        Base::Console().log("Render Studio mode disabled (prefs restored)\n");
    }
}

void ViewportStyleManager::setGroundPlaneVisible(bool visible)
{
    viewGroup()->SetBool("ShowRenderGroundPlane", visible);
    // Keep grid in sync when enabling ground for a "on floor" look
    if (visible) {
        viewGroup()->SetBool("ShowUniversalGrid", true);
    }
    applyPreferencesToViewers();
}

bool ViewportStyleManager::isGroundPlaneVisible() const
{
    return viewGroup()->GetBool("ShowRenderGroundPlane", false);
}

void ViewportStyleManager::setSSAOEnabled(bool enabled)
{
    auto hGrp = viewGroup();
    hGrp->SetBool("EnableSSAO", enabled);
    hGrp->SetBool(
        "EnableEdgeOutline",
        hGrp->GetBool("EnableEdgeOutline", true)
    );
    applyPreferencesToViewers();
}

void ViewportStyleManager::setEdgeOutlineEnabled(bool enabled)
{
    viewGroup()->SetBool("EnableEdgeOutline", enabled);
    applyPreferencesToViewers();
}

void ViewportStyleManager::applyPreferencesToViewers() const
{
    auto* mw = getMainWindow();
    if (!mw) {
        return;
    }

    auto views = mw->findChildren<View3DInventor*>();
    for (auto* view3d : views) {
        if (!view3d) {
            continue;
        }
        auto* viewer = view3d->getViewer();
        if (!viewer) {
            continue;
        }

        // Background + lights via existing preference bridge
        View3DSettings settings(viewGroup(), viewer);
        settings.ignoreNavigationStyle = true;
        settings.applySettings();

        applyToViewer(viewer);
        viewer->getSoRenderManager()->scheduleRedraw();
    }
}

void ViewportStyleManager::applyToViewer(View3DInventorViewer* viewer) const
{
    if (!viewer) {
        return;
    }

    auto hGrp = viewGroup();

    // Grid styling
    const bool showGrid = hGrp->GetBool("ShowUniversalGrid", true);
    viewer->setUniversalGridVisible(showGrid);
    viewer->setUniversalGridOriginVisible(showGrid);

    if (auto* grid = viewer->getUniversalGrid()) {
        unsigned long gc = hGrp->GetUnsigned("UniversalGridColor", packRGB(70, 70, 74));
        unsigned long sc = hGrp->GetUnsigned("UniversalSubGridColor", packRGB(50, 50, 54));
        float r = ((gc >> 24) & 0xff) / 255.0f;
        float g = ((gc >> 16) & 0xff) / 255.0f;
        float b = ((gc >> 8) & 0xff) / 255.0f;
        grid->setGridColor(SbColor(r, g, b));
        r = ((sc >> 24) & 0xff) / 255.0f;
        g = ((sc >> 16) & 0xff) / 255.0f;
        b = ((sc >> 8) & 0xff) / 255.0f;
        grid->setSubGridColor(SbColor(r, g, b));
    }

    viewer->setGroundPlaneVisible(hGrp->GetBool("ShowRenderGroundPlane", false));

    viewer->setPostProcessEnabled(
        hGrp->GetBool("EnableSSAO", true) || hGrp->GetBool("EnableEdgeOutline", true)
    );
    viewer->setSSAOEnabled(hGrp->GetBool("EnableSSAO", true));
    viewer->setEdgeOutlineEnabled(hGrp->GetBool("EnableEdgeOutline", true));

    unsigned long oc = hGrp->GetUnsigned("EdgeOutlineColor", packRGB(8, 8, 10));
    viewer->setEdgeOutlineColor(
        ((oc >> 24) & 0xff) / 255.0f,
        ((oc >> 16) & 0xff) / 255.0f,
        ((oc >> 8) & 0xff) / 255.0f
    );
    viewer->setEdgeOutlineThreshold(
        static_cast<float>(hGrp->GetFloat("EdgeOutlineThreshold", 0.018))
    );
}
