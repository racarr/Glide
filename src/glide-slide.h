/*
 * glide-slide.h
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

#ifndef __GLIDE_SLIDE_H__
#define __GLIDE_SLIDE_H__

#include <glib-object.h>
#include <clutter/clutter.h>

#include "glide-actor.h"

G_BEGIN_DECLS

#define GLIDE_TYPE_SLIDE                  (glide_slide_get_type())
#define GLIDE_SLIDE(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GLIDE_TYPE_SLIDE, GlideSlide))
#define GLIDE_SLIDE_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), GLIDE_TYPE_SLIDE, GlideSlideClass))
#define GLIDE_IS_SLIDE(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GLIDE_TYPE_SLIDE))
#define GLIDE_IS_SLIDE_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), GLIDE_TYPE_SLIDE))
#define GLIDE_SLIDE_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), GLIDE_TYPE_SLIDE, GlideSlideClass))

typedef struct _GlideSlideClass   GlideSlideClass;
typedef struct _GlideSlidePrivate GlideSlidePrivate;

struct _GlideSlide
{
  GlideActor parent;
  
  GlideSlidePrivate *priv;
}; 

struct _GlideSlideClass 
{
  GlideActorClass parent_class;
};

GType glide_slide_get_type (void) G_GNUC_CONST;

GlideSlide *glide_slide_new  (GlideDocument *document);

void glide_slide_construct_from_json (GlideSlide *slide, JsonObject *slide_obj, GlideStageManager *manager);

void glide_slide_set_background (GlideSlide *slide, const gchar *background);
const gchar *glide_slide_get_background (GlideSlide *slide);

G_END_DECLS

#endif /* __GLIDE_SLIDE_H__ */
