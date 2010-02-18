/*
 * glide-image.c
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
/* Based on clutter-texture.c from Wed Feb 17 2010 */
/*
 * Clutter.
 *
 * An OpenGL based 'interactive canvas' library.
 *
 * Authored By Matthew Allum  <mallum@openedhand.com>
 *
 * Copyright (C) 2006 OpenedHand
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include <clutter/clutter.h>
#include "glide-image.h"

#include <cogl/cogl.h>


G_DEFINE_TYPE (GlideImage,
	       glide_image,
	       CLUTTER_TYPE_ACTOR)

#define GLIDE_IMAGE_GET_PRIVATE(obj)        (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GLIDE_TYPE_IMAGE, GlideImagePrivate))

struct _GlideImagePrivate
{
  gint                         image_width;
  gint                         image_height;

  CoglHandle                   material;
  gboolean                     no_slice;

  ClutterActor                *fbo_source;
  CoglHandle                   fbo_handle;

  guint                        sync_actor_size : 1;
  guint                        repeat_x : 1;
  guint                        repeat_y : 1;
  guint                        keep_aspect_ratio : 1;

};

enum
{
  PROP_0,
  PROP_NO_SLICE,
  PROP_MAX_TILE_WASTE,
  PROP_PIXEL_FORMAT,
  PROP_SYNC_SIZE,
  PROP_REPEAT_Y,
  PROP_REPEAT_X,
  PROP_COGL_TEXTURE,
  PROP_COGL_MATERIAL,
  PROP_FILENAME,
  PROP_KEEP_ASPECT_RATIO,
};

enum
{
  SIZE_CHANGE,
  PIXBUF_CHANGE,
  LOAD_SUCCESS,
  LOAD_FINISHED,
  LAST_SIGNAL
};

static int image_signals[LAST_SIGNAL] = { 0 };

static guint        repaint_upload_func = 0;
static GList       *upload_list = NULL;
static GStaticMutex upload_list_mutex = G_STATIC_MUTEX_INIT;

static void
image_fbo_free_resources (GlideImage *image);

GQuark
glide_image_error_quark (void)
{
  return g_quark_from_static_string ("clutter-image-error-quark");
}

static void
image_free_gl_resources (GlideImage *image)
{
  GlideImagePrivate *priv = image->priv;


  if (priv->material != COGL_INVALID_HANDLE)
    /* We want to keep the layer so that the filter settings will
       remain but we want to free its resources so we clear the
       image handle */
    cogl_material_set_layer (priv->material, 0, COGL_INVALID_HANDLE);
}

static void
glide_image_unrealize (ClutterActor *actor)
{
  GlideImage        *image;
  GlideImagePrivate *priv;

  image = GLIDE_IMAGE(actor);
  priv = image->priv;

  if (priv->material == COGL_INVALID_HANDLE)
    return;


  if (priv->fbo_source != NULL)
    {
      /* Free up our fbo handle and image resources, realize will recreate */
      cogl_handle_unref (priv->fbo_handle);
      priv->fbo_handle = COGL_INVALID_HANDLE;
      image_free_gl_resources (image);
      return;
    }


}

static void
glide_image_realize (ClutterActor *actor)
{
  GlideImage       *image;
  GlideImagePrivate *priv;

  image = GLIDE_IMAGE(actor);
  priv = image->priv;



  if (priv->fbo_source)
    {
      CoglTextureFlags flags = COGL_TEXTURE_NONE;
      CoglHandle tex;

      /* Handle FBO's */

      if (priv->no_slice)
        flags |= COGL_TEXTURE_NO_SLICING;

      tex = cogl_texture_new_with_size (priv->image_width,
                                        priv->image_height,
                                        flags,
                                        COGL_PIXEL_FORMAT_RGBA_8888_PRE);

      cogl_material_set_layer (priv->material, 0, tex);

      priv->fbo_handle = cogl_offscreen_new_to_image (tex);

      /* The material now has a reference to the image so it will
         stick around */
      cogl_handle_unref (tex);

      if (priv->fbo_handle == COGL_INVALID_HANDLE)
        {
          g_warning ("%s: Offscreen image creation failed", G_STRLOC);
	  CLUTTER_ACTOR_UNSET_FLAGS (actor, CLUTTER_ACTOR_REALIZED);
          return;
        }

      clutter_actor_set_size (actor, priv->image_width, priv->image_height);

      return;
    }

  /* If the image is not a FBO, then realization is a no-op but
   * we still want to be in REALIZED state to maintain invariants.
   * We may have already created the image if someone set some
   * data earlier, or we may create it later if someone sets some
   * data later. The fact that we may have created it earlier is
   * really a bug, since it means GlideImage can have GL
   * resources without being realized.
   */


}

static void
glide_image_get_preferred_width (ClutterActor *self,
                                     gfloat        for_height,
                                     gfloat       *min_width_p,
                                     gfloat       *natural_width_p)
{
  GlideImage *image = GLIDE_IMAGE (self);
  GlideImagePrivate *priv = image->priv;

  /* Min request is always 0 since we can scale down or clip */
  if (min_width_p)
    *min_width_p = 0;

  if (priv->sync_actor_size)
    {
      if (natural_width_p)
        {
          if (!priv->keep_aspect_ratio ||
              for_height < 0 ||
              priv->image_height <= 0)
            {
              *natural_width_p = priv->image_width;
            }
          else
            {
              /* Set the natural width so as to preserve the aspect ratio */
              gfloat ratio = (gfloat) priv->image_width
                           / (gfloat) priv->image_height;

              *natural_width_p = ratio * for_height;
            }
        }
    }
  else
    {
      if (natural_width_p)
        *natural_width_p = 0;
    }
}

