/*
 * glide-image.h
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
/* Based off clutter-texture.h */
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
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __GLIDE_IMAGE_H__
#define __GLIDE_IMAGE_H__

#include <clutter/clutter.h>
#include <cogl/cogl.h>
#include "glide-actor.h"

G_BEGIN_DECLS

#define GLIDE_TYPE_IMAGE            (glide_image_get_type ())
#define GLIDE_IMAGE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GLIDE_TYPE_IMAGE, GlideImage))
#define GLIDE_IMAGE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GLIDE_TYPE_IMAGE, GlideImageClass))
#define GLIDE_IS_IMAGE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GLIDE_TYPE_IMAGE))
#define GLIDE_IS_IMAGE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GLIDE_TYPE_IMAGE))
#define GLIDE_IMAGE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GLIDE_TYPE_IMAGE, GlideImageClass))

typedef enum {
  GLIDE_IMAGE_ERROR_OUT_OF_MEMORY,
  GLIDE_IMAGE_ERROR_NO_YUV,
  GLIDE_IMAGE_ERROR_BAD_FORMAT
} GlideImageError;

#define GLIDE_IMAGE_ERROR   (glide_image_error_quark ())
GQuark glide_image_error_quark (void);

typedef struct _GlideImage        GlideImage;
typedef struct _GlideImageClass   GlideImageClass;
typedef struct _GlideImagePrivate GlideImagePrivate;

struct _GlideImage
{
  /*< private >*/
  GlideActor         parent;

  GlideImagePrivate *priv;
};

struct _GlideImageClass
{
  GlideActorClass parent_class;


  void (* size_change)   (GlideImage *image,
                          gint            width,
                          gint            height);
  void (* pixbuf_change) (GlideImage *image);
  void (* load_finished) (GlideImage *image,
                          const GError   *error);
};

typedef enum { /*< prefix=GLIDE_IMAGE >*/
  GLIDE_IMAGE_NONE             = 0,
  GLIDE_IMAGE_RGB_FLAG_BGR     = 1 << 1,
  GLIDE_IMAGE_RGB_FLAG_PREMULT = 1 << 2, /* FIXME: not handled */
  GLIDE_IMAGE_YUV_FLAG_YUV2    = 1 << 3

  /* FIXME: add compressed types ? */
} GlideImageFlags;

GType glide_image_get_type (void) G_GNUC_CONST;

ClutterActor *       glide_image_new                    (void);
ClutterActor *       glide_image_new_from_file          (const gchar            *filename,
                                                             GError                **error);
ClutterActor *       glide_image_new_from_actor         (ClutterActor           *actor);
gboolean             glide_image_set_from_file          (GlideImage         *image,
                                                             const gchar            *filename,
                                                             GError                **error);
gboolean             glide_image_set_from_rgb_data      (GlideImage         *image,
                                                             const guchar           *data,
                                                             gboolean                has_alpha,
                                                             gint                    width,
                                                             gint                    height,
                                                             gint                    rowstride,
                                                             gint                    bpp,
                                                             GlideImageFlags     flags,
                                                             GError                **error);
gboolean              glide_image_set_from_yuv_data     (GlideImage         *image,
                                                             const guchar           *data,
                                                             gint                    width,
                                                             gint                    height,
                                                             GlideImageFlags     flags,
                                                             GError                **error);
gboolean             glide_image_set_area_from_rgb_data (GlideImage         *image,
                                                             const guchar           *data,
                                                             gboolean                has_alpha,
                                                             gint                    x,
                                                             gint                    y,
                                                             gint                    width,
                                                             gint                    height,
                                                             gint                    rowstride,
                                                             gint                    bpp,
                                                             GlideImageFlags     flags,
                                                             GError                **error);
void                  glide_image_get_base_size         (GlideImage         *image,
                                                             gint                   *width,
                                                             gint                   *height);
CoglHandle            glide_image_get_cogl_texture      (GlideImage         *image);
void                  glide_image_set_cogl_texture      (GlideImage         *image,
                                                             CoglHandle              cogl_tex);
CoglHandle            glide_image_get_cogl_material     (GlideImage         *image);
void                  glide_image_set_cogl_material     (GlideImage         *image,
                                                             CoglHandle              cogl_material);
void                  glide_image_set_sync_size         (GlideImage         *image,
                                                             gboolean                sync_size);
gboolean              glide_image_get_sync_size         (GlideImage         *image);
void                  glide_image_set_repeat            (GlideImage         *image,
                                                             gboolean                repeat_x,
                                                             gboolean                repeat_y);
void                  glide_image_get_repeat            (GlideImage         *image,
                                                             gboolean               *repeat_x,
                                                             gboolean               *repeat_y);
CoglPixelFormat       glide_image_get_pixel_format      (GlideImage         *image);
gint                  glide_image_get_max_tile_waste    (GlideImage         *image);
void                  glide_image_set_keep_aspect_ratio (GlideImage         *image,
                                                             gboolean                keep_aspect);
gboolean              glide_image_get_keep_aspect_ratio (GlideImage         *image);

G_END_DECLS

#endif /* __GLIDE_IMAGE_H__ */
g
