/*
 * glide-undo-manager-priv.h
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
 
#ifndef __GLIDE_UNDO_MANAGER_PRIVATE_H__
#define __GLIDE_UNDO_MANAGER_PRIVATE_H__

#include "glide-undo-manager.h"

G_BEGIN_DECLS

struct _GlideUndoManagerPrivate
{
  ClutterActor *recorded_actor;
  // TODO: Check. do we leak the nodes?
  JsonObject *recorded_state;

  GList *infos;
  GList *position;
};

G_END_DECLS

#endif  /* __GLIDE_UNDO_MANAGER_PRIVATE_H__  */
