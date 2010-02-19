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
#include "glide-image-priv.h"

#include "glide-debug.h"

G_DEFINE_TYPE (GlideImage, glide_image, GLIDE_TYPE_ACTOR);

#define GLIDE_IMAGE_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GLIDE_TYPE_IMAGE, GlideImagePrivate))

static void
glide_image_paint (ClutterActor *self)
{
  GlideImage *image = GLIDE_IMAGE (self);
  GlideImagePrivate *priv = image->priv;
  ClutterActorBox box = {0, };
  gfloat t_w, t_h;
  guint8 paint_opacity = clutter_actor_get_paint_opacity (self);

  if (paint_opacity == 0)
    {
      return;
    }
  
  GLIDE_NOTE (PAINT,
	      "painting image '%s'",
	      GLIDE_ACTOR_DISPLAY_NAME (self));
  
  cogl_material_set_color4ub (priv->material, paint_opacity, paint_opacity, paint_opacity, paint_opacity);
  clutter_actor_get_allocation_box (self, &box);
  
  GLIDE_NOTE (PAINT, "paint to x1: %f, y1: %f x2: %f, y2: %f "
	      "opacity: %i",
	      box.x1, box.y1, box.x2, box.y2,
	      clutter_actor_get_opacity (self));
  
  t_w = 1.0;
  t_h = 1.0;
  
  cogl_set_source (priv->material);
  cogl_rectangle_with_texture_coords (0, 0,
				      box.x2 - box.x1, box.y2 - box.y1,
				      0, 0, t_w, t_h);
}

static void
glide_image_get_preferred_width (ClutterActor *self,
				 gfloat for_height,
				 gfloat *min_width_p,
				 gfloat *natural_width_p)
{
  GlideImage *image = GLIDE_IMAGE (self);
  
  if (min_width_p)
    *min_width_p = 0;
  
  *natural_width_p = image->priv->image_width;
}

static void
glide_image_get_preferred_height (ClutterActor *self,
				  gfloat for_width,
				  gfloat *min_height_p,
				  gfloat *natural_height_p)
{
  GlideImage *image = GLIDE_IMAGE (self);
  
  if (min_height_p)
    *min_height_p = 0;
  
  *natural_height_p = image->priv->image_height;
}

static void
image_free_gl_resources (GlideImage *image)
{
  if (image->priv->material != COGL_INVALID_HANDLE)
    cogl_material_set_layer (image->priv->material, 0, COGL_INVALID_HANDLE);
}

static gboolean
glide_image_button_press (ClutterActor *actor,
			  ClutterButtonEvent *event)
{
  GlideStageManager *m;
  GlideActor *ga = GLIDE_ACTOR (actor);
  GlideImage *image = GLIDE_IMAGE (actor);
  gfloat ax, ay;
  
  m = glide_actor_get_stage_manager (ga);
  
  glide_stage_manager_set_selection (m, actor);

  clutter_actor_get_position (actor, &ax, &ay);

  image->priv->drag_center_x = event->x - ax;
  image->priv->drag_center_y = event->y - ay;
  image->priv->dragging = TRUE;
  
  clutter_grab_pointer (actor);
  
  return TRUE;
}

static gboolean
glide_image_button_release (ClutterActor *actor,
			    ClutterButtonEvent *bev)
{
  GlideImage *image = GLIDE_IMAGE (actor);
  if (image->priv->dragging)
    {
      clutter_ungrab_pointer ();
      image->priv->dragging = FALSE;
      
      return TRUE;
    }
  
  return FALSE;
}

static gboolean
glide_image_motion (ClutterActor *actor,
		    ClutterMotionEvent *mev)
{
  GlideImage *image = GLIDE_IMAGE (actor);
  
  if (image->priv->dragging)
    {
      clutter_actor_set_position (actor,
				  mev->x - image->priv->drag_center_x,
				  mev->y - image->priv->drag_center_y);
      
      return TRUE;
    }
  
  return FALSE;
}

