/*
 * glide-gtk-util.c
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

#include "glide-gtk-util.h"

gchar *
glide_gtk_util_get_clipboard_text ()
{
  GtkClipboard *c = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
  return gtk_clipboard_wait_for_text (c);
}

GtkWidget *
glide_gtk_util_show_image_dialog (GCallback callback, gpointer user_data)
{
  GtkWidget *d;
  GtkFileFilter *f;
  
  d = gtk_file_chooser_dialog_new ("Open image",
				   NULL,
				   GTK_FILE_CHOOSER_ACTION_OPEN,
				   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				   GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				   NULL);
  g_signal_connect (d, "response", callback, user_data);
  
  f = gtk_file_filter_new ();
  gtk_file_filter_set_name (f, "Supported Images");
  gtk_file_filter_add_mime_type (f, "image/*");
  
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (d), f);
  
  gtk_widget_show (d);
  
  return d;
}
