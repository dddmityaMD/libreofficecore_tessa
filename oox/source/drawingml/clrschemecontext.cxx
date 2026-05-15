/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <drawingml/clrschemecontext.hxx>
#include <oox/core/xmlfilterbase.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>

using namespace ::oox::core;

namespace oox::drawingml {

static void setClrMap( const ::oox::AttributeList& rAttributes,
            ClrMap& rClrMap, sal_Int32 nToken )
{
    if ( rAttributes.hasAttribute( nToken ) )
    {
        sal_Int32 nMappedToken = rAttributes.getToken( nToken, 0 );
        rClrMap.setColorMap( nToken, nMappedToken );
    }
}

clrMapContext::clrMapContext( ContextHandler2Helper const & rParent,
    const ::oox::AttributeList& rAttributes, ClrMap& rClrMap )
: ContextHandler2( rParent )
{
    setClrMap( rAttributes, rClrMap, XML_bg1 );
    setClrMap( rAttributes, rClrMap, XML_tx1 );
    setClrMap( rAttributes, rClrMap, XML_bg2 );
    setClrMap( rAttributes, rClrMap, XML_tx2 );
    setClrMap( rAttributes, rClrMap, XML_accent1 );
    setClrMap( rAttributes, rClrMap, XML_accent2 );
    setClrMap( rAttributes, rClrMap, XML_accent3 );
    setClrMap( rAttributes, rClrMap, XML_accent4 );
    setClrMap( rAttributes, rClrMap, XML_accent5 );
    setClrMap( rAttributes, rClrMap, XML_accent6 );
    setClrMap( rAttributes, rClrMap, XML_hlink );
    setClrMap( rAttributes, rClrMap, XML_folHlink );
}

clrSchemeColorContext::clrSchemeColorContext(ContextHandler2Helper const & rParent, ClrScheme& rClrScheme, model::ColorSet& rColorSet, sal_Int32 nColorToken)
    : ColorContext(rParent, this, &maComplexColor)
    , mrClrScheme(rClrScheme)
    , mrColorSet(rColorSet)
    , mnColorToken(nColorToken)
{
}

clrSchemeColorContext::~clrSchemeColorContext()
{
    ::Color aColor = getColor(getFilter().getGraphicHelper());
    mrClrScheme.setColor(mnColorToken, aColor);

    // Map the OOXML token (<a:dk1> etc.) to the docmodel ThemeColorType
    // slot in the ColorSet, so the RGB value can later be retrieved by
    // ThemeExport on save.
    model::ThemeColorType eThemeType = model::ThemeColorType::Unknown;
    switch (mnColorToken)
    {
        case XML_tx1:
        case XML_dk1:      eThemeType = model::ThemeColorType::Dark1;             break;
        case XML_bg1:
        case XML_lt1:      eThemeType = model::ThemeColorType::Light1;            break;
        case XML_tx2:
        case XML_dk2:      eThemeType = model::ThemeColorType::Dark2;             break;
        case XML_bg2:
        case XML_lt2:      eThemeType = model::ThemeColorType::Light2;            break;
        case XML_accent1:  eThemeType = model::ThemeColorType::Accent1;           break;
        case XML_accent2:  eThemeType = model::ThemeColorType::Accent2;           break;
        case XML_accent3:  eThemeType = model::ThemeColorType::Accent3;           break;
        case XML_accent4:  eThemeType = model::ThemeColorType::Accent4;           break;
        case XML_accent5:  eThemeType = model::ThemeColorType::Accent5;           break;
        case XML_accent6:  eThemeType = model::ThemeColorType::Accent6;           break;
        case XML_hlink:    eThemeType = model::ThemeColorType::Hyperlink;         break;
        case XML_folHlink: eThemeType = model::ThemeColorType::FollowedHyperlink; break;
        default: break;
    }
    if (eThemeType == model::ThemeColorType::Unknown)
        return;

    mrColorSet.add(eThemeType, aColor);

    // If the source theme bound this slot via <a:sysClr> (e.g. dk1 ⇄
    // windowText), preserve the binding alongside the resolved RGB.
    // ThemeExport reads it back to emit <a:sysClr> instead of
    // <a:srgbClr>, which keeps dark-mode adaptability across the
    // round-trip. Without this, Excel running in dark mode would show
    // hard-coded #000000 where the source asked for the OS dark text
    // colour.
    if (maComplexColor.getType() == model::ColorType::System)
        mrColorSet.setSystemColorType(eThemeType, maComplexColor.getSystemColorType());
}

clrSchemeContext::clrSchemeContext(ContextHandler2Helper const & rParent, ClrScheme& rClrScheme, model::ColorSet& rColorSet)
    : ContextHandler2(rParent)
    , mrClrScheme(rClrScheme)
    , mrColorSet(rColorSet)
{
}

ContextHandlerRef clrSchemeContext::onCreateContext(
        sal_Int32 nElement, const AttributeList& )
{
    switch( nElement )
    {
        case A_TOKEN( dk1 ):
        case A_TOKEN( tx1 ):
        case A_TOKEN( lt1 ):
        case A_TOKEN( bg1 ):
        case A_TOKEN( dk2 ):
        case A_TOKEN( tx2 ):
        case A_TOKEN( lt2 ):
        case A_TOKEN( bg2 ):
        case A_TOKEN( accent1 ):
        case A_TOKEN( accent2 ):
        case A_TOKEN( accent3 ):
        case A_TOKEN( accent4 ):
        case A_TOKEN( accent5 ):
        case A_TOKEN( accent6 ):
        case A_TOKEN( hlink ):
        case A_TOKEN( folHlink ):
            return new clrSchemeColorContext(*this, mrClrScheme, mrColorSet, getBaseToken(nElement));
    }
    return nullptr;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
