/*
 * glide-slide.c
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

#include "glide-slide.h"
#include "glide-slide-priv.h"

#include "glide-json-util.h"

#include "glide-debug.h"

static void clutter_container_iface_init (ClutterContainerIface *iface);

G_DEFINE_TYPE_WITH_CODE (GlideSlide, glide_slide, GLIDE_TYPE_ACTOR,
	 G_IMPLEMENT_INTERFACE (CLUTTER_TYPE_CONTAINER,
				clutter_container_iface_init));

#define GLIDE_SLIDE_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GLIDE_TYPE_SLIDE, GlideSlidePrivate))

enum {
  PROP_0,
  PROP_DOCUMENT,
  PROP_BACKGROUND,
  PROP_ANIMATION,
  PROP_COLOR
};

static const ClutterColor default_slide_color = { 0xff, 0xff, 0xff, 0xff };

static void
glide_slide_get_property (GObject *object, 
			  guint prop_id,
			  GValue *value,
			  GParamSpec *pspec)
{
  GlideSlide *slide = GLIDE_SLIDE (object);
  
  switch (prop_id)
    {
    case PROP_DOCUMENT:
      g_value_set_object (value, slide->priv->document);
      break;
    case PROP_BACKGROUND:
      g_value_set_string (value, slide->priv->background);
      break;
    case PROP_ANIMATION:
      g_value_set_string (value, slide->priv->animation);
      break;
    case PROP_COLOR:
      clutter_value_set_color (value, &slide->priv->color);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
glide_slide_set_property (GObject *object,
			  guint prop_id,
			  const GValue *value,
			  GParamSpec *pspec)
{
  GlideSlide *slide = GLIDE_SLIDE (object);
  
  switch (prop_id)
    {
    case PROP_DOCUMENT:
      g_return_if_fail (slide->priv->document == NULL);
      slide->priv->document = GLIDE_DOCUMENT (g_value_get_object (value));
      break;
    case PROP_BACKGROUND:
      glide_slide_set_background (slide, g_value_get_string (value));
      break;
    case PROP_ANIMATION:
      glide_slide_set_animation (slide, g_value_get_string (value));
      break;
    case PROP_COLOR:
      glide_slide_set_color (slide, clutter_value_get_color (value));
      break;
    default: 
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static gint
sort_by_depth (gconstpointer a,
               gconstpointer b)
{
  gfloat depth_a = clutter_actor_get_depth (CLUTTER_ACTOR(a));
  gfloat depth_b = clutter_actor_get_depth (CLUTTER_ACTOR(b));

  if (depth_a < depth_b)
    return -1;

  if (depth_a > depth_b)
    return 1;

  return 0;
}

static void
glide_slide_add (ClutterContainer *container,
		 ClutterActor     *actor)
{
  GlideSlidePrivate *priv = GLIDE_SLIDE (container)->priv;

  g_object_ref (actor);

  priv->children = g_list_append (priv->children, actor);
  clutter_actor_set_parent (actor, CLUTTER_ACTOR (container));

  /* queue a relayout, to get the correct positioning inside
   * the ::actor-added signal handlers
   */
  clutter_actor_queue_relayout (CLUTTER_ACTOR (container));

  g_signal_emit_by_name (container, "actor-added", actor);

  clutter_container_sort_depth_order (container);

  g_object_unref (actor);
}

static void
glide_slide_remove (ClutterContainer *container,
                           ClutterActor     *actor)
{
  GlideSlidePrivate *priv = GLIDE_SLIDE (container)->priv;

  g_object_ref (actor);

  priv->children = g_list_remove (priv->children, actor);
  clutter_actor_unparent (actor);

  /* queue a relayout, to get the correct positioning inside
   * the ::actor-removed signal handlers
   */
  clutter_actor_queue_relayout (CLUTTER_ACTOR (container));

  /* at this point, the actor passed to the "actor-removed" signal
   * handlers is not parented anymore to the container but since we
   * are holding a reference on it, it's still valid
   */
  g_signal_emit_by_name (container, "actor-removed", actor);

  clutter_actor_queue_redraw (CLUTTER_ACTOR (container));

  g_object_unref (actor);
}

