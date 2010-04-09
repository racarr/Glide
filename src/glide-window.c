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

static void glide_window_setup_stage (GlideWindow *window);

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
  GlideWindow *w = (GlideWindow *)user_data;
  
  GLIDE_NOTE (WINDOW, "Inserting new image.");

  glide_gtk_util_show_image_dialog (G_CALLBACK (glide_window_image_open_response_callback), w);
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
  GlideSlide *slide, *oslide;
  
  oslide = glide_document_get_nth_slide (window->priv->document,
					 glide_stage_manager_get_current_slide (window->priv->manager));
  
  slide = glide_document_add_slide (window->priv->document);
  glide_slide_set_background (slide, glide_slide_get_background (oslide));
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
glide_window_new_document_real (GlideWindow *w)
{
  GLIDE_NOTE (WINDOW, "New document");
  
  if (w->priv->document)
    g_object_unref (w->priv->document);
  if (w->priv->manager)
    g_object_unref (w->priv->manager);

  w->priv->document = glide_document_new ("New document");

  clutter_group_remove_all (CLUTTER_GROUP (w->priv->stage));
  glide_window_setup_stage (w);
  
  glide_document_add_slide (w->priv->document);
}

static void
glide_window_new_document (GtkWidget *toolitem, gpointer data)
{
  GlideWindow *w = (GlideWindow *) data;

  GLIDE_NOTE (WINDOW, "New document");
  
  glide_window_new_document_real (w);
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

  g_object_unref (window->priv->document);
  g_object_unref (window->priv->manager);
  
  window->priv->document = glide_document_new (glide_json_object_get_string (root_obj, "name"));
  clutter_group_remove_all (CLUTTER_GROUP (window->priv->stage));
  glide_window_setup_stage (window);
  
  slide_n = json_object_get_member (root_obj, "slides");
  slide_array = json_node_get_array (slide_n);
  
  glide_stage_manager_load_slides (window->priv->manager, slide_array);
  
  g_object_unref (p);
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
      glide_window_open_document_real (w, filename);
      g_free (filename);
    }
  
  gtk_widget_destroy (GTK_WIDGET (dialog));
}

