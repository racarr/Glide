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

#include "glide-json-util.h"
#include "glide-gtk-util.h"

#include "glide-slide.h"

#include "glide-debug.h"


#define GLIDE_WINDOW_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object),	\
								      GLIDE_TYPE_WINDOW, \
								      GlideWindowPrivate))

#define PRESENTATION_WIDTH 800
#define PRESENTATION_HEIGHT 600

G_DEFINE_TYPE(GlideWindow, glide_window, GTK_TYPE_WINDOW)

static void glide_window_load_ui (GlideWindow *w);
void glide_window_new_action_activate (GtkAction *a, gpointer user_static);
static void glide_window_new_document_real (GlideWindow *w);
static GtkWidget *glide_window_make_embed ();
static void glide_window_stage_enter_notify (GtkWidget *w, GdkEventCrossing *e, gpointer user_data);

static void glide_window_insert_stage (GlideWindow *w);
static void glide_window_close_document (GlideWindow *w);

static void
glide_window_enable_action (GlideWindow *w, const gchar *action)
{
  GtkAction *a = 
    GTK_ACTION (gtk_builder_get_object (w->priv->builder, action));
  gtk_action_set_sensitive (a, TRUE);
}

static void
glide_window_enable_document_actions (GlideWindow *w)
{
  glide_window_enable_action (w, "new-image-action");
  glide_window_enable_action (w, "new-text-action");
  glide_window_enable_action (w, "next-slide-action");
  glide_window_enable_action (w, "prev-slide-action");
  glide_window_enable_action (w, "add-slide-action");
  glide_window_enable_action (w, "present-action");
  glide_window_enable_action (w, "background-action");
}
static void
glide_window_set_document (GlideWindow *w, GlideDocument *d)
{
  if (!w->priv->document)
    glide_window_enable_document_actions (w);
  w->priv->document = d;
}

static void
glide_window_close_document (GlideWindow *w)
{
  if (w->priv->document)
    g_object_unref (w->priv->document);
  if (w->priv->manager)
    g_object_unref (w->priv->manager);
  clutter_group_remove_all (CLUTTER_GROUP (w->priv->stage));
}

static void
glide_window_open_document_real (GlideWindow *window,
				 const gchar *filename)
{
  JsonParser *p = json_parser_new ();
  GError *e = NULL;
  JsonNode *root, *slide_n;
  JsonArray *slide_array;
  JsonObject *root_obj;

  json_parser_load_from_file (p, filename, &e);
  if (e)
    {
      g_warning("Error loading file: %s", e->message);
      g_error_free (e);
      
      return;
    }
  root = json_parser_get_root (p);
  root_obj = json_node_get_object (root);

  glide_window_set_document (window, glide_document_new (glide_json_object_get_string (root_obj, "name")));
   window->priv->manager = glide_stage_manager_new (window->priv->document, CLUTTER_STAGE (window->priv->stage));
  
  slide_n = json_object_get_member (root_obj, "slides");
  slide_array = json_node_get_array (slide_n);
  
  glide_stage_manager_load_slides (window->priv->manager, slide_array);
  
  g_object_unref (p);
}

static void
glide_window_new_document_real (GlideWindow *w)
{
  GlideDocument *d = glide_document_new ("New Document...");
  glide_window_set_document (w, d);
  
  w->priv->manager = glide_stage_manager_new (w->priv->document,
					      CLUTTER_STAGE (w->priv->stage));
  glide_document_add_slide (d);
}

static void
glide_window_insert_stage (GlideWindow *w)
{
  ClutterColor white = {0xff, 0xff, 0xff, 0xff};
  GtkWidget *fixed = GTK_WIDGET (gtk_builder_get_object (w->priv->builder, "embed-fixed"));
  GtkWidget *embed = glide_window_make_embed ();
  
  w->priv->stage = gtk_clutter_embed_get_stage (GTK_CLUTTER_EMBED (embed));
  clutter_actor_set_size (w->priv->stage, 800, 600);
  
  clutter_actor_show (w->priv->stage);
  
  clutter_stage_set_color (CLUTTER_STAGE (w->priv->stage), &white);
  
  gtk_fixed_put (GTK_FIXED (fixed), embed, 0, 0);
  gtk_widget_set_size_request (fixed, 800, 600);
  gtk_widget_set_size_request (embed, 800, 600);
}

static void
glide_window_stage_enter_notify (GtkWidget *widget,
				 GdkEventCrossing *event,
				 gpointer user_data)
{
  gtk_widget_grab_focus (widget);
}

