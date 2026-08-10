/***************************************************************************
 *   Copyright (c) 2026 UNITRONIX                                          *
 *   UniCAD - A fork of FreeCAD                                            *
 ***************************************************************************/

#include "PreCompiled.h"

#ifndef _PreComp_
# include <cmath>
# include <QCheckBox>
# include <QDoubleSpinBox>
# include <QFormLayout>
# include <QLabel>
# include <QSpinBox>
# include <QVBoxLayout>
# include <QWidget>
#endif

#include "TaskRenderStudio.h"

#include "BitmapFactory.h"
#include "ViewportStyleManager.h"

#include <App/Application.h>
#include <Base/Builder3D.h>
#include <Base/Parameter.h>
#include <Base/Tools.h>
#include <Base/Vector3D.h>
#include <Gui/View3DSettings.h>

using namespace Gui;

namespace
{

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

Base::Vector3d azimuthElevationToDirection(double azimuthDeg, double elevationDeg)
{
    const double azimuth = Base::toRadians(azimuthDeg);
    const double elevation = Base::toRadians(elevationDeg);
    Base::Vector3d direction {
        std::sin(azimuth) * std::cos(elevation),
        std::cos(azimuth) * std::cos(elevation),
        std::sin(elevation)
    };
    direction.Normalize();
    return direction;
}

std::pair<double, double> directionToAzimuthElevation(Base::Vector3d direction)
{
    const double azimuth = std::atan2(direction[0], direction[1]);
    const double elevation = std::atan2(
        direction[2],
        std::sqrt(direction[1] * direction[1] + direction[0] * direction[0])
    );
    return {Base::toDegrees(azimuth), Base::toDegrees(elevation)};
}

}  // namespace

/* TRANSLATOR Gui::TaskRenderStudioBox */

TaskRenderStudioBox::TaskRenderStudioBox(QWidget* parent)
    : TaskBox(Gui::BitmapFactory().pixmap("bulb"), tr("Render Lighting"), true, parent)
{
    auto* proxy = new QWidget(this);
    auto* layout = new QFormLayout(proxy);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(6);

    m_azimuth = new QDoubleSpinBox(proxy);
    m_azimuth->setRange(-180.0, 180.0);
    m_azimuth->setDecimals(1);
    m_azimuth->setSuffix(QStringLiteral(" °"));
    m_azimuth->setToolTip(tr("Horizontal light angle (azimuth)"));

    m_elevation = new QDoubleSpinBox(proxy);
    m_elevation->setRange(-89.0, 89.0);
    m_elevation->setDecimals(1);
    m_elevation->setSuffix(QStringLiteral(" °"));
    m_elevation->setToolTip(tr("Vertical light angle (elevation)"));

    m_intensity = new QSpinBox(proxy);
    m_intensity->setRange(0, 100);
    m_intensity->setSuffix(QStringLiteral(" %"));
    m_intensity->setToolTip(tr("Main light intensity"));

    m_ambient = new QSpinBox(proxy);
    m_ambient->setRange(0, 100);
    m_ambient->setSuffix(QStringLiteral(" %"));
    m_ambient->setToolTip(tr("Ambient light intensity"));

    m_ground = new QCheckBox(tr("Place on ground"), proxy);
    m_fill = new QCheckBox(tr("Fill light"), proxy);
    m_back = new QCheckBox(tr("Back light"), proxy);

    layout->addRow(tr("Azimuth"), m_azimuth);
    layout->addRow(tr("Elevation"), m_elevation);
    layout->addRow(tr("Intensity"), m_intensity);
    layout->addRow(tr("Ambient"), m_ambient);
    layout->addRow(m_ground);
    layout->addRow(m_fill);
    layout->addRow(m_back);

    groupLayout()->addWidget(proxy);

    loadFromPreferences();

    connect(m_azimuth, qOverload<double>(&QDoubleSpinBox::valueChanged),
            this, &TaskRenderStudioBox::onLightingChanged);
    connect(m_elevation, qOverload<double>(&QDoubleSpinBox::valueChanged),
            this, &TaskRenderStudioBox::onLightingChanged);
    connect(m_intensity, qOverload<int>(&QSpinBox::valueChanged),
            this, &TaskRenderStudioBox::onLightingChanged);
    connect(m_ambient, qOverload<int>(&QSpinBox::valueChanged),
            this, &TaskRenderStudioBox::onAmbientChanged);
    connect(m_ground, &QCheckBox::toggled, this, &TaskRenderStudioBox::onGroundToggled);
    connect(m_fill, &QCheckBox::toggled, this, &TaskRenderStudioBox::onFillToggled);
    connect(m_back, &QCheckBox::toggled, this, &TaskRenderStudioBox::onBackToggled);
}