static void
glide_window_open_document (GtkWidget *toolitem, gpointer data)
{
  GtkWidget *d;
  GlideWindow *w = (GlideWindow *)data;
  
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
glide_window_center_stage (GlideWindow *w)
{
  GtkAllocation a;
  gtk_widget_get_allocation (w->priv->fixed, &a);
  gtk_fixed_move (GTK_FIXED (w->priv->fixed), w->priv->embed, (a.width-PRESENTATION_WIDTH)/2.0, (a.height-PRESENTATION_HEIGHT)/2.0);
}
/* hack */
static void
glide_window_center_stage_fullscreen (GlideWindow *w)
{
  GtkRequisition a, e;
  
  gdk_flush();
  
  gtk_widget_size_request (w->priv->fixed, &a);
  gtk_widget_size_request (w->priv->embed, &e);
  
  //  gtk_widget_size_request (w->priv->fixed, &r);
  
  gtk_fixed_move (GTK_FIXED (w->priv->fixed), w->priv->embed, (a.width-e.width)/2.0, (a.height-e.height)/2.0);
}

static void
glide_window_fullscreen_stage (GlideWindow *w)
{
  GtkAllocation a;
  GdkScreen *s = gtk_window_get_screen (GTK_WINDOW (w));
  
  gtk_widget_get_allocation (w->priv->fixed, &a);
  w->priv->of_width = a.width;
  w->priv->of_height = a.height;
  
  gtk_window_fullscreen (GTK_WINDOW (w));
  
  gtk_widget_hide_all (GTK_WIDGET (w));

  gtk_widget_show (GTK_WIDGET (w));
  gtk_widget_show_all (w->priv->fixed);
  gtk_widget_show (gtk_widget_get_parent (w->priv->fixed));
  
  gtk_widget_set_size_request (w->priv->fixed, gdk_screen_get_width (s), gdk_screen_get_height (s));
  
  glide_window_center_stage_fullscreen (w);
}

static void
glide_window_unfullscreen_stage (GlideWindow *w)
{
  gtk_window_unfullscreen (GTK_WINDOW (w));
  gtk_widget_show_all (GTK_WIDGET (w));
  
  gtk_fixed_move (GTK_FIXED (w->priv->fixed), w->priv->embed, 0, 0);
  gtk_widget_set_size_request (w->priv->embed, PRESENTATION_WIDTH, PRESENTATION_HEIGHT);
  gtk_widget_set_size_request (w->priv->fixed, PRESENTATION_WIDTH, PRESENTATION_HEIGHT);
  gtk_window_resize (GTK_WINDOW (w), 1, 1);

  gtk_fixed_move (GTK_FIXED (w->priv->fixed), w->priv->embed, (w->priv->of_width-PRESENTATION_WIDTH)/2.0, (w->priv->of_height-PRESENTATION_HEIGHT)/2.0);
}

static void
glide_window_present_document (GtkWidget *toolitem, gpointer data)
{
  GlideWindow *w = (GlideWindow *)data;
  GLIDE_NOTE (WINDOW, "Presenting document.");

  glide_window_fullscreen_stage (w);
  
  glide_stage_manager_set_presenting (w->priv->manager, TRUE);  
}

static void
glide_window_slide_background_cb (GtkDialog *dialog,
				  int response,
				  gpointer user_data)
{
  GlideWindow *window = (GlideWindow *)user_data;
  
  if (response == GTK_RESPONSE_ACCEPT)
    {
      gchar *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
      
      glide_stage_manager_set_slide_background (window->priv->manager, filename);
      
      g_free (filename);
    }
  
  gtk_widget_destroy (GTK_WIDGET (dialog));
}

static void
glide_window_set_slide_background (GtkWidget *toolitem, gpointer data)
{
  GlideWindow *w = (GlideWindow *)data;
  GLIDE_NOTE (WINDOW, "Setting slide background");
  
  glide_gtk_util_show_image_dialog (G_CALLBACK (glide_window_slide_background_cb), w);
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
  GtkWidget *toolbar, *image, *image2, *image3, *image4, *image5, *image6, *image7, *image8, *image9, *image10;

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
  image7 =
    gtk_image_new_from_stock (GTK_STOCK_NEW, GTK_ICON_SIZE_LARGE_TOOLBAR);
  image8 =
    gtk_image_new_from_stock (GTK_STOCK_OPEN, GTK_ICON_SIZE_LARGE_TOOLBAR);
  image9 =
    gtk_image_new_from_stock (GTK_STOCK_FULLSCREEN, GTK_ICON_SIZE_LARGE_TOOLBAR);
  image10 =
    gtk_image_new_from_stock (GTK_STOCK_FILE, GTK_ICON_SIZE_LARGE_TOOLBAR);

  
  gtk_toolbar_append_item (GTK_TOOLBAR(toolbar), "Image", 
			   "Insert a new image in to the document", 
			   NULL, image, G_CALLBACK(glide_window_new_image), 
			   w);  
  gtk_toolbar_append_item (GTK_TOOLBAR(toolbar), "Text", 
			   "Insert a new text object in to the document", 
			   NULL, image2, G_CALLBACK(glide_window_new_text), 
			   w);  
  gtk_toolbar_append_item (GTK_TOOLBAR(toolbar), "New slide", 
			   "Insert a new slide in to the document",
			   NULL, image3, G_CALLBACK(glide_window_new_slide), 
			   w);  
  gtk_toolbar_append_item (GTK_TOOLBAR(toolbar), "Previous", 
			   "Move to the previous slide",
			   NULL, image5, G_CALLBACK(glide_window_slide_prev), 
			   w);  
  gtk_toolbar_append_item (GTK_TOOLBAR(toolbar), "Next", 
			   "Move to the Next slide",
			   NULL, image4, G_CALLBACK(glide_window_slide_next), 
			   w);  
  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), "Save",
			   "Save document",
			   NULL, image6, G_CALLBACK (glide_window_save_document),
			   w);
  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), "New",
			   "New document",
			   NULL, image7, G_CALLBACK (glide_window_new_document),
			   w);
  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), "Open",
			   "Open document",
			   NULL, image8, G_CALLBACK (glide_window_open_document),
			   w);  
  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), "Present",
			   "Present Document",
			   NULL, image9, G_CALLBACK (glide_window_present_document),
			   w);  
  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), "Background",
			   "Set slide background",
			   NULL, image10, G_CALLBACK (glide_window_set_slide_background),
			   w);  
  
  return toolbar;
}


