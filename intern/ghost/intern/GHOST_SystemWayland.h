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
 * Declaration of GHOST_SystemWayland class.
 */

#ifndef __GHOST_SYSTEMWAYLAND_H__
#define __GHOST_SYSTEMWAYLAND_H__

#include <stdint.h>

#include "GHOST_System.h"

class GHOST_SystemWayland : public GHOST_System {
  friend class GHOST_WaylandRegistryListener;

 public:
  GHOST_SystemWayland();
  ~GHOST_SystemWayland();

  GHOST_IWindow *createWindow(const STR_String &title,
                              GHOST_TInt32 left,
                              GHOST_TInt32 top,
                              GHOST_TUns32 width,
                              GHOST_TUns32 height,
                              GHOST_TWindowState state,
                              GHOST_TDrawingContextType type,
                              GHOST_GLSettings glSettings,
                              bool exclusive,
                              bool is_dialog,
                              const GHOST_IWindow *parentWindow);

  bool processEvents(bool);
  GHOST_TUns8 getNumDisplays() const;
  void getMainDisplayDimensions(GHOST_TUns32 &width, GHOST_TUns32 &height) const;
  void getAllDisplayDimensions(GHOST_TUns32 &width, GHOST_TUns32 &height) const;
  GHOST_IContext *createOffscreenContext();
  GHOST_TSuccess disposeContext(GHOST_IContext *context);
  GHOST_TSuccess getCursorPosition(GHOST_TInt32 &x, GHOST_TInt32 &y) const;
  GHOST_TSuccess setCursorPosition(GHOST_TInt32 x, GHOST_TInt32 y);
  int toggleConsole(int action);
  GHOST_TUns8 *getClipboard(bool selection) const;
  void putClipboard(GHOST_TInt8 *buffer, bool selection) const;
  GHOST_TSuccess getModifierKeys(GHOST_ModifierKeys &keys) const;
  GHOST_TSuccess getButtons(GHOST_Buttons &buttons) const;

 private:
  struct wl_display *m_display;
  struct wl_compositor *m_compositor;
  struct wl_shell *m_shell;
};

#endif  // __GHOST_SYSTEMWAYLAND_H__
