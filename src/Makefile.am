srcdir=.
topsrcdir=..

AM_CFLAGS=-I$(topsrcdir)/include/

INCLUDES = -Wall -g $(GLIB_CFLAGS) $(PTHREAD_CFLAGS) $(LIBZMQ_CFLAGS) $(LIBXML2_CFLAGS)

lib_LTLIBRARIES = libpackedobjectsd.la

libpackedobjectsd_la_LIBADD = $(LIBZMQ_LIBS) $(LIBXML2_LIBS)

libpackedobjectsd_la_SOURCES = 	$(topsrcdir)/include/packedobjectsd.h \
				$(topsrcdir)/include/message.h 	 $(topsrcdir)/include/database.h   \
				$(topsrcdir)/include/broker.h    $(topsrcdir)/include/xmlutils.h				
library_includedir = $(includedir)/packedobjectsd

library_include_HEADERS = $(topsrcdir)/include/packedobjectsd.h \
				$(topsrcdir)/include/message.h 	 $(topsrcdir)/include/database.h   \
				$(topsrcdir)/include/broker.h    $(topsrcdir)/include/xmlutils.h

bin_PROGRAMS = server 

server_SOURCES = config.h main.c server.c server.h packedobjectsd.db

server_LDADD = $(GLIB_LIBS) $(PTHREAD_LIBS) $(LIBZMQ_LIBS) $(LIBXML2_LIBS)

server_DATA = packedobjectsd.db 

serverdir = #/var/lib/misc
