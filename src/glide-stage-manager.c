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
#include "glide-manipulator.h"
#include "glide-actor.h"

#include "glide-debug.h"

G_DEFINE_TYPE(GlideStageManager, glide_stage_manager, G_TYPE_OBJECT)

#define GLIDE_STAGE_MANAGER_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GLIDE_TYPE_STAGE_MANAGER, GlideStageManagerPrivate))

enum {
  PROP_0,
  PROP_STAGE,
  PROP_SELECTION,
  PROP_DOCUMENT
};

enum {
  SELECTION_CHANGED,
  LAST_SIGNAL
};

static guint stage_manager_signals[LAST_SIGNAL] = { 0, };

static void
glide_stage_manager_finalize (GObject *object)
{
  GLIDE_NOTE (STAGE_MANAGER, "Finalizing stage manager: %s",
	      GLIDE_ACTOR_DISPLAY_NAME (CLUTTER_ACTOR (object)));

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
    case PROP_DOCUMENT:
      g_value_set_object (value, manager->priv->document);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
glide_stage_manager_set_selection_real (GlideStageManager *m,
					GlideActor *a)
{
  GlideActor *old = m->priv->selection;
  
  if (old == a)
    return;
  
  GLIDE_NOTE (STAGE_MANAGER, "Selection changed from: %s (%p) to %s (%p)",
	      old ? GLIDE_ACTOR_DISPLAY_NAME (old) : "unknown", old,
	      a ? GLIDE_ACTOR_DISPLAY_NAME (a) : "unknown", a);

  m->priv->selection = a;

  if (a)
    clutter_actor_raise_top (CLUTTER_ACTOR (a));
  
  glide_manipulator_set_target(m->priv->manip, CLUTTER_ACTOR (a));
  glide_manipulator_set_width_only(m->priv->manip, FALSE);
  
  g_signal_emit (m, stage_manager_signals[SELECTION_CHANGED], 0, old);
}

static void
glide_stage_manager_add_manipulator (GlideStageManager *manager, ClutterActor *stage)
{
  GlideManipulator *manip = glide_manipulator_new(NULL);
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), CLUTTER_ACTOR (manip));
  
  clutter_actor_hide_all (CLUTTER_ACTOR (manip));
  
  manager->priv->manip = manip;
}

void
glide_stage_manager_set_document (GlideStageManager *manager,
				  GlideDocument *document)
{
  manager->priv->document = document;
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
      glide_stage_manager_add_manipulator (manager, manager->priv->stage);
      break;
    case PROP_DOCUMENT:
      g_return_if_fail (manager->priv->document == NULL);
      glide_stage_manager_set_document (manager, GLIDE_DOCUMENT (g_value_get_object (value)));
      break;
    case PROP_SELECTION:
      glide_stage_manager_set_selection_real (manager,
	      GLIDE_ACTOR (g_value_get_object (value)));
      break;
    default: 
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static GObject *
glide_stage_manager_constructor (GType type,
				 guint n_properties,
				 GObjectConstructParam *properties)
{
  GObject *obj;
  guint i, n;
  GlideStageManager *manager;

  {
    /* Always chain up to the parent constructor */
    GObjectClass *parent_class;  
    parent_class = G_OBJECT_CLASS (glide_stage_manager_parent_class);
    obj = parent_class->constructor (type, n_properties, properties);
  }
  manager = GLIDE_STAGE_MANAGER (obj);
  n = glide_document_get_n_slides (manager->priv->document);
  for (i = 0; i < n; i++)
    {
      GlideSlide *slide = glide_document_get_nth_slide (manager->priv->document, i);
      clutter_container_add_actor (CLUTTER_CONTAINER (manager->priv->stage),
				   CLUTTER_ACTOR (slide));
      clutter_actor_hide (CLUTTER_ACTOR (slide));
    }
  
  manager->priv->current_slide = 0;
  clutter_actor_show (CLUTTER_ACTOR (glide_document_get_nth_slide (manager->priv->document, 0)));  

  return obj;
}

static void
glide_stage_manager_class_init (GlideStageManagerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  
  object_class->finalize = glide_stage_manager_finalize;
  object_class->set_property = glide_stage_manager_set_property;
  object_class->get_property = glide_stage_manager_get_property;
  object_class->constructor = glide_stage_manager_constructor;
  
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
							GLIDE_TYPE_ACTOR,
							G_PARAM_READWRITE |
							G_PARAM_CONSTRUCT_ONLY |
							G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class,
				   PROP_DOCUMENT,
				   g_param_spec_object ("document",
							"Document",
							"The document rendered by the stage manager",
							GLIDE_TYPE_DOCUMENT,
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
glide_stage_manager_new (GlideDocument *document, ClutterStage *stage)
{
  return g_object_new (GLIDE_TYPE_STAGE_MANAGER,
		       "document", document,
		       "stage", stage,
		       NULL);
}

ClutterStage *
glide_stage_manager_get_stage (GlideStageManager *m)
{
  return (ClutterStage *)m->priv->stage;
}

GlideActor *
glide_stage_manager_get_selection (GlideStageManager *m)
{
  return m->priv->selection;
}

void
glide_stage_manager_set_selection (GlideStageManager *m,
				   GlideActor *a)
{
  glide_stage_manager_set_selection_real (m, a);
}

GlideManipulator *
glide_stage_manager_get_manipulator (GlideStageManager *m)
{
  return m->priv->manip;
}

void 
glide_stage_manager_add_actor (GlideStageManager *manager,
			       GlideActor *actor)
{
  GlideSlide *current_slide;
  glide_actor_set_stage_manager (actor, manager);
  
  current_slide = glide_document_get_nth_slide (manager->priv->document,
						manager->priv->current_slide);
  
  clutter_actor_set_position (CLUTTER_ACTOR (actor), 200, 200);
  clutter_container_add_actor (CLUTTER_CONTAINER (current_slide),
			       CLUTTER_ACTOR (actor));

  clutter_actor_show (CLUTTER_ACTOR (actor));
  
  glide_stage_manager_set_selection (manager, actor);
}
