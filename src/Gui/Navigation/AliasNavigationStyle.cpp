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

// Alias-style navigation (Fusion 360 preset):
//   Shift + Alt + LMB = Orbit
//   Shift + Alt + MMB = Pan
//   Shift + Alt + RMB = Zoom
//   Scroll            = Zoom
//   LMB               = Select

#include <Inventor/nodes/SoCamera.h>

#include "Navigation/NavigationStyle.h"
#include "View3DInventorViewer.h"


using namespace Gui;

// ----------------------------------------------------------------------------------

/* TRANSLATOR Gui::AliasNavigationStyle */

TYPESYSTEM_SOURCE(Gui::AliasNavigationStyle, Gui::UserNavigationStyle)

AliasNavigationStyle::AliasNavigationStyle()
    : lockButton1(false)
{}

AliasNavigationStyle::~AliasNavigationStyle() = default;

const char* AliasNavigationStyle::mouseButtons(ViewerMode mode)
{
    switch (mode) {
        case NavigationStyle::SELECTION:
            return QT_TR_NOOP("Press left mouse button");
        case NavigationStyle::PANNING:
            return QT_TR_NOOP("Press Shift+Alt and middle mouse button");
        case NavigationStyle::DRAGGING:
            return QT_TR_NOOP("Press Shift+Alt and left mouse button");
        case NavigationStyle::ZOOMING:
            return QT_TR_NOOP("Press Shift+Alt and right mouse button or scroll middle mouse button");
        default:
            return "No description";
    }
}

std::string AliasNavigationStyle::userFriendlyName() const
{
    return "Alias";
}

SbBool AliasNavigationStyle::processSoEvent(const SoEvent* const ev)
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

    if (type.isDerivedFrom(SoKeyboardEvent::getClassTypeId())) {
        const auto event = static_cast<const SoKeyboardEvent*>(ev);
        processed = processKeyboardEvent(event);
    }

    const SbBool aliasMods = this->shiftdown && this->altdown;

    if (type.isDerivedFrom(SoMouseButtonEvent::getClassTypeId())) {
        const auto* const event = (const SoMouseButtonEvent*)ev;
        const int button = event->getButton();
        const SbBool press = event->getState() == SoButtonEvent::DOWN ? true : false;

        switch (button) {
            case SoMouseButtonEvent::BUTTON1:
                this->lockrecenter = true;
                this->button1down = press;
                if (aliasMods) {
                    if (press) {
                        setupPanningPlane(getCamera());
                        saveCursorPosition(ev);
                        this->centerTime = ev->getTime();
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
                if (aliasMods) {
                    // Shift+Alt+RMB = zoom — never open context menu
                    if (press) {
                        setupPanningPlane(getCamera());
                        this->centerTime = ev->getTime();
                    }
                    processed = true;
                }
                else if (!press && (hasDragged || hasPanned || hasZoomed)) {
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
                if (aliasMods) {
                    processed = true;
                }
                break;
            default:
                break;
        }
    }

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

    if (type.isDerivedFrom(SoMotion3Event::getClassTypeId())) {
        const auto* const event = static_cast<const SoMotion3Event*>(ev);
        if (event) {
            this->processMotionEvent(event);
        }
        processed = true;
    }

    enum
    {
        BUTTON1DOWN = 1 << 0,
        BUTTON2DOWN = 1 << 1,
        BUTTON3DOWN = 1 << 2,
        SHIFTDOWN = 1 << 3,
        ALTDOWN = 1 << 4
    };
    unsigned int combo = (this->button1down ? BUTTON1DOWN : 0)
        | (this->button2down ? BUTTON2DOWN : 0) | (this->button3down ? BUTTON3DOWN : 0)
        | (this->shiftdown ? SHIFTDOWN : 0) | (this->altdown ? ALTDOWN : 0);

    switch (combo) {
        case 0:
            if (curmode == NavigationStyle::SPINNING) {
                break;
            }
            newmode = NavigationStyle::IDLE;
            if (this->lockButton1) {
                this->lockButton1 = false;
                if (curmode != NavigationStyle::SELECTION) {
                    processed = true;
                }
            }
            break;
        case BUTTON1DOWN:
            if (curmode == NavigationStyle::SPINNING
                || (this->lockButton1 && curmode != NavigationStyle::SELECTION)) {
                newmode = NavigationStyle::IDLE;
            }
            else {
                newmode = NavigationStyle::SELECTION;
            }
            break;
        case SHIFTDOWN | ALTDOWN | BUTTON1DOWN:
            // Shift+Alt+LMB = orbit
            if (newmode != NavigationStyle::DRAGGING) {
                saveCursorPosition(ev);
            }
            newmode = NavigationStyle::DRAGGING;
            break;
        case SHIFTDOWN | ALTDOWN | BUTTON3DOWN:
            // Shift+Alt+MMB = pan
            newmode = NavigationStyle::PANNING;
            break;
        case SHIFTDOWN | ALTDOWN | BUTTON2DOWN:
            // Shift+Alt+RMB = zoom
            newmode = NavigationStyle::ZOOMING;
            break;
        default:
            if ((curmode == NavigationStyle::PANNING || curmode == NavigationStyle::ZOOMING
                 || curmode == NavigationStyle::DRAGGING)
                && !(aliasMods
                     && (this->button1down || this->button2down || this->button3down))) {
                newmode = NavigationStyle::IDLE;
            }
            break;
    }

    if (this->button1down && (this->button2down || this->button3down || aliasMods)) {
        this->lockButton1 = true;
        processed = true;
    }

    if (viewer->isEditing() && curmode == NavigationStyle::SELECTION
        && newmode != NavigationStyle::IDLE) {
        if (!aliasMods) {
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
