## Process this file with automake to produce Makefile.in

## Created by Anjuta


AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	$(GTK_CFLAGS) \
	$(JSON_GLIB_CFLAGS) \
	$(CLUTTER_CFLAGS) \
	$(CLUTTER_GTK_CFLAGS) \
	$(GLIDE_DEBUG_CFLAGS) \
	$(GOBJECT_INTROSPECTION_CFLAGS)

AM_CFLAGS =\
	 -Wall \
	 -Werror 	

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
	glide-image.h \
	glide-text.c \
	glide-text.h \
	glide-dirs.c \
	glide-dirs.h \
	glide-document.c \
	glide-document.h \
	glide-types.h \
	glide-slide.h \
	glide-slide.c \
	glide-json-util.h \
	glide-json-util.c 

glide_LDFLAGS = \
	-Wl,--export-dynamic

glide_LDADD = $(GTK_LIBS) $(CLUTTER_LIBS) $(CLUTTER_GTK_LIBS) $(GOBJECT_INTROSPECTION_LIBS) $(JSON_GLIB_LIBS)

EXTRA_DIST = $(ui_DATA)

# Remove ui directory on uninstall
uninstall-local:
	-rm -r $(uidir)
	-rm -r $(datadir)/glide
