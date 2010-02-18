/*
 * glide-manipulator.h
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

#ifndef __GLIDE_MANIPULATOR_H__
#define __GLIDE_MANIPULATOR_H__

#include <gtk/gtk.h>

#include <clutter/clutter.h>
#include <clutter-gtk/clutter-gtk.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define GLIDE_TYPE_MANIPULATOR              (glide_manipulator_get_type())
#define GLIDE_MANIPULATOR(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), GLIDE_TYPE_MANIPULATOR, GlideManipulator))
#define GLIDE_MANIPULATOR_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), GLIDE_TYPE_MANIPULATOR, GlideManipulatorClass))
#define GLIDE_IS_MANIPULATOR(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), GLIDE_TYPE_MANIPULATOR))
#define GLIDE_IS_MANIPULATOR_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GLIDE_TYPE_MANIPULATOR))
#define GLIDE_MANIPULATOR_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), GLIDE_TYPE_MANIPULATOR, GlideManipulatorClass))

/* Private structure type */
typedef struct _GlideManipulatorPrivate GlideManipulatorPrivate;

/*
 * Main object structure
 */
typedef struct _GlideManipulator GlideManipulator;

struct _GlideManipulator 
{
  ClutterRectangle rectangle;
  
  GlideManipulatorPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _GlideManipulatorClass GlideManipulatorClass;

struct _GlideManipulatorClass 
{
	ClutterRectangleClass parent_class;
};

/*
 * Public methods
 */
GType 		 glide_manipulator_get_type 			(void) G_GNUC_CONST;
GlideManipulator     *glide_manipulator_new                     (ClutterActor *target);

ClutterActor         *glide_manipulator_get_target                (GlideManipulator *manip);
void glide_manipulator_set_target (GlideManipulator *manip, ClutterActor *actor);

G_END_DECLS

#endif  /* __GLIDE_MANIPULATOR_H__  */
