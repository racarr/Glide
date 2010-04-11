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
glide_gtk_util_show_save_dialog (GCallback callback, gpointer user_data)
{
  GtkWidget *d;
  
  d = gtk_file_chooser_dialog_new ("Save File",
				   NULL,
				   GTK_FILE_CHOOSER_ACTION_SAVE,
				   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				   GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
				   NULL);
  gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (d), TRUE);

  g_signal_connect (d, "response", callback, user_data);
  
  gtk_widget_show (d);

  return d;
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

GtkWidget *
glide_gtk_util_show_file_dialog (GCallback callback, gpointer user_data)
{
  GtkWidget *d;
  GtkFileFilter *f;
  
  d = gtk_file_chooser_dialog_new ("Open document",
				   NULL,
				   GTK_FILE_CHOOSER_ACTION_OPEN,
				   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				   GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				   NULL);
  g_signal_connect (d, "response", callback, user_data);
  
  f = gtk_file_filter_new ();
  gtk_file_filter_set_name (f, "Glide Documents");
  gtk_file_filter_add_pattern (f, "*.glide");
  
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (d), f);
  
  gtk_widget_show (d);
  
  return d;
}

void
glide_gdk_color_from_clutter_color (ClutterColor *cc, GdkColor *c)
{
  c->red = (cc->red/255.0) * 65535;
  c->green = (cc->green/255.0) * 65535;
  c->blue = (cc->blue/255.0) * 65535;
}

void
glide_clutter_color_from_gdk_color (GdkColor *c, ClutterColor *cc)
{
  cc->alpha = 0xff;
  cc->red = (c->red/65535.0)*255.0;
  cc->blue = (c->blue/65535.0)*255.0;
  cc->green = (c->green/65535.0)*255.0;
}

static void
glide_error_response_callback (GtkDialog *dialog,
			       int response,
			       gpointer user_data)
{
  gtk_widget_destroy (GTK_WIDGET (dialog));
}

GtkWidget *
glide_gtk_util_show_error_dialog (const gchar *text,
				  const gchar *secondary)
{
  GtkWidget *d = gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL,
					 GTK_MESSAGE_ERROR,
					 GTK_BUTTONS_OK,
					 text, NULL);
  gtk_message_dialog_format_secondary_markup (GTK_MESSAGE_DIALOG (d), secondary, NULL);
  
  gtk_widget_show (d);
  g_signal_connect (d, "response", G_CALLBACK (glide_error_response_callback), NULL);
  
  return d;
}
