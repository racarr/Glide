/*
 * glide-document.h
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

#ifndef __GLIDE_DOCUMENT_H__
#define __GLIDE_DOCUMENT_H__

#include <glib.h>
#include <glib-object.h>

typedef struct _GlideDocument GlideDocument;

#include "glide-slide.h"


G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define GLIDE_TYPE_DOCUMENT              (glide_document_get_type())
#define GLIDE_DOCUMENT(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), GLIDE_TYPE_DOCUMENT, GlideDocument))
#define GLIDE_DOCUMENT_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), GLIDE_TYPE_DOCUMENT, GlideDocumentClass))
#define GLIDE_IS_DOCUMENT(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), GLIDE_TYPE_DOCUMENT))
#define GLIDE_IS_DOCUMENT_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GLIDE_TYPE_DOCUMENT))
#define GLIDE_DOCUMENT_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), GLIDE_TYPE_DOCUMENT, GlideDocumentClass))

/* Private structure type */
typedef struct _GlideDocumentPrivate GlideDocumentPrivate;

/*
 * Main object structure
 */


struct _GlideDocument 
{
  GObject object;
  
  GlideDocumentPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _GlideDocumentClass GlideDocumentClass;

struct _GlideDocumentClass 
{
  GObjectClass parent_class;
};

/*
 * Public methods
 */
GType 		 glide_document_get_type 			(void) G_GNUC_CONST;

GlideDocument   *glide_document_new (const gchar *name);

const gchar     *glide_document_get_name (GlideDocument *document);

gint             glide_document_get_n_slides (GlideDocument *document);
GlideSlide      *glide_document_get_nth_slide (GlideDocument *document, gint n);



G_END_DECLS

#endif  /* __GLIDE_DOCUMENT_H__  */
