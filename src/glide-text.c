/*
 * glide-text.c
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
 
#include <glib/gi18n.h>
#include "glide-text.h"


G_DEFINE_TYPE(GlideText, glide_text, CLUTTER_TYPE_TEXT)

static void
glide_text_finalize (GObject *object)
{
  /* Debug? */

  G_OBJECT_CLASS (glide_text_parent_class)->finalize (object);
}

static void
glide_text_class_init (GlideTextClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  
  object_class->finalize = glide_text_finalize;
}

static void
glide_text_init (GlideText *text)
{
  ClutterColor white = { 0xff, 0xff, 0xff, 0xff};
  
  clutter_text_set_color (CLUTTER_TEXT (text), &white);
  clutter_text_set_text (CLUTTER_TEXT (text), "This is a test of text"
			 " in Glide.");
  clutter_text_set_font_name (CLUTTER_TEXT (text), "Sans 12");
  clutter_text_set_editable (CLUTTER_TEXT (text), TRUE);
  clutter_text_set_line_wrap (CLUTTER_TEXT (text), TRUE);
  
  clutter_actor_set_size (CLUTTER_ACTOR (text), 400, 20);
  clutter_actor_set_reactive (CLUTTER_ACTOR (text), TRUE);
}

GlideText *
glide_text_new ()
{
  return g_object_new (GLIDE_TYPE_TEXT, NULL);
}
