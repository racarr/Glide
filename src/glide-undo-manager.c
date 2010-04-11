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

#include "glide-undo-manager-priv.h"

#include "glide-debug.h"

G_DEFINE_TYPE(GlideUndoManager, glide_undo_manager, G_TYPE_OBJECT)

#define GLIDE_UNDO_MANAGER_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GLIDE_TYPE_UNDO_MANAGER, GlideUndoManagerPrivate))

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