void TaskRenderStudioBox::loadFromPreferences()
{
    const QSignalBlocker b1(m_azimuth);
    const QSignalBlocker b2(m_elevation);
    const QSignalBlocker b3(m_intensity);
    const QSignalBlocker b4(m_ambient);
    const QSignalBlocker b5(m_ground);
    const QSignalBlocker b6(m_fill);
    const QSignalBlocker b7(m_back);

    auto hLight = lightGroup();
    auto hView = viewGroup();

    try {
        const Base::Vector3f direction = Base::stringToVector(
            hLight->GetASCII("HeadlightDirection", View3DSettings::defaultHeadLightDirection)
        );
        const auto [azimuth, elevation] = directionToAzimuthElevation(
            Base::Vector3d(direction.x, direction.y, direction.z)
        );
        m_azimuth->setValue(azimuth);
        m_elevation->setValue(elevation);
    }
    catch (...) {
        m_azimuth->setValue(45.0);
        m_elevation->setValue(45.0);
    }

    m_intensity->setValue(static_cast<int>(hLight->GetInt("HeadlightIntensity", 90)));
    m_ambient->setValue(static_cast<int>(hLight->GetInt("AmbientLightIntensity", 20)));
    m_ground->setChecked(hView->GetBool("ShowRenderGroundPlane", false));
    m_fill->setChecked(hLight->GetBool("EnableFillLight", true));
    m_back->setChecked(hLight->GetBool("EnableBacklight", true));
}

void TaskRenderStudioBox::applyLightingToPreferences()
{
    auto hLight = lightGroup();
    const auto direction = azimuthElevationToDirection(m_azimuth->value(), m_elevation->value());
    const Base::Vector3f dirF(
        static_cast<float>(direction.x),
        static_cast<float>(direction.y),
        static_cast<float>(direction.z)
    );
    hLight->SetASCII("HeadlightDirection", Base::vectorToString(dirF).c_str());
    hLight->SetInt("HeadlightIntensity", m_intensity->value());
    ViewportStyleManager::instance().applyPreferencesToViewers();
}

void TaskRenderStudioBox::onLightingChanged()
{
    applyLightingToPreferences();
}

void TaskRenderStudioBox::onAmbientChanged(int value)
{
    lightGroup()->SetInt("AmbientLightIntensity", value);
    ViewportStyleManager::instance().applyPreferencesToViewers();
}

void TaskRenderStudioBox::onGroundToggled(bool checked)
{
    ViewportStyleManager::instance().setGroundPlaneVisible(checked);
}

void TaskRenderStudioBox::onFillToggled(bool checked)
{
    lightGroup()->SetBool("EnableFillLight", checked);
    ViewportStyleManager::instance().applyPreferencesToViewers();
}

void TaskRenderStudioBox::onBackToggled(bool checked)
{
    lightGroup()->SetBool("EnableBacklight", checked);
    ViewportStyleManager::instance().applyPreferencesToViewers();
}

/* TRANSLATOR Gui::TaskRenderStudio */

TaskRenderStudio::TaskRenderStudio()
{
    m_box = new TaskRenderStudioBox();
    Content.push_back(m_box);
}

#include "moc_TaskRenderStudio.cpp"
