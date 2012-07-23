INCLUDES = -Wall $(GLIB_CFLAGS)  $(LIBZMQ_CFLAGS) $(LIBXML2_CFLAGS)

bin_PROGRAMS = server broker node

broker_SOURCES = config.h broker.c forwarder.c forwarder.h xmlutils.h

broker_LDADD = $(GLIB_LIBS)  $(LIBZMQ_LIBS)  $(LIBXML2_LIBS)

broker_DATA =

brokerdir = /var/lib/misc

server_SOURCES = config.h server.c lookup.c lookup.h #packedobjectsd.db

server_LDADD = $(GLIB_LIBS)  $(LIBZMQ_LIBS)

server_DATA = #packedobjectsd.db

serverdir = /var/lib/misc

node_SOURCES = config.h node.c subscriber.h publisher.h xmlutils.h

node_LDADD = $(GLIB_LIBS) $(LIBZMQ_LIBS) $(LIBXML2_LIBS)

node_DATA = subscriber.h publisher.h 

nodedir = /usr/local/include/