## Process this file with automake to produce Makefile.in

## Created by Anjuta


AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	$(GTK_CFLAGS) \
	$(CLUTTER_CFLAGS) \
	$(CLUTTER_GTK_CFLAGS)

AM_CFLAGS =\
	 -Wall\
	 -g

bin_PROGRAMS = glide

glide_SOURCES = \
	main.c \
	glide-window.c \
	glide-window.h \
	glide-manipulator.c \
	glide-manipulator.h \
	glide-actor.c \
	glide-actor.h \
	glide-rectangle.c \
	glide-rectangle.h \	
	glide-stage-manager.c \
	glide-stage-manager.h \
	glide-image.c \
	glide-image.h

glide_LDFLAGS = \
	-Wl,--export-dynamic

glide_LDADD = $(GTK_LIBS) $(CLUTTER_LIBS) $(CLUTTER_GTK_LIBS)

EXTRA_DIST = $(ui_DATA)

# Remove ui directory on uninstall
uninstall-local:
	-rm -r $(uidir)
	-rm -r $(datadir)/glide
