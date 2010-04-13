/*
 * glide-undo-manager.h
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

#ifndef __GLIDE_UNDO_MANAGER_H__
#define __GLIDE_UNDO_MANAGER_H__

#include "glide-types.h"
#include "glide-window.h"

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define GLIDE_TYPE_UNDO_MANAGER              (glide_undo_manager_get_type())
#define GLIDE_UNDO_MANAGER(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), GLIDE_TYPE_UNDO_MANAGER, GlideUndoManager))
#define GLIDE_UNDO_MANAGER_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), GLIDE_TYPE_UNDO_MANAGER, GlideUndoManagerClass))
#define GLIDE_IS_UNDO_MANAGER(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), GLIDE_TYPE_UNDO_MANAGER))
#define GLIDE_IS_UNDO_MANAGER_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GLIDE_TYPE_UNDO_MANAGER))
#define GLIDE_UNDO_MANAGER_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), GLIDE_TYPE_UNDO_MANAGER, GlideUndoManagerClass))

/* Private structure type */
typedef struct _GlideUndoManagerPrivate GlideUndoManagerPrivate;

/*
 * Main object structure
 */
typedef struct _GlideUndoManager GlideUndoManager;

struct _GlideUndoManager 
{
  GObject object;
  
  GlideUndoManagerPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _GlideUndoManagerClass GlideUndoManagerClass;

struct _GlideUndoManagerClass 
{
  GObjectClass parent_class;
};

typedef struct _GlideUndoInfo GlideUndoInfo;
typedef gboolean (*GlideUndoActionCallback) (GlideUndoManager *undo_manager, GlideUndoInfo *info);
typedef void (*GlideUndoInfoFreeCallback) (GlideUndoInfo *info);

struct _GlideUndoInfo {
  GlideUndoActionCallback undo_callback;
  GlideUndoActionCallback redo_callback;

  GlideUndoInfoFreeCallback free_callback;
  gpointer user_data;
};

/*
 * Public methods
 */
GType 		 glide_undo_manager_get_type 			(void) G_GNUC_CONST;
GlideUndoManager *glide_undo_manager_new ();

void glide_undo_manager_append_info (GlideUndoManager *manager, GlideUndoInfo *info);
gboolean glide_undo_manager_undo (GlideUndoManager *manager);
gboolean glide_undo_manager_redo (GlideUndoManager *manager);

void glide_undo_manager_start_actor_action (GlideUndoManager *manager, GlideActor *a);
void glide_undo_manager_end_actor_action (GlideUndoManager *manager, GlideActor *a);

void glide_undo_manager_cancel_actor_action (GlideUndoManager *manager);

G_END_DECLS

#endif
