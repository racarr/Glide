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

#include "glide-debug.h"
#include "glide-actor.h"

#include <math.h>

#include "glide-manipulator-priv.h"
#include "glide-dirs.h"


G_DEFINE_TYPE(GlideManipulator, glide_manipulator, CLUTTER_TYPE_RECTANGLE)

#define GLIDE_MANIPULATOR_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GLIDE_TYPE_MANIPULATOR, GlideManipulatorPrivate))

enum {
  PROP_0,
  PROP_TARGET,
  PROP_WIDTH_ONLY
};

#define GLIDE_MANIPULATOR_BORDER_WIDTH 1.5
#define GLIDE_MANIPULATOR_WIDGET_WIDTH 7.5


static void
glide_manipulator_finalize (GObject *object)
{
  GlideManipulator *m = GLIDE_MANIPULATOR(object);
  GLIDE_NOTE (MANIPULATOR,
	      "finalizing manipulator '%s'",
	      GLIDE_ACTOR_DISPLAY_NAME (CLUTTER_ACTOR (object)));
  
  cogl_handle_unref (m->priv->widget_material);
  m->priv->widget_material = COGL_INVALID_HANDLE;

  cogl_handle_unref (m->priv->widget_active_material);
  m->priv->widget_active_material = COGL_INVALID_HANDLE;

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
  
  if ((ax > -2*GLIDE_MANIPULATOR_WIDGET_WIDTH) &&
      (ax < 0))
    {
      if ((ay > -2 * GLIDE_MANIPULATOR_WIDGET_WIDTH) &&
	  (ay < 0) &&
	  (!self->priv->width_only))
	{
	  return WIDGET_TOP_LEFT;
	}
      else if ((ay > geom.height) &&
	       (ay < 2*GLIDE_MANIPULATOR_WIDGET_WIDTH + geom.height) &&
	       (!self->priv->width_only))
	{
	  return WIDGET_BOTTOM_LEFT;
	}
      else if ((ay > -GLIDE_MANIPULATOR_WIDGET_WIDTH + geom.height/2.0) &&
	       (ay < GLIDE_MANIPULATOR_WIDGET_WIDTH + geom.height/2.0))
	{
	  return WIDGET_LEFT;
	}
    }
  else if ((ax > geom.width) &&
      (ax < 2*GLIDE_MANIPULATOR_WIDGET_WIDTH+geom.width))
    {
      if ((ay > - 2 *GLIDE_MANIPULATOR_WIDGET_WIDTH) &&
	  (ay < 0) &&
	  (!self->priv->width_only))
	{
	  return WIDGET_TOP_RIGHT;
	}
      else if ((ay > geom.height) &&
	       (ay < 2*GLIDE_MANIPULATOR_WIDGET_WIDTH + geom.height) &&
	       (!self->priv->width_only))
	{
	  return WIDGET_BOTTOM_RIGHT;
	}
      else if ((ay > -GLIDE_MANIPULATOR_WIDGET_WIDTH + geom.height/2.0) &&
	       (ay < GLIDE_MANIPULATOR_WIDGET_WIDTH + geom.height/2.0))
	{
	  return WIDGET_RIGHT;
	}
    }
  else if ((ax > -GLIDE_MANIPULATOR_WIDGET_WIDTH + geom.width/2.0) &&
	   (ax < GLIDE_MANIPULATOR_WIDGET_WIDTH + geom.width/2.0) &&
	   (!self->priv->width_only))
    {
      if ((ay > -2*GLIDE_MANIPULATOR_WIDGET_WIDTH) &&
	  (ay < 0))
	{
	  return WIDGET_TOP;
	}
      else if ((ay > geom.height) &&
	       (ay < 2*GLIDE_MANIPULATOR_WIDGET_WIDTH + geom.height))
	{
	  return WIDGET_BOTTOM;
	}
    }
  return WIDGET_NONE;
}


