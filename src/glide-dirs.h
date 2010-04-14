/*
 * glide-dirs.c
 * Copyright (C) Robert Carr 2010 <racarr@gnome.org>
 * 
 * Glide is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Glide is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef __GLIDE_DIRS_H__
#define __GLIDE_DIRS_H__

#include <glib.h>

gchar *
glide_dirs_get_glide_data_dir (void);

gchar *
glide_dirs_get_glide_image_dir (void);

gchar *
glide_dirs_get_glide_ui_dir (void);

#endif