static void
glide_image_finalize (GObject *object)
{
  GlideImage *image = GLIDE_IMAGE (object);

  GLIDE_NOTE (IMAGE,
	      "finalizing image '%s'",
	      GLIDE_ACTOR_DISPLAY_NAME (CLUTTER_ACTOR (object)));
  
  if (image->priv->material != COGL_INVALID_HANDLE)
    {
      cogl_handle_unref (image->priv->material);
      image->priv->material = COGL_INVALID_HANDLE;
    }
  
  G_OBJECT_CLASS (glide_image_parent_class)->finalize (object);
}

static void
glide_image_class_init (GlideImageClass *klass)
{
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  
  actor_class->paint = glide_image_paint;
  actor_class->button_press_event = glide_image_button_press;
  actor_class->button_release_event = glide_image_button_release;
  actor_class->motion_event = glide_image_motion;
  
  actor_class->get_preferred_width = glide_image_get_preferred_width;
  actor_class->get_preferred_height = glide_image_get_preferred_height;

  
  object_class->finalize = glide_image_finalize;
  g_type_class_add_private (object_class, sizeof(GlideImagePrivate));
}

static void
glide_image_init (GlideImage *self)
{
  self->priv = GLIDE_IMAGE_GET_PRIVATE (self);
  
  self->priv->material = cogl_material_new ();
  
  cogl_material_set_layer_filters (self->priv->material, 0,
				   COGL_MATERIAL_FILTER_LINEAR_MIPMAP_LINEAR,
				   COGL_MATERIAL_FILTER_LINEAR);
}

ClutterActor*
glide_image_new (GlideStageManager *m)
{
  return g_object_new (GLIDE_TYPE_IMAGE, 
		       "stage-manager", m,
		       NULL);
}

void
glide_image_set_cogl_texture (GlideImage *image,
			      CoglHandle new_texture)
{
  guint width, height;
  
  width = cogl_texture_get_width (new_texture);
  height = cogl_texture_get_height (new_texture);
  
  cogl_handle_ref (new_texture);
  
  image_free_gl_resources (image);
  
  cogl_material_set_layer (image->priv->material, 0, new_texture);
  
  image->priv->image_width = width;
  image->priv->image_height = height;
  
  cogl_handle_unref (new_texture);
}

gboolean
glide_image_set_from_file (GlideImage *image,
			   const gchar *filename,
			   GError **error)
{
  GlideImagePrivate *priv;
  CoglHandle new_texture = COGL_INVALID_HANDLE;
  GError *internal_error = NULL;
  CoglTextureFlags flags = COGL_TEXTURE_NONE;
  
  priv = image->priv;
  
  new_texture = cogl_texture_new_from_file (filename,
					    flags,
					    COGL_PIXEL_FORMAT_ANY,
					    &internal_error);
  
  if (internal_error == NULL && new_texture == COGL_INVALID_HANDLE)
    {
      g_set_error (&internal_error, CLUTTER_TEXTURE_ERROR,
		   CLUTTER_TEXTURE_ERROR_BAD_FORMAT,
		   "Failed to create COGL texture");
    }
  if (internal_error != NULL)
    {
      g_propagate_error (error, internal_error);
      
      return FALSE;
    }
  
  glide_image_set_cogl_texture (image, new_texture);
  
  cogl_handle_unref (new_texture);
  
  return TRUE;
}

ClutterActor *
glide_image_new_from_file (GlideStageManager *m, 
			   const gchar *filename, 
			   GError **error)
{
  ClutterActor *image = glide_image_new (m);
  
  if (!glide_image_set_from_file (GLIDE_IMAGE (image),
				  filename, error))
    {
      g_object_ref_sink (image);
      g_object_unref (image);
      
      return NULL;
    }
  else
    {
      clutter_actor_queue_relayout (image);
      return image;
    }
}
