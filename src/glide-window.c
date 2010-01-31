/*
 * glide-window.c
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
 
#include <glib/gi18n.h>


#include "glide-window.h"
#include "glide-window-private.h"

#define GLIDE_WINDOW_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object),	\
								      GLIDE_TYPE_WINDOW, \
								      GlideWindowPrivate))

G_DEFINE_TYPE(GlideWindow, glide_window, GTK_TYPE_WINDOW)


static void
glide_window_finalize (GObject *object)
{
  /* Debug? */

  G_OBJECT_CLASS (glide_window_parent_class)->finalize (object);
}

static void
glide_window_class_init (GlideWindowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = glide_window_finalize;

  g_type_class_add_private (object_class, sizeof(GlideWindowPrivate));
}


static void
glide_window_init (GlideWindow *window)
{
  ClutterActor *stage;
  ClutterColor black = {0x00, 0x00, 0x00, 0xff};
  GtkWidget *vbox, *embed;
  
  vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (window), vbox);
  
  embed = gtk_clutter_embed_new ();
  gtk_container_add (GTK_CONTAINER (vbox), embed);
  
  stage = gtk_clutter_embed_get_stage (GTK_CLUTTER_EMBED (embed));
  clutter_actor_set_size (stage, 800, 600);
  gtk_widget_set_size_request (embed, 800, 600);
  
  clutter_stage_set_color (CLUTTER_STAGE (stage), &black);
  clutter_actor_show (stage);

  gtk_widget_show_all (GTK_WIDGET (window));
}

GlideWindow *
glide_window_new ()
{
  return g_object_new (GLIDE_TYPE_WINDOW, NULL);
}
