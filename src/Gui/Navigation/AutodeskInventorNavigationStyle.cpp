// SPDX-License-Identifier: LGPL-2.1-or-later
/***************************************************************************
 *   Copyright (c) 2026 UniCAD Contributors                            *
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

// Autodesk Inventor-style navigation (Fusion 360 preset):
//   F2 + LMB = Pan
//   F3 + LMB = Zoom
//   F4 + LMB = Orbit
//   Scroll   = Zoom
//   LMB      = Select

#include <Inventor/nodes/SoCamera.h>

#include "Navigation/NavigationStyle.h"
#include "View3DInventorViewer.h"


using namespace Gui;

// ----------------------------------------------------------------------------------

/* TRANSLATOR Gui::AutodeskInventorNavigationStyle */

TYPESYSTEM_SOURCE(Gui::AutodeskInventorNavigationStyle, Gui::UserNavigationStyle)

AutodeskInventorNavigationStyle::AutodeskInventorNavigationStyle()
    : lockButton1(false)
    , f2down(false)
    , f3down(false)
    , f4down(false)
{}

AutodeskInventorNavigationStyle::~AutodeskInventorNavigationStyle() = default;

const char* AutodeskInventorNavigationStyle::mouseButtons(ViewerMode mode)
{
    switch (mode) {
        case NavigationStyle::SELECTION:
            return QT_TR_NOOP("Press left mouse button");
        case NavigationStyle::PANNING:
            return QT_TR_NOOP("Press F2 and left mouse button");
        case NavigationStyle::DRAGGING:
            return QT_TR_NOOP("Press F4 and left mouse button");
        case NavigationStyle::ZOOMING:
            return QT_TR_NOOP("Press F3 and left mouse button or scroll middle mouse button");
        default:
            return "No description";
    }
}

std::string AutodeskInventorNavigationStyle::userFriendlyName() const
{
    return "Autodesk Inventor";
}

