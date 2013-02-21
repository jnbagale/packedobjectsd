
INCLUDES = -Wall -g $(LIBPACKEDOBJECTSD_CFLAGS)

bin_PROGRAMS = video_searcher video_client

video_searcher_SOURCES = video_searcher.c video.xsd

video_searcher_LDADD = $(LIBPACKEDOBJECTSD_LIBS)

video_client_SOURCES = video_client.c database.xml video.xsd

video_client_LDADD = $(LIBPACKEDOBJECTSD_LIBS)
