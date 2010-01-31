/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
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

static void
glide_setup_main_window (GtkBuilder *builder)
{
	ClutterActor *stage;
	ClutterColor black = {0x00, 0x00, 0x00, 0xff};
	GtkWidget *embed, *window, *vbox;

	window = GTK_WIDGET (gtk_builder_get_object (builder, "window1"));
	gtk_window_set_default_size (GTK_WINDOW (window), 600, 500);

	vbox = GTK_WIDGET (gtk_builder_get_object (builder, "vbox1"));
	
	embed = gtk_clutter_embed_new ();
	gtk_container_add (GTK_CONTAINER (vbox), embed);
	

	stage = gtk_clutter_embed_get_stage (GTK_CLUTTER_EMBED (embed));
	clutter_actor_set_size (stage, 500, 500);
	
	gtk_widget_set_size_request (embed, 500, 500);
	
	clutter_stage_set_color (CLUTTER_STAGE (stage), &black);	
	clutter_actor_show (stage);

	gtk_widget_show_all (window);
}

int
main (int argc, char *argv[])
{
	GlideWindow *window;

	gtk_set_locale ();
	gtk_init (&argc, &argv);
	gtk_clutter_init (&argc, &argv);

	window = glide_window_new ();

	gtk_main ();
	return 0;
}
