/*
 * glide-gtk-util.h
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, 
 * Boston, MA 02111-1307, USA.
 */

#ifndef __GLIDE_GTK_UTIL_H__
#define __GLIDE_GTK_UTIL_H__

#include <gtk/gtk.h>
#include <clutter/clutter.h>

GtkWidget *glide_gtk_util_show_image_dialog (GCallback callback, gpointer user_data);
GtkWidget *glide_gtk_util_show_file_dialog (GCallback callback, gpointer user_data);

GtkWidget *glide_gtk_util_show_save_dialog (GCallback callback, gpointer user_data);
gchar *glide_gtk_util_get_clipboard_text ();

void glide_gdk_color_from_clutter_color (ClutterColor *cc, GdkColor *c);
void glide_clutter_color_from_gdk_color (GdkColor *c, ClutterColor *cc);

GtkWidget * glide_gtk_util_show_error_dialog (const gchar *text, const gchar *secondary);


#endif
