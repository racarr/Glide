/*
 * glide-stage-manager.c
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
#include "glide-stage-manager.h"

#include <math.h>

#include "glide-stage-manager-priv.h"


G_DEFINE_ABSTRACT_TYPE(GlideStageManager, glide_stage_manager, G_TYPE_OBJECT)

#define GLIDE_STAGE_MANAGER_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GLIDE_TYPE_STAGE_MANAGER, GlideStageManagerPrivate))

/*enum {
  PROP_0,
  PROP_TARGET
  };*/

static void
glide_stage_manager_finalize (GObject *object)
{
  /* Debug? */

  G_OBJECT_CLASS (glide_stage_manager_parent_class)->finalize (object);
}

/*
static void
glide_stage_manager_get_property (GObject *object,
				guint prop_id,
				GValue *value,
				GParamSpec *pspec)
{
  GlideStageManager *manip = GLIDE_STAGE_MANAGER (object);
  
  switch (prop_id)
    {
    case PROP_TARGET:
      g_value_set_object (value, manip->priv->target);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
    }*/

/*static void
glide_stage_manager_set_property (GObject *object,
				guint prop_id,
				const GValue *value,
				GParamSpec *pspec)
{
  GlideStageManager *manip = GLIDE_STAGE_MANAGER (object);
  
  switch (prop_id)
    {
    case PROP_TARGET:
      g_return_if_fail (manip->priv->target == NULL);
      glide_stage_manager_set_target_real (manip, CLUTTER_STAGE_MANAGER(g_value_get_object (value)));
      break;
    default: 
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
    }*/

static void
glide_stage_manager_class_init (GlideStageManagerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  
  /*  object_class->finalize = glide_stage_manager_finalize;
  object_class->set_property = glide_stage_manager_set_property;
  object_class->get_property = glide_stage_manager_get_property;
  
  g_object_class_install_property (object_class,
				   PROP_TARGET,
				   g_param_spec_object ("target",
							"Target",
							"The target of the actor object",
							CLUTTER_TYPE_STAGE_MANAGER,
							G_PARAM_READWRITE |
							G_PARAM_CONSTRUCT_ONLY |
							G_PARAM_STATIC_STRINGS));
  */
  g_type_class_add_private (object_class, sizeof(GlideStageManagerPrivate));
}

static void
glide_stage_manager_init (GlideStageManager *manager)
{
}