static GtkWidget *
glide_window_make_embed ()
{
  GtkWidget *embed = gtk_clutter_embed_new ();
  
  // TODO: Leaks signal.
  g_signal_connect (embed, "enter-notify-event",
		    G_CALLBACK (glide_window_stage_enter_notify),
		    NULL);
  
  return embed;
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

void
glide_window_new_image_action_activate (GtkAction *a, 
					gpointer user_data)
{
  GlideWindow *w = (GlideWindow *)user_data;
  
  GLIDE_NOTE (WINDOW, "Inserting new image.");

  glide_gtk_util_show_image_dialog (G_CALLBACK (glide_window_image_open_response_callback), w);
}

void
glide_window_new_text_action_activate (GtkAction *a,
				       gpointer user_data)
{
  GlideWindow *w = (GlideWindow *)user_data;
  ClutterActor *text = glide_text_new ();
  
  glide_stage_manager_add_actor (w->priv->manager, GLIDE_ACTOR (text));
}

void
glide_window_add_slide_action_activate (GtkAction *a,
					gpointer user_data)
{
  GlideWindow *window = (GlideWindow *)user_data;
  GlideSlide *slide, *oslide;
  
  oslide = glide_document_get_nth_slide (window->priv->document,
					 glide_stage_manager_get_current_slide (window->priv->manager));
  
  slide = glide_document_add_slide (window->priv->document);
  glide_slide_set_background (slide, glide_slide_get_background (oslide));
}

void
glide_window_next_slide_action_activate (GtkAction *a,
					 gpointer user_data)
{
  GlideWindow *w = (GlideWindow *) user_data;
  glide_stage_manager_set_slide_next (w->priv->manager);
}

void
glide_window_prev_slide_action_activate (GtkAction *a,
					 gpointer user_data)
{
  GlideWindow *w = (GlideWindow *) user_data;
  glide_stage_manager_set_slide_prev (w->priv->manager);
}

void
glide_window_new_action_activate (GtkAction *a,
				  gpointer user_data)
{
  GlideWindow *w = (GlideWindow *)user_data;

  glide_window_close_document (w);
  glide_window_new_document_real (w);
}

static void
glide_window_file_open_response_callback (GtkDialog *dialog,
					  int response,
					  gpointer user_data)
{
  GlideWindow *w = (GlideWindow *) user_data;
  if (response == GTK_RESPONSE_ACCEPT)
    {
      gchar *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
      
      g_message("Loading file: %s \n", filename);
      
      glide_window_close_document (w);
      glide_window_open_document_real (w, filename);
      g_free (filename);
    }
  
  gtk_widget_destroy (GTK_WIDGET (dialog));
}

void
glide_window_open_action_activate (GtkAction *a,
				   gpointer user_data)
{
  GtkWidget *d;
  GlideWindow *w = (GlideWindow *)user_data;
  
  GLIDE_NOTE (WINDOW, "Loading file.");
  d = gtk_file_chooser_dialog_new ("Load presentation",
				   NULL,
				   GTK_FILE_CHOOSER_ACTION_OPEN,
				   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				   GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				   NULL);
  g_signal_connect (d, "response",
		    G_CALLBACK (glide_window_file_open_response_callback),
		    w);
  
  gtk_widget_show (d);
}

static void
glide_window_load_ui (GlideWindow *w)
{
  GtkBuilder *b = gtk_builder_new ();
  GtkWidget *main_box;
  
  w->priv->builder = b;
  
  // Todo: Error checking
  gtk_builder_add_from_file (b, "glide-window.ui", NULL);
  
  gtk_builder_connect_signals (b, w);
  
  main_box = GTK_WIDGET (gtk_builder_get_object (b, "main-vbox"));
  gtk_widget_reparent (main_box, GTK_WIDGET (w));
}

static void
glide_window_finalize (GObject *object)
{
  GlideWindow *w = GLIDE_WINDOW (object);
  GLIDE_NOTE (WINDOW, "Finalizing GlideWindow: %p",
	       object);
  
  g_object_unref (w->priv->builder);

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
  window->priv = GLIDE_WINDOW_GET_PRIVATE (window);
  
  glide_window_load_ui (window);
  glide_window_insert_stage (window);

  GLIDE_NOTE (WINDOW, "Intializing Glide window (%p)", window);
  
  gtk_widget_show_all (GTK_WIDGET (window));
}

GlideWindow *
glide_window_new ()
{
  return g_object_new (GLIDE_TYPE_WINDOW, NULL);
}