SbBool AutodeskInventorNavigationStyle::processSoEvent(const SoEvent* const ev)
{
    if (this->isSeekMode()) {
        return inherited::processSoEvent(ev);
    }
    if (!this->isSeekMode() && !this->isAnimating() && this->isViewing()) {
        this->setViewing(false);
    }

    const SoType type(ev->getTypeId());

    const SbViewportRegion& vp = viewer->getSoRenderManager()->getViewportRegion();
    const SbVec2s pos(ev->getPosition());
    const SbVec2f posn = normalizePixelPos(pos);

    const SbVec2f prevnormalized = this->lastmouseposition;
    this->lastmouseposition = posn;

    SbBool processed = false;

    const ViewerMode curmode = this->currentmode;
    ViewerMode newmode = curmode;

    syncModifierKeys(ev);

    if (!viewer->isEditing()) {
        processed = handleEventInForeground(ev);
        if (processed) {
            return true;
        }
    }

    // Keyboard handling — track F2/F3/F4 without consuming them alone
    // so application shortcuts (e.g. rename on F2) still work when unused.
    if (type.isDerivedFrom(SoKeyboardEvent::getClassTypeId())) {
        const auto* const event = static_cast<const SoKeyboardEvent*>(ev);
        const SbBool press = event->getState() == SoButtonEvent::DOWN ? true : false;
        switch (event->getKey()) {
            case SoKeyboardEvent::F2:
                this->f2down = press;
                break;
            case SoKeyboardEvent::F3:
                this->f3down = press;
                break;
            case SoKeyboardEvent::F4:
                this->f4down = press;
                break;
            default:
                processed = processKeyboardEvent(event);
                break;
        }
    }

    // Mouse Button handling
    if (type.isDerivedFrom(SoMouseButtonEvent::getClassTypeId())) {
        const auto* const event = (const SoMouseButtonEvent*)ev;
        const int button = event->getButton();
        const SbBool press = event->getState() == SoButtonEvent::DOWN ? true : false;
        const SbBool navKey = this->f2down || this->f3down || this->f4down;

        switch (button) {
            case SoMouseButtonEvent::BUTTON1:
                this->lockrecenter = true;
                this->button1down = press;
                if (navKey) {
                    if (press) {
                        setupPanningPlane(getCamera());
                        this->centerTime = ev->getTime();
                        if (this->f4down) {
                            saveCursorPosition(ev);
                        }
                    }
                    processed = true;
                }
                else if (press && (this->currentmode == NavigationStyle::SEEK_WAIT_MODE)) {
                    newmode = NavigationStyle::SEEK_MODE;
                    this->seekToPoint(pos);
                    processed = true;
                }
                else if (press
                         && (this->currentmode == NavigationStyle::PANNING
                             || this->currentmode == NavigationStyle::ZOOMING)) {
                    newmode = NavigationStyle::DRAGGING;
                    saveCursorPosition(ev);
                    this->centerTime = ev->getTime();
                    processed = true;
                }
                else if (!press && (this->currentmode == NavigationStyle::DRAGGING)) {
                    processed = true;
                }
                else if (viewer->isEditing() && (this->currentmode == NavigationStyle::SPINNING)) {
                    processed = true;
                }
                else {
                    processed = processClickEvent(event);
                }
                break;

            case SoMouseButtonEvent::BUTTON2:
                this->lockrecenter = true;
                if (!press && (hasDragged || hasPanned || hasZoomed)) {
                    processed = true;
                }
                else if (!press && !viewer->isEditing()) {
                    if (this->currentmode != NavigationStyle::ZOOMING
                        && this->currentmode != NavigationStyle::PANNING
                        && this->currentmode != NavigationStyle::DRAGGING) {
                        if (this->isPopupMenuEnabled()) {
                            this->openPopupMenu(event->getPosition());
                        }
                    }
                }
                this->button2down = press;
                break;

            case SoMouseButtonEvent::BUTTON3:
                if (press) {
                    this->centerTime = ev->getTime();
                    setupPanningPlane(getCamera());
                    this->lockrecenter = false;
                }
                this->button3down = press;
                break;
            default:
                break;
        }
    }

    // Mouse Movement handling
    if (type.isDerivedFrom(SoLocation2Event::getClassTypeId())) {
        this->lockrecenter = true;
        const auto* const event = (const SoLocation2Event*)ev;
        if (this->currentmode == NavigationStyle::ZOOMING) {
            this->zoomByCursor(posn, prevnormalized);
            processed = true;
        }
        else if (this->currentmode == NavigationStyle::PANNING) {
            float ratio = vp.getViewportAspectRatio();
            panCamera(
                viewer->getSoRenderManager()->getCamera(),
                ratio,
                this->panningplane,
                posn,
                prevnormalized
            );
            processed = true;
        }
        else if (this->currentmode == NavigationStyle::DRAGGING) {
            this->addToLog(event->getPosition(), event->getTime());
            this->spin(posn);
            moveCursorPosition();
            processed = true;
        }
    }

    // Spaceball & Joystick handling
    if (type.isDerivedFrom(SoMotion3Event::getClassTypeId())) {
        const auto* const event = static_cast<const SoMotion3Event*>(ev);
        if (event) {
            this->processMotionEvent(event);
        }
        processed = true;
    }

    // Mode switching: F2+LMB pan, F3+LMB zoom, F4+LMB orbit
    if (this->button1down && this->f2down) {
        newmode = NavigationStyle::PANNING;
    }
    else if (this->button1down && this->f3down) {
        newmode = NavigationStyle::ZOOMING;
    }
    else if (this->button1down && this->f4down) {
        if (newmode != NavigationStyle::DRAGGING) {
            saveCursorPosition(ev);
        }
        newmode = NavigationStyle::DRAGGING;
    }
    else if (this->button1down) {
        if (curmode == NavigationStyle::SPINNING
            || (this->lockButton1 && curmode != NavigationStyle::SELECTION)) {
            newmode = NavigationStyle::IDLE;
        }
        else if (curmode == NavigationStyle::PANNING || curmode == NavigationStyle::ZOOMING
                 || curmode == NavigationStyle::DRAGGING) {
            // F-key released while LMB still down — stop navigation
            newmode = NavigationStyle::IDLE;
            this->lockButton1 = true;
            processed = true;
        }
        else {
            newmode = NavigationStyle::SELECTION;
        }
    }
    else {
        if (curmode != NavigationStyle::SPINNING) {
            newmode = NavigationStyle::IDLE;
        }
        if (this->lockButton1) {
            this->lockButton1 = false;
            if (curmode != NavigationStyle::SELECTION) {
                processed = true;
            }
        }
    }

    if (this->button1down && (this->f2down || this->f3down || this->f4down)) {
        processed = true;
    }

    if (viewer->isEditing() && curmode == NavigationStyle::SELECTION
        && newmode != NavigationStyle::IDLE) {
        if (!(this->f2down || this->f3down || this->f4down)) {
            newmode = NavigationStyle::SELECTION;
            processed = false;
        }
    }

    if (newmode == IDLE && !button1down && !button2down && !button3down) {
        hasPanned = false;
        hasDragged = false;
        hasZoomed = false;
    }

    if (newmode != curmode) {
        this->setViewingMode(newmode);
    }

    if (!processed) {
        processed = inherited::processSoEvent(ev);
    }

    return processed;
}
