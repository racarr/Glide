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


G_DEFINE_TYPE(GlideStageManager, glide_stage_manager, G_TYPE_OBJECT)

#define GLIDE_STAGE_MANAGER_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GLIDE_TYPE_STAGE_MANAGER, GlideStageManagerPrivate))

enum {
  PROP_0,
  PROP_STAGE,
  PROP_SELECTION
};

enum {
  SELECTION_CHANGED,
  LAST_SIGNAL
};

static guint stage_manager_signals[LAST_SIGNAL] = { 0, };

static void
glide_stage_manager_finalize (GObject *object)
{
  /* Debug? */

  G_OBJECT_CLASS (glide_stage_manager_parent_class)->finalize (object);
}


static void
glide_stage_manager_get_property (GObject *object,
				  guint prop_id,
				  GValue *value,
				  GParamSpec *pspec)
{
  GlideStageManager *manager = GLIDE_STAGE_MANAGER (object);
  
  switch (prop_id)
    {
    case PROP_STAGE:
      g_value_set_object (value, manager->priv->stage);
      break;
    case PROP_SELECTION:
      g_value_set_object (value, manager->priv->selection);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
glide_stage_manager_set_selection_real (GlideStageManager *m,
					ClutterActor *a)
{
  ClutterActor *old = m->priv->selection;
  m->priv->selection = a;
  
  g_signal_emit (m, stage_manager_signals[SELECTION_CHANGED], 0, old);
}

static void
glide_stage_manager_set_property (GObject *object,
				  guint prop_id,
				  const GValue *value,
				  GParamSpec *pspec)
{
  GlideStageManager *manager = GLIDE_STAGE_MANAGER (object);
  
  switch (prop_id)
    {
    case PROP_STAGE:
      g_return_if_fail (manager->priv->stage == NULL);
      manager->priv->stage = CLUTTER_ACTOR (g_value_get_object (value));
      break;
    case PROP_SELECTION:
      glide_stage_manager_set_selection_real (manager,
	      CLUTTER_ACTOR (g_value_get_object (value)));
      break;
    default: 
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
glide_stage_manager_class_init (GlideStageManagerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  
  object_class->finalize = glide_stage_manager_finalize;
  object_class->set_property = glide_stage_manager_set_property;
  object_class->get_property = glide_stage_manager_get_property;
  
  g_object_class_install_property (object_class,
				   PROP_STAGE,
				   g_param_spec_object ("stage",
							"Stage",
							"The ClutterStage to manage",
							CLUTTER_TYPE_STAGE,
							G_PARAM_READWRITE |
							G_PARAM_CONSTRUCT_ONLY |
							G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class,
				   PROP_SELECTION,
				   g_param_spec_object ("selection",
							"Selection",
							"The GlideActor currently selected",
							CLUTTER_TYPE_ACTOR,
							G_PARAM_READWRITE |
							G_PARAM_CONSTRUCT_ONLY |
							G_PARAM_STATIC_STRINGS));
  
  // Argument is old selection
  stage_manager_signals[SELECTION_CHANGED] = 
    g_signal_new ("selection-changed",
		  G_TYPE_FROM_CLASS (object_class),
		  G_SIGNAL_RUN_LAST,
		  0,
		  NULL, NULL,
		  g_cclosure_marshal_VOID__OBJECT,
		  G_TYPE_NONE, 1,
		  G_TYPE_OBJECT);
    

  g_type_class_add_private (object_class, sizeof(GlideStageManagerPrivate));
}

static void
glide_stage_manager_init (GlideStageManager *manager)
{
  manager->priv = GLIDE_STAGE_MANAGER_GET_PRIVATE (manager);
}

GlideStageManager *
glide_stage_manager_new (ClutterStage *stage)
{
  return g_object_new (GLIDE_TYPE_STAGE_MANAGER,
		       "stage", stage,
		       NULL);
}

ClutterStage *
glide_stage_manager_get_stage (GlideStageManager *m)
{
  return (ClutterStage *)m->priv->stage;
}

ClutterActor *
glide_stage_manager_get_selection (GlideStageManager *m)
{
  return m->priv->selection;
}

void
glide_stage_manager_set_selection (GlideStageManager *m,
				   ClutterActor *a)
{
  glide_stage_manager_set_selection_real (m, a);
}

