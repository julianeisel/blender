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

/** \file blender/editors/interface/interface_bwidgets_intern.c
 *  \ingroup edinterface
 */

#ifndef __INTERFACE_BWIDGETS_INTERN_H__
#define __INTERFACE_BWIDGETS_INTERN_H__

#include "bwIconInterface.h"
#include "bwPaintEngine.h"
#include "bwStyle.h"

/**
 * \brief A bWidgets style for Blender themes.
 */
class BlenderThemeStyle : public bWidgets::bwStyle {
 public:
  BlenderThemeStyle();

  void setupGlobals();
  void setWidgetStyle(bWidgets::bwWidget &) override;
  void polish(bWidgets::bwWidget &) override;
};

/**
 * \brief A paint-engine implementation based on the Gawain graphics library.
 */
class GawainPaintEngine : public bWidgets::bwPaintEngine {
 public:
  void setupViewport(const bWidgets::bwRectanglePixel &rect,
                     const bWidgets::bwColor &clear_color) override;
  void enableMask(const bWidgets::bwRectanglePixel &rect) override;
  void drawPolygon(const bWidgets::bwPainter &, const bWidgets::bwPolygon &) override;
  void drawText(const bWidgets::bwPainter &painter,
                const std::string &text,
                const bWidgets::bwRectanglePixel &rect,
                const bWidgets::TextAlignment alignment) override;
  void drawIcon(const bWidgets::bwPainter &,
                const bWidgets::bwIconInterface &,
                const bWidgets::bwRectanglePixel &) override;
};

class Icon : public bWidgets::bwIconInterface {
 public:
  Icon(BIFIconID icon_id, float aspect) : iconid(icon_id), aspect(aspect)
  {
  }

  bool isValid() const override
  {
    return iconid != ICON_NONE;
  }
  BIFIconID iconid;
  float aspect;
};

#endif /* __INTERFACE_BWIDGETS_INTERN_H__ */