static void
glide_manipulator_paint_border (const ClutterColor *border_color, 
				ClutterGeometry *geom)
{

  cogl_set_source_color4ub (border_color->red, border_color->green, 
			    border_color->blue, border_color->alpha);

  
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
  gdouble ra = 0;

  if (manip->priv->hovered == widg ||
      manip->priv->resize_widget == widg)
    cogl_set_source (manip->priv->widget_active_material);

  else
    cogl_set_source (manip->priv->widget_material);
  
  switch (widg)
    {
    case WIDGET_TOP_LEFT:
      center_x = center_y = -GLIDE_MANIPULATOR_WIDGET_WIDTH;
      ra = 45;
      break;
    case WIDGET_TOP_RIGHT:
      center_x = geom->width + GLIDE_MANIPULATOR_WIDGET_WIDTH; 
      center_y = 0 - GLIDE_MANIPULATOR_WIDGET_WIDTH; 
      ra = 135;
      break;
    case WIDGET_BOTTOM_LEFT:
      center_x = 0-GLIDE_MANIPULATOR_WIDGET_WIDTH;
      center_y = geom->height+GLIDE_MANIPULATOR_WIDGET_WIDTH;
      ra = 135;
      break;
    case WIDGET_BOTTOM_RIGHT:
      center_x = geom->width + GLIDE_MANIPULATOR_WIDGET_WIDTH;
      center_y = geom->height + GLIDE_MANIPULATOR_WIDGET_WIDTH;
      ra = 45;
      break;
    case WIDGET_TOP:
      center_x = geom->width / 2.0;
      center_y = -GLIDE_MANIPULATOR_WIDGET_WIDTH;
      ra = 90;
      break;
    case WIDGET_BOTTOM:
      center_x = geom->width/2.0;
      center_y = geom->height + GLIDE_MANIPULATOR_WIDGET_WIDTH;
      ra = 90;
      break;
    case WIDGET_LEFT:
      center_x = -GLIDE_MANIPULATOR_WIDGET_WIDTH;
      center_y = geom->height/2.0;
      break;
    case WIDGET_RIGHT:
      center_x = geom->width + GLIDE_MANIPULATOR_WIDGET_WIDTH;
      center_y = geom->height/2.0;
      break;
    default:
      break;
    }
  
  cogl_push_matrix();

  cogl_translate(center_x, center_y, 0);
  cogl_rotate (ra, 0, 0, 1);
  cogl_translate(-center_x, -center_y, 0);
  
  if (manip->priv->mode == WIDGET_MODE_RESIZE)
    {
      cogl_rectangle_with_texture_coords  
	(-GLIDE_MANIPULATOR_WIDGET_WIDTH + center_x,
	 -GLIDE_MANIPULATOR_WIDGET_WIDTH + center_y,
	 GLIDE_MANIPULATOR_WIDGET_WIDTH + center_x,
	 GLIDE_MANIPULATOR_WIDGET_WIDTH + center_y,
	 0, 0, 1, 1);
    }
  else
    {
      cogl_path_new ();
      cogl_path_ellipse (center_x, center_y,
			 GLIDE_MANIPULATOR_WIDGET_WIDTH,
			 GLIDE_MANIPULATOR_WIDGET_WIDTH);
      cogl_path_fill ();
    }
  cogl_pop_matrix ();
}

