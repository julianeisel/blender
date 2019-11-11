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

#include <string>

#include <wayland-client.h>

#include "GHOST_WindowManager.h"
#include "GHOST_WindowWayland.h"

#include "GHOST_SystemWayland.h"

#pragma GCC diagnostic ignored "-Wunused-parameter"

class GHOST_WaylandRegistryListener {
 private:
  static void add_object(void *data,
                         struct wl_registry *registry,
                         uint32_t name,
                         const char *interface_char,
                         uint32_t version)
  {
    GHOST_SystemWayland *system = (GHOST_SystemWayland *)data;
    std::string interface_str{interface_char};

    if (interface_str == "wl_compositor") {
      system->m_compositor = (wl_compositor *)wl_registry_bind(
          registry, name, &wl_compositor_interface, version);
    }
    else if (interface_str == "wl_shell") {
      system->m_shell = (wl_shell *)wl_registry_bind(registry, name, &wl_shell_interface, version);
    }
  }

  static void remove_object(void *data, struct wl_registry *registry, uint32_t name)
  {
  }

 public:
  static wl_registry_listener listener;
};

wl_registry_listener GHOST_WaylandRegistryListener::listener = wl_registry_listener{
    GHOST_WaylandRegistryListener::add_object, GHOST_WaylandRegistryListener::remove_object};

GHOST_SystemWayland::GHOST_SystemWayland()
{
  wl_registry *registry;

  m_display = wl_display_connect(nullptr);
  registry = wl_display_get_registry(m_display);
  wl_registry_add_listener(registry, &GHOST_WaylandRegistryListener::listener, this);
  wl_display_roundtrip(m_display);
}

GHOST_SystemWayland::~GHOST_SystemWayland()
{
  wl_display_disconnect(m_display);
}

bool GHOST_SystemWayland::processEvents(bool waitForEvent)
{
  wl_display_dispatch_pending(m_display);

  return true;
}

GHOST_IWindow *GHOST_SystemWayland::createWindow(const STR_String &title,
                                                 GHOST_TInt32 left,
                                                 GHOST_TInt32 top,
                                                 GHOST_TUns32 width,
                                                 GHOST_TUns32 height,
                                                 GHOST_TWindowState state,
                                                 GHOST_TDrawingContextType type,
                                                 GHOST_GLSettings,
                                                 bool exclusive,
                                                 bool is_dialog,
                                                 const GHOST_IWindow *parent)
{
  if (!m_display || !m_compositor || !m_shell) {
    return NULL;
  }

  GHOST_WindowWayland *window = new GHOST_WindowWayland(m_display,
                                                        m_compositor,
                                                        m_shell,
                                                        title,
                                                        left,
                                                        top,
                                                        width,
                                                        height,
                                                        state,
                                                        (GHOST_WindowWayland *)parent,
                                                        type,
                                                        is_dialog,
                                                        false,
                                                        exclusive);

  if (window) {
    if (window->getValid()) {
      /* Store the pointer to the window */
      m_windowManager->addWindow(window);
      m_windowManager->setActiveWindow(window);
    }
    else {
      GHOST_PRINT("GHOST_SystemWin32::createWindow(): window invalid\n");
      delete window;
      window = NULL;
    }
  }

  return window;
}

int GHOST_SystemWayland::toggleConsole(int action)
{
  return 0;
}

GHOST_TSuccess GHOST_SystemWayland::getModifierKeys(GHOST_ModifierKeys &keys) const
{
  return GHOST_kSuccess;
}

GHOST_TSuccess GHOST_SystemWayland::getButtons(GHOST_Buttons &buttons) const
{
  return GHOST_kSuccess;
}

GHOST_TUns8 *GHOST_SystemWayland::getClipboard(bool selection) const
{
  return NULL;
}
void GHOST_SystemWayland::putClipboard(GHOST_TInt8 *buffer, bool selection) const
{
}

GHOST_TUns8 GHOST_SystemWayland::getNumDisplays() const
{
  return GHOST_TUns8(1);
}

GHOST_TSuccess GHOST_SystemWayland::getCursorPosition(GHOST_TInt32 &x, GHOST_TInt32 &y) const
{
  return GHOST_kFailure;
}

GHOST_TSuccess GHOST_SystemWayland::setCursorPosition(GHOST_TInt32 x, GHOST_TInt32 y)
{
  return GHOST_kFailure;
}

void GHOST_SystemWayland::getMainDisplayDimensions(GHOST_TUns32 &width, GHOST_TUns32 &height) const
{
}

void GHOST_SystemWayland::getAllDisplayDimensions(GHOST_TUns32 &width, GHOST_TUns32 &height) const
{
}

GHOST_IContext *GHOST_SystemWayland::createOffscreenContext()
{
  return NULL;
}

GHOST_TSuccess GHOST_SystemWayland::disposeContext(GHOST_IContext *context)
{
  return GHOST_kFailure;
}
