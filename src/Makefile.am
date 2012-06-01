INCLUDES = -Wall $(GLIB_CFLAGS) $(GTHREAD_CFLAGS) $(LIBZMQ_CFLAGS)

bin_PROGRAMS = server broker node

broker_SOURCES = config.h broker.c forwarder.c forwarder.h

broker_LDADD = $(GLIB_LIBS) $(GTHREAD_LIBS) $(LIBZMQ_LIBS)

broker_DATA =

brokerdir = /var/lib/misc

server_SOURCES = config.h server.c

server_LDADD = $(GLIB_LIBS) $(GTHREAD_LIBS) $(LIBZMQ_LIBS)

node_SOURCES = config.h node.c subscriber.c subscriber.h publisher.c publisher.h

node_LDADD = $(GLIB_LIBS) $(GTHREAD_LIBS) $(LIBZMQ_LIBS)