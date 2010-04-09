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
glide_window_update_slide_label (GlideWindow *w)
{
  gchar *message;
  gint current_slide, n_slides;
  
  current_slide = glide_stage_manager_get_current_slide (w->priv->manager) + 1;
  n_slides = glide_document_get_n_slides (w->priv->document);
  
  message = g_strdup_printf("(<b>%d</b> of <b>%d</b>)", current_slide, n_slides);
  
  gtk_label_set_markup (GTK_LABEL (w->priv->slide_label), message);

  g_free (message);
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
  GdkColor c;
  ClutterColor cc;

  gtk_color_button_get_color (GTK_COLOR_BUTTON (window->priv->color_button), &c);
  
  text = glide_text_new ();

  cc.alpha = 0xff;
  cc.red = (c.red/65535.0)*255.0;
  cc.blue = (c.blue/65535.0)*255.0;
  cc.green = (c.green/65535.0)*255.0;
  
  glide_text_set_color (GLIDE_TEXT (text), &cc);
  glide_text_set_font_name (GLIDE_TEXT (text), gtk_font_button_get_font_name (GTK_FONT_BUTTON (window->priv->font_button)));

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
glide_window_slide_next (GtkWidget *button, gpointer data)
{
  GlideWindow *w = (GlideWindow *) data;
  glide_stage_manager_set_slide_next (w->priv->manager);
}

static void
glide_window_slide_prev (GtkWidget *button, gpointer data)
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
  
  clutter_actor_grab_key_focus (w->priv->stage);
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
  GtkWidget *toolbar, *image, *image2, *image3, *image6, *image7, *image8, *image9, *image10;

  toolbar = gtk_toolbar_new ();
  
  image = 
    gtk_image_new_from_stock (GTK_STOCK_ADD, GTK_ICON_SIZE_LARGE_TOOLBAR);
  image2 = 
    gtk_image_new_from_stock (GTK_STOCK_ADD, GTK_ICON_SIZE_LARGE_TOOLBAR);
  image3 = 
    gtk_image_new_from_stock (GTK_STOCK_ADD, GTK_ICON_SIZE_LARGE_TOOLBAR);
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


  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), "New",
			   "New document",
			   NULL, image7, G_CALLBACK (glide_window_new_document),
			   w);
 gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), "Save",
			   "Save document",
			   NULL, image6, G_CALLBACK (glide_window_save_document),
			   w);
  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), "Open",
			   "Open document",
			   NULL, image8, G_CALLBACK (glide_window_open_document),
			   w);  
 
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
  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), "Background",
			   "Set slide background",
			   NULL, image10, G_CALLBACK (glide_window_set_slide_background),
			   w);  
  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), "Present",
			   "Present Document",
			   NULL, image9, G_CALLBACK (glide_window_present_document),
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
glide_window_color_set_cb (GtkWidget *b, gpointer user_data)
{
  GlideActor *selection;
  GlideWindow *w = (GlideWindow *)user_data;
  GdkColor c;
  ClutterColor cc;
  
  selection = glide_stage_manager_get_selection (w->priv->manager);
  
  if (!selection || !GLIDE_IS_TEXT(selection))
    return;
  
  gtk_color_button_get_color (GTK_COLOR_BUTTON (b), &c);
  
  cc.alpha = 0xff;
  cc.red = (c.red/65535.0)*255.0;
  cc.blue = (c.blue/65535.0)*255.0;
  cc.green = (c.green/65535.0)*255.0;
  
  glide_text_set_color (GLIDE_TEXT (selection), &cc);
}



static void
glide_window_font_set_cb (GtkWidget *b, gpointer user_data)
{
  GlideActor *selection;
  GlideWindow *w = (GlideWindow *)user_data;
  
  selection = glide_stage_manager_get_selection (w->priv->manager);
  
  if (!selection || !GLIDE_IS_TEXT(selection))
    return;
  
  glide_text_set_font_name (GLIDE_TEXT (selection), gtk_font_button_get_font_name (GTK_FONT_BUTTON (b)));
}

static void
glide_window_animations_box_changed_cb (GtkWidget *cbox, gpointer user_data)
{
  GlideWindow *w = (GlideWindow *)user_data;
  gchar *text = gtk_combo_box_get_active_text (GTK_COMBO_BOX (cbox));
  GlideSlide *s = glide_document_get_nth_slide (w->priv->document,
						glide_stage_manager_get_current_slide (w->priv->manager));
  
  glide_slide_set_animation (s, text);  
  
  g_free (text);
}

