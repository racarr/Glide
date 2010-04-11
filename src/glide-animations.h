/*
 * glide-animations.h
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

#ifndef __GLIDE_ANIMATIONS_H__
#define __GLIDE_ANIMATIONS_H__

#include <clutter/clutter.h>

ClutterTimeline *glide_animations_animate_fade (ClutterActor *a, ClutterActor *b, guint duration);
ClutterTimeline *glide_animations_animate_drop (ClutterActor *a, ClutterActor *b, guint duration);
ClutterTimeline *glide_animations_animate_zoom (ClutterActor *a, ClutterActor *b, guint duration);
ClutterTimeline *glide_animations_animate_zoom_contents (ClutterActor *a, ClutterActor *b, guint duration);
ClutterTimeline *glide_animations_animate_pivot (ClutterActor *a, ClutterActor *b, guint duration);
ClutterTimeline *glide_animations_animate_slide (ClutterActor *a, ClutterActor *b, guint duration);
ClutterTimeline *glide_animations_animate_doorway (ClutterActor *a, ClutterActor *b, guint duration);

#endif
