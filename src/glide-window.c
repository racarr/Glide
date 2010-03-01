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

#include "glide-stage-manager.h"

#include "glide-image.h"
#include "glide-text.h"

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
glide_window_image_open_response_callback (GtkDialog *dialog,
					   int response,
					   gpointer user_data)
{
  GlideWindow *window = (GlideWindow *)user_data;

  if (response == GTK_RESPONSE_ACCEPT)
    {
      ClutterActor *im;
      // Todo: URI
      gchar *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
      
      im =  glide_image_new_from_file (filename, NULL);
      glide_stage_manager_add_actor (window->priv->manager, GLIDE_ACTOR (im));
      
      g_free (filename);
      
      clutter_actor_show (im);
    }
  
  gtk_widget_destroy (GTK_WIDGET (dialog));
}

static void
glide_window_new_image (GtkWidget *toolitem, gpointer user_data)
{
  GtkWidget *d;
  GlideWindow *w = (GlideWindow *)user_data;
  
  GLIDE_NOTE (WINDOW, "Inserting new image.");
  d = gtk_file_chooser_dialog_new ("Open image",
				   NULL,
				   GTK_FILE_CHOOSER_ACTION_OPEN,
				   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				   GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				   NULL);
  g_signal_connect (d, "response",
		    G_CALLBACK (glide_window_image_open_response_callback),
		    w);
  
  // TODO: Make it start on our current font...
  gtk_widget_show (d);
}

static void
glide_window_new_text (GtkWidget *toolitem, gpointer data)
{
  GlideWindow *window = (GlideWindow *)data;
  ClutterActor *text;
  
  text = glide_text_new ();
  glide_stage_manager_add_actor (window->priv->manager, GLIDE_ACTOR (text));

  GLIDE_NOTE (WINDOW, "Inserting new text, stage manager: %p", window->priv->manager);
}

static void
glide_window_new_slide (GtkWidget *toolitem, gpointer data)
{
  GlideWindow *window = (GlideWindow *)data;
  
  glide_document_add_slide (window->priv->document);
}

static void
glide_window_slide_next (GtkWidget *toolitem, gpointer data)
{
  GlideWindow *w = (GlideWindow *) data;
  glide_stage_manager_set_slide_next (w->priv->manager);
}

static void
glide_window_slide_prev (GtkWidget *toolitem, gpointer data)
{
  GlideWindow *w = (GlideWindow *) data;
  glide_stage_manager_set_slide_prev (w->priv->manager);
}

static void
glide_window_save_document (GtkWidget *toolitem, gpointer data)
{
  GlideWindow *w = (GlideWindow *) data;
  JsonNode *node;
  JsonGenerator *gen;
  
  node = glide_document_serialize (w->priv->document);
  
  gen = json_generator_new ();
  json_generator_set_root (gen, node);

  // Error
  json_generator_to_file (gen, "/tmp/test.glide", NULL);
}

static gboolean
glide_window_stage_button_press_cb (ClutterActor *actor,
				    ClutterEvent *event,
				    gpointer user_data)
{
  GlideWindow *w = (GlideWindow *) user_data;
  
  glide_stage_manager_set_selection (w->priv->manager, NULL);
  
  return TRUE;
}

static void
glide_window_stage_enter_notify (GtkWidget *widget,
				 GdkEventCrossing *event,
				 gpointer user_data)
{
  gtk_widget_grab_focus (widget);
}

