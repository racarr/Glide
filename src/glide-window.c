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
#include "glide-manipulator.h"


#include "glide-rectangle.h"
#include "glide-stage-manager.h"
#include "glide-window-private.h"
#include "glide-image.h"

#include "glide-debug.h"

#define GLIDE_WINDOW_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object),	\
								      GLIDE_TYPE_WINDOW, \
								      GlideWindowPrivate))

G_DEFINE_TYPE(GlideWindow, glide_window, GTK_TYPE_WINDOW)


static void
glide_window_finalize (GObject *object)
{
  GLIDE_NOTE (WINDOW, "Finalizing GlideWindow: %p",
	       object);

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
glide_window_new_image (GtkWidget *toolitem, gpointer data)
{
  GlideStageManager *manager = (GlideStageManager *)data;
  ClutterActor *stage = (ClutterActor *)
    glide_stage_manager_get_stage (manager);
  ClutterActor *im;
  
  GLIDE_NOTE (WINDOW, "Inserting new image, stage manager: %p", manager);
  
  im = (ClutterActor *)glide_image_new_from_file (manager,
						  "/home/racarr/surprise.jpg",
						  NULL);
  clutter_actor_set_position(im, 0, 0);
  clutter_actor_set_size(im, 100, 100);
  
  //  g = (ClutterActor *)glide_manipulator_new (im);
  //  clutter_actor_set_size(g, 100, 100);

  //  clutter_actor_set_position(g, 200, 200);
  clutter_actor_set_position(im, 200, 200);
  
  //  clutter_container_add_actor (CLUTTER_CONTAINER(stage), g);
  clutter_container_add_actor (CLUTTER_CONTAINER(stage), im);

  //  clutter_actor_raise (g, im);
  
  clutter_actor_show_all (stage);

  g_message("New image!");
}

static void
glide_window_new_text (GtkWidget *toolitem, gpointer data)
{
  GlideStageManager *manager = (GlideStageManager *)data;
  ClutterActor *stage = 
    (ClutterActor *)glide_stage_manager_get_stage (manager);
  ClutterActor *im;
  
  im = (ClutterActor *)glide_rectangle_new(manager);

  GLIDE_NOTE (WINDOW, "Inserting new text, stage manager: %p", manager);
  
  //  g = (ClutterActor *)glide_manipulator_new (im);

  //  clutter_actor_set_position(g, 400, 200);
  clutter_actor_set_position(im, 400, 200);
  
  //  clutter_actor_set_size(g, 100, 100);
  clutter_actor_set_size(im, 100, 100);
  
  //  clutter_container_add_actor (CLUTTER_CONTAINER(stage), g);
  clutter_container_add_actor (CLUTTER_CONTAINER(stage), im);
  
  //  clutter_actor_lower(g, im);
  
  clutter_actor_show_all (stage);

  g_message("New text!");
}

static GtkWidget *
glide_window_make_toolbar (GlideWindow *w)
{
  GtkWidget *toolbar, *image, *image2;
  GlideStageManager *manager = 
    glide_stage_manager_new (CLUTTER_STAGE(w->priv->stage));

  toolbar = gtk_toolbar_new ();
  
  image = 
    gtk_image_new_from_stock (GTK_STOCK_ADD, GTK_ICON_SIZE_LARGE_TOOLBAR);
  image2 = 
    gtk_image_new_from_stock (GTK_STOCK_ADD, GTK_ICON_SIZE_LARGE_TOOLBAR);
  
  gtk_toolbar_append_item (GTK_TOOLBAR(toolbar), "New image", 
			   "Insert a new image in to the document", 
			   NULL, image, G_CALLBACK(glide_window_new_image), 
			   manager);  
  gtk_toolbar_append_item (GTK_TOOLBAR(toolbar), "New text", 
			   "Insert a new text object in to the document", 
			   NULL, image2, G_CALLBACK(glide_window_new_text), 
			   manager);  
  
  
  return toolbar;
}

static void
glide_window_init (GlideWindow *window)
{
  ClutterActor *stage;
  ClutterColor black = {0x00, 0x00, 0x00, 0xff};
  GtkWidget *vbox, *embed, *toolbar;
  
  GLIDE_NOTE (WINDOW, "Intializing Glide window");
  
  window->priv = GLIDE_WINDOW_GET_PRIVATE (window);
  
  vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (window), vbox);
  
  embed = gtk_clutter_embed_new ();
  stage = gtk_clutter_embed_get_stage (GTK_CLUTTER_EMBED (embed));
  
  window->priv->stage = stage;
  toolbar = glide_window_make_toolbar (window);

  gtk_container_add (GTK_CONTAINER (vbox), toolbar);
  gtk_container_add (GTK_CONTAINER (vbox), embed);
  
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
