
INCLUDES = -Wall -g $(LIBPACKEDOBJECTSD_CFLAGS) $(UUID_CFLAGS)

bin_PROGRAMS = video_searcher video_client

video_searcher_SOURCES = video_searcher.c video.xsd

video_searcher_LDADD = $(LIBPACKEDOBJECTSD_LIBS) $(UUID_LIBS)

video_client_SOURCES = video_client.c video.xsd database.xml

video_client_LDADD = $(LIBPACKEDOBJECTSD_LIBS) $(UUID_LIBS)
