INCLUDES = -Wall -g $(GLIB_CFLAGS) $(PTHREAD_CFLAGS) $(LIBZMQ_CFLAGS) $(LIBXML2_CFLAGS)

bin_PROGRAMS = server node #broker

#broker_SOURCES = config.h old_broker.c broker.c broker.h xmlutils.h

#broker_LDADD = $(GLIB_LIBS) $(LIBZMQ_LIBS)  $(LIBXML2_LIBS)

#broker_DATA =

#brokerdir = #/var/lib/misc

server_SOURCES = config.h main.c server.c server.h broker.c broker.h database.c database.h message.c message.h xmlutils.c xmlutils.h packedobjectsd.db
 
server_LDADD = $(GLIB_LIBS) $(PTHREAD_LIBS) $(LIBZMQ_LIBS) $(LIBXML2_LIBS)

server_DATA = packedobjectsd.db

serverdir = #/var/lib/misc

node_SOURCES = config.h node.c subscriber.h publisher.h message.c message.h xmlutils.c xmlutils.h 

node_LDADD = $(GLIB_LIBS) $(PTHREAD_LIBS) $(LIBZMQ_LIBS) $(LIBXML2_LIBS) 

node_DATA = subscriber.h publisher.h 

nodedir = /usr/local/include/