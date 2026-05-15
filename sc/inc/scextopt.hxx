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
#pragma once

#include <map>
#include <memory>
#include <set>
#include <vector>
#include <tools/gen.hxx>
#include <tools/color.hxx>
#include "rangelst.hxx"

/** Extended settings for the document, used in import/export filters. */
struct ScExtDocSettings
{
    OUString            maGlobCodeName;     ///< Global codename (VBA module name).
    double              mfTabBarWidth;      ///< Width of the tabbar, relative to frame window width (0.0 ... 1.0).
    sal_uInt32          mnLinkCnt;          ///< Recursive counter for loading external documents.
    SCTAB               mnDisplTab;         ///< Index of displayed sheet.
    /** moLowestEdited:  oldest Excel version that edited this .XLSX - limit feature set to that.
     *  4 is 2007, 5 is 2010, 6 is 2013 and 2016, 7 is 2019-2024+
     */
    std::optional<sal_Int16> moLowestEdited;

    /** OOXML workbookPr@defaultThemeVersion. Excel-emitted theme stamp
        used by theme-update detection. Captured on xlsx import and
        re-emitted on xlsx export so the workbook round-trips with the
        same theme version identifier. */
    std::optional<sal_Int32> moDefaultThemeVersion;

    /** OOXML workbookPr@hidePivotFieldList. When true, Excel hides the
        right-side pivot field list panel by default when a pivot is
        selected. Captured on xlsx import and re-emitted on xlsx export. */
    std::optional<bool> moHidePivotFieldList;

    /** OOXML workbook.xml `<extLst><ext uri="{B58B0392-...}">` calc-feature
        flag list (Excel 2019+ `xcalcf:feature name="microsoft.com:XXX"`
        entries — RD, Single, FV, CNMTM, LET_WF, LAMBDA_WF, ARRAYTEXT_WF
        etc.). Captured on xlsx import and re-emitted on xlsx export so
        downstream Excel readers continue to see the same feature-gating
        markers. Empty when source had no such extension. */
    std::vector<OUString> maOoxCalcFeatures;

    /** OOXML pivotCache/pivotCacheDefinition*.xml `<s u="1">` SharePoint
        "unresolved template" string markers — captured on import,
        re-emitted on save so SharePoint-aware tooling (EOS, Tessa SED
        report regen) continues to see which cache strings are
        placeholders that need re-resolution. The set holds the literal
        string value (e.g. `{*t_DocRegDate}`); on save, any cache string
        whose value is in the set gets `u="1"` re-emitted. */
    std::set<OUString> maOoxPivotCacheUnusedStrings;

    /** OOXML `<font><scheme val="..."/></font>` theme-binding lookup,
        keyed by font name. When a font is bound to the theme major/
        minor font slot, the binding is preserved so a later theme
        change restyles the cells. Sc's internal font model doesn't
        track this attribute, so we shadow it by name. Mapping value
        is the XML token id (XML_major / XML_minor). */
    std::map<OUString, sal_Int32> maOoxFontSchemeByName;

    explicit            ScExtDocSettings();
};

/** Enumerates possible positions of panes in split sheets. */
enum ScExtPanePos
{
    SCEXT_PANE_TOPLEFT,         ///< Single, top, left, or top-left pane.
    SCEXT_PANE_TOPRIGHT,        ///< Right, or top-right pane.
    SCEXT_PANE_BOTTOMLEFT,      ///< Bottom, or bottom-left pane.
    SCEXT_PANE_BOTTOMRIGHT      ///< Bottom-right pane.
};