static void
glide_image_get_preferred_height (ClutterActor *self,
                                      gfloat        for_width,
                                      gfloat       *min_height_p,
                                      gfloat       *natural_height_p)
{
  GlideImage *image = GLIDE_IMAGE (self);
  GlideImagePrivate *priv = image->priv;

  /* Min request is always 0 since we can scale down or clip */
  if (min_height_p)
    *min_height_p = 0;

  if (priv->sync_actor_size)
    {
      if (natural_height_p)
        {
          if (!priv->keep_aspect_ratio ||
              for_width < 0 ||
              priv->image_width <= 0)
            {
              *natural_height_p = priv->image_height;
            }
          else
            {
              /* Set the natural height so as to preserve the aspect ratio */
              gfloat ratio = (gfloat) priv->image_height
                           / (gfloat) priv->image_width;

              *natural_height_p = ratio * for_width;
            }
        }
    }
  else
    {
      if (natural_height_p)
        *natural_height_p = 0;
    }
}

static void
glide_image_allocate (ClutterActor           *self,
			  const ClutterActorBox  *box,
                          ClutterAllocationFlags  flags)
{
  GlideImagePrivate *priv = GLIDE_IMAGE (self)->priv;

  /* chain up to set actor->allocation */
  CLUTTER_ACTOR_CLASS (glide_image_parent_class)->allocate (self,
                                                                box,
                                                                flags);

  /* If we adopted the source fbo then allocate that at its preferred
     size */
  if (priv->fbo_source && clutter_actor_get_parent (priv->fbo_source) == self)
    clutter_actor_allocate_preferred_size (priv->fbo_source, flags);
}

static void
set_viewport_with_buffer_under_fbo_source (ClutterActor *fbo_source,
                                           int viewport_width,
                                           int viewport_height)
{
  ClutterVertex verts[4];
  float x_min = G_MAXFLOAT, y_min = G_MAXFLOAT;
  int x_offset, y_offset;
  int i;

  /* Get the actors allocation transformed into screen coordinates.
   *
   * XXX: Note: this may not be a bounding box for the actor, since an
   * actor with depth may escape the box due to its perspective
   * projection. */
  clutter_actor_get_abs_allocation_vertices (fbo_source, verts);

  for (i = 0; i < G_N_ELEMENTS (verts); ++i)
    {
      if (verts[i].x < x_min)
	x_min = verts[i].x;
      if (verts[i].y < y_min)
	y_min = verts[i].y;
    }

  /* XXX: It's not good enough to round by simply truncating the fraction here
   * via a cast, as it results in offscreen rendering being offset by 1 pixel
   * in many cases... */
#define ROUND(x) ((x) >= 0 ? (long)((x) + 0.5) : (long)((x) - 0.5))

  x_offset = ROUND (-x_min);
  y_offset = ROUND (-y_min);

#undef ROUND

  /* translate the viewport so that the source actor lands on the
   * sub-region backed by the offscreen framebuffer... */
  cogl_set_viewport (x_offset, y_offset, viewport_width, viewport_height);
}

static void
update_fbo (ClutterActor *self)
{
  GlideImage        *image = GLIDE_IMAGE (self);
  GlideImagePrivate *priv = image->priv;
  ClutterMainContext    *context;
  ClutterShader         *shader = NULL;
  ClutterActor          *stage = NULL;
  ClutterPerspective     perspective;
  CoglColor              transparent_col;

  context = _clutter_context_get_default ();

  if (context->shaders)
    shader = clutter_actor_get_shader (context->shaders->data);

  /* Temporarily turn off the shader on the top of the context's shader stack,
   * to restore the GL pipeline to it's natural state.
   */
  if (shader)
    clutter_shader_set_is_enabled (shader, FALSE);

  /* Redirect drawing to the fbo */
  cogl_push_framebuffer (priv->fbo_handle);

  if ((stage = clutter_actor_get_stage (self)))
    {
      gfloat stage_width, stage_height;
      ClutterActor *source_parent;

      /* We copy the projection and modelview matrices from the stage to
       * the offscreen framebuffer and create a viewport larger than the
       * offscreen framebuffer - the same size as the stage.
       *
       * The fbo source actor gets rendered into this stage size viewport at the
       * same position it normally would after applying all it's usual parent
       * transforms and it's own scale and rotate transforms etc.
       *
       * The viewport is offset such that the offscreen buffer will be positioned
       * under the actor.
       */

      clutter_stage_get_perspective (CLUTTER_STAGE (stage), &perspective);
      clutter_actor_get_size (stage, &stage_width, &stage_height);

      /* Set the projection matrix modelview matrix and viewport size as
       * they are for the stage... */
      _cogl_setup_viewport (stage_width, stage_height,
                            perspective.fovy,
                            perspective.aspect,
                            perspective.z_near,
                            perspective.z_far);

      /* Negatively offset the viewport so that the offscreen framebuffer is
       * position underneath the fbo_source actor... */
      set_viewport_with_buffer_under_fbo_source (priv->fbo_source,
                                                 stage_width,
                                                 stage_height);

      /* Reapply the source's parent transformations */
      if ((source_parent = clutter_actor_get_parent (priv->fbo_source)))
        _clutter_actor_apply_modelview_transform_recursive (source_parent,
                                                            NULL);
    }


  /* cogl_clear is called to clear the buffers */
  cogl_color_set_from_4ub (&transparent_col, 0, 0, 0, 0);
  cogl_clear (&transparent_col,
              COGL_BUFFER_BIT_COLOR |
              COGL_BUFFER_BIT_DEPTH);
  cogl_disable_fog ();

  /* Render the actor to the fbo */
  clutter_actor_paint (priv->fbo_source);

  /* Restore drawing to the previous framebuffer */
  cogl_pop_framebuffer ();

  /* If there is a shader on top of the shader stack, turn it back on. */
  if (shader)
    clutter_shader_set_is_enabled (shader, TRUE);
}

