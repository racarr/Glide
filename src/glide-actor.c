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

#include <girepository.h>
#include <math.h>

#include "glide-actor-priv.h"
#include "glide-json-util.h"

#include "glide-image.h"
#include "glide-text.h"


G_DEFINE_ABSTRACT_TYPE(GlideActor, glide_actor, CLUTTER_TYPE_ACTOR)

#define GLIDE_ACTOR_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GLIDE_TYPE_ACTOR, GlideActorPrivate))

enum {
  PROP_0,
  PROP_STAGE_MANAGER,
  PROP_SELECTED
  };

enum {
  SELECTED,
  DESELECTED,
  LAST_SIGNAL
};

static guint actor_signals[LAST_SIGNAL] = { 0, };

static void
glide_actor_finalize (GObject *object)
{
  /* Debug? */

  G_OBJECT_CLASS (glide_actor_parent_class)->finalize (object);
}


static void
glide_actor_get_property (GObject *object,
				guint prop_id,
				GValue *value,
				GParamSpec *pspec)
{
  GlideActor *actor = GLIDE_ACTOR (object);
  
  switch (prop_id)
    {
    case PROP_STAGE_MANAGER:
      g_value_set_object (value, actor->priv->manager);
      break;
    case PROP_SELECTED:
      g_value_set_boolean (value, actor->priv->selected);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
glide_actor_selection_changed_callback (GlideStageManager *manager,
					GObject *old_selection,
					gpointer data)
{
  GlideActor *this = (GlideActor *)data;
  
  if (((GlideActor *)glide_stage_manager_get_selection (manager) != this) &&
      (this->priv->selected == TRUE))
    {
      this->priv->selected = FALSE;
      
      g_object_notify (G_OBJECT (this), "selected");
      g_signal_emit (this, actor_signals[DESELECTED], 0);
    }
  if ((GlideActor *)glide_stage_manager_get_selection (manager) == this)
    {
      this->priv->selected = TRUE;

      g_object_notify (G_OBJECT (this), "selected");
      g_signal_emit (this, actor_signals[SELECTED], 0);
    }
}

static void
glide_actor_set_stage_manager_real (GlideActor *actor,
				    GlideStageManager *manager)
{
  g_return_if_fail (actor->priv->manager == NULL);
  actor->priv->manager = manager;

  g_signal_connect (actor->priv->manager, "selection-changed",
		    G_CALLBACK (glide_actor_selection_changed_callback),
		    actor);
  g_object_notify (G_OBJECT (actor), "stage-manager");
}

static void
glide_actor_set_property (GObject *object,
			  guint prop_id,
			  const GValue *value,
			  GParamSpec *pspec)
{
  GlideActor *actor = GLIDE_ACTOR (object);
  
  switch (prop_id)
    {
    case PROP_STAGE_MANAGER:
      glide_actor_set_stage_manager_real (actor, g_value_get_object (value));
      break;
    default: 
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
glide_actor_class_init (GlideActorClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  
  object_class->finalize = glide_actor_finalize;
  object_class->set_property = glide_actor_set_property;
  object_class->get_property = glide_actor_get_property;
  
  g_object_class_install_property (object_class,
				   PROP_STAGE_MANAGER,
				   g_param_spec_object ("stage-manager",
							"Stage manager",
							"The stage manager responsible for the actor.",
							GLIDE_TYPE_STAGE_MANAGER,
							G_PARAM_READWRITE |
							G_PARAM_STATIC_STRINGS));
  
  g_object_class_install_property (object_class,
				   PROP_SELECTED,
				   g_param_spec_boolean("selected",
							"Selected",
							"Whether the glide actor is selected",
							FALSE,
							G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
  
  actor_signals[SELECTED] = 
    g_signal_new ("selected",
		  G_TYPE_FROM_CLASS (object_class),
		  G_SIGNAL_RUN_FIRST,
		  G_STRUCT_OFFSET (GlideActorClass, selected),
		  NULL, NULL,
		  gi_cclosure_marshal_generic,
		  G_TYPE_NONE, 0);
  actor_signals[DESELECTED] = 
    g_signal_new ("deselected",
		  G_TYPE_FROM_CLASS (object_class),
		  G_SIGNAL_RUN_FIRST,
		  G_STRUCT_OFFSET (GlideActorClass, deselected),
		  NULL, NULL,
		  gi_cclosure_marshal_generic,
		  G_TYPE_NONE, 0);
		  

  g_type_class_add_private (object_class, sizeof(GlideActorPrivate));
}


static void
glide_actor_init (GlideActor *actor)
{
  actor->priv = GLIDE_ACTOR_GET_PRIVATE (actor);
  
  clutter_actor_set_reactive (CLUTTER_ACTOR (actor), TRUE);
}

GlideStageManager *
glide_actor_get_stage_manager (GlideActor *actor)
{
  return actor->priv->manager;
}

gboolean
glide_actor_get_selected (GlideActor *actor)
{
  return actor->priv->selected;
}

void
glide_actor_set_stage_manager (GlideActor *actor, GlideStageManager *manager)
{
  glide_actor_set_stage_manager_real (actor, manager);
}

JsonNode *
glide_actor_serialize (GlideActor *actor)
{
  GlideActorClass *klass;
  
  klass = GLIDE_ACTOR_GET_CLASS (actor);
  if (klass->serialize)
    return klass->serialize (actor);
  
  return NULL;
}

void
glide_actor_deserialize (GlideActor *actor, JsonObject *actor_obj)
{
  GlideActorClass *klass;
  
  klass = GLIDE_ACTOR_GET_CLASS (actor);
  if (klass->deserialize)
    klass->deserialize (actor, actor_obj);
  
  glide_json_object_restore_actor_geometry (actor_obj, CLUTTER_ACTOR (actor));
}


GlideActor *
glide_actor_construct_from_json (JsonObject *actor_obj)
{
  const gchar *a_type = glide_json_object_get_string (actor_obj, "type");
  GlideActor *ret;
  
  if (!strcmp(a_type, "image"))
    {
      ret = (GlideActor *)glide_image_new ();
    }
  else if (!strcmp (a_type, "text"))
    {
      ret = (GlideActor *)glide_text_new ();
    }
  else
    {
      return NULL;
    }
  glide_actor_deserialize (ret, actor_obj);

  return ret;
}

GlideUndoManager *
glide_actor_get_undo_manager (GlideActor *actor)
{
  return glide_stage_manager_get_undo_manager (actor->priv->manager);
}
