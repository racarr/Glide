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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, 
 * Boston, MA 02111-1307, USA.
 */
 
#include <glib/gi18n.h>
#include "glide-image.h"


G_DEFINE_TYPE(GlideImage, glide_image, CLUTTER_TYPE_TEXTURE)

static void
glide_image_finalize (GObject *object)
{
  /* Debug? */

  G_OBJECT_CLASS (glide_image_parent_class)->finalize (object);
}

static void
glide_image_class_init (GlideImageClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  
  object_class->finalize = glide_image_finalize;
}

static void
glide_image_init (GlideImage *image)
{
}

GlideImage *
glide_image_new ()
{
  return g_object_new (GLIDE_TYPE_IMAGE, "filename", "/home/racarr/surprise.jpg", NULL);
}
