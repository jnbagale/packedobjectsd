
INCLUDES = -Wall -g $(LIBPACKEDOBJECTSD_CFLAGS)

bin_PROGRAMS = video_searcher video_receiver video_server

video_searcher_SOURCES = video_searcher.c search.xml video.xsd

video_searcher_LDADD = $(LIBPACKEDOBJECTSD_LIBS)

video_receiver_SOURCES = video_receiver.c video.xsd

video_receiver_LDADD = $(LIBPACKEDOBJECTSD_LIBS)

video_server_SOURCES = video_server.c video.xml video.xsd

video_server_LDADD = $(LIBPACKEDOBJECTSD_LIBS)