static void
glide_manipulator_paint_widget_geom (ClutterGeometry *geom,
				     GlideManipulatorWidget widg)
{
  gfloat center_x = 0, center_y = 0;

  switch (widg)
    {
    case WIDGET_TOP_LEFT:
      center_x = center_y = -GLIDE_MANIPULATOR_WIDGET_WIDTH;
      break;
    case WIDGET_TOP_RIGHT:
      center_x = geom->width + GLIDE_MANIPULATOR_WIDGET_WIDTH; 
      center_y = 0 - GLIDE_MANIPULATOR_WIDGET_WIDTH; 
      break;
    case WIDGET_BOTTOM_LEFT:
      center_x = 0-GLIDE_MANIPULATOR_WIDGET_WIDTH;
      center_y = geom->height+GLIDE_MANIPULATOR_WIDGET_WIDTH;
      break;
    case WIDGET_BOTTOM_RIGHT:
      center_x = geom->width + GLIDE_MANIPULATOR_WIDGET_WIDTH;
      center_y = geom->height + GLIDE_MANIPULATOR_WIDGET_WIDTH;
      break;
    case WIDGET_TOP:
      center_x = geom->width / 2.0;
      center_y = -GLIDE_MANIPULATOR_WIDGET_WIDTH;
      break;
    case WIDGET_BOTTOM:
      center_x = geom->width/2.0;
      center_y = geom->height + GLIDE_MANIPULATOR_WIDGET_WIDTH;
      break;
    case WIDGET_LEFT:
      center_x = -GLIDE_MANIPULATOR_WIDGET_WIDTH;
      center_y = geom->height/2.0;
      break;
    case WIDGET_RIGHT:
      center_x = geom->width + GLIDE_MANIPULATOR_WIDGET_WIDTH;
      center_y = geom->height/2.0;
      break;
    default:
      break;
    }
  
  cogl_rectangle(-GLIDE_MANIPULATOR_WIDGET_WIDTH + center_x,
		 -GLIDE_MANIPULATOR_WIDGET_WIDTH + center_y,
		 GLIDE_MANIPULATOR_WIDGET_WIDTH + center_x,
		 GLIDE_MANIPULATOR_WIDGET_WIDTH + center_y);
  
}

static void
glide_manipulator_paint_widgets (GlideManipulator *manip,
				 ClutterGeometry *geom)
{
  if (!manip->priv->width_only)
    {
      glide_manipulator_paint_widget (manip, geom, 
				      WIDGET_TOP_LEFT);
      glide_manipulator_paint_widget (manip, geom, 
				      WIDGET_BOTTOM_LEFT);
      glide_manipulator_paint_widget (manip, geom, 
				      WIDGET_TOP_RIGHT);
      glide_manipulator_paint_widget (manip, geom, 
				      WIDGET_BOTTOM_RIGHT);
      glide_manipulator_paint_widget (manip, geom, 
				      WIDGET_TOP);
       glide_manipulator_paint_widget (manip, geom, 
				       WIDGET_BOTTOM);
    }

  glide_manipulator_paint_widget (manip, geom, 
				  WIDGET_LEFT);
  glide_manipulator_paint_widget (manip, geom, 
				  WIDGET_RIGHT);

}

static void
glide_manipulator_paint_widgets_geom (GlideManipulator *manip,
				      ClutterGeometry *geom)
{
  if (!manip->priv->width_only)
    {
      glide_manipulator_paint_widget_geom (geom, WIDGET_TOP_LEFT);
      glide_manipulator_paint_widget_geom (geom, WIDGET_BOTTOM_LEFT);
      glide_manipulator_paint_widget_geom (geom, WIDGET_TOP_RIGHT);
      glide_manipulator_paint_widget_geom (geom, WIDGET_BOTTOM_RIGHT);
      glide_manipulator_paint_widget_geom (geom, WIDGET_TOP);
      glide_manipulator_paint_widget_geom (geom, WIDGET_BOTTOM);
    }

  glide_manipulator_paint_widget_geom (geom, WIDGET_LEFT);
  glide_manipulator_paint_widget_geom (geom, WIDGET_RIGHT);

}
  
static void
glide_manipulator_sync_transforms (ClutterActor *manipulator,
				   ClutterActor *target)
{
  gfloat tx, ty, tw, th;
  
  clutter_actor_get_size (target, &tw, &th);
  clutter_actor_get_position (target, &tx, &ty);
  
  clutter_actor_set_position (manipulator, tx, ty);
  clutter_actor_set_size (manipulator, tw, th);
}


