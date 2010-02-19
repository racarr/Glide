/*
 * This file is part of Glide, the GObject Introspection<->Javascript bindings.
 *
 * Glide is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 * Glide is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * You should have received a copy of the GNU Lesser General Public License
 * along with Glide.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright (C) Robert Carr 2009 <carrr@rpi.edu>
 */

#ifndef _GLIDE_DEBUG_H
#define _GLIDE_DEBUG_H

// Borrowed from Clutter, more or less.

#include <glib.h>

typedef enum
{
  GLIDE_DEBUG_ALL = 1 << 0,
  GLIDE_DEBUG_MISC = 1 << 1,
  GLIDE_DEBUG_IMAGE = 1 << 2,
  GLIDE_DEBUG_MANIPULATOR = 1 << 3,
  GLIDE_DEBUG_STAGE_MANAGER = 1 << 4,
  GLIDE_DEBUG_WINDOW = 1 << 5,
  GLIDE_DEBUG_PAINT = 1 << 6,
  GLIDE_DEBUG_TEXT = 1 << 7,
  GLIDE_DEBUG_DOCUMENT = 1 << 8
} GlideDebugFlag;

#ifdef GLIDE_ENABLE_DEBUG

#define GLIDE_NOTE(type,...)  G_STMT_START {                 \
    if ((glide_debug_flags & GLIDE_DEBUG_##type) ||           \
        glide_debug_flags & GLIDE_DEBUG_ALL)                  \
    {                                                       \
        gchar * _fmt = g_strdup_printf (__VA_ARGS__);       \
        g_message ("[" #type "] " G_STRLOC ": %s",_fmt);    \
        g_free (_fmt);                                      \
    }                                                       \
} G_STMT_END

#define GLIDE_MARK()      GLIDE_NOTE(MISC, "== mark ==")
#define GLIDE_DBG(x) { a }

#else /* !GLIDE_ENABLE_DEBUG */

#define GLIDE_NOTE(type,...)
#define GLIDE_MARK()
#define GLIDE_DBG(x)

#endif /* GLIDE_ENABLE_DEBUG */

extern guint glide_debug_flags;

#endif
