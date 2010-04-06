/*
 * glide-json-util.c
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

#include "glide-json-util.h"

void
glide_json_object_set_string (JsonObject *obj, const gchar *prop, const gchar *value)
{
  JsonNode *n = json_node_new (JSON_NODE_VALUE);
  
  json_node_set_string (n, value);
  json_object_set_member (obj, prop, n);
}

const gchar *
glide_json_object_get_string (JsonObject *obj, const gchar *prop)
{
  JsonNode *n = json_object_get_member (obj, prop);
  
  return json_node_get_string (n);
}

void
glide_json_object_set_double (JsonObject *obj, const gchar *prop, gdouble value)
{
  gchar *s;
  JsonNode *n = json_node_new (JSON_NODE_VALUE);
  
  s = g_strdup_printf("%g",value);
  
  json_node_set_string(n, s);
  
  g_free (s);

  json_object_set_member (obj, prop, n);
}

void
glide_json_object_add_actor_geometry (JsonObject *obj, ClutterActor *actor)
{
  JsonNode *n = json_node_new (JSON_NODE_OBJECT);
  JsonObject *geom_obj = json_object_new ();
  gfloat width, height, x, y;
  
  json_node_set_object (n, geom_obj);
  
  clutter_actor_get_position (actor, &x, &y);
  clutter_actor_get_size (actor, &width, &height);  

  glide_json_object_set_double (geom_obj, "x", x);
  glide_json_object_set_double (geom_obj, "y", y);
  glide_json_object_set_double (geom_obj, "width", width);
  glide_json_object_set_double (geom_obj, "height", height);
  
  json_object_set_member (obj, "geometry", n);
}
