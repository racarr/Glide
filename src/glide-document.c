/*
 * glide-document.c
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
 
#include "glide-document.h"
#include "glide-document-priv.h"

#include <girepository.h>

#include "glide-debug.h"

#include "glide-slide.h"

G_DEFINE_TYPE(GlideDocument, glide_document, G_TYPE_OBJECT)

#define GLIDE_DOCUMENT_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GLIDE_TYPE_DOCUMENT, GlideDocumentPrivate))

enum {
  PROP_0,
  PROP_NAME,
  PROP_PATH,
  PROP_WIDTH,
  PROP_HEIGHT,
  PROP_DIRTY
};

#define DEFAULT_PRESENTATION_WIDTH 800
#define DEFAULT_PRESENTATION_HEIGHT 600

enum {
  SLIDE_ADDED,
  SLIDE_REMOVED,
  RESIZED,
  LAST_SIGNAL
};

static guint document_signals[LAST_SIGNAL] = { 0, };

static void
glide_document_finalize (GObject *object)
{
  GlideDocument *document = GLIDE_DOCUMENT (object);

  GLIDE_NOTE (DOCUMENT, "Finalizing document: %s",
	      document->priv->name);
  
  g_free (document->priv->name);

  G_OBJECT_CLASS (glide_document_parent_class)->finalize (object);
}


static void
glide_document_get_property (GObject *object,
				  guint prop_id,
				  GValue *value,
				  GParamSpec *pspec)
{
  GlideDocument *document = GLIDE_DOCUMENT (object);
  
  switch (prop_id)
    {
    case PROP_NAME:
      g_value_set_string (value, document->priv->name);
      break;
    case PROP_PATH:
      g_value_set_string (value, document->priv->path);
      break;
    case PROP_WIDTH:
      g_value_set_int (value, document->priv->width);
      break;
    case PROP_HEIGHT:
      g_value_set_int (value, document->priv->height);
      break;
    case PROP_DIRTY:
      g_value_set_boolean (value, document->priv->dirty);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
glide_document_set_property (GObject *object,
			     guint prop_id,
			     const GValue *value,
			     GParamSpec *pspec)
{
  GlideDocument *document = GLIDE_DOCUMENT (object);
  
  switch (prop_id)
    {
    case PROP_NAME:
      g_return_if_fail (document->priv->name == NULL);
      document->priv->name = g_value_dup_string (value);
      GLIDE_NOTE (DOCUMENT, "Constructing new GlideDocument (%p): %s",
		  object, document->priv->name);
      break;
    case PROP_WIDTH:
      document->priv->width = g_value_get_int (value);
      break;
    case PROP_HEIGHT:
      document->priv->height = g_value_get_int (value);
      break;
    case PROP_PATH:
      if (document->priv->path)
	g_free (document->priv->path);
      document->priv->path = g_value_dup_string (value);
      break;
    case PROP_DIRTY:
      glide_document_set_dirty (document, g_value_get_boolean (value));
      break;
    default: 
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
glide_document_class_init (GlideDocumentClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  
  object_class->finalize = glide_document_finalize;
  object_class->set_property = glide_document_set_property;
  object_class->get_property = glide_document_get_property;

  g_object_class_install_property (object_class, PROP_NAME,
				   g_param_spec_string ("name",
							"Name",
							"Name of the document",
							NULL,
							G_PARAM_READWRITE |
							G_PARAM_STATIC_STRINGS |
							G_PARAM_CONSTRUCT_ONLY));
  g_object_class_install_property (object_class, PROP_PATH,
				   g_param_spec_string ("path",
							"Path",
							"Path of the document",
							NULL,
							G_PARAM_READWRITE |
							G_PARAM_STATIC_STRINGS |
							G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_WIDTH,
				   g_param_spec_int ("width",
						     "Width",
						     "Width of the document",
						     -1, G_MAXINT, DEFAULT_PRESENTATION_WIDTH,
						     G_PARAM_READWRITE |
						     G_PARAM_STATIC_STRINGS |
						     G_PARAM_CONSTRUCT));

  g_object_class_install_property (object_class, PROP_HEIGHT,
				   g_param_spec_int ("height",
						     "Height",
						     "Height of the document",
						     -1, G_MAXINT, DEFAULT_PRESENTATION_HEIGHT,
						     G_PARAM_READWRITE |
						     G_PARAM_STATIC_STRINGS |
						     G_PARAM_CONSTRUCT));
  
  g_object_class_install_property (object_class, PROP_DIRTY,
				   g_param_spec_boolean ("dirty",
							 "Dirty",
							 "Whether the document has dirty changes",
							 FALSE,
							 G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

  document_signals[SLIDE_ADDED] = 
    g_signal_new ("slide-added",
		  G_TYPE_FROM_CLASS (object_class),
		  G_SIGNAL_RUN_LAST,
		  0,
		  NULL, NULL,
		  gi_cclosure_marshal_generic,
		  G_TYPE_NONE, 1,
		  G_TYPE_OBJECT);

  document_signals[SLIDE_REMOVED] = 
    g_signal_new ("slide-removed",
		  G_TYPE_FROM_CLASS (object_class),
		  G_SIGNAL_RUN_LAST,
		  0,
		  NULL, NULL,
		  gi_cclosure_marshal_generic,
		  G_TYPE_NONE, 1,
		  G_TYPE_OBJECT);
  
  document_signals[RESIZED] = 
    g_signal_new ("resized",
		  G_TYPE_FROM_CLASS (object_class),
		  G_SIGNAL_RUN_LAST,
		  0,
		  NULL, NULL,
		  gi_cclosure_marshal_generic,
		  G_TYPE_NONE, 0, NULL);
							
  
  g_type_class_add_private (object_class, sizeof(GlideDocumentPrivate));
}

static void
glide_document_init (GlideDocument *d)
{
  d->priv = GLIDE_DOCUMENT_GET_PRIVATE (d);
  
  //  glide_document_add_slide (d);
}

GlideDocument *
glide_document_new (const gchar *name)
{
  return g_object_new (GLIDE_TYPE_DOCUMENT,
		       "name", name,
		       NULL);
}

const gchar *
glide_document_get_name (GlideDocument *document)
{
  return document->priv->name;
}

const gchar *
glide_document_get_path (GlideDocument *document)
{
  return document->priv->path;
}

void
glide_document_set_path (GlideDocument *document, const gchar *path)
{
  document->priv->path = g_strdup (path);
  g_object_notify (G_OBJECT (document), "path");
}

guint
glide_document_get_n_slides (GlideDocument *document)
{
  return g_list_length (document->priv->slides);
}

GlideSlide *
glide_document_get_nth_slide (GlideDocument *document,
			      guint n)
{
  return GLIDE_SLIDE (g_list_nth_data (document->priv->slides, n));
}

GlideSlide *
glide_document_append_slide (GlideDocument *document)
{
  GlideSlide *s = glide_slide_new (document);
  document->priv->slides = g_list_append (document->priv->slides, s);
  
  g_signal_emit (document, document_signals[SLIDE_ADDED], 0, s);
  
  return s;
}

GlideSlide *
glide_document_insert_slide (GlideDocument *document, gint after)
{
  GlideSlide *s = glide_slide_new (document);
  document->priv->slides = g_list_insert (document->priv->slides, s, after+1);
  
  g_signal_emit (document, document_signals[SLIDE_ADDED], 0, s);
  
  return s;
}

void
glide_document_remove_slide (GlideDocument *document, gint slide)
{
  GlideSlide *s = g_list_nth_data (document->priv->slides, slide);
  document->priv->slides = g_list_remove (document->priv->slides, s);

  g_signal_emit (document, document_signals[SLIDE_REMOVED], 0, s);
}

static void
glide_document_json_obj_set_name (GlideDocument *document, JsonObject *obj)
{
  JsonNode *node = json_node_new (JSON_NODE_VALUE);
  json_node_set_string (node, document->priv->name);
  
  json_object_set_member (obj, "name", node);
}

static void
glide_document_json_obj_set_slides (GlideDocument *document, JsonObject *obj)
{
  JsonNode *node = json_node_new (JSON_NODE_ARRAY);
  JsonArray *array = json_array_new ();
  GList *s;
  
  for (s = document->priv->slides; s; s = s->next)
    {
      JsonNode *n;
      GlideSlide *slide = (GlideSlide *)(s->data);
      
      n = glide_actor_serialize (GLIDE_ACTOR (slide));
      json_array_add_element (array, n);
    }
  json_node_take_array (node, array);

  json_object_set_member (obj, "slides", node);
  
}

JsonNode *
glide_document_serialize(GlideDocument *document)
{
  JsonNode *node = json_node_new (JSON_NODE_OBJECT);
  JsonObject *obj;
  
  obj = json_object_new ();
  json_node_set_object (node, obj);
  
  glide_document_json_obj_set_name (document, obj);
  glide_document_json_obj_set_slides (document, obj);
  
  return node;
}

gint 
glide_document_get_height (GlideDocument *document)
{
  return document->priv->height;
}

gint 
glide_document_get_width (GlideDocument *document)
{
  return document->priv->width;
}

void
glide_document_get_size (GlideDocument *document, gint *width, gint *height)
{
  *width = document->priv->width;
  *height = document->priv->height;
}

void
glide_document_resize (GlideDocument *document, gint width, gint height)
{
  GList *s;

  document->priv->width = width;
  document->priv->height = height;

  g_signal_emit (document, document_signals[RESIZED], 0);  

  for (s=document->priv->slides; s; s=s->next)
    {
            GlideSlide *slide = (GlideSlide *)s->data;

	    glide_slide_resize (slide, width, height);
    }
}


gboolean
glide_document_get_dirty (GlideDocument *d)
{
  return d->priv->dirty;
}

void
glide_document_set_dirty (GlideDocument *d, gboolean dirty)
{
  d->priv->dirty = dirty;
  g_object_notify (G_OBJECT (d), "dirty");
}