static void
glide_slide_foreach (ClutterContainer *container,
                            ClutterCallback   callback,
                            gpointer          user_data)
{
  GlideSlidePrivate *priv = GLIDE_SLIDE (container)->priv;

  /* Using g_list_foreach instead of iterating the list manually
     because it has better protection against the current node being
     removed. This will happen for example if someone calls
     clutter_container_foreach(container, clutter_actor_destroy) */
  g_list_foreach (priv->children, (GFunc) callback, user_data);
}

static void
glide_slide_raise (ClutterContainer *container,
		   ClutterActor     *actor,
		   ClutterActor     *sibling)
{
  GlideSlidePrivate *priv = GLIDE_SLIDE (container)->priv;

  priv->children = g_list_remove (priv->children, actor);

  /* Raise at the top */
  if (!sibling)
    {
      GList *last_item;

      last_item = g_list_last (priv->children);

      if (last_item)
	sibling = last_item->data;

      priv->children = g_list_append (priv->children, actor);
    }
  else
    {
      gint index_ = g_list_index (priv->children, sibling) + 1;

      priv->children = g_list_insert (priv->children, actor, index_);
    }

  /* set Z ordering a value below, this will then call sort
   * as values are equal ordering shouldn't change but Z
   * values will be correct.
   *
   * FIXME: optimise
   */
  if (sibling &&
      clutter_actor_get_depth (sibling) != clutter_actor_get_depth (actor))
    {
      clutter_actor_set_depth (actor, clutter_actor_get_depth (sibling));
    }

  clutter_actor_queue_redraw (CLUTTER_ACTOR (container));
}

static void
glide_slide_lower (ClutterContainer *container,
		   ClutterActor     *actor,
		   ClutterActor     *sibling)
{
  GlideSlide *self = GLIDE_SLIDE (container);
  GlideSlidePrivate *priv = self->priv;

  priv->children = g_list_remove (priv->children, actor);

  /* Push to bottom */
  if (!sibling)
    {
      GList *last_item;

      last_item = g_list_first (priv->children);

      if (last_item)
	sibling = last_item->data;

      priv->children = g_list_prepend (priv->children, actor);
    }
  else
    {
      gint index_ = g_list_index (priv->children, sibling);

      priv->children = g_list_insert (priv->children, actor, index_);
    }

  /* See comment in group_raise for this */
  if (sibling &&
      clutter_actor_get_depth (sibling) != clutter_actor_get_depth (actor))
    {
      clutter_actor_set_depth (actor, clutter_actor_get_depth (sibling));
    }

  clutter_actor_queue_redraw (CLUTTER_ACTOR (container));
}

static void
glide_slide_sort_depth_order (ClutterContainer *container)
{
  GlideSlidePrivate *priv = GLIDE_SLIDE (container)->priv;

  priv->children = g_list_sort (priv->children, sort_by_depth);

  clutter_actor_queue_redraw (CLUTTER_ACTOR (container));
}

static void
clutter_container_iface_init (ClutterContainerIface *iface)
{
  iface->add = glide_slide_add;
  iface->remove = glide_slide_remove;
  iface->foreach = glide_slide_foreach;
  iface->raise = glide_slide_raise;
  iface->lower = glide_slide_lower;
  iface->sort_depth_order = glide_slide_sort_depth_order;
}