static void
glide_manipulator_paint (ClutterActor *self)
{
  GlideManipulator *manip = GLIDE_MANIPULATOR (self);
  ClutterGeometry geom;
  ClutterColor border_color = {0x00, 0x00, 0x00, 0xcc};
  
  glide_manipulator_sync_transforms (CLUTTER_ACTOR (manip), manip->priv->target);

  GLIDE_NOTE (PAINT,
	      "painting manipulator '%s'",
	      GLIDE_ACTOR_DISPLAY_NAME (self));
  clutter_actor_get_allocation_geometry (self, &geom);
  
  GLIDE_NOTE (PAINT, 
	      "paint to x: %d, y: %d, w: %d, h: %d "
	      "opacity: %i",
	      geom.x, geom.y, geom.width, geom.height,
	      clutter_actor_get_opacity (self));
  
  glide_manipulator_paint_border (&border_color, &geom);
  glide_manipulator_paint_widgets (manip, &geom);

  if (manip->priv->target)
    clutter_actor_queue_redraw (manip->priv->target);
}

static void
glide_manipulator_pick (ClutterActor *actor,
			const ClutterColor *color)
{
  ClutterGeometry geom;

  clutter_actor_get_allocation_geometry (actor, &geom);
  
  glide_manipulator_paint_border (color, &geom);
  glide_manipulator_paint_widgets_geom (GLIDE_MANIPULATOR (actor), &geom);

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
      
      manip->priv->motion_since_press = FALSE;
      

      manip->priv->rot_angle = 
	clutter_actor_get_rotation (CLUTTER_ACTOR (actor), 						           CLUTTER_Z_AXIS,
				    &rx, &ry, &rz);


      clutter_grab_pointer (actor);
      
      glide_undo_manager_start_actor_action (glide_actor_get_undo_manager (GLIDE_ACTOR (manip->priv->target)),
					     GLIDE_ACTOR (manip->priv->target),
					     "Resize object");

      return TRUE;
    }
   

  return FALSE;
}

static gboolean
glide_manipulator_button_release (ClutterActor *actor,
				  ClutterButtonEvent *bev)
{
  GlideManipulator *manip = GLIDE_MANIPULATOR (actor);
  gboolean ret = FALSE;
  
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

      //      clutter_actor_queue_redraw (actor);
      manip->priv->swap_widgets = FALSE;
      
      ret = TRUE;
    }
  
  if (manip->priv->transforming)
    {
      clutter_ungrab_pointer ();

      if (manip->priv->motion_since_press)
	glide_undo_manager_end_actor_action (glide_actor_get_undo_manager (GLIDE_ACTOR (manip->priv->target)),
					     GLIDE_ACTOR (manip->priv->target));
      else
	glide_undo_manager_cancel_actor_action (glide_actor_get_undo_manager (GLIDE_ACTOR (manip->priv->target)));

      manip->priv->transforming = FALSE;
      manip->priv->resize_widget = WIDGET_NONE;
      
      return TRUE;
    }
  
    return ret;
}

