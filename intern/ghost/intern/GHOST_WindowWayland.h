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

#ifndef __GHOST_WINDOWWAYLAND_H__
#define __GHOST_WINDOWWAYLAND_H__

#include "GHOST_Window.h"

class GHOST_WindowWayland : public GHOST_Window {
 public:
  GHOST_WindowWayland(struct wl_display *display,
                      struct wl_compositor *compositor,
                      struct wl_shell *shell,
                      const STR_String &title,
                      GHOST_TInt32 left,
                      GHOST_TInt32 top,
                      GHOST_TUns32 width,
                      GHOST_TUns32 height,
                      GHOST_TWindowState state,
                      GHOST_WindowWayland *parentWindow,
                      GHOST_TDrawingContextType type = GHOST_kDrawingContextTypeNone,
                      const bool is_dialog = false,
                      const bool stereoVisual = false,
                      const bool exclusive = false,
                      const bool alphaBackground = false,
                      const bool is_debug = false);
  ~GHOST_WindowWayland();

  bool getValid() const;

  void setTitle(const STR_String &title);
  void getTitle(STR_String &title) const;
  void getWindowBounds(GHOST_Rect &bounds) const;
  void getClientBounds(GHOST_Rect &bounds) const;
  GHOST_TSuccess setClientWidth(GHOST_TUns32 width);
  GHOST_TSuccess setClientHeight(GHOST_TUns32 height);
  GHOST_TSuccess setClientSize(GHOST_TUns32 width, GHOST_TUns32 height);
  void screenToClient(GHOST_TInt32 inX,
                      GHOST_TInt32 inY,
                      GHOST_TInt32 &outX,
                      GHOST_TInt32 &outY) const;
  void clientToScreen(GHOST_TInt32 inX,
                      GHOST_TInt32 inY,
                      GHOST_TInt32 &outX,
                      GHOST_TInt32 &outY) const;
  GHOST_TWindowState getState() const;
  GHOST_TSuccess setState(GHOST_TWindowState state);
  GHOST_TSuccess setOrder(GHOST_TWindowOrder order);
  GHOST_TSuccess invalidate();
  const GHOST_TabletData *GetTabletData();
  GHOST_TSuccess hasCursorShape(GHOST_TStandardCursor cursorShape);
  GHOST_TSuccess beginFullScreen() const;
  GHOST_TSuccess endFullScreen() const;

 protected:
  GHOST_Context *newDrawingContext(GHOST_TDrawingContextType type);
  GHOST_TSuccess setWindowCursorVisibility(bool visible);
  GHOST_TSuccess setWindowCursorShape(GHOST_TStandardCursor shape);
  GHOST_TSuccess setWindowCustomCursorShape(GHOST_TUns8 *bitmap,
                                            GHOST_TUns8 *mask,
                                            int szx,
                                            int szy,
                                            int hotX,
                                            int hotY,
                                            bool canInvertColor);

 private:
  struct wl_display *m_display; /* Non-owning! */
  struct wl_surface *m_surface;
  struct wl_shell_surface *m_shell_surface;
  struct wl_egl_window *m_egl_window;

  bool m_valid_setup;
  bool m_debug_context;
};

#endif  // __GHOST_WINDOWWAYLAND_H__