static void
glide_slide_paint (ClutterActor *actor)
{
  GlideSlidePrivate *priv = GLIDE_SLIDE (actor)->priv;
  guint8 paint_opacity = clutter_actor_get_paint_opacity (actor);
  gfloat width, height;
  
  if (paint_opacity == 0)
    {
      return;
    }

  clutter_actor_get_size (clutter_actor_get_parent (actor), &width, &height);
  cogl_set_source_color4ub (priv->color.red,
			    priv->color.green,
			    priv->color.blue,
			    paint_opacity * priv->color.alpha/255);
  cogl_rectangle(0, 0, width, height);
  
  if (priv->background_material) 
    {
      
      cogl_material_set_color4ub (priv->background_material,
				  paint_opacity, paint_opacity, paint_opacity, paint_opacity);
      
      cogl_set_source (priv->background_material);
      cogl_rectangle_with_texture_coords (0, 0,
					  width, 
					  height,
					  0, 0, 1, 1);
    }

  GLIDE_NOTE (PAINT, "GlideSlide paint enter '%s'",
                clutter_actor_get_name (actor) ? clutter_actor_get_name (actor)
                                               : "unknown");

  g_list_foreach (priv->children, (GFunc) clutter_actor_paint, NULL);

  GLIDE_NOTE (PAINT, "GlideSlide paint leave '%s'",
                clutter_actor_get_name (actor) ? clutter_actor_get_name (actor)
                                               : "unknown");
}

static void
glide_slide_pick (ClutterActor       *actor,
		  const ClutterColor *pick)
{
  GlideSlidePrivate *priv = GLIDE_SLIDE (actor)->priv;
  
  if (glide_stage_manager_get_presenting 
      (glide_actor_get_stage_manager (GLIDE_ACTOR (actor))))
    return;

  /* Chain up so we get a bounding box pained (if we are reactive) */
  CLUTTER_ACTOR_CLASS (glide_slide_parent_class)->pick (actor, pick);

  g_list_foreach (priv->children, (GFunc) clutter_actor_paint, NULL);
}

static void
glide_slide_get_preferred_width (ClutterActor *actor,
                                        gfloat        for_height,
                                        gfloat       *min_width,
                                        gfloat       *natural_width)
{
  GlideSlidePrivate *priv = GLIDE_SLIDE (actor)->priv;

  clutter_layout_manager_get_preferred_width (priv->layout,
                                              CLUTTER_CONTAINER (actor),
                                              for_height,
                                              min_width, natural_width);
}

static void
glide_slide_get_preferred_height (ClutterActor *actor,
                                         gfloat        for_width,
                                         gfloat       *min_height,
                                         gfloat       *natural_height)
{
  GlideSlidePrivate *priv = GLIDE_SLIDE (actor)->priv;

  clutter_layout_manager_get_preferred_height (priv->layout,
                                               CLUTTER_CONTAINER (actor),
                                               for_width,
                                               min_height, natural_height);
}

static void
glide_slide_allocate (ClutterActor           *actor,
		      const ClutterActorBox  *allocation,
		      ClutterAllocationFlags  flags)
{
  GlideSlidePrivate *priv = GLIDE_SLIDE (actor)->priv;
  ClutterActorClass *klass;

  klass = CLUTTER_ACTOR_CLASS (glide_slide_parent_class);
  klass->allocate (actor, allocation, flags);

  if (priv->children == NULL)
    return;

  clutter_layout_manager_allocate (priv->layout,
                                   CLUTTER_CONTAINER (actor),
                                   allocation, flags);
}



static void
glide_slide_show_all (ClutterActor *actor)
{
  clutter_container_foreach (CLUTTER_CONTAINER (actor),
                             CLUTTER_CALLBACK (clutter_actor_show),
                             NULL);
  clutter_actor_show (actor);
}

static void
glide_slide_hide_all (ClutterActor *actor)
{
  clutter_actor_hide (actor);
  clutter_container_foreach (CLUTTER_CONTAINER (actor),
                             CLUTTER_CALLBACK (clutter_actor_hide),
                             NULL);
}


