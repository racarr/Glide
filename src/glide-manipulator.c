/*
 * glide-manipulator.c
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
#include "glide-manipulator.h"

#include <math.h>

#include "glide-manipulator-priv.h"


G_DEFINE_TYPE(GlideManipulator, glide_manipulator, CLUTTER_TYPE_GROUP)

#define GLIDE_MANIPULATOR_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GLIDE_TYPE_MANIPULATOR, GlideManipulatorPrivate))

enum {
  PROP_0,
  PROP_TARGET
};

#define GLIDE_MANIPULATOR_BORDER_WIDTH 2.5
#define GLIDE_MANIPULATOR_WIDGET_WIDTH 4


static void
glide_manipulator_finalize (GObject *object)
{
  /* Debug? */

  G_OBJECT_CLASS (glide_manipulator_parent_class)->finalize (object);
}

static GlideManipulatorWidget
glide_manipulator_get_widget_at (GlideManipulator *self,
				 gfloat x, 
				 gfloat y)
{
  ClutterGeometry geom;
  gfloat ax, ay;
  
  clutter_actor_transform_stage_point (CLUTTER_ACTOR (self), x, y, &ax, &ay);
  
  clutter_actor_get_allocation_geometry (CLUTTER_ACTOR (self), &geom);
  
  if ((ax > -GLIDE_MANIPULATOR_WIDGET_WIDTH) &&
      (ax < GLIDE_MANIPULATOR_WIDGET_WIDTH))
    {
      if ((ay > -GLIDE_MANIPULATOR_WIDGET_WIDTH) &&
	  (ay < GLIDE_MANIPULATOR_WIDGET_WIDTH))
	{
	  return WIDGET_TOP_LEFT;
	}
      else if ((ay > -GLIDE_MANIPULATOR_WIDGET_WIDTH + geom.height) &&
	       (ay < GLIDE_MANIPULATOR_WIDGET_WIDTH + geom.height))
	{
	  return WIDGET_BOTTOM_LEFT;
	}
    }
  else if ((ax > -GLIDE_MANIPULATOR_WIDGET_WIDTH+geom.width) &&
      (ax < GLIDE_MANIPULATOR_WIDGET_WIDTH+geom.width))
    {
      if ((ay > -GLIDE_MANIPULATOR_WIDGET_WIDTH) &&
	  (ay < GLIDE_MANIPULATOR_WIDGET_WIDTH))
	{
	  return WIDGET_TOP_RIGHT;
	}
      else if ((ay > -GLIDE_MANIPULATOR_WIDGET_WIDTH + geom.height) &&
	       (ay < GLIDE_MANIPULATOR_WIDGET_WIDTH + geom.height))
	{
	  return WIDGET_BOTTOM_RIGHT;
	}
    }
  return WIDGET_NONE;
}

static void
glide_manipulator_pick (ClutterActor *actor,
		    const ClutterColor *color)
{
  ClutterGeometry geom;

  clutter_actor_get_allocation_geometry (actor, &geom);
  
  CLUTTER_ACTOR_CLASS (glide_manipulator_parent_class)->pick (actor, color);
  
  cogl_rectangle (-GLIDE_MANIPULATOR_WIDGET_WIDTH,
		  -GLIDE_MANIPULATOR_WIDGET_WIDTH,
		  geom.height + GLIDE_MANIPULATOR_WIDGET_WIDTH,
		  geom.height + GLIDE_MANIPULATOR_WIDGET_WIDTH);
}

static void
glide_manipulator_paint_border (ClutterGeometry *geom)
{

  cogl_set_source_color4ub (0xcc, 0xcc, 0xcc, 0xff);

  
  cogl_rectangle (GLIDE_MANIPULATOR_BORDER_WIDTH, 0, 
		  geom->width, GLIDE_MANIPULATOR_BORDER_WIDTH);
  cogl_rectangle (geom->width - GLIDE_MANIPULATOR_BORDER_WIDTH, 
		  GLIDE_MANIPULATOR_BORDER_WIDTH, 
		  geom->width, geom->height);
  cogl_rectangle (0, geom->height - GLIDE_MANIPULATOR_BORDER_WIDTH, 
		  geom->width - GLIDE_MANIPULATOR_BORDER_WIDTH, geom->height);
  cogl_rectangle (0, 0, 
		  GLIDE_MANIPULATOR_BORDER_WIDTH, 
		  geom->height - GLIDE_MANIPULATOR_BORDER_WIDTH);
}

