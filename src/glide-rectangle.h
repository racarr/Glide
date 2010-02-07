/*
 * glide-rectangle.h
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

#ifndef __GLIDE_RECTANGLE_H__
#define __GLIDE_RECTANGLE_H__

#include <glib-object.h>
#include <clutter/clutter.h>
#include "glide-actor.h"


G_BEGIN_DECLS

#define GLIDE_TYPE_RECTANGLE                  (glide_rectangle_get_type())
#define GLIDE_RECTANGLE(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GLIDE_TYPE_RECTANGLE, GlideRectangle))
#define GLIDE_RECTANGLE_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), GLIDE_TYPE_RECTANGLE, GlideRectangleClass))
#define GLIDE_IS_RECTANGLE(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GLIDE_TYPE_RECTANGLE))
#define GLIDE_IS_RECTANGLE_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), GLIDE_TYPE_RECTANGLE))
#define GLIDE_RECTANGLE_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), GLIDE_TYPE_RECTANGLE, GlideRectangleClass))

typedef struct _GlideRectangle        GlideRectangle;
typedef struct _GlideRectangleClass   GlideRectangleClass;

struct _GlideRectangle
{
  GlideActor           parent;

  ClutterRectanglePrivate *priv;
}; 

struct _GlideRectangleClass 
{
  /*< private >*/
  GlideActorClass parent_class;
};

GType glide_rectangle_get_type (void) G_GNUC_CONST;

ClutterActor *glide_rectangle_new              (void);

G_END_DECLS

#endif /* __CLUTTER_RECTANGLE_H__ */
