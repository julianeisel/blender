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

/** \file blender/editors/interface/interface_bwidgets.c
 *  \ingroup edinterface
 */

#include "bwLabel.h"
#include "bwPainter.h"
#include "bwPolygon.h"
#include "bwPushButton.h"
#include "bwStyle.h"
#include "bwStyleManager.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "BLI_rect.h"
#include "BLI_utildefines.h"

#include "DNA_userdef_types.h"

#include "UI_interface.h"

#include "interface_intern.h"

#ifdef __cplusplus
}
#endif

#include "interface_bwidgets_intern.h"

using namespace bWidgets;

static std::unique_ptr<bwStyle> G_style = nullptr;

void ui_widgets_init(void)
{
  bwStyleManager &style_manager = bwStyleManager::getStyleManager();

  style_manager.registerDefaultStyleTypes();
  G_style = std::make_unique<BlenderThemeStyle>();

  bwPainter::s_paint_engine = std::make_unique<GawainPaintEngine>();
}

static unsigned int ui_widget_convert_roundbox(int blender_roundbox)
{
  unsigned int bwidgets_roundbox = 0;

  if (blender_roundbox & UI_CNR_TOP_LEFT) {
    bwidgets_roundbox |= TOP_LEFT;
  }
  if (blender_roundbox & UI_CNR_TOP_RIGHT) {
    bwidgets_roundbox |= TOP_RIGHT;
  }
  if (blender_roundbox & UI_CNR_BOTTOM_RIGHT) {
    bwidgets_roundbox |= BOTTOM_RIGHT;
  }
  if (blender_roundbox & UI_CNR_BOTTOM_LEFT) {
    bwidgets_roundbox |= BOTTOM_LEFT;
  }

  return bwidgets_roundbox;
}

static bwWidget *ui_widget_create_from_button_type(const eButType button_type, const char *text)
{
  switch (button_type) {
    case UI_BTYPE_LABEL:
      return new bwLabel(text);
    case UI_BTYPE_BUT:
      return new bwPushButton(text);
    default:
      BLI_assert(0);
      return nullptr;
  }
}

static bwWidget::State ui_widget_convert_state(const uiBut &blender_button)
{
  if (blender_button.flag & UI_SELECT) {
    return bwWidget::State::SUNKEN;
  }
  else if (blender_button.flag & UI_ACTIVE) {
    return bwWidget::State::HIGHLIGHTED;
  }

  return bwWidget::State::NORMAL;
}

static std::unique_ptr<bwWidget> ui_widget_convert_label_button(const uiBut *but,
                                                                const rcti *rect,
                                                                const Icon &icon)
{
  bwLabel *label = static_cast<bwLabel *>(
      ui_widget_create_from_button_type(but->type, but->drawstr));

  label->rectangle.set(rect->xmin, BLI_rcti_size_x(rect), rect->ymin, BLI_rcti_size_y(rect));
  label->setIcon(icon);

  return std::unique_ptr<bwWidget>(label);
}
static std::unique_ptr<bwWidget> ui_widget_convert_push_button(const uiBut *but,
                                                               const rcti *rect,
                                                               const Icon &icon,
                                                               const int roundboxalign)
{
  auto button = static_cast<bwPushButton *>(
      ui_widget_create_from_button_type(but->type, but->drawstr));

  button->rectangle.set(rect->xmin, BLI_rcti_size_x(rect), rect->ymin, BLI_rcti_size_y(rect));
  button->setIcon(icon);
  button->rounded_corners = ui_widget_convert_roundbox(roundboxalign);
  button->state = ui_widget_convert_state(*but);

  /* From widget_draw_text(). */
  TextAlignment align;
  if (but->editstr || (but->drawflag & UI_BUT_TEXT_LEFT)) {
    align = TextAlignment::LEFT;
  }
  else if (but->drawflag & UI_BUT_TEXT_RIGHT) {
    align = TextAlignment::RIGHT;
  }
  else {
    align = TextAlignment::CENTER;
  }
  button->base_style.text_alignment = align;

  return std::unique_ptr<bwWidget>(button);
}

void ui_widget_draw(const uiBut *but, const rcti *rect, const int roundboxalign)
{
  std::unique_ptr<bwWidget> widget;
  const float aspect = but->block->aspect / UI_DPI_FAC;
  Icon icon(ui_but_widget_icon_id(but),
            aspect);  // Widget will reference this, keep alive until widget is destroyed.

  switch (but->type) {
    case UI_BTYPE_LABEL:
      widget = ui_widget_convert_label_button(but, rect, icon);
      break;
    case UI_BTYPE_BUT:
      widget = ui_widget_convert_push_button(but, rect, icon, roundboxalign);
      break;
    default:
      break;
  }

  auto &style = static_cast<BlenderThemeStyle &>(*G_style);
  style.setupGlobals();  // Only needs to be called once before drawing any widget, but leaving
                         // here for now.

  if (widget) {
    style.setWidgetStyle(*widget);
    widget->draw(style);
  }
}