/** Extended settings for a sheet, used in import/export filters. */
struct ScExtTabSettings
{
    ScRange             maUsedArea;         ///< Used area in the sheet (columns/rows only).
    ScRangeList         maSelection;        ///< Selected cell ranges (columns/rows only).
    ScAddress           maCursor;           ///< The cursor position (column/row only).
    ScAddress           maFirstVis;         ///< Top-left visible cell (column/row only).
    ScAddress           maSecondVis;        ///< Top-left visible cell in add. panes (column/row only).
    ScAddress           maFreezePos;        ///< Position of frozen panes (column/row only).
    Point               maSplitPos;         ///< Position of split.
    ScExtPanePos        meActivePane;       ///< Active (focused) pane.
    Color               maGridColor;        ///< Grid color.
    tools::Long                mnNormalZoom;       ///< Zoom in percent for normal view.
    tools::Long                mnPageZoom;         ///< Zoom in percent for pagebreak preview.
    std::optional<sal_uInt16> moExportZoom; ///< Zoom in percent - use when NormalZoom isn't accurate
    bool                mbSelected;         ///< true = Sheet is selected.
    bool                mbFrozenPanes;      ///< true = Frozen panes; false = Normal splits.
    bool                mbPageMode;         ///< true = Pagebreak mode; false = Normal view mode.
    bool                mbShowGrid;         ///< Whether or not to display gridlines.

    /** OOXML xl/printerSettings/printerSettingsN.bin raw DEVMODE blob.
        Captured at xlsx import (pagesettings.cxx), re-emitted unchanged
        at xlsx export (xepage.cxx) so customer print configuration
        survives a round-trip through LO. Empty when source had no
        printerSettings relation on the worksheet. */
    std::vector<sal_uInt8> maOoxPrinterSettingsBin;

    /** OOXML worksheet-level `<sortState>` passthrough. Captures the
        `ref` attribute on `<sortState>` and the `ref` of each child
        `<sortCondition>` so Excel's "Redo last sort" affordance still
        has the prior sort to redo after a LO round-trip. Empty
        `maOoxSortStateRef` means the source had no worksheet-level
        sortState. */
    OUString               maOoxSortStateRef;
    std::vector<OUString>  maOoxSortConditionRefs;

    /** OOXML `<col width="..."/>` raw-string preservation, keyed by 0-based
        SCCOL. LO stores column widths in twips (sal_uInt16) which loses
        the decimal precision Excel emits (e.g. 26.28515625). Round-trip
        replays the source string verbatim when present, falls back to
        LO's computed value for cols not in the map. */
    std::map<SCCOL, OUString> maOoxColWidthStrings;

    explicit            ScExtTabSettings();
};

struct ScExtDocOptionsImpl;

/** Extended options held by an ScDocument containing additional settings for filters.

    This object is owned by a Calc document. It contains global document settings
    (struct ScExtDocSettings), settings for all sheets in the document
    (struct ScExtTabSettings), and a list of codenames used for VBA import/export.
 */
class SC_DLLPUBLIC ScExtDocOptions
{
public:
    explicit            ScExtDocOptions();
                        ScExtDocOptions( const ScExtDocOptions& rSrc );
                        ~ScExtDocOptions();

    ScExtDocOptions&    operator=( const ScExtDocOptions& rSrc );

    /** @return true, if the data needs to be copied to the view data after import. */
    bool                IsChanged() const;
    /** If set to true, the data will be copied to the view data after import. */
    void                SetChanged( bool bChanged );

    /** @return read access to the global document settings. */
    const ScExtDocSettings& GetDocSettings() const;
    /** @return read/write access to the global document settings. */
    ScExtDocSettings&   GetDocSettings();

    /** @return read access to the settings of a sheet, if extant; otherwise 0. */
    const ScExtTabSettings* GetTabSettings( SCTAB nTab ) const;

    /**
     * @return index of the last sheet that has settings, or -1 if no tab
     *         settings are present.
     */
    SCTAB GetLastTab() const;

    /** @return read/write access to the settings of a sheet, may create a new struct. */
    ScExtTabSettings&   GetOrCreateTabSettings( SCTAB nTab );

    /** @return the number of sheet codenames. */
    SCTAB               GetCodeNameCount() const;
    /** @return the specified codename (empty string = no codename). */
    OUString     GetCodeName( SCTAB nTab ) const;
    /** Appends a codename for a sheet. */
    void                SetCodeName( SCTAB nTab, const OUString& rCodeName );

private:
    ::std::unique_ptr< ScExtDocOptionsImpl > mxImpl;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
