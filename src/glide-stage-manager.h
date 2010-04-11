/*
 * glide-stage-manager.h
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

#ifndef __GLIDE_STAGE_MANAGER_H__
#define __GLIDE_STAGE_MANAGER_H__

#include <glib-object.h>
#include <clutter/clutter.h>

#include "glide-types.h"

#include "glide-manipulator.h"
#include "glide-document.h"

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define GLIDE_TYPE_STAGE_MANAGER              (glide_stage_manager_get_type())
#define GLIDE_STAGE_MANAGER(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), GLIDE_TYPE_STAGE_MANAGER, GlideStageManager))
#define GLIDE_STAGE_MANAGER_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), GLIDE_TYPE_STAGE_MANAGER, GlideStageManagerClass))
#define GLIDE_IS_STAGE_MANAGER(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), GLIDE_TYPE_STAGE_MANAGER))
#define GLIDE_IS_STAGE_MANAGER_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GLIDE_TYPE_STAGE_MANAGER))
#define GLIDE_STAGE_MANAGER_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), GLIDE_TYPE_STAGE_MANAGER, GlideStageManagerClass))

/* Private structure type */
typedef struct _GlideStageManagerPrivate GlideStageManagerPrivate;

/*
 * Main object structure
 */
typedef struct _GlideStageManager GlideStageManager;

struct _GlideStageManager 
{
  GObject object;
  
  GlideStageManagerPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _GlideStageManagerClass GlideStageManagerClass;

struct _GlideStageManagerClass 
{
  GObjectClass parent_class;
};

/*
 * Public methods
 */
GType 		 glide_stage_manager_get_type 			(void) G_GNUC_CONST;

GlideStageManager *glide_stage_manager_new (GlideDocument *document, ClutterStage *stage);
ClutterStage *glide_stage_manager_get_stage (GlideStageManager *manager);
GlideDocument *glide_stage_manager_get_document (GlideStageManager *manager);


GlideActor *glide_stage_manager_get_selection (GlideStageManager *manager);
void glide_stage_manager_set_selection (GlideStageManager *manager,
					GlideActor *actor);

GlideManipulator *glide_stage_manager_get_manipulator (GlideStageManager *manager);

void glide_stage_manager_add_actor (GlideStageManager *manager,
				    GlideActor *actor);

void glide_stage_manager_set_slide_prev (GlideStageManager *manager);
void glide_stage_manager_set_slide_next (GlideStageManager *manager);

gint glide_stage_manager_get_current_slide (GlideStageManager *manager);
void glide_stage_manager_set_current_slide (GlideStageManager *manager, guint slide);

void glide_stage_manager_load_slides (GlideStageManager *manager, JsonArray *slides);

gboolean glide_stage_manager_get_presenting (GlideStageManager *manager);
void glide_stage_manager_set_presenting (GlideStageManager *manager, gboolean presenting);

void glide_stage_manager_set_slide_background (GlideStageManager *manager, const gchar *bg);

void glide_stage_manager_delete_selection (GlideStageManager *manager);


G_END_DECLS

#endif  /* __GLIDE_STAGE_MANAGER_H__  */
