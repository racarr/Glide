/*
 * main.c
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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <config.h>

#include <gtk/gtk.h>

#include <clutter/clutter.h>
#include <clutter-gtk/clutter-gtk.h>

#include <glib/gi18n.h>

#include "glide-window.h"
#include "glide-debug.h"

guint glide_debug_flags = 0;

#ifdef GLIDE_ENABLE_DEBUG
static const GDebugKey glide_debug_keys[] = {
  {"misc", GLIDE_DEBUG_MISC},
  {"image", GLIDE_DEBUG_IMAGE},
  {"manipulator", GLIDE_DEBUG_MANIPULATOR},
  {"stage-manager", GLIDE_DEBUG_STAGE_MANAGER},
  {"window", GLIDE_DEBUG_WINDOW},
  {"paint", GLIDE_DEBUG_PAINT},
  {"text", GLIDE_DEBUG_TEXT},
  {"document", GLIDE_DEBUG_DOCUMENT}
};

static gboolean
glide_arg_debug_cb (const char *key, const char *value, gpointer user_data)
{
  glide_debug_flags |=
	g_parse_debug_string (value, glide_debug_keys, G_N_ELEMENTS (glide_debug_keys));
  return TRUE;
}

static gboolean
glide_arg_no_debug_cb (const char *key, const char *value, gpointer user_data)
{
  glide_debug_flags &=
	~g_parse_debug_string (value, glide_debug_keys, G_N_ELEMENTS (glide_debug_keys));
  return TRUE;
}
#endif

static GOptionEntry glide_args[] = {
#ifdef GLIDE_ENABLE_DEBUG
  {"glide-debug", 0, 0, G_OPTION_ARG_CALLBACK, glide_arg_debug_cb,
   "Glide debugging messages to show. Comma seperated list of: all, misc, image, manipulator, stage-manager, window, text, document, or paint",
   "FLAGS"},
  {"glide-no-debug", 0, 0, G_OPTION_ARG_CALLBACK, glide_arg_no_debug_cb,
   "Disable glide debugging", "FLAGS"},
#endif
  {NULL,},
};

GOptionGroup *
glide_get_option_group (void)
{
  GOptionGroup *group;
	
  group = g_option_group_new ("glide", "Glide Options",
							  "Show Glide Options", NULL, NULL);
  g_option_group_add_entries (group, glide_args);
	
  return group;
}

static gboolean
glide_parse_args (int *argc, char ***argv)
{
  GOptionContext *option_context;
  GOptionGroup *glide_group;
  GError *error = NULL;
  gboolean ret = TRUE;
  
  option_context = g_option_context_new (NULL);
  g_option_context_set_ignore_unknown_options (option_context, TRUE);
  g_option_context_set_help_enabled (option_context, TRUE);
  
  glide_group = glide_get_option_group ();
  g_option_context_add_group (option_context, glide_group);
  
  if (!g_option_context_parse (option_context, argc, argv, &error))
	{
	  if (error)
		{
		  g_warning ("%s", error->message);
		  g_error_free (error);
		}
	  
	  ret = FALSE;
	}
  g_option_context_free (option_context);
  
  return ret;
}

int
main (int argc, char *argv[])
{
  GlideWindow *window;

  gtk_set_locale ();
  gtk_init (&argc, &argv);
  gtk_clutter_init (&argc, &argv);
  
  g_set_application_name ("Glide");
  
  if (glide_parse_args (&argc, &argv) == FALSE)
	{
	  g_critical ("Failed to parse arguments");
	  return 1;
	}
  
  GLIDE_NOTE (MISC, "Starting Glide");
  window = glide_window_new ();
  if (argc >= 2)
    glide_window_open_document (GLIDE_WINDOW (window), argv[1]);

  gtk_main ();
  return 0;
}
