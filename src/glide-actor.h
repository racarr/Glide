/*
 * glide-actor.h
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
 * MERCHANACTORILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, 
 * Boston, MA 02111-1307, USA.
 */

#ifndef __GLIDE_ACTOR_H__
#define __GLIDE_ACTOR_H__

#include <clutter/clutter.h>
#include <json-glib/json-glib.h>
#include "glide-stage-manager.h"

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define GLIDE_TYPE_ACTOR              (glide_actor_get_type())
#define GLIDE_ACTOR(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), GLIDE_TYPE_ACTOR, GlideActor))
#define GLIDE_ACTOR_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), GLIDE_TYPE_ACTOR, GlideActorClass))
#define GLIDE_IS_ACTOR(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), GLIDE_TYPE_ACTOR))
#define GLIDE_IS_ACTOR_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GLIDE_TYPE_ACTOR))
#define GLIDE_ACTOR_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), GLIDE_TYPE_ACTOR, GlideActorClass))

/* Private structure type */
typedef struct _GlideActorPrivate GlideActorPrivate;

/*
 * Main object structure
 */
//typedef struct _GlideActor GlideActor;

struct _GlideActor 
{
  ClutterActor rectangle;
  
  GlideActorPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _GlideActorClass GlideActorClass;

struct _GlideActorClass 
{
  ClutterActorClass parent_class;
  
  JsonNode* (* serialize) (GlideActor *actor);
  void (* deserialize) (GlideActor *actor, JsonObject *obj);
  
  void (* selected) (GlideActor *actor);
  void (* deselected) (GlideActor *actor);
};

#define GLIDE_ACTOR_DISPLAY_NAME(actor) \
  (clutter_actor_get_name ((ClutterActor *)actor) ?  \
   clutter_actor_get_name ((ClutterActor *)actor) : "unknown")

/*
 * Public methods
 */
GType 		 glide_actor_get_type 			(void) G_GNUC_CONST;

GlideStageManager *glide_actor_get_stage_manager (GlideActor *actor);
void glide_actor_set_stage_manager (GlideActor *actor, GlideStageManager *manager);

gboolean glide_actor_get_selected (GlideActor *actor);

JsonNode *glide_actor_serialize (GlideActor *actor);

GlideActor *glide_actor_construct_from_json (JsonObject *obj);
void glide_actor_deserialize (GlideActor *actor, JsonObject *obj);

GlideUndoManager *glide_actor_get_undo_manager (GlideActor *actor);



G_END_DECLS

#endif  /* __GLIDE_ACTOR_H__  */
