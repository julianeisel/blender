/*
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
 */

/** \file
 * \ingroup GHOST
 */

#include <wayland-client.h>

#include "GHOST_WindowWayland.h"

#pragma GCC diagnostic ignored "-Wunused-parameter"

GHOST_WindowWayland::GHOST_WindowWayland(wl_display *display,
                                         wl_compositor *compositor,
                                         wl_shell *shell,
                                         const STR_String &title,
                                         GHOST_TInt32 left,
                                         GHOST_TInt32 top,
                                         GHOST_TUns32 width,
                                         GHOST_TUns32 height,
                                         GHOST_TWindowState state,
                                         GHOST_WindowWayland *parentWindow,
                                         GHOST_TDrawingContextType type,
                                         const bool is_dialog,
                                         const bool stereoVisual,
                                         const bool exclusive,
                                         const bool alphaBackground,
                                         const bool is_debug)
    : GHOST_Window(width, height, state, stereoVisual, exclusive)
{
  m_surface = wl_compositor_create_surface(compositor);
  m_shell_surface = wl_shell_get_shell_surface(shell, m_surface);
}

GHOST_WindowWayland::~GHOST_WindowWayland()
{
  wl_shell_surface_destroy(m_shell_surface);
  wl_surface_destroy(m_surface);
}

GHOST_TSuccess GHOST_WindowWayland::setWindowCursorShape(GHOST_TStandardCursor shape)
{
  return GHOST_kSuccess;
}

GHOST_TSuccess GHOST_WindowWayland::setWindowCustomCursorShape(GHOST_TUns8 *bitmap,
                                                               GHOST_TUns8 *mask,
                                                               int sizex,
                                                               int sizey,
                                                               int hotX,
                                                               int hotY,
                                                               bool canInvertColor)
{
  return GHOST_kSuccess;
}

bool GHOST_WindowWayland::getValid() const
{
  return true;
}

void GHOST_WindowWayland::setTitle(const STR_String &title)
{
}

void GHOST_WindowWayland::getTitle(STR_String &title) const
{
  title = "untitled";
}

void GHOST_WindowWayland::getWindowBounds(GHOST_Rect &bounds) const
{
  getClientBounds(bounds);
}

void GHOST_WindowWayland::getClientBounds(GHOST_Rect &bounds) const
{
}

GHOST_TSuccess GHOST_WindowWayland::setClientWidth(GHOST_TUns32 width)
{
  return GHOST_kFailure;
}

GHOST_TSuccess GHOST_WindowWayland::setClientHeight(GHOST_TUns32 height)
{
  return GHOST_kFailure;
}

GHOST_TSuccess GHOST_WindowWayland::setClientSize(GHOST_TUns32 width, GHOST_TUns32 height)
{
  return GHOST_kFailure;
}

void GHOST_WindowWayland::screenToClient(GHOST_TInt32 inX,
                                         GHOST_TInt32 inY,
                                         GHOST_TInt32 &outX,
                                         GHOST_TInt32 &outY) const
{
  outX = inX;
  outY = inY;
}
void GHOST_WindowWayland::clientToScreen(GHOST_TInt32 inX,
                                         GHOST_TInt32 inY,
                                         GHOST_TInt32 &outX,
                                         GHOST_TInt32 &outY) const
{
  outX = inX;
  outY = inY;
}

GHOST_TSuccess GHOST_WindowWayland::setWindowCursorVisibility(bool visible)
{
  return GHOST_kSuccess;
}

GHOST_TSuccess GHOST_WindowWayland::setState(GHOST_TWindowState state)
{
  return GHOST_kSuccess;
}
GHOST_TWindowState GHOST_WindowWayland::getState() const
{
  return GHOST_kWindowStateNormal;
}

GHOST_TSuccess GHOST_WindowWayland::invalidate()
{
  return GHOST_kSuccess;
}

const GHOST_TabletData *GHOST_WindowWayland::GetTabletData()
{
  return nullptr;
}

GHOST_TSuccess GHOST_WindowWayland::hasCursorShape(GHOST_TStandardCursor cursorShape)
{
  return GHOST_kSuccess;
}

GHOST_TSuccess GHOST_WindowWayland::setOrder(GHOST_TWindowOrder order)
{
  return GHOST_kSuccess;
}

GHOST_TSuccess GHOST_WindowWayland::beginFullScreen() const
{
  return GHOST_kSuccess;
}
GHOST_TSuccess GHOST_WindowWayland::endFullScreen() const
{
  return GHOST_kSuccess;
}

GHOST_Context *GHOST_WindowWayland::newDrawingContext(GHOST_TDrawingContextType type)
{
  return NULL;
}
