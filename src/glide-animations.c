/*
 * glide-animations.c
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

#include "glide-animations.h"
#include "glide-slide.h"

static void
glide_animations_fade_completed (ClutterTimeline *t, gpointer user_data)
{
  ClutterActor *a = (ClutterActor *) user_data;

  clutter_actor_set_opacity (a, 0xff);
  clutter_actor_hide (a);
}


ClutterTimeline *
glide_animations_animate_fade (ClutterActor *a, ClutterActor *b, guint duration)
{
  ClutterTimeline *timeline = clutter_timeline_new (duration);

  clutter_actor_set_opacity (b, 0x00);
  clutter_actor_show_all (b);
  
  clutter_actor_raise (b, a);
  
  clutter_actor_animate_with_timeline (b, CLUTTER_LINEAR, timeline, "opacity", 0xff, NULL);
  
  clutter_timeline_start (timeline);
  
  g_signal_connect (timeline, "completed", G_CALLBACK (glide_animations_fade_completed), a);
  
  return timeline;
}

static void
glide_animations_slide_completed (ClutterTimeline *t, gpointer user_data)
{
  ClutterActor *a = (ClutterActor *) user_data;

  clutter_actor_hide (a);
  
  clutter_actor_set_x (a, 0);
}

ClutterTimeline *
glide_animations_animate_slide (ClutterActor *a, ClutterActor *b, guint duration)
{
  ClutterTimeline *timeline = clutter_timeline_new (duration);
  ClutterActor *stage = clutter_actor_get_stage (a);
  gfloat width, height;
  
  clutter_actor_show_all (b);

  
  clutter_actor_get_size (stage, &width, &height);
  
  clutter_actor_set_x (b, width);
  clutter_actor_animate_with_timeline(b, CLUTTER_EASE_IN_OUT_SINE, timeline,
				      "x", 0, NULL);
  clutter_actor_animate_with_timeline(a, CLUTTER_EASE_IN_OUT_SINE, timeline,
				      "x", -width, NULL);
  
  
  clutter_timeline_start (timeline);

  g_signal_connect (timeline, "completed", G_CALLBACK (glide_animations_slide_completed), a);
  
  return timeline;
}

ClutterTimeline *
glide_animations_animate_pivot (ClutterActor *a, ClutterActor *b, guint duration)
{
  ClutterTimeline *timeline = clutter_timeline_new (duration);
  ClutterActor *stage = clutter_actor_get_stage (a);
  gfloat width, height;
  
  clutter_actor_show_all (b);

  
  clutter_actor_get_size (stage, &width, &height);
  
  clutter_actor_set_rotation (b, CLUTTER_X_AXIS, 80, 0, 0, 0);
  
  clutter_actor_animate_with_timeline (b, CLUTTER_EASE_OUT_BOUNCE, timeline, "rotation-angle-x", (gdouble)0, NULL);
  
  clutter_timeline_start (timeline);

  g_signal_connect (timeline, "completed", G_CALLBACK (glide_animations_fade_completed), a);
  
  return timeline;
}

ClutterTimeline *
glide_animations_animate_zoom (ClutterActor *a, ClutterActor *b, guint duration)
{
  ClutterTimeline *timeline = clutter_timeline_new (duration);
  ClutterActor *stage = clutter_actor_get_stage (a);
  gfloat width, height;
  
  clutter_actor_show_all (b);
  clutter_actor_raise (b, a);
  
  clutter_actor_get_size (stage, &width, &height);
  clutter_actor_set_scale_full (b, 0, 0, width/2.0, -height/2.0);
  //  clutter_actor_set_opacity (b, 0x00);

  clutter_actor_animate_with_timeline (b, CLUTTER_EASE_IN_OUT_SINE, timeline, "scale-x", (gdouble)1, "scale-y", (gdouble)1, NULL);
  clutter_actor_animate_with_timeline (a, CLUTTER_EASE_IN_EXPO, timeline, "opacity", 0, NULL);
  
  clutter_timeline_start (timeline);
  g_signal_connect (timeline, "completed", G_CALLBACK (glide_animations_fade_completed), a);
  
  return timeline;
  
}

ClutterTimeline *
glide_animations_animate_drop (ClutterActor *a, ClutterActor *b, guint duration)
{
  ClutterTimeline *timeline = clutter_timeline_new (duration);
  ClutterActor *stage = clutter_actor_get_stage (a);
  
  clutter_actor_show_all (b);
  clutter_actor_raise (b, a);

  clutter_actor_set_y (b, -clutter_actor_get_height (CLUTTER_ACTOR (stage)));
  clutter_actor_animate_with_timeline (b, CLUTTER_EASE_OUT_BOUNCE, timeline, "y", 0, NULL);  
  
  clutter_timeline_start (timeline);
  
  g_signal_connect (timeline, "completed", G_CALLBACK (glide_animations_fade_completed), a);
  
  return timeline;
}


ClutterTimeline *
glide_animations_animate_zoom_contents (ClutterActor *a, ClutterActor *b, guint duration)
{

  ClutterTimeline *timeline = clutter_timeline_new (duration);
  ClutterActor *stage = clutter_actor_get_stage (a);
  ClutterActor *ac, *bc;
  gfloat width, height;
  
  ac = glide_slide_get_contents (GLIDE_SLIDE (a));
  bc = glide_slide_get_contents (GLIDE_SLIDE (b));
  
  clutter_actor_show_all (b);
  clutter_actor_raise (b, a);
  
  clutter_actor_set_opacity (b, 0x00);
  clutter_actor_get_size (stage, &width, &height);
  
  clutter_actor_set_scale_full (bc, 1.5, 1.5, width/2.0, height/2.0);
  
  clutter_actor_animate_with_timeline (bc, CLUTTER_EASE_IN_OUT_SINE, timeline, "scale-x", (gdouble)1, "scale-y", (gdouble)1, NULL);
  clutter_actor_animate_with_timeline (b, CLUTTER_EASE_IN_OUT_SINE, timeline,
				       "opacity", 0xff, NULL);
  clutter_actor_animate_with_timeline (a, CLUTTER_EASE_IN_OUT_SINE, timeline,
				       "opacity", 0x00, NULL);
  
  clutter_timeline_start (timeline);
  
  g_signal_connect (timeline, "completed", G_CALLBACK (glide_animations_fade_completed), a);
  
  return timeline;
}

ClutterTimeline *
glide_animations_animate_pivot_contents (ClutterActor *a, ClutterActor *b, guint duration)
{
  ClutterTimeline *timeline = clutter_timeline_new (duration);
  ClutterActor *stage = clutter_actor_get_stage (a);
  ClutterActor *ac, *bc;
  gfloat width, height;
  
  ac = glide_slide_get_contents (GLIDE_SLIDE (a));
  bc = glide_slide_get_contents (GLIDE_SLIDE (b));
  
  clutter_actor_show_all (b);
  clutter_actor_raise (b, a);
  
  clutter_actor_set_opacity (b, 0x00);
  clutter_actor_get_size (stage, &width, &height);
  
  clutter_actor_set_scale_full (bc, 1.5, 1.5, width/2.0, height/2.0);
  
  clutter_actor_animate_with_timeline (bc, CLUTTER_EASE_IN_OUT_SINE, timeline, "scale-x", (gdouble)1, "scale-y", (gdouble)1, NULL);
  clutter_actor_animate_with_timeline (b, CLUTTER_EASE_IN_OUT_SINE, timeline,
				       "opacity", 0xff, NULL);
  clutter_actor_animate_with_timeline (a, CLUTTER_EASE_IN_OUT_SINE, timeline,
				       "opacity", 0x00, NULL);
  
  clutter_timeline_start (timeline);
  
  g_signal_connect (timeline, "completed", G_CALLBACK (glide_animations_fade_completed), a);
  
  return timeline;
}

typedef struct
_doorway_finished_data_t {
  ClutterActor *a;
  ClutterActor *b;
  ClutterActor *left;
  ClutterActor *right;
  ClutterActor *reflection;
  ClutterActor *group;
} doorway_finished_data_t;

static void
glide_animations_doorway_completed (ClutterTimeline *t, gpointer user_data)
{
  ClutterActor *stage;
  doorway_finished_data_t *d = (doorway_finished_data_t *)user_data;

  clutter_actor_set_opacity (d->a, 0xff);
  clutter_actor_hide (d->a);
  
  stage = clutter_actor_get_stage (d->a);
    
  clutter_actor_reparent (d->b, stage);

  clutter_container_remove_actor (CLUTTER_CONTAINER (stage), d->left);
  clutter_container_remove_actor (CLUTTER_CONTAINER (stage), d->right);
  clutter_container_remove_actor (CLUTTER_CONTAINER (d->group), d->reflection);
  clutter_container_remove_actor (CLUTTER_CONTAINER (stage), d->group);
  
  g_free (d);
}

ClutterTimeline *
glide_animations_animate_doorway (ClutterActor *a, ClutterActor *b, guint duration)
{
  ClutterTimeline *timeline = clutter_timeline_new (duration);
  ClutterActor *stage = clutter_actor_get_stage (a);
  ClutterActor *left, *right, *reflection;
  ClutterVertex rotation_center;
  gfloat width, height;
  ClutterActor *group = clutter_group_new ();
  doorway_finished_data_t *d = (doorway_finished_data_t *)g_malloc (sizeof (doorway_finished_data_t));
  
  clutter_actor_show_all (b);
  clutter_actor_raise (a, b);
  
  left = clutter_clone_new (a);
  right = clutter_clone_new (a);
  
  reflection = clutter_clone_new (b);
  
  rotation_center.x = width;
  rotation_center.y = 0;
  rotation_center.z = 0;
  
  g_object_set (reflection, "rotation-center-z", &rotation_center, NULL);

  g_object_set (reflection, "rotation-angle-z", (gdouble)180, NULL);
  g_object_set (reflection, "rotation-angle-y", (gdouble)180, NULL);
  
  
  
  clutter_actor_set_size (a, 800, 600);
  clutter_actor_set_size (b, 800, 600);
  
  clutter_actor_get_size (stage, &width, &height);
  clutter_actor_set_size (left, width, height);
  clutter_actor_set_size (right, width, height);
  clutter_actor_set_size (reflection, width, height);

  clutter_actor_set_opacity (reflection, 80);
  clutter_actor_set_position (reflection, 0, height*2);

  
  rotation_center.x = width;
  rotation_center.y = 0;
  rotation_center.z = 0;
  
  g_object_set (right, "rotation-center-y", &rotation_center, NULL);
  
  clutter_actor_set_clip (left, 0, 0, width/2.0, height);
  clutter_actor_set_clip (right, width/2.0, 0, width, height);
  
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), left);
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), right);  
  clutter_actor_show (left);
  clutter_actor_show (right);

  clutter_actor_show (reflection);
  
  clutter_actor_set_position (left, 0, 0);
  clutter_actor_set_position (right, 0, 0);
  
  clutter_actor_set_opacity (a, 0x00);
  
  clutter_container_remove_actor (CLUTTER_CONTAINER (stage), b);
  clutter_container_add_actor (CLUTTER_CONTAINER (group), b);
  clutter_container_add_actor (CLUTTER_CONTAINER (group), reflection);
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), group);
  

  clutter_actor_set_size (group, 800, 600);
  clutter_actor_set_scale_full (group, 0.5, 0.5, width/2.0, height/2.0);
  
  clutter_actor_show (group);
  
  clutter_actor_raise (left, group);
  clutter_actor_raise (right, group);

  clutter_actor_animate_with_timeline (left, CLUTTER_EASE_OUT_SINE,
				       timeline,
				       "x", -width/2.0,
				       "rotation-angle-y", (gdouble)10,
				       "opacity", 0x00,
				       NULL);
  clutter_actor_animate_with_timeline (right, CLUTTER_EASE_OUT_SINE,
  				       timeline,
  				       "x", width-width/2.0,
				       "rotation-angle-y", (gdouble)-10,
				       "opacity", 0x00,
  				       NULL);
  clutter_actor_animate_with_timeline (group, CLUTTER_EASE_OUT_QUAD,
				       timeline,
				       "scale-x", (gdouble)1,
				       "scale-y", (gdouble)1,
				       NULL);
  clutter_timeline_start (timeline);
  
  d->a = a;
  d->b = b;
  d->left = left;
  d->right = right;
  d->reflection = reflection;
  d->group = group;
  
  g_signal_connect (timeline, "completed", G_CALLBACK (glide_animations_doorway_completed), d);
  
  return timeline;
}
