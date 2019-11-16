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

#include <iostream>

#include <GL/eglew.h>

#include <wayland-client.h>
#include <wayland-egl.h>

#include "GHOST_ContextEGL.h"
#include "GHOST_Debug.h"

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
    : GHOST_Window(width, height, state, stereoVisual, exclusive),
      m_display(display),
      m_valid_setup(false),
      m_debug_context(is_debug)
{
  /* XXX bad size is passed here, breaking wayland calls. Need to update system desktop size
   * getters first. */
  width = 420;
  height = 700;

  m_surface = wl_compositor_create_surface(compositor);
  m_shell_surface = wl_shell_get_shell_surface(shell, m_surface);
  wl_shell_surface_set_toplevel(m_shell_surface);

  m_egl_window = wl_egl_window_create(m_surface, width, height);

  /* now set up the rendering context. */
  if (setDrawingContextType(type) == GHOST_kSuccess) {
    m_valid_setup = true;
    GHOST_PRINT("Created window\n");
  }
}

GHOST_WindowWayland::~GHOST_WindowWayland()
{
  wl_egl_window_destroy(m_egl_window);
  wl_shell_surface_destroy(m_shell_surface);
  wl_surface_destroy(m_surface);
}

GHOST_Context *GHOST_WindowWayland::newDrawingContext(GHOST_TDrawingContextType type)
{
  if (type == GHOST_kDrawingContextTypeOpenGL) {
    GHOST_ContextEGL *context;

#if defined(WITH_GL_PROFILE_CORE)
    for (int minor = 5; minor >= 0; --minor) {
      context = new GHOST_ContextEGL(m_wantStereoVisual,
                                     m_egl_window,
                                     m_display,
                                     EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR,
                                     4,
                                     minor,
                                     (m_debug_context ? EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR : 0),
                                     0,
                                     EGL_OPENGL_API);

      if (context->initializeDrawingContext()) {
        return context;
      }
      else {
        delete context;
      }
    }
    context = new GHOST_ContextEGL(m_wantStereoVisual,
                                   m_egl_window,
                                   m_display,
                                   EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR,
                                   3,
                                   3,
                                   (m_debug_context ? EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR : 0),
                                   0,
                                   EGL_OPENGL_API);

    if (context->initializeDrawingContext()) {
      return context;
    }
    else {
      /* TODO proper error dialog? */
      std::cout
          << "Blender - Unsupported Graphics Card or Driver\n\n"
          << "A graphics card and driver with support for OpenGL 3.3 or higher is required.\n"
             "Installing the latest driver for your graphics card may resolve the issue.\n\n"
             "The program will now close."
          << std::endl;
      delete context;
      exit(0);
    }
#else
    /* Only core profile supported for Wayland backend, could add compatible profile support but
     * it's not used currently anyway. */
#  error
#endif
  }

  return NULL;
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
  return GHOST_Window::getValid() && m_valid_setup;
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
