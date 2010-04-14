/*
 * glide-window.h
 * This file is part of glide
 *
 * Copyright (C) 2010 - Robert Carr
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANWINDOWILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, 
 * Boston, MA 02111-1307, USA.
 */

#ifndef __GLIDE_WINDOW_H__
#define __GLIDE_WINDOW_H__

#include <gtk/gtk.h>

#include <clutter/clutter.h>
#include <clutter-gtk/clutter-gtk.h>

#include "glide-undo-manager.h"

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define GLIDE_TYPE_WINDOW              (glide_window_get_type())
#define GLIDE_WINDOW(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), GLIDE_TYPE_WINDOW, GlideWindow))
#define GLIDE_WINDOW_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), GLIDE_TYPE_WINDOW, GlideWindowClass))
#define GLIDE_IS_WINDOW(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), GLIDE_TYPE_WINDOW))
#define GLIDE_IS_WINDOW_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GLIDE_TYPE_WINDOW))
#define GLIDE_WINDOW_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), GLIDE_TYPE_WINDOW, GlideWindowClass))

/* Private structure type */
typedef struct _GlideWindowPrivate GlideWindowPrivate;

/*
 * Main object structure
 */
typedef struct _GlideWindow GlideWindow;

struct _GlideWindow 
{
	GtkWindow window;

	/*< private > */
	GlideWindowPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _GlideWindowClass GlideWindowClass;

struct _GlideWindowClass 
{
	GtkWindowClass parent_class;
};

/*
 * Public methods
 */
GType 		 glide_window_get_type 			(void) G_GNUC_CONST;
GlideWindow     *glide_window_new                       (void);

void glide_window_open_document (GlideWindow *w, const gchar *filename);

G_END_DECLS

#endif  /* __GLIDE_WINDOW_H__  */
