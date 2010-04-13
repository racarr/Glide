/*
 * glide-manipulator-private.h
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
 * MERCHANMANIPULATORILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, 
 * Boston, MA 02111-1307, USA.
 */
 
#ifndef __GLIDE_MANIPULATOR_PRIVATE_H__
#define __GLIDE_MANIPULATOR_PRIVATE_H__

#include "glide-manipulator.h"

G_BEGIN_DECLS

typedef enum
{
  WIDGET_NONE,
  WIDGET_TOP_LEFT,
  WIDGET_TOP_RIGHT,
  WIDGET_BOTTOM_LEFT,
  WIDGET_BOTTOM_RIGHT,
  WIDGET_TOP,
  WIDGET_LEFT,
  WIDGET_RIGHT,
  WIDGET_BOTTOM
} GlideManipulatorWidget;

typedef enum
  {
    WIDGET_MODE_RESIZE,
    WIDGET_MODE_ROTATE
  } GlideManipulatorWidgetMode;

struct _GlideManipulatorPrivate
{
  ClutterActor *target;
  
  GlideManipulatorWidgetMode mode;

  GlideManipulatorWidget hovered;
  
  gboolean transforming;
  GlideManipulatorWidget resize_widget;

  gboolean swap_widgets;
  
  gfloat rot_angle;
  
  gboolean width_only;
  
  CoglHandle widget_material;
  CoglHandle widget_active_material;
  
  gboolean motion_since_press;
};

G_END_DECLS

#endif  /* __GLIDE_MANIPULATOR_PRIVATE_H__  */
