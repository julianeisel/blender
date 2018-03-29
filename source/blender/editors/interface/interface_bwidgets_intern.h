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

/** \file blender/editors/interface/interface_bwidgets_intern.c
 *  \ingroup edinterface
 */

#ifndef __INTERFACE_BWIDGETS_INTERN_H__
#define __INTERFACE_BWIDGETS_INTERN_H__

#include "bwPaintEngine.h"

/**
 * \brief A paint-engine implementation based on the Gawain graphics library.
 */
class GawainPaintEngine : public bWidgets::bwPaintEngine
{
public:
	void setupViewport(
	        const bWidgets::bwRectanglePixel& rect,
	        const bWidgets::bwColor& clear_color) override;
	void drawPolygon(
	        const bWidgets::bwPainter&,
	        const bWidgets::bwPolygon&) override;
	void drawText(
	        const bWidgets::bwPainter& painter,
	        const std::string& text,
	        const bWidgets::bwRectanglePixel& rect,
	        const bWidgets::TextAlignment alignment) override;
};

#endif /* __INTERFACE_BWIDGETS_INTERN_H__ */
