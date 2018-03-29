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

/** \file blender/editors/interface/interface_bwidgets_draw.c
 *  \ingroup edinterface
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "BLI_utildefines.h"

#include "DNA_userdef_types.h"
#include "DNA_vec_types.h"

#include "GPU_immediate.h"
#include "GPU_matrix.h"

#include "UI_interface.h"

#include "interface_intern.h"

#ifdef __cplusplus
}
#endif

#include "bwPainter.h"
#include "bwPolygon.h"

#include "interface_bwidgets_intern.h"

using namespace bWidgets;


void GawainPaintEngine::setupViewport(
        const bwRectanglePixel &,
        const bwColor &)
{
	
}


#define WIDGET_AA_JITTER 8

static const float jit[WIDGET_AA_JITTER][2] = {
	{ 0.468813, -0.481430}, {-0.155755, -0.352820},
	{ 0.219306, -0.238501}, {-0.393286, -0.110949},
	{-0.024699,  0.013908}, { 0.343805,  0.147431},
	{-0.272855,  0.269918}, { 0.095909,  0.388710}
};


static Gwn_PrimType paint_engine_polygon_drawtype_convert(
        const bwPainter::DrawType& drawtype)
{
	switch (drawtype) {
		case bwPainter::DRAW_TYPE_FILLED:
			return GWN_PRIM_TRI_FAN;
		case bwPainter::DRAW_TYPE_OUTLINE:
			return GWN_PRIM_TRI_STRIP;
		case bwPainter::DRAW_TYPE_LINE:
			return GWN_PRIM_LINE_STRIP;
	}

	return GWN_PRIM_NONE;
}

static void paint_engine_polygon_draw_geometry_uniform_color(
        const bwPolygon& polygon,
        const bwColor& color,
        const Gwn_PrimType prim_type,
        const unsigned int attr_pos)
{
	const bwPointVec& vertices = polygon.getVertices();

	immUniformColor4fv(color);

	immBegin(prim_type, vertices.size());
	for (const bwPoint& vertex : vertices) {
		immVertex2f(attr_pos, vertex.x, vertex.y);
	}
	immEnd();
}
static void paint_engine_polygon_draw_geometry_shaded(
        const bwPainter& painter,
        const bwPolygon& polygon,
        const Gwn_PrimType prim_type,
        const unsigned int attr_pos, const unsigned int attr_color)
{
	const bwPointVec& vertices = polygon.getVertices();
	size_t vert_count = vertices.size();

	immBegin(prim_type, vert_count);
	for (int i = 0; i < vert_count; i++) {
		immAttrib4fv(attr_color, painter.getVertexColor(i));
		immVertex2f(attr_pos, vertices[i].x, vertices[i].y);
	}
	immEnd();
}
static void paint_engine_polygon_draw_geometry(
        const bwPainter& painter,
        const bwPolygon& polygon,
        const bwColor& color,
        const Gwn_PrimType type,
        const unsigned int attr_pos, const unsigned int attr_color)
{
	if (painter.isGradientEnabled()) {
		paint_engine_polygon_draw_geometry_shaded(painter, polygon, type, attr_pos, attr_color);
	}
	else {
		paint_engine_polygon_draw_geometry_uniform_color(polygon, color, type , attr_pos);
	}
}
void GawainPaintEngine::drawPolygon(
        const bwPainter& painter,
        const bwPolygon& polygon)
{
	const bool is_shaded = painter.isGradientEnabled();
	const bwColor& color = painter.getActiveColor();
	const Gwn_PrimType prim_type = paint_engine_polygon_drawtype_convert(painter.active_drawtype);
	Gwn_VertFormat *format = immVertexFormat();
	unsigned int attr_pos = GWN_vertformat_attr_add(format, "pos", GWN_COMP_F32, 2, GWN_FETCH_FLOAT);
	unsigned int attr_color = is_shaded ? GWN_vertformat_attr_add(
	                                          format, "color", GWN_COMP_F32, 4, GWN_FETCH_FLOAT) : 0;

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	immBindBuiltinProgram(is_shaded ? GPU_SHADER_2D_SMOOTH_COLOR : GPU_SHADER_2D_UNIFORM_COLOR);

	if (painter.active_drawtype == bwPainter::DRAW_TYPE_OUTLINE) {
		bwColor drawcolor = color;

		drawcolor[3] /= WIDGET_AA_JITTER;

		for (int i = 0; i < WIDGET_AA_JITTER; i++) {
			gpuTranslate2fv(jit[i]);
			paint_engine_polygon_draw_geometry(painter, polygon, drawcolor, prim_type, attr_pos, attr_color);
			gpuTranslate2f(-jit[i][0], -jit[i][1]);
		}
	}
	else {
		paint_engine_polygon_draw_geometry(painter, polygon, color, prim_type, attr_pos, attr_color);
	}

	immUnbindProgram();
	glDisable(GL_BLEND);
}

void GawainPaintEngine::drawText(
        const bwPainter& painter,
        const std::string& text,
        const bwRectanglePixel& rectangle,
        const TextAlignment /*alignment*/)
{
	uiStyle *style = UI_style_get_dpi();
	const bwColor &bw_col = painter.getActiveColor();
	unsigned char col[] = {
		(unsigned char)(bw_col[0] * 255),
		(unsigned char)(bw_col[1] * 255),
		(unsigned char)(bw_col[2] * 255),
		(unsigned char)(bw_col[3] * 255)
	};
	rcti b_rect = {
		.xmin = (int)rectangle.xmin,
		.xmax = (int)rectangle.xmax,
		.ymin = (int)rectangle.ymin,
		.ymax = (int)rectangle.ymax
	};

	b_rect.xmin += UI_TEXT_MARGIN_X * U.widget_unit;

	UI_fontstyle_draw(&style->widget, &b_rect, text.c_str(), col);
}
