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
 *
 * The Original Code is Copyright (C) 2016 by Mike Erwin.
 * All rights reserved.
 */

/** \file
 * \ingroup gpu
 *
 * GPU vertex buffer
 */

#pragma once

#include "GPU_vertex_buffer.h"

namespace blender::gpu {

struct VertBuf {
  static size_t memory_usage;

  GPUVertFormat format = {};
  /** Number of verts we want to draw. */
  uint vertex_len = 0;
  /** Number of verts data. */
  uint vertex_alloc = 0;
  /** 0 indicates not yet allocated. */
  uint32_t vbo_id = 0;
  /** Usage hint for GL optimisation. */
  GPUUsageType usage = GPU_USAGE_STATIC;
  /** Status flag. */
  GPUVertBufStatus flag = GPU_VERTBUF_INVALID;
  /** This counter will only avoid freeing the GPUVertBuf, not the data. */
  char handle_refcount = 0;
  /** NULL indicates data in VRAM (unmapped) */
  uchar *data = NULL;
};

/* Syntacting suggar. */
static inline GPUVertBuf *wrap(VertBuf *vert)
{
  return reinterpret_cast<GPUVertBuf *>(vert);
}
static inline VertBuf *unwrap(GPUVertBuf *vert)
{
  return reinterpret_cast<VertBuf *>(vert);
}
static inline const VertBuf *unwrap(const GPUVertBuf *vert)
{
  return reinterpret_cast<const VertBuf *>(vert);
}

}  // namespace blender::gpu