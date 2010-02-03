/*
 * glide-manipulator.c
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
#include "glide-manipulator.h"

#include "glide-manipulator-priv.h"


G_DEFINE_TYPE(GlideManipulator, glide_manipulator, CLUTTER_TYPE_GROUP)

#define GLIDE_MANIPULATOR_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GLIDE_TYPE_MANIPULATOR, GlideManipulatorPrivate))


static void
glide_manipulator_finalize (GObject *object)
{
  /* Debug? */

  G_OBJECT_CLASS (glide_manipulator_parent_class)->finalize (object);
}

static void
glide_manipulator_paint (ClutterActor *self)
{
  ClutterGeometry geom;
  guint n_children, i;
  
  n_children = clutter_group_get_n_children (CLUTTER_GROUP (self));
  
  for (i = 0; i < n_children; i++)
    {
      ClutterActor *c = clutter_group_get_nth_child (CLUTTER_GROUP (self), i);
      
      clutter_actor_paint (c);
    }
  
  clutter_actor_get_allocation_geometry (self, &geom);
  
  cogl_set_source_color4ub (0xcc, 0xcc, 0xcc, 0xff);
  
  cogl_rectangle (2.5, 0, geom.width, 2.5);
  cogl_rectangle (geom.width - 2.5, 2.5, geom.width, geom.height);
  cogl_rectangle (0, geom.height - 2.5, geom.width - 2.5, geom.height);
  cogl_rectangle (0, 0, 2.5, geom.height - 2.5);
  
  cogl_set_source_color4ub (0xcc, 0xcc, 0xff, 0xff);
  
  cogl_rectangle (-4, -4, 4, 4);
  cogl_rectangle (-4, -4+geom.height, 4, 4+geom.height);
  cogl_rectangle (-4+geom.width, -4+geom.height, 4+geom.width, 4+geom.height);
  cogl_rectangle (-4+geom.width, -4, 4+geom.width, 4);
}

static void
glide_manipulator_class_init (GlideManipulatorClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  
  actor_class->paint = glide_manipulator_paint;

  object_class->finalize = glide_manipulator_finalize;
  
  g_type_class_add_private (object_class, sizeof(GlideManipulatorPrivate));
}

static void
glide_manipulator_init (GlideManipulator *manipulator)
{
  manipulator->priv = GLIDE_MANIPULATOR_GET_PRIVATE (manipulator);
}

GlideManipulator *
glide_manipulator_new ()
{
  return g_object_new (GLIDE_TYPE_MANIPULATOR, NULL);
}
