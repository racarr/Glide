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

#include "glide-debug.h"

G_DEFINE_TYPE(GlideDocument, glide_document, G_TYPE_OBJECT)

#define GLIDE_DOCUMENT_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GLIDE_TYPE_DOCUMENT, GlideDocumentPrivate))

enum {
  PROP_0,
  PROP_NAME
};

static void
glide_document_finalize (GObject *object)
{
  GlideDocument *document = GLIDE_DOCUMENT (object);

  GLIDE_NOTE (DOCUMENT, "Finalizing document: %s",
	      document->priv->name);

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
							
  
  g_type_class_add_private (object_class, sizeof(GlideDocumentPrivate));
}

static void
glide_document_init (GlideDocument *d)
{
  d->priv = GLIDE_DOCUMENT_GET_PRIVATE (d);
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