static GtkWidget *
glide_window_make_embed (GlideWindow *window)
{
  GtkWidget *embed = gtk_clutter_embed_new ();
  window->priv->stage = gtk_clutter_embed_get_stage (GTK_CLUTTER_EMBED (embed));
  window->priv->embed = embed;
  
  g_signal_connect (embed, "enter-notify-event",
		    G_CALLBACK (glide_window_stage_enter_notify),
		    NULL);
  gtk_widget_set_can_focus (GTK_WIDGET (embed), TRUE);
  
  return embed;
}

static void
glide_window_make_bottom_bar (GlideWindow *window, GtkWidget *hbox)
{
  GtkWidget *color_button = gtk_color_button_new ();
  
  gtk_box_pack_start (GTK_BOX(hbox), color_button, FALSE, FALSE, 0);
}

static void
glide_window_setup_chrome (GlideWindow *window)
{
  GtkWidget *vbox, *embed, *toolbar, *fixed, *hbox;
  GdkColor black;
  
  vbox = gtk_vbox_new (FALSE, 0);
  hbox = gtk_hbox_new (FALSE, 0);
  
  embed = glide_window_make_embed (window);
  toolbar = glide_window_make_toolbar (window);
  
  fixed = gtk_fixed_new();
  gtk_fixed_set_has_window (GTK_FIXED (fixed), TRUE); 
  window->priv->fixed = fixed;
  
  gdk_color_parse ("black", &black);
  gtk_widget_modify_bg (fixed, GTK_STATE_NORMAL, &black);

  gtk_box_pack_start (GTK_BOX (vbox), toolbar,
		      FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), fixed,
		      FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox,
		      FALSE, FALSE, 0);
  
  glide_window_make_bottom_bar (window, hbox);
  
  gtk_fixed_put (GTK_FIXED (fixed), embed, 0, 0);
  
  gtk_widget_set_size_request (fixed, PRESENTATION_WIDTH, PRESENTATION_HEIGHT);
  gtk_widget_set_size_request (embed, PRESENTATION_WIDTH, PRESENTATION_HEIGHT);

  gtk_container_add (GTK_CONTAINER (window), vbox);

}

static void
glide_window_stage_manager_presenting_changed_cb (GObject *object,
						  GParamSpec *pspec,
						  gpointer user_data)
{
  GlideWindow *w = (GlideWindow *)user_data;
  
  if (!glide_stage_manager_get_presenting (w->priv->manager))
    {
      glide_window_unfullscreen_stage (w);
    }
}

static void
glide_window_setup_stage (GlideWindow *window)
{
  ClutterColor white = {0xff, 0xff, 0xff, 0xff};
  ClutterActor *stage = window->priv->stage;

  clutter_actor_set_size (stage, PRESENTATION_WIDTH, PRESENTATION_HEIGHT);
  clutter_stage_set_color (CLUTTER_STAGE (stage), &white);
  
  window->priv->manager = glide_stage_manager_new (window->priv->document, 
						   CLUTTER_STAGE (stage));

  clutter_actor_show (stage);

  g_signal_connect (window->priv->manager, "notify::presenting",
		    G_CALLBACK (glide_window_stage_manager_presenting_changed_cb),
		    window);
}


static void
glide_window_init (GlideWindow *window)
{
  window->priv = GLIDE_WINDOW_GET_PRIVATE (window);

  GLIDE_NOTE (WINDOW, "Intializing Glide window");
  
  glide_window_setup_chrome (window);

  glide_window_new_document_real (window);

  gtk_widget_show_all (GTK_WIDGET (window));
  glide_window_center_stage (window);
}

GlideWindow *
glide_window_new ()
{
  return g_object_new (GLIDE_TYPE_WINDOW, NULL);
}
