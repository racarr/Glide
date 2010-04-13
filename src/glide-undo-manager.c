/*
 * glide-undo-manager.c
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

#include "glide-undo-manager.h"
#include "glide-stage-manager.h"
#include "glide-actor.h"

#include "glide-undo-manager-priv.h"

#include "glide-debug.h"

G_DEFINE_TYPE(GlideUndoManager, glide_undo_manager, G_TYPE_OBJECT)

#define GLIDE_UNDO_MANAGER_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GLIDE_TYPE_UNDO_MANAGER, GlideUndoManagerPrivate))

typedef struct _GlideUndoActorData {
  ClutterActor *actor;
  JsonObject *old_state;
} GlideUndoActorData;

/*static*/ void
glide_undo_actor_info_free_callback (GlideUndoInfo *info)
{
  GlideUndoActorData *data = (GlideUndoActorData *)info->user_data;
  
  g_object_unref (G_OBJECT (data->actor));
  g_object_unref (G_OBJECT (data->old_state));
  
  g_free (data);
}

/*static*/ gboolean
glide_undo_actor_action_callback (GlideUndoManager *undo_manager,
				  GlideUndoInfo *info)
{
  GlideUndoActorData *data = (GlideUndoActorData *)info->user_data;
  ClutterActor *parent = clutter_actor_get_parent (data->actor);
  GlideActor *actor = glide_actor_construct_from_json (data->old_state);
  GlideStageManager *manager = glide_actor_get_stage_manager (GLIDE_ACTOR (data->actor));
							      
  clutter_container_remove_actor (CLUTTER_CONTAINER (parent), data->actor);
  clutter_container_add_actor (CLUTTER_CONTAINER (parent), CLUTTER_ACTOR (actor));
  
  glide_actor_set_stage_manager (GLIDE_ACTOR (actor), manager);
  clutter_actor_show (CLUTTER_ACTOR (actor));
  
  return TRUE;
}

void
glide_undo_manager_start_actor_action (GlideUndoManager *manager,
				       GlideActor *a)
{
  JsonNode *anode;
  manager->priv->recorded_actor = (ClutterActor *)a;
  
  anode = glide_actor_serialize (a);
  manager->priv->recorded_state = json_node_get_object (anode);
}

void
glide_undo_manager_end_actor_action (GlideUndoManager *manager,
				     GlideActor *a)
{
  GlideUndoInfo *info;
  GlideUndoActorData *data;
  
  if (manager->priv->recorded_actor != (ClutterActor *)a)
    {
      g_warning ("Error, mismatched undo manager start/end actor actions.");
      return;
    }
  info = g_malloc (sizeof (GlideUndoInfo));
  data = g_malloc (sizeof (GlideUndoActorData));
  
  info->callback = glide_undo_actor_action_callback;
  info->free_callback = glide_undo_actor_info_free_callback;
  info->user_data = data;
  
  data->actor = (ClutterActor *)g_object_ref (G_OBJECT (a));
  data->old_state = manager->priv->recorded_state;
  
  glide_undo_manager_append_info (manager, info);
}

static void
glide_undo_manager_init (GlideUndoManager *manager)
{
  manager->priv = GLIDE_UNDO_MANAGER_GET_PRIVATE (manager);
}

static void
glide_undo_manager_class_init (GlideUndoManagerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  
  g_type_class_add_private (object_class, sizeof(GlideUndoManagerPrivate));
}

GlideUndoManager *
glide_undo_manager_new ()
{
  return g_object_new (GLIDE_TYPE_UNDO_MANAGER,
		       NULL);
}

void
glide_undo_manager_append_info (GlideUndoManager *manager, GlideUndoInfo *info)
{
  manager->priv->infos = g_list_append (manager->priv->infos, info);
}
