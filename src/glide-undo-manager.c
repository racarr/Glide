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

enum {
  POSITION_CHANGED,
  LAST_SIGNAL
};

static guint undo_manager_signals[LAST_SIGNAL] = { 0, };

typedef struct _GlideUndoActorData {
  ClutterActor *actor;

  JsonObject *old_state;
  JsonObject *new_state;
} GlideUndoActorData;

static void
glide_undo_actor_info_free_callback (GlideUndoInfo *info)
{
  GlideUndoActorData *data = (GlideUndoActorData *)info->user_data;
  
  g_object_unref (G_OBJECT (data->actor));
  g_object_unref (G_OBJECT (data->old_state));
  g_object_unref (G_OBJECT (data->new_state));
  
  g_free (data);
}

static gboolean
glide_undo_actor_action_undo_callback (GlideUndoManager *undo_manager,
				       GlideUndoInfo *info)
{
  GlideUndoActorData *data = (GlideUndoActorData *)info->user_data;
  
  glide_actor_deserialize (GLIDE_ACTOR (data->actor), data->old_state);
							      
  return TRUE;
}

static gboolean
glide_undo_actor_action_redo_callback (GlideUndoManager *undo_manager,
				       GlideUndoInfo *info)
{
  GlideUndoActorData *data = (GlideUndoActorData *)info->user_data;
  
  glide_actor_deserialize (GLIDE_ACTOR (data->actor), data->new_state);
							      
  return TRUE;
}

void
glide_undo_manager_start_actor_action (GlideUndoManager *manager,
				       GlideActor *a,
				       const gchar *label)
{
  JsonNode *anode;
  manager->priv->recorded_actor = (ClutterActor *)a;
  
  anode = glide_actor_serialize (a);
  manager->priv->recorded_state = json_node_get_object (anode);
  
  manager->priv->recorded_label = g_strdup (label);
}

void
glide_undo_manager_cancel_actor_action (GlideUndoManager *manager)
{
  manager->priv->recorded_actor = NULL;
  manager->priv->recorded_state = NULL;
  
  g_free (manager->priv->recorded_label);
}

void
glide_undo_manager_end_actor_action (GlideUndoManager *manager,
				     GlideActor *a)
{
  GlideUndoInfo *info;
  GlideUndoActorData *data;
  JsonNode *new_node;
  
  if (manager->priv->recorded_actor != (ClutterActor *)a)
    {
      g_warning ("Error, mismatched undo manager start/end actor actions.");
      return;
    }
  
  new_node = glide_actor_serialize (a);

  info = g_malloc (sizeof (GlideUndoInfo));
  data = g_malloc (sizeof (GlideUndoActorData));
  
  info->undo_callback = glide_undo_actor_action_undo_callback;
  info->redo_callback = glide_undo_actor_action_redo_callback;
  info->free_callback = glide_undo_actor_info_free_callback;
  info->label = manager->priv->recorded_label;
  info->user_data = data;

  
  data->actor = (ClutterActor *)g_object_ref (G_OBJECT (a));
  data->old_state = manager->priv->recorded_state;
  data->new_state = json_node_get_object (new_node);
  
  glide_undo_manager_append_info (manager, info);
}

static void
glide_undo_manager_init (GlideUndoManager *manager)
{
  manager->priv = GLIDE_UNDO_MANAGER_GET_PRIVATE (manager);
  
  manager->priv->infos = g_list_append (manager->priv->infos, NULL);
}

static void
glide_undo_manager_class_init (GlideUndoManagerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  
  undo_manager_signals[POSITION_CHANGED] = 
    g_signal_new ("position-changed",
		  G_TYPE_FROM_CLASS (object_class),
		  G_SIGNAL_RUN_LAST,
		  0,
		  NULL, NULL,
		  g_cclosure_marshal_VOID__OBJECT,
		  G_TYPE_NONE, 0, NULL);
  
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
  manager->priv->position = g_list_last (manager->priv->infos);
  
  g_signal_emit (manager, undo_manager_signals[POSITION_CHANGED], 0);
}
// TODO: Handle failed redo/undos.
gboolean
glide_undo_manager_redo (GlideUndoManager *manager)
{
  GlideUndoInfo *info;
  
  if (!manager->priv->position->next)
    return FALSE;
  else
    info = (GlideUndoInfo *)manager->priv->position->next->data;
  
  manager->priv->position = manager->priv->position->next;
  g_signal_emit (manager, undo_manager_signals[POSITION_CHANGED], 0);  

  return info->redo_callback (manager, info);
}

gboolean
glide_undo_manager_undo (GlideUndoManager *manager)
{
  GlideUndoInfo *info;

  if (!manager->priv->position->data)
    return FALSE;
  else
    info = (GlideUndoInfo *)manager->priv->position->data;
  
  manager->priv->position = manager->priv->position->prev;
  g_signal_emit (manager, undo_manager_signals[POSITION_CHANGED], 0);  
  
  return info->undo_callback (manager, info);
}

