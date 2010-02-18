/*
 * glide-image.c
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

#include "glide-image.h"

G_DEFINE_TYPE (GlideImage, glide_image, GLIDE_TYPE_ACTOR);

static void
glide_image_paint (ClutterActor *self)
{
  ClutterGeometry          geom;

  clutter_actor_get_allocation_geometry (self, &geom);

  cogl_set_source_color4ub (0xff, 0x00, 0x00, 0xff);

  
  cogl_rectangle (10, 0,
		  geom.width,
		  10);
  
  cogl_rectangle (geom.width - 10,
		  10,
		  geom.width,
		  geom.height);
  
  cogl_rectangle (0, geom.height - 10,
		  geom.width - 10,
		  geom.height);
  
  cogl_rectangle (0, 0,
		  10,
		  geom.height - 10);
  
  
  cogl_set_source_color4ub (0x00, 0xff, 0x00, 0xff);

  cogl_rectangle (10, 10,
		  geom.width - 10,
		  geom.height - 10);


}

static gboolean
glide_image_button_press (ClutterActor *actor,
			      ClutterButtonEvent *event)
{
  GlideStageManager *m;
  GlideActor *ga = GLIDE_ACTOR (actor);
  
  m = glide_actor_get_stage_manager (ga);
  
  if (event->button != 1)
    return FALSE;
  
  glide_stage_manager_set_selection (m, actor);
  return TRUE;
}

static void
glide_image_class_init (GlideImageClass *klass)
{
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  
  actor_class->paint = glide_image_paint;
  actor_class->button_press_event = glide_image_button_press;
}

static void
glide_image_init (GlideImage *self)
{
}

ClutterActor*
glide_image_new (GlideStageManager *m)
{
  return g_object_new (GLIDE_TYPE_IMAGE, 
		       "stage-manager", m,
		       NULL);
}