static void
glide_image_paint (ClutterActor *self)
{
  GlideImage *image = GLIDE_IMAGE (self);
  GlideImagePrivate *priv = image->priv;
  ClutterActorBox box = { 0, };
  gfloat          t_w, t_h;
  guint8          paint_opacity = clutter_actor_get_paint_opacity (self);

  if (paint_opacity == 0)
    {
      /* Bail early if painting the actor would be a no-op, custom actors that
       * might cause a lot of work/state changes should all do this.
       */
      return;
    }

  if (priv->fbo_handle != COGL_INVALID_HANDLE)
    update_fbo (self);



  cogl_material_set_color4ub (priv->material,
			      paint_opacity, paint_opacity, paint_opacity, paint_opacity);

  clutter_actor_get_allocation_box (self, &box);



  if (priv->repeat_x && priv->image_width > 0)
    t_w = (box.x2 - box.x1) / (gfloat) priv->image_width;
  else
    t_w = 1.0;

  if (priv->repeat_y && priv->image_height > 0)
    t_h = (box.y2 - box.y1) / (gfloat) priv->image_height;
  else
    t_h = 1.0;

  /* Paint will have translated us */
  cogl_set_source (priv->material);
  cogl_rectangle_with_image_coords (0, 0,
			              box.x2 - box.x1,
                                      box.y2 - box.y1,
			              0, 0, t_w, t_h);
}

static void
glide_image_dispose (GObject *object)
{
  GlideImage *image = GLIDE_IMAGE (object);

  image_free_gl_resources (image);
  image_fbo_free_resources (image);

  G_OBJECT_CLASS (glide_image_parent_class)->dispose (object);
}

static void
glide_image_finalize (GObject *object)
{
  GlideImage *image = GLIDE_IMAGE (object);
  GlideImagePrivate *priv = image->priv;

  if (priv->material != COGL_INVALID_HANDLE)
    {
      cogl_handle_unref (priv->material);
      priv->material = COGL_INVALID_HANDLE;
    }

  G_OBJECT_CLASS (glide_image_parent_class)->finalize (object);
}

