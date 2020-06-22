/*
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * ***** END GPL LICENSE BLOCK *****
 */

/** \file blender/editors/interface/interface_bwidgets_style.c
 *  \ingroup edinterface
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "BLI_utildefines.h"

#include "DNA_userdef_types.h"
#include "DNA_vec_types.h"

#include "UI_interface.h"

#include "interface_intern.h"

#ifdef __cplusplus
}
#endif

#include "bwAbstractButton.h"
#include "bwCheckbox.h"
#include "bwLabel.h"
#include "bwNumberSlider.h"
#include "bwPainter.h"
#include "bwPanel.h"
#include "bwPushButton.h"
#include "bwRadioButton.h"
#include "bwScrollBar.h"
#include "bwTextBox.h"

#include "interface_bwidgets_intern.h"

using namespace bWidgets;

BlenderThemeStyle::BlenderThemeStyle()
    : /* XXX custom (non-builtin) styles not supported by bWidgets yet (in
       * principle, in practice it works ;) ). Just using STYLE_BUILTIN_TOT for
       * now. Style management should be reworked anyway */
      bwStyle(bwStyle::TypeID::BUILTIN_TOT)
{
}

void BlenderThemeStyle::setupGlobals()
{
  dpi_fac = U.dpi_fac;
}

static bwColor convert_uchar_to_bwColor(const uchar col[4])
{
  return bwColor(uint(col[0]), uint(col[1]), uint(col[2]), uint(col[3]));
}

static eFontStyle_Align convert_font_bw_alignment_to_bfstyle(bWidgets::TextAlignment bw_align)
{
  eFontStyle_Align b_align;
  switch (bw_align) {
    case TextAlignment::LEFT:
      b_align = UI_STYLE_TEXT_LEFT;
      break;
    case TextAlignment::CENTER:
      b_align = UI_STYLE_TEXT_CENTER;
      break;
    case TextAlignment::RIGHT:
      b_align = UI_STYLE_TEXT_RIGHT;
      break;
  }
}

static void update_base_style_from_blender_widget_colors(const uiWidgetColors &wcol,
                                                         const bwWidget &widget,
                                                         const uint highlight_shade,
                                                         bwWidgetBaseStyle &base_style)
{
  base_style.background_color = convert_uchar_to_bwColor(wcol.inner);
  base_style.text_color = convert_uchar_to_bwColor(wcol.text);
  base_style.border_color = convert_uchar_to_bwColor(wcol.outline);
  base_style.decoration_color = convert_uchar_to_bwColor(wcol.item);
  base_style.shade_top = wcol.shaded ? wcol.shadetop : 0;
  base_style.shade_bottom = wcol.shaded ? wcol.shadedown : 0;
  base_style.corner_radius = wcol.roundness * U.widget_unit;

  if (widget.state == bwWidget::State::HIGHLIGHTED) {
    base_style.background_color.shade(highlight_shade);
  }
  else if (widget.state == bwWidget::State::SUNKEN) {
    base_style.background_color = convert_uchar_to_bwColor(wcol.inner_sel);
    base_style.text_color = convert_uchar_to_bwColor(wcol.text_sel);
    std::swap(base_style.shade_top, base_style.shade_bottom);
  }
}

static void update_panel_base_style_from_blender_theme(bwWidgetBaseStyle &base_style)
{
  float tmp_col[4];

  // NOTE This is untested
  // TODO text color from TH_TITLE etc.

  UI_GetThemeColor4fv(TH_PANEL_BACK, tmp_col);
  base_style.background_color = tmp_col;
  base_style.border_color =
      114u;  // TODO Fixed for now, panel subdivider got removed from Blender.
}

static void widget_base_style_set(const bwWidget &widget, bwWidgetBaseStyle &base_style)
{
  bTheme *btheme = UI_GetTheme();
  uiWidgetColors *wcol = nullptr;
  uint highlight_shade = 15;

  // NOTE most of those are untested.
  if (widget_cast<bwCheckbox>(widget)) {
    wcol = &btheme->tui.wcol_option;
  }
  else if (widget_cast<bwNumberSlider>(widget)) {
    wcol = &btheme->tui.wcol_numslider;
  }
  else if (widget_cast<bwPushButton>(widget)) {
    wcol = &btheme->tui.wcol_tool;
  }
  else if (widget_cast<bwRadioButton>(widget)) {
    wcol = &btheme->tui.wcol_radio;
  }
  else if (widget_cast<bwScrollBar>(widget)) {
    wcol = &btheme->tui.wcol_scroll;
  }
  else if (widget_cast<bwTextBox>(widget)) {
    wcol = &btheme->tui.wcol_text;
  }
  else if (widget_cast<bwPanel>(widget)) {
    update_panel_base_style_from_blender_theme(base_style);
  }

  if (wcol) {
    update_base_style_from_blender_widget_colors(*wcol, widget, highlight_shade, base_style);
  }
}

static void widget_style_properties_set_to_default(bwWidget &widget)
{
  for (auto &property : widget.style_properties) {
    property->setValueToDefault();
  }
}

void BlenderThemeStyle::setWidgetStyle(bwWidget &widget)
{
  bwWidgetBaseStyle *base_style = nullptr;

  polish(widget);
  widget_style_properties_set_to_default(widget);

  if (auto *button = widget_cast<bwAbstractButton *>(&widget)) {
    button->base_style.roundbox_corners = button->rounded_corners;
    base_style = &button->base_style;
  }
  else if (auto *panel = widget_cast<bwPanel *>(&widget)) {
    base_style = &panel->base_style;
  }
  else if (auto *text_box = widget_cast<bwTextBox *>(&widget)) {
    text_box->base_style.roundbox_corners =
        RoundboxCorner::ALL;  // XXX Incorrect, should set this in layout.
    base_style = &text_box->base_style;
  }
  else if (auto *label = widget_cast<bwLabel *>(&widget)) {
    if (auto property_wrap = label->style_properties.lookup("color")) {
      bTheme *btheme = UI_GetTheme();
      // FIXME eek, const_cast...
      bwStyleProperty &prop = const_cast<bwStyleProperty &>(*property_wrap);

      prop.setValue(convert_uchar_to_bwColor(btheme->tui.wcol_regular.text));
    }
  }

  if (base_style) {
    widget_base_style_set(widget, *base_style);
  }
}

void BlenderThemeStyle::polish(bwWidget & /*widget*/)
{
}