static void
glide_slide_dispose (GObject *object)
{
  GlideSlide *self = GLIDE_SLIDE (object);
  GlideSlidePrivate *priv = self->priv;

  if (priv->children)
    {
      g_list_foreach (priv->children, (GFunc) clutter_actor_destroy, NULL);
      g_list_free (priv->children);

      priv->children = NULL;
    }

  if (priv->layout)
    {
      g_object_unref (priv->layout);
      priv->layout = NULL;
    }
  if (priv->background_material)
    {
      cogl_handle_unref (priv->background_material);
    }
  
  g_free (priv->background);
  g_free (priv->animation);

  G_OBJECT_CLASS (glide_slide_parent_class)->dispose (object);
}

static void
glide_slide_json_obj_set_actors (GlideSlide *slide, JsonObject *obj)
{
  JsonNode *node = json_node_new (JSON_NODE_ARRAY);
  JsonArray *array = json_array_new ();
  GList *s;
  
  for (s = clutter_container_get_children (CLUTTER_CONTAINER (slide->priv->contents_group)); s; s = s->next)
    {
      GlideActor *actor = (GlideActor *)(s->data);
      JsonNode *n;
      
      n = glide_actor_serialize (actor);
      if (n)
	json_array_add_element (array, n);
    }
  json_node_take_array (node, array);
  json_object_set_member (obj, "actors", node);			
}

static JsonNode *
glide_slide_serialize (GlideActor *self)
{
  GlideSlide *slide = GLIDE_SLIDE (self);
  JsonNode *node = json_node_new (JSON_NODE_OBJECT);
  JsonObject *obj;
  
  obj = json_object_new ();
  json_node_set_object (node, obj);
  
  glide_slide_json_obj_set_actors (slide, obj);
  
  if (slide->priv->background)
    glide_json_object_set_string (obj, "background", slide->priv->background); 
  if (slide->priv->animation)
    glide_json_object_set_string (obj, "animation", slide->priv->animation); 
  else
    glide_json_object_set_string (obj, "animation", "None"); 
  
  return node;
}

