/*
 * glide-image.h
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
 * MERCHANMANIPULATORILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, 
 * Boston, MA 02111-1307, USA.
 */

#ifndef __GLIDE_IMAGE_H__
#define __GLIDE_IMAGE_H__

#include <glib-object.h>
#include <clutter/clutter.h>
#include "glide-actor.h"


G_BEGIN_DECLS

#define GLIDE_TYPE_IMAGE                  (glide_image_get_type())
#define GLIDE_IMAGE(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GLIDE_TYPE_IMAGE, GlideImage))
#define GLIDE_IMAGE_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), GLIDE_TYPE_IMAGE, GlideImageClass))
#define GLIDE_IS_IMAGE(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GLIDE_TYPE_IMAGE))
#define GLIDE_IS_IMAGE_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), GLIDE_TYPE_IMAGE))
#define GLIDE_IMAGE_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), GLIDE_TYPE_IMAGE, GlideImageClass))

typedef struct _GlideImage        GlideImage;
typedef struct _GlideImageClass   GlideImageClass;
typedef struct _GlideImagePrivate GlideImagePrivate;

struct _GlideImage
{
  GlideActor           parent;

  GlideImagePrivate *priv;
}; 

struct _GlideImageClass 
{
  /*< private >*/
  GlideActorClass parent_class;
};

GType glide_image_get_type (void) G_GNUC_CONST;

ClutterActor *glide_image_new              (GlideStageManager *manager);
ClutterActor *glide_image_new_from_file    (GlideStageManager *manager, const gchar *filename, GError **error);

gboolean glide_image_set_from_file         (GlideImage *image, const gchar *filename, GError **error);
void glide_image_set_cogl_texture          (GlideImage *image, CoglHandle new_texture);

G_END_DECLS

#endif /* __CLUTTER_IMAGE_H__ */