static void
glide_manipulator_process_resize (GlideManipulator *manip,
				  ClutterGeometry *geom,
				  ClutterMotionEvent *mev)
{
  //  ClutterActor *actor = CLUTTER_ACTOR(manip);
  switch (manip->priv->resize_widget)
    {
    case WIDGET_BOTTOM_RIGHT:
      clutter_actor_set_size(manip->priv->target, mev->x-geom->x, mev->y-geom->y);
      //      clutter_actor_set_size(CLUTTER_ACTOR (manip), mev->x-geom->x, mev->y-geom->y);
      break;
    case WIDGET_TOP_RIGHT:
      //      clutter_actor_set_position (actor, geom->x, mev->y);
      clutter_actor_set_position (manip->priv->target, geom->x, mev->y);
      clutter_actor_set_size (manip->priv->target, mev->x-geom->x, (geom->height+geom->y)-(mev->y));
      //      clutter_actor_set_size (CLUTTER_ACTOR(manip), mev->x-geom->x, (geom->height+geom->y)-(mev->y));
      break;
    case WIDGET_BOTTOM_LEFT:
      //      clutter_actor_set_position (actor, mev->x, geom->y);
      clutter_actor_set_position (manip->priv->target, mev->x, geom->y);
      clutter_actor_set_size (manip->priv->target, (geom->width+geom->x)-mev->x,
			      mev->y-geom->y);
      //      clutter_actor_set_size (CLUTTER_ACTOR (manip), (geom->width+geom->x)-mev->x,
      ///		      mev->y-geom->y);
      break;
    case WIDGET_TOP_LEFT:
      //      clutter_actor_set_position (actor, mev->x, mev->y);
      clutter_actor_set_position (manip->priv->target, mev->x, mev->y);
      clutter_actor_set_size (manip->priv->target,
			      (geom->width+geom->x)-mev->x,
			      (geom->height+geom->y)-mev->y);
      //      clutter_actor_set_size (CLUTTER_ACTOR (manip),
      //		      (geom->width+geom->x)-mev->x,
      //		      (geom->height+geom->y)-mev->y);
      break;
    case WIDGET_TOP:
      clutter_actor_set_position (manip->priv->target, geom->x, mev->y);
      clutter_actor_set_size (manip->priv->target,
			      geom->width,
			      (geom->height+geom->y)-mev->y);
      break;
    case WIDGET_BOTTOM:
      clutter_actor_set_size (manip->priv->target, geom->width,
			      mev->y-geom->y);
      break;
    case WIDGET_RIGHT:
      clutter_actor_set_size (manip->priv->target, mev->x-geom->x,
			      geom->height);
      break;
    case WIDGET_LEFT:
      clutter_actor_set_position (manip->priv->target, mev->x, geom->y);
      clutter_actor_set_size (manip->priv->target,
			      (geom->width+geom->x)-mev->x,
			      geom->height);
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
  ClutterVertex click_point = {0, 0, 0};
  ClutterVertex screen_click_point;
  gfloat v1x, v1y, v2x, v2y, h1, h2, deg;
  
  clutter_actor_apply_transform_to_point (CLUTTER_ACTOR (manip), &click_point,
					  &screen_click_point);
  g_message("screen_click: %f %f", screen_click_point.x,
	    screen_click_point.y);
  
  v1x = screen_click_point.x - (geom->x + geom->width/2.0);
  v1y = screen_click_point.y - (geom->y + geom->height/2.0);
  h1 = sqrt (v1x*v1x+v1y*v1y);
  
  v1x /= h1;
  v1y /= h1;

  v2x = mev->x - (geom->x + geom->width/2.0);
  v2y = mev->y - (geom->y + geom->height/2.0);
  h2 = sqrt (v2x*v2x+v2y*v2y);
  
  v2x /= h2;
  v2y /= h2;
  
  g_message("v1 x/y: %f %f", 
	    v1x, v1y);
  g_message("v2 x/y: %f %f", 
	    v2x, v2y);

  deg = acos((v1x*v2x+v1y*v2y))*(180/M_PI);
  
  g_message("h1, h2 %f %f", h1, h2);
  //  if (v2x < v1x || v2y < v1y)
  //    deg = -deg;
  
  clutter_actor_set_rotation (CLUTTER_ACTOR (manip),
			      CLUTTER_Z_AXIS,
			      manip->priv->rot_angle+deg+180,
			      geom->width/2.0,
			      geom->height/2.0,
			      0);
  clutter_actor_set_rotation (CLUTTER_ACTOR (manip->priv->target),
			      CLUTTER_Z_AXIS,
			      manip->priv->rot_angle+deg+180,
			      geom->width/2.0,
			      geom->height/2.0,
			      0);
  manip->priv->rot_angle += deg;
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
      manip->priv->motion_since_press = TRUE;

      if (manip->priv->mode == WIDGET_MODE_RESIZE)
	glide_manipulator_process_resize (manip, &geom, mev);
      else
	glide_manipulator_process_rotate (manip, &geom, mev);

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
    case PROP_WIDTH_ONLY:
      g_value_set_boolean (value, manip->priv->width_only);
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
  gfloat x,y, width, height, rx, ry, rz, za;
  
  if (!target)
    {
      clutter_actor_hide (CLUTTER_ACTOR (manip));
      return ;
    }
  
  clutter_actor_show (CLUTTER_ACTOR (manip));
  clutter_actor_get_position (target, &x, &y);
  clutter_actor_get_size (target, &width, &height);
  clutter_actor_set_position (CLUTTER_ACTOR (manip), x, y);
  clutter_actor_set_size (CLUTTER_ACTOR (manip), width, height);
  
  za = clutter_actor_get_rotation (CLUTTER_ACTOR (target), CLUTTER_Z_AXIS, &rx, &ry, &rz);
  
  clutter_actor_set_rotation (CLUTTER_ACTOR (manip),
			      CLUTTER_Z_AXIS,
			      za,
			      rx,
			      ry,
			      rz);
  
  clutter_actor_raise (CLUTTER_ACTOR (manip), target);
  
  manip->priv->target = target;
  
  clutter_actor_grab_key_focus (manip->priv->target);
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
      glide_manipulator_set_target_real (manip, CLUTTER_ACTOR(g_value_get_object (value)));
      break;
    case PROP_WIDTH_ONLY:
      glide_manipulator_set_width_only (manip, g_value_get_boolean (value));
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
							G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (object_class,
				   PROP_WIDTH_ONLY,
				   g_param_spec_boolean ("width-only",
							 "Manipulate width only",
							 "Whether to only expose manipulators "
							 "for width",
							 FALSE,
							 G_PARAM_READWRITE));
  
  g_type_class_add_private (object_class, sizeof(GlideManipulatorPrivate));
}

static CoglHandle
glide_manipulator_material_for_file (const gchar *filename)
{
  GError *e = NULL;
  CoglHandle m, t;
  
  m = cogl_material_new ();
  t = cogl_texture_new_from_file(filename,
				 COGL_TEXTURE_NONE,
				 COGL_PIXEL_FORMAT_ANY,
				 &e);
  if (e || t == COGL_INVALID_HANDLE)
    {
      g_warning ("glide-manipulator.c failed to load widget image: %s", filename);
      g_error_free (e);
    }
  cogl_material_set_layer (m, 0, t);
  cogl_material_set_layer_filters (m, 0,
				   COGL_MATERIAL_FILTER_LINEAR_MIPMAP_LINEAR,
				   COGL_MATERIAL_FILTER_LINEAR);
  cogl_handle_unref (t);
  
  return m;
}

static void
glide_manipulator_load_textures (GlideManipulator *manipulator)
{
  gchar *image_dir = glide_dirs_get_glide_image_dir ();
  gchar *p1 = g_build_filename(image_dir, "manipulator-widget.png", NULL);
  gchar *p2 = g_build_filename(image_dir, "manipulator-widget-active.png",
			       NULL);

  manipulator->priv->widget_material = glide_manipulator_material_for_file(p1);
  manipulator->priv->widget_active_material = glide_manipulator_material_for_file(p2);  
}

static void
glide_manipulator_init (GlideManipulator *manipulator)
{
  manipulator->priv = GLIDE_MANIPULATOR_GET_PRIVATE (manipulator);

  
  manipulator->priv->mode = WIDGET_MODE_RESIZE;

  glide_manipulator_load_textures (manipulator);
  
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

void
glide_manipulator_set_target (GlideManipulator *manip, ClutterActor *actor)
{
  glide_manipulator_set_target_real (manip, actor);
  g_object_notify (G_OBJECT (manip), "target");
}

gboolean
glide_manipulator_get_width_only (GlideManipulator *manip)
{
  return manip->priv->width_only;
}

void
glide_manipulator_set_width_only (GlideManipulator *manip,
				  gboolean width_only)
{
  if (width_only == manip->priv->width_only)
    return;
  manip->priv->width_only = width_only;
  clutter_actor_queue_redraw (CLUTTER_ACTOR (manip));
  
  g_object_notify (G_OBJECT (manip), "width-only");
}
