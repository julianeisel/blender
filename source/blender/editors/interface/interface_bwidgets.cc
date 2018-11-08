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
 * The Original Code is Copyright (C) 2001-2002 by NaN Holding BV.
 * All rights reserved.
 *
 * ***** END GPL LICENSE BLOCK *****
 */

/** \file blender/editors/interface/interface_bwidgets.c
 *  \ingroup edinterface
 */

#include "bwPainter.h"
#include "bwPolygon.h"
#include "bwPushButton.h"
#include "bwStyleManager.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "BLI_rect.h"
#include "BLI_utildefines.h"

#include "UI_interface.h"

#include "interface_intern.h"

#ifdef __cplusplus
}
#endif

#include "interface_bwidgets_intern.h"

using namespace bWidgets;


static bwPtr<bwStyle> G_style = nullptr;

void ui_widgets_init(void)
{
	bwStyleManager &style_manager = bwStyleManager::getStyleManager();

	style_manager.registerDefaultStyleTypes();
	G_style = bwPtr<bwStyle>(style_manager.createStyleFromTypeID(bwStyle::STYLE_CLASSIC));

	bwPainter::paint_engine = bwPtr_new<GawainPaintEngine>();
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

static bwAbstractButton *ui_widget_convert_button(
        const eButType button_type,
        const char *text)
{
	switch (button_type) {
		case UI_BTYPE_BUT:
			return new bwPushButton(text);
		default:
			BLI_assert(0);
			return nullptr;
	}
}

static bwWidget::WidgetState ui_widget_convert_state(
        const uiBut& blender_button)
{
	if (blender_button.flag & UI_SELECT) {
		return bwWidget::STATE_SUNKEN;
	}
	else if (blender_button.flag & UI_ACTIVE) {
		return bwWidget::STATE_HIGHLIGHTED;
	}

	return bwWidget::STATE_NORMAL;
}

static void ui_widget_draw_push_button(
        const uiBut *but,
        const rcti *rect,
        const int roundboxalign)
{
	bwAbstractButton *button = ui_widget_convert_button(but->type, but->drawstr);

	button->rectangle.set(rect->xmin, BLI_rcti_size_x(rect), rect->ymin, BLI_rcti_size_y(rect));
	button->rounded_corners = ui_widget_convert_roundbox(roundboxalign);
	button->state           = ui_widget_convert_state(*but);

	button->draw(*G_style);

	delete button;
}

void ui_widget_draw(
        const uiBut *but,
        const rcti *rect,
        const int roundboxalign)
{
	switch (but->type) {
		case UI_BTYPE_BUT:
			ui_widget_draw_push_button(but, rect, roundboxalign);
			break;
		default:
			break;
	}
}
