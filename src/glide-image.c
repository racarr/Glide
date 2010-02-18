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
  
  cogl_material_set_color4ub (priv->material, paint_opacity, paint_opacity, paint_opacity, paint_opacity);
  clutter_actor_get_allocation_box (self, &box);
  
  t_w = 1.0;
  t_h = 1.0;
  
  cogl_set_source (priv->material);
  cogl_rectangle_with_texture_coords (0, 0,
				      box.x2 - box.x1, box.y2 - box.y1,
				      0, 0, t_w, t_h);
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
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  
  actor_class->paint = glide_image_paint;
  actor_class->button_press_event = glide_image_button_press;
  
  g_type_class_add_private (object_class, sizeof(GlideImagePrivate));
}

static void
glide_image_init (GlideImage *self)
{
  self->priv = GLIDE_IMAGE_GET_PRIVATE (self);
  
  self->priv->material = cogl_material_new();
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
  
  /* TODO: Free old texture */
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
    return image;
}
