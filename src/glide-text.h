/*
 * glide-text.h
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
/* Originally based off of clutter-text.h */
/*
 * Clutter.
 *
 * An OpenGL based 'interactive canvas' library.
 *
 * Copyright (C) 2008  Intel Corporation.
 *
 * Authored By: Øyvind Kolås <pippin@o-hand.com>
 *              Emmanuele Bassi <ebassi@linux.intel.com>
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
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __GLIDE_TEXT_H__
#define __GLIDE_TEXT_H__

#include <clutter/clutter.h>
#include <cogl/cogl.h>

#include <pango/pango.h>

#include "glide-actor.h"
#include "glide-stage-manager.h"

G_BEGIN_DECLS

#define GLIDE_TYPE_TEXT               (glide_text_get_type ())
#define GLIDE_TEXT(obj)               (G_TYPE_CHECK_INSTANCE_CAST ((obj), GLIDE_TYPE_TEXT, GlideText))
#define GLIDE_TEXT_CLASS(klass)       (G_TYPE_CHECK_CLASS_CAST ((klass), GLIDE_TYPE_TEXT, GlideTextClass))
#define GLIDE_IS_TEXT(obj)            (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GLIDE_TYPE_TEXT))
#define GLIDE_IS_TEXT_CLASS(klass)    (G_TYPE_CHECK_CLASS_TYPE ((klass), GLIDE_TYPE_TEXT))
#define GLIDE_TEXT_GET_CLASS(obj)     (G_TYPE_INSTANCE_GET_CLASS ((obj), GLIDE_TYPE_TEXT, GlideTextClass))

typedef struct _GlideText        GlideText;
typedef struct _GlideTextPrivate GlideTextPrivate;
typedef struct _GlideTextClass   GlideTextClass;

/**
 * GlideText:
 *
 * The #GlideText struct contains only private data.
 *
 * Since: 1.0
 */
struct _GlideText
{
  /*< private >*/
  GlideActor parent_instance;

  GlideTextPrivate *priv;
};

/**
 * GlideTextClass:
 * @text_changed: class handler for the #GlideText::text-changed signal
 * @activate: class handler for the #GlideText::activate signal
 * @cursor_event: class handler for the #GlideText::cursor_event signal
 *
 * The #GlideTextClass struct contains only private data.
 *
 * Since: 1.0
 */
struct _GlideTextClass
{
  /*< private >*/
  GlideActorClass parent_class;

  /*< public >*/
  /* signals, not vfuncs */
  void (* text_changed) (GlideText           *self);
  void (* activate)     (GlideText           *self);
  void (* cursor_event) (GlideText           *self,
                         const ClutterGeometry *geometry);
};

GType glide_text_get_type (void) G_GNUC_CONST;

ClutterActor *        glide_text_new                  ();
ClutterActor *        glide_text_new_full             (const gchar          *font_name,
						       const gchar          *text,
						       const ClutterColor   *color);
ClutterActor *        glide_text_new_with_text        (const gchar          *font_name,
						       const gchar          *text);
G_CONST_RETURN gchar *glide_text_get_text             (GlideText          *self);
void                  glide_text_set_text             (GlideText          *self,
                                                         const gchar          *text);
void                  glide_text_set_markup           (GlideText          *self,
                                                         const gchar          *markup);
void                  glide_text_set_color            (GlideText          *self,
                                                         const ClutterColor   *color);
void                  glide_text_get_color            (GlideText          *self,
                                                         ClutterColor         *color);
void                  glide_text_set_font_name        (GlideText          *self,
                                                         const gchar          *font_name);
G_CONST_RETURN gchar *glide_text_get_font_name        (GlideText          *self);
void                  glide_text_set_font_description (GlideText          *self,
                                                         PangoFontDescription *font_desc);
PangoFontDescription *glide_text_get_font_description (GlideText          *self);

void                  glide_text_set_ellipsize        (GlideText          *self,
                                                         PangoEllipsizeMode    mode);
PangoEllipsizeMode    glide_text_get_ellipsize        (GlideText          *self);
void                  glide_text_set_line_wrap        (GlideText          *self,
                                                         gboolean              line_wrap);
gboolean              glide_text_get_line_wrap        (GlideText          *self);
void                  glide_text_set_line_wrap_mode   (GlideText          *self,
                                                         PangoWrapMode         wrap_mode);
