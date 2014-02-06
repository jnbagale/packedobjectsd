
INCLUDES = -Wall -g  $(LIBZMQ_CFLAGS) $(LIBXML2_CFLAGS) $(LIBPACKEDOBJECTS_CFLAGS)

lib_LTLIBRARIES = libpackedobjectsd.la

libpackedobjectsd_la_LIBADD = $(LIBZMQ_LIBS) $(LIBXML2_LIBS) $(LIBPACKEDOBJECTS_LIBS) 

libpackedobjectsd_la_SOURCES = 	packedobjectsd.c packedobjectsd.h message.c message.h broker.c broker.h \
				response.c response.h request.c request.h xmlutils.c xmlutils.h md5_hash.c md5_hash.h global.h \
				$(top_builddir)/pkgconfig/libpackedobjectsd.pc \
				$(top_builddir)/schema/packedobjectsd.xsd 

library_includedir = $(includedir)/packedobjectsd

library_include_HEADERS = packedobjectsd.h message.h broker.h request.h response.h xmlutils.h md5_hash.h config.h


library_datadirdir = $(datadir)/@PACKAGE@

library_datadir_DATA = $(top_builddir)/schema/packedobjectsd.xsd


check_PROGRAMS = packedobjectsdtest

packedobjectsdtest_SOURCES = main.c

packedobjectsdtest_LDADD = $(lib_LTLIBRARIES)


pkgconfigdir = $(libdir)/pkgconfig

pkgconfig_DATA = $(top_builddir)/pkgconfig/libpackedobjectsd.pc
