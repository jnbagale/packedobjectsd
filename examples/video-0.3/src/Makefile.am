
INCLUDES = -Wall -g $(LIBPACKEDOBJECTSD_CFLAGS) $(UUID_CFLAGS)

bin_PROGRAMS = video_searcher video_responder

video_searcher_SOURCES = video_searcher.c video.xsd

video_searcher_LDADD = $(LIBPACKEDOBJECTSD_LIBS) $(UUID_LIBS)

video_responder_SOURCES = video_responder.c video.xsd video.xml

video_responder_LDADD = $(LIBPACKEDOBJECTSD_LIBS) $(UUID_LIBS)