static void
glide_image_set_property (GObject      *object,
			      guint         prop_id,
			      const GValue *value,
			      GParamSpec   *pspec)
{
  GlideImage *image;
  GlideImagePrivate *priv;

  image = GLIDE_IMAGE (object);
  priv = image->priv;

  switch (prop_id)
    {
    case PROP_SYNC_SIZE:
      glide_image_set_sync_size (image, g_value_get_boolean (value));
      break;

    case PROP_REPEAT_X:
      glide_image_set_repeat (image,
                                  g_value_get_boolean (value),
                                  priv->repeat_y);
      break;

    case PROP_REPEAT_Y:
      glide_image_set_repeat (image,
                                  priv->repeat_x,
                                  g_value_get_boolean (value));
      break;

    case PROP_COGL_TEXTURE:
      {
        CoglHandle hnd = g_value_get_boxed (value);

        glide_image_set_cogl_texture (image, hnd);
      }
      break;

    case PROP_COGL_MATERIAL:
      {
        CoglHandle hnd = g_value_get_boxed (value);

        glide_image_set_cogl_material (image, hnd);
      }
      break;

    case PROP_FILENAME:
      glide_image_set_from_file (image,
                                     g_value_get_string (value),
                                     NULL);
      break;

    case PROP_NO_SLICE:
      priv->no_slice = g_value_get_boolean (value);
      break;

    case PROP_KEEP_ASPECT_RATIO:
      glide_image_set_keep_aspect_ratio (image,
                                             g_value_get_boolean (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
clutter_image_get_property (GObject    *object,
			      guint       prop_id,
			      GValue     *value,
			      GParamSpec *pspec)
{
  GlideImage        *image;
  GlideImagePrivate *priv;

  image = GLIDE_IMAGE(object);
  priv = image->priv;

  switch (prop_id)
    {
    case PROP_PIXEL_FORMAT:
      g_value_set_enum (value, glide_image_get_pixel_format (image));
      break;

    case PROP_MAX_TILE_WASTE:
      g_value_set_int (value, glide_image_get_max_tile_waste (image));
      break;

    case PROP_SYNC_SIZE:
      g_value_set_boolean (value, priv->sync_actor_size);
      break;

    case PROP_REPEAT_X:
      g_value_set_boolean (value, priv->repeat_x);
      break;

    case PROP_REPEAT_Y:
      g_value_set_boolean (value, priv->repeat_y);
      break;

    case PROP_FILTER_QUALITY:
      g_value_set_enum (value, glide_image_get_filter_quality (image));
      break;

    case PROP_COGL_TEXTURE:
      g_value_set_boxed (value, glide_image_get_cogl_texture (image));
      break;

    case PROP_COGL_MATERIAL:
      g_value_set_boxed (value, glide_image_get_cogl_material (image));
      break;

    case PROP_NO_SLICE:
      g_value_set_boolean (value, priv->no_slice);
      break;

    case PROP_KEEP_ASPECT_RATIO:
      g_value_set_boolean (value, priv->keep_aspect_ratio);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
glide_image_class_init (GlideImageClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

  g_type_class_add_private (klass, sizeof (GlideImagePrivate));

  actor_class->paint          = glide_image_paint;
  actor_class->realize        = glide_image_realize;
  actor_class->unrealize      = glide_image_unrealize;

  actor_class->get_preferred_width  = glide_image_get_preferred_width;
  actor_class->get_preferred_height = glide_image_get_preferred_height;
  actor_class->allocate             = glide_image_allocate;

  gobject_class->dispose      = glide_image_dispose;
  gobject_class->finalize     = glide_image_finalize;
  gobject_class->set_property = glide_image_set_property;
  gobject_class->get_property = glide_image_get_property;

  g_object_class_install_property
    (gobject_class, PROP_SYNC_SIZE,
     g_param_spec_boolean ("sync-size",
			   "Sync size of actor",
			   "Auto sync size of actor to underlying pixbuf "
			   "dimensions",
			   TRUE,
			   G_PARAM_READWRITE));

  g_object_class_install_property
    (gobject_class, PROP_NO_SLICE,
     g_param_spec_boolean ("disable-slicing",
			   "Disable Slicing",
			   "Force the underlying image to be singlular"
			   "and not made of of smaller space saving "
                           "inidivual images.",
			   FALSE,
			   G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE));

  g_object_class_install_property
    (gobject_class, PROP_MAX_TILE_WASTE,
     g_param_spec_int ("tile-waste",
                       "Tile Waste",
                       "Maximum waste area of a sliced image",
                       -1, G_MAXINT,
                       COGL_TEXTURE_MAX_WASTE,
                       G_PARAM_READABLE));

  g_object_class_install_property
    (gobject_class, PROP_REPEAT_X,
     g_param_spec_boolean ("repeat-x",
			   "Tile underlying pixbuf in x direction",
			   "Repeat underlying pixbuf rather than scale "
			   "in x direction.",
			   FALSE,
			   G_PARAM_READWRITE));

  g_object_class_install_property
    (gobject_class, PROP_REPEAT_Y,
     g_param_spec_boolean ("repeat-y",
			   "Tile underlying pixbuf in y direction",
			   "Repeat underlying pixbuf rather than scale "
			   "in y direction.",
			   FALSE,
			   G_PARAM_READWRITE));

  g_object_class_install_property
    (gobject_class, PROP_PIXEL_FORMAT,
     g_param_spec_enum ("pixel-format",
		        "Image pixel format",
		        "CoglPixelFormat to use.",
                        COGL_TYPE_PIXEL_FORMAT,
		        COGL_PIXEL_FORMAT_RGBA_8888,
		        G_PARAM_READABLE));

  g_object_class_install_property
    (gobject_class, PROP_COGL_TEXTURE,
     g_param_spec_boxed ("cogl-texture",
			 "COGL Image",
			 "The underlying COGL image handle used to draw "
			 "this actor",
			 COGL_TYPE_HANDLE,
			 G_PARAM_READWRITE));

  g_object_class_install_property
    (gobject_class, PROP_COGL_MATERIAL,
     g_param_spec_boxed ("cogl-material",
			 "COGL Material",
			 "The underlying COGL material handle used to draw "
			 "this actor",
			 COGL_TYPE_HANDLE,
			 G_PARAM_READWRITE));

  g_object_class_install_property
    (gobject_class, PROP_FILENAME,
     g_param_spec_string ("filename",
                          "Filename",
                          "The full path of the file containing the image",
                          NULL,
                          G_PARAM_WRITABLE));

  g_object_class_install_property
    (gobject_class, PROP_KEEP_ASPECT_RATIO,
     g_param_spec_boolean ("keep-aspect-ratio",
			   "Keep Aspect Ratio",
			   "Keep the aspect ratio of the image when "
			   "requesting the preferred width or height",
			   FALSE,
			   G_PARAM_READWRITE));


  
  
  image_signals[SIZE_CHANGE] =
    g_signal_new ("size-change",
		  G_TYPE_FROM_CLASS (gobject_class),
		  G_SIGNAL_RUN_LAST,
		  G_STRUCT_OFFSET (GlideImageClass, size_change),
		  NULL, NULL,
		  g_marshal_VOID__INT_INT,
		  G_TYPE_NONE, 2,
                  G_TYPE_INT,
                  G_TYPE_INT);
  
  image_signals[PIXBUF_CHANGE] =
    g_signal_new ("pixbuf-change",
		  G_TYPE_FROM_CLASS (gobject_class),
		  G_SIGNAL_RUN_LAST,
		  G_STRUCT_OFFSET (GlideImageClass, pixbuf_change),
		  NULL, NULL,
		  g_cclosure_marshal_VOID__VOID,
		  G_TYPE_NONE,
		  0);
  
  image_signals[LOAD_FINISHED] =
    g_signal_new (I_("load-finished"),
		  G_TYPE_FROM_CLASS (gobject_class),
		  G_SIGNAL_RUN_LAST,
		  G_STRUCT_OFFSET (GlideImageClass, load_finished),
		  NULL, NULL,
		  g_cclosure_marshal_VOID__POINTER,
		  G_TYPE_NONE,
		  1,
                  G_TYPE_POINTER);
}

static ClutterScriptableIface *parent_scriptable_iface = NULL;

static void
glide_image_set_custom_property (ClutterScriptable *scriptable,
                                     ClutterScript     *script,
                                     const gchar       *name,
                                     const GValue      *value)
{
  GlideImage *image = GLIDE_IMAGE (scriptable);

  if (strcmp ("filename", name) == 0)
    {
      const gchar *str = g_value_get_string (value);
      gchar *path;
      GError *error;

      path = clutter_script_lookup_filename (script, str);
      if (G_UNLIKELY (!path))
        return;

      error = NULL;
      glide_image_set_from_file (image, path, &error);
      if (error)
        {
          g_warning ("Unable to open image path at '%s': %s",
                     path,
                     error->message);
          g_error_free (error);
        }

      g_free (path);
    }
  else
    {
      /* chain up */
      if (parent_scriptable_iface->set_custom_property)
        parent_scriptable_iface->set_custom_property (scriptable, script,
                                                      name,
                                                      value);
    }
}

static void
clutter_scriptable_iface_init (ClutterScriptableIface *iface)
{
  parent_scriptable_iface = g_type_interface_peek_parent (iface);

  if (!parent_scriptable_iface)
    parent_scriptable_iface = g_type_default_interface_peek
                                          (CLUTTER_TYPE_SCRIPTABLE);

  iface->set_custom_property = glide_image_set_custom_property;
}

static void
glide_image_init (GlideImage *self)
{
  GlideImagePrivate *priv;

  self->priv = priv = GLIDE_IMAGE_GET_PRIVATE (self);

  priv->repeat_x          = FALSE;
  priv->repeat_y          = FALSE;
  priv->sync_actor_size   = TRUE;
  priv->material          = cogl_material_new ();
  priv->fbo_handle        = COGL_INVALID_HANDLE;
  priv->keep_aspect_ratio = FALSE;
  
  cogl_material_set_layer_filters (priv->material, 0, COGL_MATERIAL_FILTER_LINEAR_MIPMAP_LINEAR,
				   COGL_MATERIAL_FILTER_LINEAR);
}


CoglHandle
glide_image_get_cogl_material (GlideImage *image)
{
  g_return_val_if_fail (GLIDE_IS_IMAGE (image), COGL_INVALID_HANDLE);

  return image->priv->material;
}


void
glide_image_set_cogl_material (GlideImage *image,
                                   CoglHandle cogl_material)
{
  CoglHandle cogl_texture;

  g_return_if_fail (GLIDE_IS_IMAGE (image));

  cogl_handle_ref (cogl_material);

  /* This */
  if (image->priv->material)
    cogl_handle_unref (image->priv->material);

  image->priv->material = cogl_material;

  /* XXX: We are re-asserting the first layer of the new material to ensure the
   * priv state is in sync with the contents of the material. */
  cogl_texture = glide_image_get_cogl_texture (image);
  glide_image_set_cogl_texture (image, cogl_texture);
  /* XXX: If we add support for more material layers, this will need
   * extending */
}


CoglHandle
glide_image_get_cogl_texture (GlideImage *image)
{
  const GList *layers;
  int n_layers;

  g_return_val_if_fail (GLIDE_IS_IMAGE (image), COGL_INVALID_HANDLE);

  layers = cogl_material_get_layers (image->priv->material);
  n_layers = g_list_length ((GList *)layers);
  if (n_layers == 0)
    return COGL_INVALID_HANDLE;

  return cogl_material_layer_get_image (layers->data);
}


void
glide_image_set_cogl_texture (GlideImage  *image,
				  CoglHandle       cogl_tex)
{
  GlideImagePrivate  *priv;
  gboolean size_changed;
  guint width, height;

  g_return_if_fail (GLIDE_IS_IMAGE (image));
  g_return_if_fail (cogl_is_texture (cogl_tex));

  /* FIXME this implementation should realize the actor if it's in a
   * stage, and warn and return if not in a stage yet. However, right
   * now everything would break if we did that, so we just fudge it
   * and we're broken: we can have a image without being realized.
   */

  priv = image->priv;

  width = cogl_texture_get_width (cogl_tex);
  height = cogl_texture_get_height (cogl_tex);

  /* Reference the new image now in case it is the same one we are
     already using */
  cogl_handle_ref (cogl_tex);

  /* Remove FBO if exisiting */
  if (priv->fbo_source)
    image_fbo_free_resources (image);

  /* Remove old image */
  image_free_gl_resources (image);

  /* Use the new image */
  cogl_material_set_layer (priv->material, 0, cogl_tex);

  /* The material now holds a reference to the image so we can
     safely release the reference we claimed above */
  cogl_handle_unref (cogl_tex);

  size_changed = (width != priv->image_width || height != priv->image_height);
  priv->image_width = width;
  priv->image_height = height;



  if (size_changed)
    {
      g_signal_emit (image, image_signals[SIZE_CHANGE], 0,
                     priv->image_width,
                     priv->image_height);

      if (priv->sync_actor_size)
        clutter_actor_queue_relayout (CLUTTER_ACTOR (image));
    }

  /* rename signal */
  g_signal_emit (image, image_signals[PIXBUF_CHANGE], 0);

  g_object_notify (G_OBJECT (image), "cogl-texture");

  /* If resized actor may need resizing but paint() will do this */
  clutter_actor_queue_redraw (CLUTTER_ACTOR (image));
}

static gboolean
glide_image_set_from_data (GlideImage     *image,
			       const guchar       *data,
			       CoglPixelFormat     source_format,
			       gint                width,
			       gint                height,
			       gint                rowstride,
			       gint                bpp,
			       GError            **error)
{
  GlideImagePrivate *priv = image->priv;
  CoglHandle new_image = COGL_INVALID_HANDLE;
  CoglTextureFlags flags = COGL_TEXTURE_NONE;

  if (priv->no_slice)
    flags |= COGL_TEXTURE_NO_SLICING;

  /* FIXME if we are not realized, we should store the data
   * for future use, instead of creating the image.
   */
  new_image = cogl_texture_new_from_data (width, height,
                                            flags,
                                            source_format,
                                            COGL_PIXEL_FORMAT_ANY,
                                            rowstride,
                                            data);

  if (G_UNLIKELY (new_image == COGL_INVALID_HANDLE))
    {
      GError *inner_error = NULL;

      g_set_error (&inner_error, GLIDE_IMAGE_ERROR,
                   GLIDE_IMAGE_ERROR_BAD_FORMAT,
                   "Failed to create COGL image");

      g_signal_emit (image, image_signals[LOAD_FINISHED], 0, inner_error);

      if (error != NULL)
        g_propagate_error (error, inner_error);
      else
        g_error_free (inner_error);

      return FALSE;
    }

  glide_image_set_cogl_texture (image, new_image);

  cogl_handle_unref (new_image);

  g_signal_emit (image, image_signals[LOAD_FINISHED], 0, NULL);

  return TRUE;
}


gboolean
glide_image_set_from_rgb_data (GlideImage       *image,
				   const guchar         *data,
				   gboolean              has_alpha,
				   gint                  width,
				   gint                  height,
				   gint                  rowstride,
				   gint                  bpp,
				   GlideImageFlags   flags,
				   GError              **error)
{
  CoglPixelFormat source_format;

  g_return_val_if_fail (GLIDE_IS_IMAGE (image), FALSE);

  /* Convert the flags to a CoglPixelFormat */
  if (has_alpha)
    {
      if (bpp != 4)
	{
	  g_set_error (error, GLIDE_IMAGE_ERROR,
		       GLIDE_IMAGE_ERROR_BAD_FORMAT,
		       "Unsupported bits per pixel value '%d': "
                       "Clutter supports only a BPP value of 4 "
                       "for RGBA data",
                       bpp);
	  return FALSE;
	}

      source_format = COGL_PIXEL_FORMAT_RGBA_8888;
    }
  else
    {
      if (bpp != 3)
	{
	  g_set_error (error, GLIDE_IMAGE_ERROR,
		       GLIDE_IMAGE_ERROR_BAD_FORMAT,
		       "Unsupported bits per pixel value '%d': "
                       "Clutter supports only a BPP value of 3 "
                       "for RGB data",
                       bpp);
	  return FALSE;
	}

      source_format = COGL_PIXEL_FORMAT_RGB_888;
    }

  if ((flags & GLIDE_IMAGE_RGB_FLAG_BGR))
    source_format |= COGL_BGR_BIT;
  if ((flags & GLIDE_IMAGE_RGB_FLAG_PREMULT))
    source_format |= COGL_PREMULT_BIT;

  return glide_image_set_from_data (image, data,
					source_format,
					width, height,
					rowstride, bpp,
					error);
}


gboolean
glide_image_set_from_yuv_data (GlideImage     *image,
				   const guchar       *data,
				   gint                width,
				   gint                height,
				   GlideImageFlags flags,
				   GError            **error)
{
  g_return_val_if_fail (GLIDE_IS_IMAGE (image), FALSE);

  /* Convert the flags to a CoglPixelFormat */
  if ((flags & GLIDE_IMAGE_YUV_FLAG_YUV2))
    {
      g_set_error (error, GLIDE_IMAGE_ERROR,
		   GLIDE_IMAGE_ERROR_BAD_FORMAT,
		   "YUV2 textues are not supported");
      return FALSE;
    }

  return glide_image_set_from_data (image, data,
					COGL_PIXEL_FORMAT_YUV,
					width, height,
					width * 3, 3,
					error);
}

gboolean
glide_image_set_from_file (GlideImage *image,
			       const gchar    *filename,
			       GError        **error)
{
  GlideImagePrivate *priv;
  CoglHandle new_image = COGL_INVALID_HANDLE;
  GError *internal_error = NULL;
  CoglTextureFlags flags = COGL_TEXTURE_NONE;

  priv = image->priv;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  if (priv->no_slice)
    flags |= COGL_TEXTURE_NO_SLICING;

  new_image = cogl_texture_new_from_file (filename,
                                            flags,
                                            COGL_PIXEL_FORMAT_ANY,
                                            &internal_error);

  /* If COGL didn't give an error then make one up */
  if (internal_error == NULL && new_image == COGL_INVALID_HANDLE)
    {
      g_set_error (&internal_error, GLIDE_IMAGE_ERROR,
                   GLIDE_IMAGE_ERROR_BAD_FORMAT,
		   "Failed to create COGL image");
    }

  if (internal_error != NULL)
    {
      g_signal_emit (image, image_signals[LOAD_FINISHED], 0,
                     internal_error);

      g_propagate_error (error, internal_error);

      return FALSE;
    }

  glide_image_set_cogl_texture (image, new_image);

  cogl_handle_unref (new_image);

  g_signal_emit (image, image_signals[LOAD_FINISHED], 0, NULL);

  return TRUE;
}


gint
glide_image_get_max_tile_waste (GlideImage *image)
{
  GlideImagePrivate *priv;
  CoglHandle             cogl_texture;

  g_return_val_if_fail (GLIDE_IS_IMAGE (image), 0);

  priv = image->priv;

  cogl_texture = glide_image_get_cogl_texture (image);

  if (cogl_texture == COGL_INVALID_HANDLE)
    return priv->no_slice ? -1 : COGL_TEXTURE_MAX_WASTE;
  else
    return cogl_texture_get_max_waste (cogl_texture);
}


ClutterActor*
glide_image_new_from_file (const gchar *filename,
			       GError     **error)
{
  ClutterActor *image = glide_image_new ();

  if (!glide_image_set_from_file (GLIDE_IMAGE (image),
				      filename, error))
    {
      g_object_ref_sink (image);
      g_object_unref (image);

      return NULL;
    }
  else
    return image;
}


ClutterActor *
glide_image_new (void)
{
  return g_object_new (GLIDE_TYPE_IMAGE, NULL);
}


void
glide_image_get_base_size (GlideImage *image,
			       gint           *width,
			       gint           *height)
{
  g_return_if_fail (GLIDE_IS_IMAGE (image));

  if (width)
    *width = image->priv->image_width;

  if (height)
    *height = image->priv->image_height;
}


gboolean
glide_image_set_area_from_rgb_data (GlideImage     *image,
                                        const guchar       *data,
                                        gboolean            has_alpha,
                                        gint                x,
                                        gint                y,
                                        gint                width,
                                        gint                height,
                                        gint                rowstride,
                                        gint                bpp,
                                        GlideImageFlags flags,
                                        GError            **error)
{
  CoglPixelFormat source_format;
  CoglHandle cogl_texture;

  if (has_alpha)
    {
      if (bpp != 4)
	{
	  g_set_error (error, GLIDE_IMAGE_ERROR,
		       GLIDE_IMAGE_ERROR_BAD_FORMAT,
		       "Unsupported BPP");
	  return FALSE;
	}

      source_format = COGL_PIXEL_FORMAT_RGBA_8888;
    }
  else
    {
      if (bpp != 3)
	{
	  g_set_error (error, GLIDE_IMAGE_ERROR,
		       GLIDE_IMAGE_ERROR_BAD_FORMAT,
		       "Unsupported BPP");
	  return FALSE;
	}

      source_format = COGL_PIXEL_FORMAT_RGB_888;
    }

  if ((flags & GLIDE_IMAGE_RGB_FLAG_BGR))
    source_format |= COGL_BGR_BIT;

  if ((flags & GLIDE_IMAGE_RGB_FLAG_PREMULT))
    source_format |= COGL_PREMULT_BIT;

  /* attempt to realize ... */
  if (!CLUTTER_ACTOR_IS_REALIZED (image) &&
      clutter_actor_get_stage (CLUTTER_ACTOR (image)) != NULL)
    {
      clutter_actor_realize (CLUTTER_ACTOR (image));
    }

  /* due to the fudging of glide_image_set_cogl_texture()
   * which allows setting a image pre-realize, we may end
   * up having a image even if we couldn't realize yet.
   */
  cogl_texture = glide_image_get_cogl_texture (image);
  if (cogl_texture == COGL_INVALID_HANDLE)
    {
      g_set_error (error, GLIDE_IMAGE_ERROR,
		   GLIDE_IMAGE_ERROR_BAD_FORMAT,
		   "Failed to realize actor");
      return FALSE;
    }

  if (!cogl_texture_set_region (cogl_texture,
				0, 0,
				x, y, width, height,
				width, height,
				source_format,
				rowstride,
				data))
    {
      g_set_error (error, GLIDE_IMAGE_ERROR,
		   GLIDE_IMAGE_ERROR_BAD_FORMAT,
		   "Failed to upload COGL image data");
      return FALSE;
    }

  /* rename signal */
  g_signal_emit (image, image_signals[PIXBUF_CHANGE], 0);

  clutter_actor_queue_redraw (CLUTTER_ACTOR (image));

  return TRUE;
}

static void
on_fbo_source_size_change (GObject          *object,
                           GParamSpec       *param_spec,
                           GlideImage   *image)
{
  GlideImagePrivate *priv = image->priv;
  gfloat w, h;

  clutter_actor_get_transformed_size (priv->fbo_source, &w, &h);

  if (w != priv->image_width || h != priv->image_height)
    {
      CoglTextureFlags flags = COGL_TEXTURE_NONE;
      CoglHandle tex;

      /* tear down the FBO */
      if (priv->fbo_handle != COGL_INVALID_HANDLE)
        cogl_handle_unref (priv->fbo_handle);

      image_free_gl_resources (image);

      priv->image_width = w;
      priv->image_height = h;

      flags |= COGL_TEXTURE_NO_SLICING;

      tex = cogl_texture_new_with_size (MAX (priv->image_width, 1),
                                        MAX (priv->image_height, 1),
                                        flags,
                                        COGL_PIXEL_FORMAT_RGBA_8888_PRE);

      cogl_material_set_layer (priv->material, 0, tex);

      priv->fbo_handle = cogl_offscreen_new_to_image (tex);

      /* The material now has a reference to the image so it will
         stick around */
      cogl_handle_unref (tex);

      if (priv->fbo_handle == COGL_INVALID_HANDLE)
        {
          g_warning ("%s: Offscreen image creation failed", G_STRLOC);
          return;
        }

      clutter_actor_set_size (CLUTTER_ACTOR (image), w, h);
    }
}

static void
on_fbo_parent_change (ClutterActor        *actor,
                      ClutterActor        *old_parent,
                      GlideImage      *image)
{
  ClutterActor        *parent = CLUTTER_ACTOR(image);

  while ((parent = clutter_actor_get_parent (parent)) != NULL)
    if (parent == actor)
      {
        g_warning ("Offscreen image is ancestor of source!");
        /* Desperate but will avoid infinite loops */
        clutter_actor_unparent (actor);
      }
}


ClutterActor *
glide_image_new_from_actor (ClutterActor *actor)
{
  GlideImage        *image;
  GlideImagePrivate *priv;
  gfloat w, h;

  g_return_val_if_fail (CLUTTER_IS_ACTOR (actor), NULL);

  if (clutter_feature_available (CLUTTER_FEATURE_OFFSCREEN) == FALSE)
    return NULL;

  if (!CLUTTER_ACTOR_IS_REALIZED (actor))
    {
      clutter_actor_realize (actor);

      if (!CLUTTER_ACTOR_IS_REALIZED (actor))
	return NULL;
    }

  clutter_actor_get_transformed_size (actor, &w, &h);

  if (w == 0 || h == 0)
    return NULL;

  /* Hopefully now were good.. */
  image = g_object_new (GLIDE_TYPE_IMAGE,
                          "disable-slicing", TRUE,
                          NULL);

  priv = image->priv;

  priv->fbo_source = g_object_ref_sink (actor);

  /* If the actor doesn't have a parent then claim it so that it will
     get a size allocation during layout */
  if (clutter_actor_get_parent (actor) == NULL)
    clutter_actor_set_parent (actor, CLUTTER_ACTOR (image));

  /* Connect up any signals which could change our underlying size */
  g_signal_connect (actor,
                    "notify::width",
                    G_CALLBACK(on_fbo_source_size_change),
                    image);
  g_signal_connect (actor,
                    "notify::height",
                    G_CALLBACK(on_fbo_source_size_change),
                    image);
  g_signal_connect (actor,
                    "notify::scale-x",
                    G_CALLBACK(on_fbo_source_size_change),
                    image);
  g_signal_connect (actor,
                    "notify::scale-y",
                    G_CALLBACK(on_fbo_source_size_change),
                    image);
  g_signal_connect (actor,
                    "notify::rotation-angle-x",
                    G_CALLBACK(on_fbo_source_size_change),
                    image);
  g_signal_connect (actor,
                    "notify::rotation-angle-y",
                    G_CALLBACK(on_fbo_source_size_change),
                    image);
  g_signal_connect (actor,
                    "notify::rotation-angle-z",
                    G_CALLBACK(on_fbo_source_size_change),
                    image);

  /* And a warning if the source becomes a child of the image */
  g_signal_connect (actor,
                    "parent-set",
                    G_CALLBACK(on_fbo_parent_change),
                    image);

  priv->image_width = w;
  priv->image_height = h;

  clutter_actor_set_size (CLUTTER_ACTOR (image),
                          priv->image_width,
                          priv->image_height);

  return CLUTTER_ACTOR (image);
}

static void
image_fbo_free_resources (GlideImage *image)
{
  GlideImagePrivate *priv;

  priv = image->priv;

  

  if (priv->fbo_source != NULL)
    {
      /* If we parented the image then unparent it again so that it
	 will lose the reference */
      if (clutter_actor_get_parent (priv->fbo_source)
	  == CLUTTER_ACTOR (image))
	clutter_actor_unparent (priv->fbo_source);

      g_signal_handlers_disconnect_by_func
                            (priv->fbo_source,
                             G_CALLBACK(on_fbo_parent_change),
                             image);

      g_signal_handlers_disconnect_by_func
                            (priv->fbo_source,
                             G_CALLBACK(on_fbo_source_size_change),
                             image);

      g_object_unref (priv->fbo_source);

      priv->fbo_source = NULL;
    }

  if (priv->fbo_handle != COGL_INVALID_HANDLE)
    {
      cogl_handle_unref (priv->fbo_handle);
      priv->fbo_handle = COGL_INVALID_HANDLE;
    }
}


void
glide_image_set_sync_size (GlideImage *image,
                               gboolean        sync_size)
{
  GlideImagePrivate *priv;

  g_return_if_fail (GLIDE_IS_IMAGE (image));

  priv = image->priv;

  if (priv->sync_actor_size != sync_size)
    {
      priv->sync_actor_size = sync_size;

      clutter_actor_queue_relayout (CLUTTER_ACTOR (image));

      g_object_notify (G_OBJECT (image), "sync-size");
    }
}


gboolean
glide_image_get_sync_size (GlideImage *image)
{
  g_return_val_if_fail (GLIDE_IS_IMAGE (image), FALSE);

  return image->priv->sync_actor_size;
}


void
glide_image_set_repeat (GlideImage *image,
                            gboolean        repeat_x,
                            gboolean        repeat_y)
{
  GlideImagePrivate *priv;
  gboolean changed = FALSE;

  g_return_if_fail (GLIDE_IS_IMAGE (image));

  priv = image->priv;

  g_object_freeze_notify (G_OBJECT (image));

  if (priv->repeat_x != repeat_x)
    {
      priv->repeat_x = repeat_x;

      g_object_notify (G_OBJECT (image), "repeat-x");

      changed = TRUE;
    }

  if (priv->repeat_y != repeat_y)
    {
      priv->repeat_y = repeat_y;

      g_object_notify (G_OBJECT (image), "repeat-y");

      changed = TRUE;
    }

  if (changed)
    clutter_actor_queue_redraw (CLUTTER_ACTOR (image));

  g_object_thaw_notify (G_OBJECT (image));
}


void
glide_image_get_repeat (GlideImage *image,
                            gboolean       *repeat_x,
                            gboolean       *repeat_y)
{
  g_return_if_fail (GLIDE_IS_IMAGE (image));

  if (repeat_x != NULL)
    *repeat_x = image->priv->repeat_x;

  if (repeat_y != NULL)
    *repeat_y = image->priv->repeat_y;
}


CoglPixelFormat
glide_image_get_pixel_format (GlideImage *image)
{
  CoglHandle cogl_texture;

  g_return_val_if_fail (GLIDE_IS_IMAGE (image), COGL_PIXEL_FORMAT_ANY);

  cogl_texture = glide_image_get_cogl_texture (image);
  if (cogl_texture == COGL_INVALID_HANDLE)
    return COGL_PIXEL_FORMAT_ANY;

  return cogl_texture_get_format (cogl_texture);
}


void
glide_image_set_keep_aspect_ratio (GlideImage *image,
                                       gboolean        keep_aspect)
{
  GlideImagePrivate *priv;

  g_return_if_fail (GLIDE_IS_IMAGE (image));

  priv = image->priv;

  if (priv->keep_aspect_ratio != keep_aspect)
    {
      priv->keep_aspect_ratio = keep_aspect;

      clutter_actor_queue_relayout (CLUTTER_ACTOR (image));

      g_object_notify (G_OBJECT (image), "keep-aspect-ratio");
    }
}


gboolean
glide_image_get_keep_aspect_ratio (GlideImage *image)
{
  g_return_val_if_fail (GLIDE_IS_IMAGE (image), FALSE);

  return image->priv->keep_aspect_ratio;
}


