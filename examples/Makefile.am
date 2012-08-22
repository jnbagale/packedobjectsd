srcdir=.
topsrcdir=..

AM_CFLAGS=-I$(topsrcdir)/include/

INCLUDES = -Wall -g $(GLIB_CFLAGS) $(PTHREAD_CFLAGS) $(LIBZMQ_CFLAGS) $(LIBXML2_CFLAGS)

bin_PROGRAMS = simple_publisher simple_subscriber simple_pub_sub

simple_publisher_SOURCES = config.h simple_publisher.c schema.xsd

simple_publisher_LDADD = $(GLIB_LIBS) $(PTHREAD_LIBS) $(LIBZMQ_LIBS) $(LIBXML2_LIBS)

simple_publisher_DATA = 

simple_publisherdir =

simple_subscriber_SOURCES = config.h simple_subscriber.c schema.xsd

simple_subscriber_LDADD = $(GLIB_LIBS) $(PTHREAD_LIBS) $(LIBZMQ_LIBS) $(LIBXML2_LIBS)

simple_subscriber_DATA = 

simple_subscriberdir =

simple_pub_sub_SOURCES = config.h simple_pub_sub.c schema.xsd

simple_pub_sub_LDADD = $(GLIB_LIBS) $(PTHREAD_LIBS) $(LIBZMQ_LIBS) $(LIBXML2_LIBS)

simple_pub_sub_DATA = 

simple_pub_subdir =