static void
glide_slide_class_init (GlideSlideClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  GlideActorClass *glide_class = GLIDE_ACTOR_CLASS (klass);
  
  glide_class->serialize = glide_slide_serialize;
  
  object_class->get_property = glide_slide_get_property;
  object_class->set_property = glide_slide_set_property;
  
  object_class->dispose = glide_slide_dispose;

  actor_class->get_preferred_width = 
    glide_slide_get_preferred_width;
  actor_class->get_preferred_height = 
    glide_slide_get_preferred_height;
  actor_class->allocate = glide_slide_allocate;
  actor_class->paint = glide_slide_paint;
  actor_class->pick = glide_slide_pick;
  actor_class->show_all = glide_slide_show_all;
  actor_class->hide_all = glide_slide_hide_all;
  
  g_object_class_install_property (object_class,
				   PROP_DOCUMENT,
				   g_param_spec_object ("document",
							"Document",
							"The document to which the slide belongs.",
							GLIDE_TYPE_DOCUMENT,
							G_PARAM_READWRITE |
							G_PARAM_CONSTRUCT_ONLY |
							G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (object_class,
				   PROP_BACKGROUND,
				   g_param_spec_string ("background",
							"Background image",
							"A file path to the background image for the slide",
							NULL,
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));    

  g_object_class_install_property (object_class,
				   PROP_ANIMATION,
				   g_param_spec_string ("animation",
							"Transition Animation",
							"The transition animation to be used in moving to the next slide.",
							NULL,
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));    
  
  g_object_class_install_property (object_class,
				   PROP_COLOR,
				   clutter_param_spec_color ("color",
							     "Color",
							     "The background color of the slide.",
							     &default_slide_color,
							     G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
							     
  
  g_type_class_add_private (object_class, sizeof(GlideSlidePrivate));
}

static void
glide_slide_init (GlideSlide *self)
{
  self->priv = GLIDE_SLIDE_GET_PRIVATE (self);
  
  self->priv->layout = clutter_fixed_layout_new ();
  g_object_ref_sink (self->priv->layout);
  
  self->priv->contents_group = clutter_group_new ();
  clutter_container_add_actor (CLUTTER_CONTAINER (self), self->priv->contents_group);
  
  CLUTTER_ACTOR_SET_FLAGS (self, CLUTTER_ACTOR_NO_LAYOUT);
}

void
glide_slide_add_actor_content (GlideSlide *s, ClutterActor *a)
{
  clutter_container_add_actor (CLUTTER_CONTAINER (s->priv->contents_group), a);
}

GlideSlide*
glide_slide_new (GlideDocument *d)
{
  return g_object_new (GLIDE_TYPE_SLIDE, 
		       "document", d,
		       NULL);
}

void
glide_slide_construct_from_json (GlideSlide *slide, JsonObject *slide_obj, GlideStageManager *manager)
{
  JsonNode *actors_n;
  JsonArray *actors;
  GList *actors_l, *a;
  const gchar *background, *animation;
  
  actors_n = json_object_get_member (slide_obj, "actors");
  actors = json_node_get_array (actors_n);
  
  background = glide_json_object_get_string (slide_obj, "background");
  if (background)
    glide_slide_set_background (slide, background);

  animation = glide_json_object_get_string (slide_obj, "animation");
  if (animation)
    glide_slide_set_animation (slide, animation);
  
  actors_l = json_array_get_elements (actors);
  for (a = actors_l; a; a = a->next)
    {
      GlideActor *actor;
      JsonNode *actor_n = a->data;
      JsonObject *actor_obj = json_node_get_object (actor_n);
      
      actor = glide_actor_construct_from_json (actor_obj);
      glide_slide_add_actor_content (slide, CLUTTER_ACTOR (actor));
      glide_actor_set_stage_manager (actor, manager);
      clutter_actor_show (CLUTTER_ACTOR (actor));      
    }
}

static CoglHandle
glide_slide_material_for_file (const gchar *filename)
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
      g_warning ("glide-slide.c failed to load widget image: %s", filename);
      g_error_free (e);
    }
  cogl_material_set_layer (m, 0, t);
  cogl_material_set_layer_filters (m, 0,
				   COGL_MATERIAL_FILTER_LINEAR_MIPMAP_LINEAR,
				   COGL_MATERIAL_FILTER_LINEAR);
  cogl_handle_unref (t);
  
  return m;
}

void 
glide_slide_set_background (GlideSlide *slide, const gchar *background)
{
  if (!background)
    return;
  
  if (slide->priv->background)
    g_free (slide->priv->background);
  
  slide->priv->background = g_strdup (background);
  
  if (slide->priv->background_material)
    cogl_handle_unref (slide->priv->background_material);
  slide->priv->background_material = glide_slide_material_for_file (background);
  
  g_object_notify (G_OBJECT (slide), "background");
  
  clutter_actor_queue_redraw (CLUTTER_ACTOR (slide));
}

const gchar *
glide_slide_get_background (GlideSlide *slide)
{
  return slide->priv->background;
}

void 
glide_slide_set_animation (GlideSlide *slide, const gchar *animation)
{
  if (slide->priv->animation)
    g_free (slide->priv->animation);
  
  slide->priv->animation = g_strdup (animation);
  g_object_notify (G_OBJECT (slide), "background");
}

const gchar *
glide_slide_get_animation (GlideSlide *slide)
{
  return slide->priv->animation;
}

ClutterActor *
glide_slide_get_contents (GlideSlide *slide)
{
  return slide->priv->contents_group;
}

void 
glide_slide_set_color (GlideSlide *slide, const ClutterColor *color)
{
  slide->priv->color = *color;
  g_object_notify (G_OBJECT (slide), "color");
}

void 
glide_slide_get_color (GlideSlide *slide, ClutterColor *color)
{
  *color = slide->priv->color;
}
