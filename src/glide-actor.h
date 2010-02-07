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
typedef struct _GlideActor GlideActor;

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
};

/*
 * Public methods
 */
GType 		 glide_actor_get_type 			(void) G_GNUC_CONST;

GlideStageManager *glide_actor_get_stage_manager (GlideActor *actor);

G_END_DECLS

#endif  /* __GLIDE_ACTOR_H__  */
