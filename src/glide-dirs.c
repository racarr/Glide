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

#include "glide-dirs.h"

gchar *
glide_dirs_get_glide_data_dir (void)
{
  gchar *datadir = g_build_filename(PACKAGE_DATA_DIR, "glide", NULL);
  
  return datadir;
}

gchar *
glide_dirs_get_glide_image_dir ()
{
  gchar *datadir = glide_dirs_get_glide_data_dir();
  gchar *imagedir = g_build_filename(datadir, "images", NULL);

  g_free (datadir);
  return imagedir;
}

gchar *
glide_dirs_get_glide_ui_dir ()
{
  gchar *datadir = glide_dirs_get_glide_data_dir();
  gchar *uidir = g_build_filename(datadir, "ui", NULL);
  
  g_free (datadir);
  return uidir;
}