static void
glide_manipulator_paint_widget (GlideManipulator *manip,
				ClutterGeometry *geom,
				GlideManipulatorWidget widg)
{
  gfloat center_x = 0, center_y = 0;

  if (manip->priv->hovered == widg ||
      manip->priv->resize_widget == widg)
    cogl_set_source_color4ub (0xff, 0xcc, 0xcc, 0xff);
  else
    cogl_set_source_color4ub (0xcc, 0xcc, 0xff, 0xff);
  
  switch (widg)
    {
    case WIDGET_TOP_LEFT:
      center_x = center_y = 0;
      break;
    case WIDGET_TOP_RIGHT:
      center_x = geom->width; 
      center_y = 0;
      break;
    case WIDGET_BOTTOM_LEFT:
      center_x = 0;
      center_y = geom->height;
      break;
    case WIDGET_BOTTOM_RIGHT:
      center_x = geom->width;
      center_y = geom->height;
      break;
    default:
      break;
    }
  
  if (manip->priv->mode == WIDGET_MODE_RESIZE)
    {
      cogl_rectangle (-GLIDE_MANIPULATOR_WIDGET_WIDTH + center_x,
		      -GLIDE_MANIPULATOR_WIDGET_WIDTH + center_y,
		      GLIDE_MANIPULATOR_WIDGET_WIDTH + center_x,
		      GLIDE_MANIPULATOR_WIDGET_WIDTH + center_y);
    }
  else
    {
      cogl_path_new ();
      cogl_path_ellipse (center_x, center_y,
			 GLIDE_MANIPULATOR_WIDGET_WIDTH,
			 GLIDE_MANIPULATOR_WIDGET_WIDTH);
      cogl_path_fill ();
    }
}

static void
glide_manipulator_paint_widgets (GlideManipulator *manip,
				 ClutterGeometry *geom)
{
  glide_manipulator_paint_widget (manip, geom, WIDGET_TOP_LEFT);
  glide_manipulator_paint_widget (manip, geom, WIDGET_BOTTOM_LEFT);
  glide_manipulator_paint_widget (manip, geom, WIDGET_TOP_RIGHT);
  glide_manipulator_paint_widget (manip, geom, WIDGET_BOTTOM_RIGHT);
}

static void
glide_manipulator_paint (ClutterActor *self)
{
  GlideManipulator *manip = GLIDE_MANIPULATOR (self);
  ClutterGeometry geom;
  guint n_children, i;
  
  n_children = clutter_group_get_n_children (CLUTTER_GROUP (self));
  
  for (i = 0; i < n_children; i++)
    {
      ClutterActor *c = clutter_group_get_nth_child (CLUTTER_GROUP (self), i);
      
      clutter_actor_paint (c);
    }
  
  clutter_actor_get_allocation_geometry (self, &geom);
  
  glide_manipulator_paint_border (&geom);
  glide_manipulator_paint_widgets (manip, &geom);
}

static gboolean
glide_manipulator_button_press (ClutterActor *actor,
				ClutterButtonEvent *event)
{
  GlideManipulator *manip = GLIDE_MANIPULATOR (actor);
  GlideManipulatorWidget widg;
  gfloat ax, ay;

  if (event->button != 1)
    {
      return FALSE;
    }
  
  manip->priv->swap_widgets = TRUE;

  clutter_actor_get_position (actor, &ax, &ay);
  
  widg = glide_manipulator_get_widget_at (manip, event->x, event->y);
  if (widg != WIDGET_NONE)
    {
      gfloat rx, ry, rz;
      manip->priv->transforming = TRUE;
      manip->priv->resize_widget = widg;
      
      manip->priv->rpx = event->x;
      manip->priv->rpy = event->y;

      manip->priv->ra = clutter_actor_get_rotation (CLUTTER_ACTOR (actor), 
						    CLUTTER_Z_AXIS,
						    &rx, &ry, &rz);


      clutter_grab_pointer (actor);

      return TRUE;
    }
    
  manip->priv->dragging = TRUE;

  //Proper point transforms?
  manip->priv->drag_center_x = event->x - ax;
  manip->priv->drag_center_y = event->y - ay;

  clutter_grab_pointer (actor);
  
  return TRUE;
}

static gboolean
glide_manipulator_button_release (ClutterActor *actor,
				  ClutterButtonEvent *bev)
{
  GlideManipulator *manip = GLIDE_MANIPULATOR (actor);
  
  if (manip->priv->swap_widgets)
    {
      if (manip->priv->mode == WIDGET_MODE_RESIZE)
	{
	  g_message("rotate");
	  manip->priv->mode = WIDGET_MODE_ROTATE;
	}
      else
	{
	  g_message("resize");
	  manip->priv->mode = WIDGET_MODE_RESIZE;
	}

      clutter_actor_queue_redraw (actor);
      manip->priv->swap_widgets = FALSE;
    }
  
  if (manip->priv->transforming)
    {
      clutter_ungrab_pointer ();
      manip->priv->transforming = FALSE;
      manip->priv->resize_widget = WIDGET_NONE;
      
      return TRUE;
    }
  
  if (manip->priv->dragging);
    {
      clutter_ungrab_pointer ();
      manip->priv->dragging = FALSE;
      
      return TRUE;
    }
    return FALSE;
}

