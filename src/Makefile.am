srcdir=.
topsrcdir=..

AM_CFLAGS=-I$(topsrcdir)/include/

INCLUDES = -Wall -g $(GLIB_CFLAGS) $(PTHREAD_CFLAGS) $(LIBZMQ_CFLAGS) $(LIBXML2_CFLAGS)

bin_PROGRAMS = server 

server_SOURCES = config.h main.c server.c server.h packedobjectsd.db

server_LDADD = $(GLIB_LIBS) $(PTHREAD_LIBS) $(LIBZMQ_LIBS) $(LIBXML2_LIBS)

server_DATA = packedobjectsd.db 

serverdir = #/var/lib/misc
