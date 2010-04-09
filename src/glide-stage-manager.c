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
#include "glide-slide.h"

#include "glide-debug.h"

G_DEFINE_TYPE(GlideStageManager, glide_stage_manager, G_TYPE_OBJECT)

#define GLIDE_STAGE_MANAGER_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GLIDE_TYPE_STAGE_MANAGER, GlideStageManagerPrivate))

enum {
  PROP_0,
  PROP_STAGE,
  PROP_SELECTION,
  PROP_DOCUMENT,
  PROP_CURRENT_SLIDE,
  PROP_PRESENTING
};

enum {
  SELECTION_CHANGED,
  LAST_SIGNAL
};

static guint stage_manager_signals[LAST_SIGNAL] = { 0, };

static void
glide_stage_manager_finalize (GObject *object)
{
  GlideStageManager *manager = GLIDE_STAGE_MANAGER (object);
  GLIDE_NOTE (STAGE_MANAGER, "Finalizing stage manager: %p",
	      object);
  
  g_object_unref (G_OBJECT (manager->priv->document));

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
    case PROP_CURRENT_SLIDE:
      g_value_set_uint (value, manager->priv->current_slide);
      break;
    case PROP_PRESENTING:
      g_value_set_boolean (value, manager->priv->presenting);
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
glide_stage_manager_add_manipulator (GlideStageManager *manager)
{
  GlideManipulator *manip;
  ClutterActor *parent;
  
  if (!manager->priv->manip)
    manip = glide_manipulator_new (NULL);
  else
    manip = manager->priv->manip;
  
  if ((parent = clutter_actor_get_parent (CLUTTER_ACTOR (manip))))
    {
      clutter_container_remove_actor (CLUTTER_CONTAINER (parent), CLUTTER_ACTOR (manip));
      manip = glide_manipulator_new (NULL);
    }
  
  clutter_container_add_actor (CLUTTER_CONTAINER (glide_document_get_nth_slide (manager->priv->document,
										manager->priv->current_slide)), 
						  CLUTTER_ACTOR (manip));
  
  clutter_actor_hide_all (CLUTTER_ACTOR (manip));
  
  manager->priv->manip = manip;
}

void
glide_stage_manager_set_slide (GlideStageManager *manager, guint slide)
{
  if (manager->priv->current_slide >= 0)
    clutter_actor_hide (CLUTTER_ACTOR (glide_document_get_nth_slide (manager->priv->document, manager->priv->current_slide)));
  manager->priv->current_slide = slide;
  clutter_actor_show_all (CLUTTER_ACTOR (glide_document_get_nth_slide (manager->priv->document, slide)));  
  glide_stage_manager_add_manipulator (manager);
  
  g_object_notify (G_OBJECT (manager), "current-slide");
}

void
glide_stage_manager_document_slide_added_cb (GlideDocument *document, 
					     GlideSlide *slide, 
					     gpointer data)
{
  GlideStageManager *manager = (GlideStageManager *)data;
  
  clutter_container_add_actor (CLUTTER_CONTAINER (manager->priv->stage), CLUTTER_ACTOR (slide));
  glide_actor_set_stage_manager (GLIDE_ACTOR (slide), manager);
  
  glide_stage_manager_set_slide (manager, manager->priv->current_slide+1);
  
  g_message ("Slide added");
}

void
glide_stage_manager_set_document (GlideStageManager *manager,
				  GlideDocument *document)
{
  manager->priv->document = g_object_ref (document);
  g_signal_connect (document, "slide-added", G_CALLBACK (glide_stage_manager_document_slide_added_cb), manager);
}

void
glide_stage_manager_advance_slide (GlideStageManager *manager)
{
  if (manager->priv->current_slide + 1 < glide_document_get_n_slides(manager->priv->document))
    glide_stage_manager_set_slide (manager, manager->priv->current_slide + 1);
  else
    glide_stage_manager_set_presenting (manager, FALSE);
}

void
glide_stage_manager_reverse_slide (GlideStageManager *manager)
{
  glide_stage_manager_set_slide_prev (manager);
}

gboolean
glide_stage_manager_button_pressed (ClutterActor *actor,
				    ClutterButtonEvent *event,
				    gpointer user_data)
{
  GlideStageManager *manager = (GlideStageManager *) user_data;
  if (event->button == 1)
    {
      glide_stage_manager_advance_slide (manager);
      return TRUE;
    }
  else if (event->button == 3)
    {
      glide_stage_manager_reverse_slide (manager);
      return TRUE;
    }
  
  return TRUE;
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
      
      g_signal_connect (G_OBJECT (manager->priv->stage), "button-press-event", G_CALLBACK(glide_stage_manager_button_pressed), manager);
      break;
    case PROP_DOCUMENT:
      g_return_if_fail (manager->priv->document == NULL);
      glide_stage_manager_set_document (manager, GLIDE_DOCUMENT (g_value_get_object (value)));
      break;
    case PROP_SELECTION:
      glide_stage_manager_set_selection_real (manager,
	      GLIDE_ACTOR (g_value_get_object (value)));
      break;
    case PROP_CURRENT_SLIDE:
      glide_stage_manager_set_slide (manager,
				     g_value_get_uint (value));
      break;
    case PROP_PRESENTING:
      glide_stage_manager_set_presenting (manager,
					  g_value_get_boolean (value));
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
  
  manager->priv->current_slide = n-1;
  if (n > 0)
    clutter_actor_show (CLUTTER_ACTOR (glide_document_get_nth_slide (manager->priv->document, 0)));  

  //  glide_stage_manager_add_manipulator (manager);

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
  
  g_object_class_install_property (object_class,
				   PROP_CURRENT_SLIDE,
				   g_param_spec_uint ("current-slide",
						      "Current Slide",
						      "The currently displayed slide",
						      0, G_MAXUINT, G_MAXUINT, 
						      G_PARAM_READWRITE));

  g_object_class_install_property (object_class,
				   PROP_PRESENTING,
				   g_param_spec_boolean("presenting",
							"Presenting",
							"Whether we are currently involved in a presentation",
							FALSE,
							G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

  
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

void
glide_stage_manager_set_slide_next (GlideStageManager *manager)
{
  if (manager->priv->current_slide + 1 < glide_document_get_n_slides(manager->priv->document))
    glide_stage_manager_set_slide (manager, manager->priv->current_slide + 1);
}

void
glide_stage_manager_set_slide_prev (GlideStageManager *manager)
{
  if (manager->priv->current_slide > 0)
    glide_stage_manager_set_slide (manager, manager->priv->current_slide - 1);
}

gint
glide_stage_manager_get_current_slide (GlideStageManager *manager)
{
  return manager->priv->current_slide;
}

void
glide_stage_manager_set_current_slide (GlideStageManager *manager, guint slide)
{
  glide_stage_manager_set_slide (manager, slide);
}

// TODO: Error handling.
void
glide_stage_manager_load_slides (GlideStageManager *manager, JsonArray *slides)
{
  GList *slides_list, *s;
  
  // Handle broken first slide.
  
  slides_list = json_array_get_elements (slides);
  for (s = slides_list; s; s = s->next)
    {
      JsonNode *n = s->data;
      JsonObject *slide = json_node_get_object (n);
      GlideSlide *gs = glide_document_add_slide (manager->priv->document);
      
      glide_slide_construct_from_json (gs, slide, manager);
    }
}

void
glide_stage_manager_set_presenting (GlideStageManager *manager, gboolean presenting)
{
  if (presenting != manager->priv->presenting)
    {
      manager->priv->presenting = presenting;
      if (presenting)
	glide_stage_manager_set_selection (manager, NULL);
      g_object_notify (G_OBJECT (manager), "presenting");
    }
}

gboolean
glide_stage_manager_get_presenting (GlideStageManager *manager)
{
  return manager->priv->presenting;
}

void
glide_stage_manager_set_slide_background (GlideStageManager *manager, const gchar *bg)
{
  glide_slide_set_background (glide_document_get_nth_slide (manager->priv->document, manager->priv->current_slide), bg);
}
