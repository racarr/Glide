SUBDIRS = ui icons

desktopdir = $(datadir)/applications
desktop_in_files = glide.desktop.in.in
desktop_DATA = $(desktop_in_files:.desktop.in.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

imagedir = $(datadir)/glide/images
image_DATA = manipulator-widget.png \
	manipulator-widget-active.png

mimedir = $(datadir)/mime/packages
mime_DATA = glide-mime.xml

mimeicondir = $(datadir)/icons/gnome/scalable/mimetypes
mimeicon_DATA = gnome-mime-application-x-glide.svg

EXTRA_DIST = \
	$(image_DATA) \
	$(desktop_in_files) \
	gnome-mime-application-x-glide.svg \
	glide-mime.xml

CLEANFILES = \
	$(desktop_DATA)

install-data-local: install-mimeDATA
	if [ -f $(DESTDIR)$(datadir)/mime/packages/freedesktop.org.xml ] ; then \
	  if which update-mime-database>/dev/null 2>&1; then \
	    update-mime-database $(DESTDIR)$(datadir)/mime; \
	  fi \
	fi

uninstall-local:
	if [ -f $(DESTDIR)$(datadir)/mime/packages/freedesktop.org.xml ] ; then \
	  if which update-mime-database>/dev/null 2>&1; then \
	    update-mime-database $(DESTDIR)$(datadir)/mime; \
	  fi \
	fi


-include $(top_srcdir)/git.mk
