// SPDX-License-Identifier: LGPL-2.1-or-later
// UniCAD: ViewProvider for Emboss/Deboss feature

#ifndef PARTDESIGNGUI_VIEWPROVIDER_EMBOSS_H
#define PARTDESIGNGUI_VIEWPROVIDER_EMBOSS_H

#include "ViewProvider.h"

namespace PartDesignGui
{

class PartDesignGuiExport ViewProviderEmboss : public ViewProvider
{
    PROPERTY_HEADER_WITH_OVERRIDE(PartDesignGui::ViewProviderEmboss);

public:
    ViewProviderEmboss()
    {
        sPixmap = "PartDesign_Pad";
    }
};

}  // namespace PartDesignGui

#endif  // PARTDESIGNGUI_VIEWPROVIDER_EMBOSS_H
