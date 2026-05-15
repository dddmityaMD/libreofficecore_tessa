/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <array>
#include <docmodel/dllapi.h>
#include <rtl/ustring.hxx>
#include <docmodel/theme/ThemeColorType.hxx>
#include <docmodel/color/ComplexColor.hxx>
#include <tools/color.hxx>

typedef struct _xmlTextWriter* xmlTextWriterPtr;

namespace model
{
class DOCMODEL_DLLPUBLIC ColorSet
{
    OUString maName;
    std::array<Color, 12> maColors;

    /** OOXML <a:sysClr> origin tracker. Indexed by ThemeColorType.
        SystemColorType::Unused means "not from a sysClr"; any other
        value means the source theme bound this slot to an OS system
        colour (e.g. dk1 ⇄ windowText). Used to round-trip themes
        whose `<a:dk1><a:sysClr/>` carries dark-mode adaptability —
        without this, OOXML round-trip resolves the sysClr to a
        literal `<a:srgbClr>` and the file no longer adapts to the
        reader's OS theme. */
    std::array<model::SystemColorType, 12> maSysColorTypes{};

public:
    ColorSet(OUString const& rName);

    void setName(OUString const& rName) { maName = rName; }

    void add(model::ThemeColorType Type, Color aColorData);

    /** Records that the given theme color slot was originally specified
        via `<a:sysClr>` in the source. The resolved RGB value still
        comes from `add()` / `getColor()`; this metadata is purely a
        round-trip signal for ThemeExport. */
    void setSystemColorType(model::ThemeColorType eThemeType,
                            model::SystemColorType eSysType);

    /** Returns the system color binding for the given slot, or
        SystemColorType::Unused if none was recorded. */
    model::SystemColorType getSystemColorType(model::ThemeColorType eThemeType) const;

    const OUString& getName() const { return maName; }

    Color resolveColor(model::ComplexColor const& rComplexColor) const;

    /// Resolve a themed ComplexColor by applying LumMod and LumOff together
    /// in a single HSL round-trip. Use this instead of resolveColor() for
    /// OOXML-originated colors, where the two transforms must be combined
    /// to produce correct results.
    Color resolveOOXMLColor(model::ComplexColor const& rComplexColor) const;

    Color getColor(model::ThemeColorType eType) const;

    void dumpAsXml(xmlTextWriterPtr pWriter) const;
};

} // end of namespace model

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
