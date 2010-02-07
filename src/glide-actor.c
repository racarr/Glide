/*
 * glide-actor.c
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
#include "glide-actor.h"

#include <math.h>

#include "glide-actor-priv.h"


G_DEFINE_ABSTRACT_TYPE(GlideActor, glide_actor, CLUTTER_TYPE_ACTOR)

#define GLIDE_ACTOR_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GLIDE_TYPE_ACTOR, GlideActorPrivate))

/*enum {
  PROP_0,
  PROP_TARGET
  };*/

static void
glide_actor_finalize (GObject *object)
{
  /* Debug? */

  G_OBJECT_CLASS (glide_actor_parent_class)->finalize (object);
}

/*
static void
glide_actor_get_property (GObject *object,
				guint prop_id,
				GValue *value,
				GParamSpec *pspec)
{
  GlideActor *manip = GLIDE_ACTOR (object);
  
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
glide_actor_set_property (GObject *object,
				guint prop_id,
				const GValue *value,
				GParamSpec *pspec)
{
  GlideActor *manip = GLIDE_ACTOR (object);
  
  switch (prop_id)
    {
    case PROP_TARGET:
      g_return_if_fail (manip->priv->target == NULL);
      glide_actor_set_target_real (manip, CLUTTER_ACTOR(g_value_get_object (value)));
      break;
    default: 
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
    }*/

static void
glide_actor_class_init (GlideActorClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  
  /*  object_class->finalize = glide_actor_finalize;
  object_class->set_property = glide_actor_set_property;
  object_class->get_property = glide_actor_get_property;
  
  g_object_class_install_property (object_class,
				   PROP_TARGET,
				   g_param_spec_object ("target",
							"Target",
							"The target of the actor object",
							CLUTTER_TYPE_ACTOR,
							G_PARAM_READWRITE |
							G_PARAM_CONSTRUCT_ONLY |
							G_PARAM_STATIC_STRINGS));
  */
  g_type_class_add_private (object_class, sizeof(GlideActorPrivate));
}

static void
glide_actor_init (GlideActor *actor)
{
}

