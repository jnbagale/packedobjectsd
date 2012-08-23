
INCLUDES = -Wall -g $(GLIB_CFLAGS) $(PTHREAD_CFLAGS) $(LIBZMQ_CFLAGS) $(LIBXML2_CFLAGS)

lib_LTLIBRARIES = libpackedobjectsd.la

libpackedobjectsd_la_LIBADD = $(LIBZMQ_LIBS) $(LIBXML2_LIBS) $(GLIB_LIBS)

libpackedobjectsd_la_SOURCES = 	packedobjectsd.c packedobjectsd.h message.c message.h \
				address.c address.h broker.c broker.h xmlutils.c xmlutils.h	

library_includedir = $(includedir)/packedobjectsd

library_include_HEADERS = packedobjectsd.h message.h broker.h xmlutils.h

pkgconfigdir = $(libdir)/pkgconfig

pkgconfig_DATA = $(top_builddir)/pkgconfig/packedobjectsd.pc


check_PROGRAMS = packedobjectsdtest

packedobjectsdtest_SOURCES = packedobjectsdtest.c

packedobjectsdtest_LDADD = $(lib_LTLIBRARIES)

bin_PROGRAMS = server 

server_SOURCES = config.h main.c server.c server.h database.c database.h packedobjectsd.db

server_LDADD = $(lib_LTLIBRARIES) $(GLIB_LIBS) $(PTHREAD_LIBS) $(LIBZMQ_LIBS) $(LIBXML2_LIBS) 

server_DATA = packedobjectsd.db 

serverdir = #/var/lib/misc