static void
glide_window_setup_animations_box (GlideWindow *window, GtkWidget *cbox)
{
  gtk_combo_box_append_text (GTK_COMBO_BOX (cbox), "None");
  gtk_combo_box_append_text (GTK_COMBO_BOX (cbox), "Fade");
  gtk_combo_box_append_text (GTK_COMBO_BOX (cbox), "Zoom");
  gtk_combo_box_append_text (GTK_COMBO_BOX (cbox), "Drop");
  gtk_combo_box_append_text (GTK_COMBO_BOX (cbox), "Pivot");
  gtk_combo_box_append_text (GTK_COMBO_BOX (cbox), "Slide");
  
  gtk_combo_box_set_active (GTK_COMBO_BOX (cbox), 0);
  
  g_signal_connect (cbox, "changed", G_CALLBACK (glide_window_animations_box_changed_cb), window);
}

static void
glide_window_align_selected_text (GlideWindow *w, PangoAlignment alignment)
{
  GlideActor *selected = glide_stage_manager_get_selection (w->priv->manager);
  
  if (!selected || !GLIDE_IS_TEXT (selected))
    return;
  glide_text_set_line_alignment (GLIDE_TEXT (selected), alignment);
}

static void
glide_window_align_left (GtkWidget *b, gpointer user_data)
{
  GlideWindow *w = (GlideWindow *)user_data;
  glide_window_align_selected_text (w, PANGO_ALIGN_LEFT);
}

static void
glide_window_align_center (GtkWidget *b, gpointer user_data)
{
  GlideWindow *w = (GlideWindow *)user_data;
  glide_window_align_selected_text (w, PANGO_ALIGN_CENTER);
}

static void
glide_window_align_right (GtkWidget *b, gpointer user_data)
{
  GlideWindow *w = (GlideWindow *)user_data;
  glide_window_align_selected_text (w, PANGO_ALIGN_RIGHT);
}