static GtkWidget *
glide_window_make_toolbar (GlideWindow *w)
{
  GtkWidget *toolbar, *image, *image2, *image3, *image4, *image5, *image6;

  toolbar = gtk_toolbar_new ();
  
  image = 
    gtk_image_new_from_stock (GTK_STOCK_ADD, GTK_ICON_SIZE_LARGE_TOOLBAR);
  image2 = 
    gtk_image_new_from_stock (GTK_STOCK_ADD, GTK_ICON_SIZE_LARGE_TOOLBAR);
  image3 = 
    gtk_image_new_from_stock (GTK_STOCK_ADD, GTK_ICON_SIZE_LARGE_TOOLBAR);
  image4 = 
    gtk_image_new_from_stock (GTK_STOCK_GO_FORWARD, GTK_ICON_SIZE_LARGE_TOOLBAR);
  image5 = 
    gtk_image_new_from_stock (GTK_STOCK_GO_BACK, GTK_ICON_SIZE_LARGE_TOOLBAR);
  image6 =
    gtk_image_new_from_stock (GTK_STOCK_SAVE, GTK_ICON_SIZE_LARGE_TOOLBAR);
  
  gtk_toolbar_append_item (GTK_TOOLBAR(toolbar), "New image", 
			   "Insert a new image in to the document", 
			   NULL, image, G_CALLBACK(glide_window_new_image), 
			   w);  
  gtk_toolbar_append_item (GTK_TOOLBAR(toolbar), "New text", 
			   "Insert a new text object in to the document", 
			   NULL, image2, G_CALLBACK(glide_window_new_text), 
			   w);  
  gtk_toolbar_append_item (GTK_TOOLBAR(toolbar), "New slide", 
			   "Insert a new slide in to the document",
			   NULL, image3, G_CALLBACK(glide_window_new_slide), 
			   w);  
  gtk_toolbar_append_item (GTK_TOOLBAR(toolbar), "Previous Slide", 
			   "Move to the previous slide",
			   NULL, image5, G_CALLBACK(glide_window_slide_prev), 
			   w);  
  gtk_toolbar_append_item (GTK_TOOLBAR(toolbar), "Next Slide", 
			   "Move to the Next slide",
			   NULL, image4, G_CALLBACK(glide_window_slide_next), 
			   w);  
  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), "Save",
			   "Save document",
			   NULL, image6, G_CALLBACK (glide_window_save_document),
			   w);

  
  
  return toolbar;
}


static GtkWidget *
glide_window_make_embed (GlideWindow *window)
{
  GtkWidget *embed = gtk_clutter_embed_new ();
  window->priv->stage = gtk_clutter_embed_get_stage (GTK_CLUTTER_EMBED (embed));
  
  g_signal_connect (embed, "enter-notify-event",
		    G_CALLBACK (glide_window_stage_enter_notify),
		    NULL);
  gtk_widget_set_can_focus (GTK_WIDGET (embed), TRUE);
  
  return embed;
}

static void
glide_window_setup_chrome (GlideWindow *window)
{
  GtkWidget *vbox, *embed, *toolbar;
  
  vbox = gtk_vbox_new (FALSE, 0);
  
  embed = glide_window_make_embed (window);
  toolbar = glide_window_make_toolbar (window);

  gtk_container_add (GTK_CONTAINER (vbox), toolbar);
  gtk_container_add (GTK_CONTAINER (vbox), embed);
  
  gtk_widget_set_size_request (embed, 800, 600);
  
  gtk_container_add (GTK_CONTAINER (window), vbox);

}

static void
glide_window_setup_stage (GlideWindow *window)
{
  ClutterColor white = {0xff, 0xff, 0xff, 0xff};
  ClutterActor *stage = window->priv->stage;

  clutter_actor_set_size (stage, 800, 600);
  clutter_stage_set_color (CLUTTER_STAGE (stage), &white);
  
  window->priv->manager = glide_stage_manager_new (window->priv->document, 
						   CLUTTER_STAGE (stage));

  clutter_actor_show (stage);

  // Used for null selections.
  g_signal_connect (window->priv->stage, "button-press-event", 
		    G_CALLBACK (glide_window_stage_button_press_cb), 
		    window);
}


static void
glide_window_init (GlideWindow *window)
{
  window->priv = GLIDE_WINDOW_GET_PRIVATE (window);

  GLIDE_NOTE (WINDOW, "Intializing Glide window");
  
  window->priv->document = glide_document_new ("First Document");

  glide_window_setup_chrome (window);
  glide_window_setup_stage (window);

  gtk_widget_show_all (GTK_WIDGET (window));
}

GlideWindow *
glide_window_new ()
{
  return g_object_new (GLIDE_TYPE_WINDOW, NULL);
}
