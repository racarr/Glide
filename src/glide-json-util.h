/*
 * glide-json-util.h
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

#ifndef __GLIDE_JSON_UTIL_H__
#define __GLIDE_JSON_UTIL_H__

#include "glide-actor.h"

void glide_json_object_set_string (JsonObject *obj, const gchar *prop, const gchar *value);
const gchar *glide_json_object_get_string (JsonObject *obj, const gchar *prop);

void glide_json_object_set_double (JsonObject *obj, const gchar *prop, gdouble value);

void glide_json_object_add_actor_geometry (JsonObject *obj, ClutterActor *actor);

#endif