static void
glide_window_make_bottom_bar (GlideWindow *window, GtkWidget *hbox)
{
  GtkWidget *color_button = gtk_color_button_new ();
  GtkWidget *font_button = gtk_font_button_new ();
  GtkWidget *prev, *next, *cbox, *left, *center, *right;
  GtkWidget *lab;
  
  window->priv->color_button = color_button;
  window->priv->font_button = font_button;

  gtk_widget_set_sensitive (window->priv->font_button, FALSE);
  gtk_widget_set_sensitive (window->priv->color_button, FALSE);
  
  prev = gtk_button_new ();
  next = gtk_button_new ();
  left = gtk_button_new ();
  center = gtk_button_new ();
  right = gtk_button_new ();
  
  cbox = gtk_combo_box_new_text();
  window->priv->animation_box = cbox;
  
  lab = gtk_label_new ("");
  window->priv->slide_label = lab;
  //  glide_window_update_slide_label (window);

  glide_window_setup_animations_box (window, cbox);
  
  g_signal_connect (prev, "clicked", G_CALLBACK (glide_window_slide_prev), window);
  g_signal_connect (next, "clicked", G_CALLBACK (glide_window_slide_next), window);

  g_signal_connect (left, "clicked", G_CALLBACK (glide_window_align_left), window);
  g_signal_connect (right, "clicked", G_CALLBACK (glide_window_align_right), window);
  g_signal_connect (center, "clicked", G_CALLBACK (glide_window_align_center), window);
  
  g_signal_connect (color_button, "color-set", G_CALLBACK (glide_window_color_set_cb), window);
  g_signal_connect (font_button, "font-set", G_CALLBACK (glide_window_font_set_cb), window);
  
  gtk_button_set_image (GTK_BUTTON (prev),
			gtk_image_new_from_stock (GTK_STOCK_GO_BACK, GTK_ICON_SIZE_SMALL_TOOLBAR));
  gtk_button_set_image (GTK_BUTTON (next),
			gtk_image_new_from_stock (GTK_STOCK_GO_FORWARD, GTK_ICON_SIZE_SMALL_TOOLBAR));

  gtk_button_set_image (GTK_BUTTON (left),
			gtk_image_new_from_stock (GTK_STOCK_JUSTIFY_LEFT, GTK_ICON_SIZE_SMALL_TOOLBAR));
  gtk_button_set_image (GTK_BUTTON (center),
			gtk_image_new_from_stock (GTK_STOCK_JUSTIFY_CENTER, GTK_ICON_SIZE_SMALL_TOOLBAR));
  gtk_button_set_image (GTK_BUTTON (right),
			gtk_image_new_from_stock (GTK_STOCK_JUSTIFY_RIGHT, GTK_ICON_SIZE_SMALL_TOOLBAR));

  gtk_box_pack_start (GTK_BOX(hbox), prev, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX(hbox), lab, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX(hbox), next, FALSE, FALSE, 0);
  
  gtk_box_pack_start (GTK_BOX(hbox), color_button, FALSE, FALSE, 5);
  gtk_box_pack_start (GTK_BOX(hbox), font_button, FALSE, FALSE, 0);

  gtk_box_pack_start (GTK_BOX(hbox), left, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX(hbox), center, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX(hbox), right, FALSE, FALSE, 0);

  gtk_box_pack_start (GTK_BOX(hbox), cbox, TRUE, TRUE, 30);
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
glide_window_stage_manager_slide_changed_cb (GObject *object,
					     GParamSpec *pspec,
					     gpointer user_data)
{
  GlideWindow *w = (GlideWindow *) user_data;
  GlideSlide *slide = glide_document_get_nth_slide (w->priv->document,
						    glide_stage_manager_get_current_slide (w->priv->manager));
  const gchar *animation = glide_slide_get_animation (GLIDE_SLIDE (slide));
  
  glide_window_update_slide_label (w);

  if (!animation)
    animation = "None";

  if (!strcmp(animation, "None"))
    gtk_combo_box_set_active (GTK_COMBO_BOX (w->priv->animation_box), 0);
  if (!strcmp(animation, "Fade"))
    gtk_combo_box_set_active (GTK_COMBO_BOX (w->priv->animation_box), 1);
  if (!strcmp(animation, "Zoom"))
    gtk_combo_box_set_active (GTK_COMBO_BOX (w->priv->animation_box), 2);
  if (!strcmp(animation, "Drop"))
    gtk_combo_box_set_active (GTK_COMBO_BOX (w->priv->animation_box), 3);
  if (!strcmp(animation, "Pivot"))
    gtk_combo_box_set_active (GTK_COMBO_BOX (w->priv->animation_box), 4);
  if (!strcmp(animation, "Slide"))
    gtk_combo_box_set_active (GTK_COMBO_BOX (w->priv->animation_box), 5);

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
glide_window_stage_selection_changed_cb (GlideStageManager *manager,
					 GObject *old_selection,
					 gpointer data)
{
  GlideWindow *w = (GlideWindow *)data;
  GdkColor c;
  ClutterColor cc;
  GlideActor *selection = glide_stage_manager_get_selection (manager);

  if (!selection || !GLIDE_IS_TEXT (selection))
    {
      gtk_widget_set_sensitive (w->priv->font_button, FALSE);
      gtk_widget_set_sensitive (w->priv->color_button, FALSE);
    }

  
  if (selection && GLIDE_IS_TEXT (selection))
    {

      gtk_widget_set_sensitive (w->priv->font_button, TRUE);
      gtk_widget_set_sensitive (w->priv->color_button, TRUE);

      glide_text_get_color (GLIDE_TEXT (selection), &cc);
      c.red = (cc.red/255.0) * 65535;
      c.green = (cc.green/255.0) * 65535;
      c.blue = (cc.blue/255.0) * 65535;
      
      gtk_color_button_set_color (GTK_COLOR_BUTTON (w->priv->color_button), &c);
      gtk_font_button_set_font_name (GTK_FONT_BUTTON (w->priv->font_button),
				     glide_text_get_font_name (GLIDE_TEXT (selection)));
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
  g_signal_connect (window->priv->manager, "notify::current-slide",
		    G_CALLBACK (glide_window_stage_manager_slide_changed_cb),
		    window);
  g_signal_connect (window->priv->manager, "selection-changed",
		    G_CALLBACK (glide_window_stage_selection_changed_cb),
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