static void
glide_manipulator_process_resize (GlideManipulator *manip,
				  ClutterGeometry *geom,
				  ClutterMotionEvent *mev)
{
  ClutterActor *actor = CLUTTER_ACTOR(manip);
  switch (manip->priv->resize_widget)
    {
    case WIDGET_BOTTOM_RIGHT:
      clutter_actor_set_size(manip->priv->target, mev->x-geom->x, mev->y-geom->y);
      break;
    case WIDGET_TOP_RIGHT:
      clutter_actor_set_position (actor, geom->x, mev->y);
      clutter_actor_set_size (manip->priv->target, mev->x-geom->x, (geom->height+geom->y)-(mev->y));
      break;
    case WIDGET_BOTTOM_LEFT:
      clutter_actor_set_position (actor, mev->x, geom->y);
      clutter_actor_set_size (manip->priv->target, (geom->width+geom->x)-mev->x,
			      mev->y-geom->y);
      break;
    case WIDGET_TOP_LEFT:
      clutter_actor_set_position (actor, mev->x, mev->y);
      clutter_actor_set_size (manip->priv->target,
			      (geom->width+geom->x)-mev->x,
			      (geom->height+geom->y)-mev->y);
      break;
    default:
      break;
    }
}

static void
glide_manipulator_process_rotate (GlideManipulator *manip,
				  ClutterGeometry *geom,
				  ClutterMotionEvent *mev)
{
  // Epic fail.
}

static gboolean
glide_manipulator_motion (ClutterActor *actor,
			  ClutterMotionEvent *mev)
{
  ClutterGeometry geom;
  GlideManipulatorWidget widg;
  GlideManipulator *manip = GLIDE_MANIPULATOR (actor);
  gfloat ax,ay;

  manip->priv->swap_widgets = FALSE;
  
  clutter_actor_get_position (actor, &ax, &ay);
  clutter_actor_get_allocation_geometry (actor, &geom);
  
  widg = glide_manipulator_get_widget_at (manip, mev->x, mev->y);
  
  if (manip->priv->hovered != widg)
    {
      manip->priv->hovered = widg;
      clutter_actor_queue_redraw (CLUTTER_ACTOR (actor));
    }
  
  if (manip->priv->transforming)
    {
      if (manip->priv->mode == WIDGET_MODE_RESIZE)
	glide_manipulator_process_resize (manip, &geom, mev);
      else
	glide_manipulator_process_rotate (manip, &geom, mev);

    }

  if (manip->priv->dragging)
    {
      clutter_actor_set_position (actor, 
				  mev->x - manip->priv->drag_center_x,
				  mev->y - manip->priv->drag_center_y);
      return TRUE;
    }
  return FALSE;
}

static void
glide_manipulator_get_property (GObject *object,
				guint prop_id,
				GValue *value,
				GParamSpec *pspec)
{
  GlideManipulator *manip = GLIDE_MANIPULATOR (object);
  
  switch (prop_id)
    {
    case PROP_TARGET:
      g_value_set_object (value, manip->priv->target);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
glide_manipulator_set_target_real (GlideManipulator *manip,
				   ClutterActor *target)
{
  clutter_container_add_actor (CLUTTER_CONTAINER (manip), target);
  
  if (CLUTTER_ACTOR_IS_MAPPED (CLUTTER_ACTOR (manip)))
    clutter_actor_show (target);
  manip->priv->target = target;
}
				   

static void
glide_manipulator_set_property (GObject *object,
				guint prop_id,
				const GValue *value,
				GParamSpec *pspec)
{
  GlideManipulator *manip = GLIDE_MANIPULATOR (object);
  
  switch (prop_id)
    {
    case PROP_TARGET:
      g_return_if_fail (manip->priv->target == NULL);
      glide_manipulator_set_target_real (manip, CLUTTER_ACTOR(g_value_get_object (value)));
      break;
    default: 
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
glide_manipulator_class_init (GlideManipulatorClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  
  actor_class->paint = glide_manipulator_paint;
  actor_class->pick = glide_manipulator_pick;

  actor_class->button_press_event = glide_manipulator_button_press;
  actor_class->button_release_event = glide_manipulator_button_release;
  actor_class->motion_event = glide_manipulator_motion;


  object_class->finalize = glide_manipulator_finalize;
  object_class->set_property = glide_manipulator_set_property;
  object_class->get_property = glide_manipulator_get_property;
  
  g_object_class_install_property (object_class,
				   PROP_TARGET,
				   g_param_spec_object ("target",
							"Target",
							"The target of the manipulator object",
							CLUTTER_TYPE_ACTOR,
							G_PARAM_READWRITE |
							G_PARAM_CONSTRUCT_ONLY |
							G_PARAM_STATIC_STRINGS));
  
  g_type_class_add_private (object_class, sizeof(GlideManipulatorPrivate));
}

static void
glide_manipulator_init (GlideManipulator *manipulator)
{
  manipulator->priv = GLIDE_MANIPULATOR_GET_PRIVATE (manipulator);
  
  manipulator->priv->mode = WIDGET_MODE_RESIZE;
  
  clutter_actor_set_reactive (CLUTTER_ACTOR (manipulator), TRUE);
}

GlideManipulator *
glide_manipulator_new (ClutterActor *target)
{
  return g_object_new (GLIDE_TYPE_MANIPULATOR, "target", target, NULL);
}

ClutterActor *
glide_manipulator_get_target (GlideManipulator *manip)
{
  return manip->priv->target;
}
