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
#include "GPU_state.h"

#include "UI_interface.h"
#include "UI_interface_icons.h"

#include "interface_intern.h"

#ifdef __cplusplus
}
#endif

#include "bwPainter.h"
#include "bwPoint.h"
#include "bwPolygon.h"

#include "interface_bwidgets_intern.h"

using namespace bWidgets;

void GawainPaintEngine::setupViewport(const bwRectanglePixel &, const bwColor &)
{
  /* TODO */
}

void GawainPaintEngine::enableMask(const bWidgets::bwRectanglePixel &rect)
{
  /* TODO */
}

#define WIDGET_AA_JITTER 8

static const float jit[WIDGET_AA_JITTER][2] = {{0.468813f, -0.481430f},
                                               {-0.155755f, -0.352820f},
                                               {0.219306f, -0.238501f},
                                               {-0.393286f, -0.110949f},
                                               {-0.024699f, 0.013908f},
                                               {0.343805f, 0.147431f},
                                               {-0.272855f, 0.269918f},
                                               {0.095909f, 0.388710f}};

static GPUPrimType paint_engine_polygon_drawtype_convert(const bwPainter::DrawType &drawtype)
{
  switch (drawtype) {
    case bwPainter::DrawType::FILLED:
      return GPU_PRIM_TRI_FAN;
    case bwPainter::DrawType::OUTLINE:
      return GPU_PRIM_TRI_STRIP;
    case bwPainter::DrawType::LINE:
      return GPU_PRIM_LINE_STRIP;
  }

  return GPU_PRIM_NONE;
}

static void paint_engine_polygon_draw_geometry_uniform_color(const bwPolygon &polygon,
                                                             const bwColor &color,
                                                             const GPUPrimType prim_type,
                                                             const unsigned int attr_pos)
{
  const bwPointVec &vertices = polygon.getVertices();

  immUniformColor4fv(color);

  immBegin(prim_type, vertices.size());
  for (const bwPoint &vertex : vertices) {
    immVertex2f(attr_pos, vertex.x, vertex.y);
  }
  immEnd();
}
static void paint_engine_polygon_draw_geometry_shaded(const bwPainter &painter,
                                                      const bwPolygon &polygon,
                                                      const GPUPrimType prim_type,
                                                      const unsigned int attr_pos,
                                                      const unsigned int attr_color)
{
  const bwPointVec &vertices = polygon.getVertices();
  size_t vert_count = vertices.size();

  immBegin(prim_type, vert_count);
  for (int i = 0; i < vert_count; i++) {
    immAttr4fv(attr_color, painter.getVertexColor(i));
    immVertex2f(attr_pos, vertices[i].x, vertices[i].y);
  }
  immEnd();
}
static void paint_engine_polygon_draw_geometry(const bwPainter &painter,
                                               const bwPolygon &polygon,
                                               const bwColor &color,
                                               const GPUPrimType type,
                                               const unsigned int attr_pos,
                                               const unsigned int attr_color)
{
  if (painter.isGradientEnabled()) {
    paint_engine_polygon_draw_geometry_shaded(painter, polygon, type, attr_pos, attr_color);
  }
  else {
    paint_engine_polygon_draw_geometry_uniform_color(polygon, color, type, attr_pos);
  }
}
void GawainPaintEngine::drawPolygon(const bwPainter &painter, const bwPolygon &polygon)
{
  const bool is_shaded = painter.isGradientEnabled();
  const bwColor &color = painter.getActiveColor();
  const GPUPrimType prim_type = paint_engine_polygon_drawtype_convert(painter.active_drawtype);
  GPUVertFormat *format = immVertexFormat();
  unsigned int attr_pos = GPU_vertformat_attr_add(format, "pos", GPU_COMP_F32, 2, GPU_FETCH_FLOAT);
  unsigned int attr_color = is_shaded ? GPU_vertformat_attr_add(
                                            format, "color", GPU_COMP_F32, 4, GPU_FETCH_FLOAT) :
                                        0;

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  immBindBuiltinProgram(is_shaded ? GPU_SHADER_2D_SMOOTH_COLOR : GPU_SHADER_2D_UNIFORM_COLOR);

  if (painter.active_drawtype == bwPainter::DrawType::OUTLINE) {
    bwColor drawcolor = color;

    drawcolor[3] /= WIDGET_AA_JITTER;

    for (int i = 0; i < WIDGET_AA_JITTER; i++) {
      GPU_matrix_translate_2fv(jit[i]);
      paint_engine_polygon_draw_geometry(
          painter, polygon, drawcolor, prim_type, attr_pos, attr_color);
      GPU_matrix_translate_2f(-jit[i][0], -jit[i][1]);
    }
  }
  else {
    paint_engine_polygon_draw_geometry(painter, polygon, color, prim_type, attr_pos, attr_color);
  }

  immUnbindProgram();
  glDisable(GL_BLEND);
}

static void convert_bwColor_to_uchar(const bwColor &bw_col, uchar *uchar_col)
{
  for (int i = 0; i < 4; i++) {
    uchar_col[i] = static_cast<unsigned char>(bw_col[i] * 255);
  }
}

static eFontStyle_Align convert_font_bw_alignment_to_bfstyle(bWidgets::TextAlignment bw_align)
{
  switch (bw_align) {
    case TextAlignment::LEFT:
      return UI_STYLE_TEXT_LEFT;
    case TextAlignment::CENTER:
      return UI_STYLE_TEXT_CENTER;
    case TextAlignment::RIGHT:
      return UI_STYLE_TEXT_RIGHT;
  }
}

void GawainPaintEngine::drawText(const bwPainter &painter,
                                 const std::string &text,
                                 const bwRectanglePixel &rectangle,
                                 const TextAlignment alignment)
{
  const uiStyle *style = UI_style_get_dpi();
  uchar col[4];
  rcti b_rect = {.xmin = (int)rectangle.xmin,
                 .xmax = (int)rectangle.xmax,
                 .ymin = (int)rectangle.ymin,
                 .ymax = (int)rectangle.ymax};

  b_rect.xmin += UI_TEXT_MARGIN_X * U.widget_unit;

  convert_bwColor_to_uchar(painter.getActiveColor(), col);

  eFontStyle_Align b_align = convert_font_bw_alignment_to_bfstyle(alignment);
  struct uiFontStyleDraw_Params font_style = {b_align, 0};
  UI_fontstyle_draw(&style->widget, &b_rect, text.c_str(), col, &font_style);
}

void GawainPaintEngine::drawIcon(const bwPainter &painter,
                                 const bwIconInterface &iicon,
                                 const bwRectanglePixel &rect)
{
  const Icon &icon = static_cast<const Icon &>(iicon);
  uchar col[4];

  convert_bwColor_to_uchar(painter.getActiveColor(), col);

  GPU_blend(true);
  UI_icon_draw_ex(
      rect.xmin, rect.ymin, static_cast<int>(icon.iconid), icon.aspect, 1.0f, 0.0f, col, false);
  GPU_blend(false);
}