PangoWrapMode         glide_text_get_line_wrap_mode   (GlideText          *self);
PangoLayout *         glide_text_get_layout           (GlideText          *self);
void                  glide_text_set_attributes       (GlideText          *self,
                                                         PangoAttrList        *attrs);
PangoAttrList *       glide_text_get_attributes       (GlideText          *self);
void                  glide_text_set_use_markup       (GlideText          *self,
                                                         gboolean              setting);
gboolean              glide_text_get_use_markup       (GlideText          *self);
void                  glide_text_set_line_alignment   (GlideText          *self,
                                                         PangoAlignment        alignment);
PangoAlignment        glide_text_get_line_alignment   (GlideText          *self);
void                  glide_text_set_justify          (GlideText          *self,
                                                         gboolean              justify);
gboolean              glide_text_get_justify          (GlideText          *self);

void                  glide_text_insert_unichar       (GlideText          *self,
                                                         gunichar              wc);
void                  glide_text_delete_chars         (GlideText          *self,
                                                         guint                 n_chars);
void                  glide_text_insert_text          (GlideText          *self,
                                                         const gchar          *text,
                                                         gssize                position);
void                  glide_text_delete_text          (GlideText          *self,
                                                         gssize                start_pos,
                                                         gssize                end_pos);
gchar *               glide_text_get_chars            (GlideText          *self,
                                                         gssize                start_pos,
                                                         gssize                end_pos);
void                  glide_text_set_editable         (GlideText          *self,
                                                         gboolean              editable);
gboolean              glide_text_get_editable         (GlideText          *self);
void                  glide_text_set_activatable      (GlideText          *self,
                                                         gboolean              activatable);
gboolean              glide_text_get_activatable      (GlideText          *self);

gint                  glide_text_get_cursor_position  (GlideText          *self);
void                  glide_text_set_cursor_position  (GlideText          *self,
                                                         gint                  position);
void                  glide_text_set_cursor_visible   (GlideText          *self,
                                                         gboolean              cursor_visible);
gboolean              glide_text_get_cursor_visible   (GlideText          *self);
void                  glide_text_set_cursor_color     (GlideText          *self,
                                                         const ClutterColor   *color);
void                  glide_text_get_cursor_color     (GlideText          *self,
                                                         ClutterColor         *color);
void                  glide_text_set_cursor_size      (GlideText          *self,
                                                         gint                  size);
guint                 glide_text_get_cursor_size      (GlideText          *self);
void                  glide_text_set_selectable       (GlideText          *self,
                                                         gboolean              selectable);
gboolean              glide_text_get_selectable       (GlideText          *self);
void                  glide_text_set_selection_bound  (GlideText          *self,
                                                         gint                  selection_bound);
gint                  glide_text_get_selection_bound  (GlideText          *self);
void                  glide_text_set_selection        (GlideText          *self,
                                                         gssize                start_pos,
                                                         gssize                end_pos);
gchar *               glide_text_get_selection        (GlideText          *self);
void                  glide_text_set_selection_color  (GlideText          *self,
                                                         const ClutterColor   *color);
void                  glide_text_get_selection_color  (GlideText          *self,
                                                         ClutterColor         *color);
gboolean              glide_text_delete_selection     (GlideText          *self);
void                  glide_text_set_password_char    (GlideText          *self,
                                                         gunichar              wc);
gunichar              glide_text_get_password_char    (GlideText          *self);
void                  glide_text_set_max_length       (GlideText          *self,
                                                         gint                  max);
gint                  glide_text_get_max_length       (GlideText          *self);
void                  glide_text_set_single_line_mode (GlideText          *self,
                                                         gboolean              single_line);
gboolean              glide_text_get_single_line_mode (GlideText          *self);

gboolean              glide_text_activate             (GlideText          *self);
gboolean              glide_text_position_to_coords   (GlideText          *self,
                                                         gint                  position,
                                                         gfloat               *x,
                                                         gfloat               *y,
                                                         gfloat               *line_height);

void                  glide_text_set_preedit_string   (GlideText          *self,
						       const gchar          *preedit_str,
						       PangoAttrList        *preedit_attrs,
						       guint                 cursor_pos);


void glide_text_set_absolute_font_size (GlideText *self, gdouble font_size);
gdouble glide_text_get_absolute_font_size (GlideText *self);

void glide_text_update_actor_size (GlideText *self);

G_END_DECLS

#endif /* __GLIDE_TEXT_H__ */
