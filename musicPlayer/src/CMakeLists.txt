
########### sources ###############

SET(cd-musicPlayer_LIB_SRCS
	applet-struct.h
	applet-init.c
	applet-init.h
	applet-config.c
	applet-config.h
	applet-cover.c
	applet-cover.h
	applet-notifications.c
	applet-notifications.h
	applet-dbus.c
	applet-dbus.h
	applet-draw.c
	applet-draw.h
	applet-musicplayer.c
	applet-musicplayer.h
	3dcover-draw.c
	3dcover-draw.h
	applet-mpris.c
	applet-mpris.h
	applet-amarok2.c
	applet-amarok2.h
	applet-amazon.c
	applet-amazon.h
	applet-banshee.c
	applet-banshee.h
	applet-exaile.c
	applet-exaile.h
	applet-exaile3.c
	applet-exaile3.h
	applet-guayadeque.c
	applet-guayadeque.h
	applet-listen.c
	applet-listen.h
	applet-qmmp.c
	applet-qmmp.h
	applet-quodlibet.c
	applet-quodlibet.h
	applet-rhythmbox.c
	applet-rhythmbox.h
	applet-songbird.c
	applet-songbird.h
	applet-xmms.c
	applet-xmms.h
	applet-audacious.c
	applet-audacious.h
	applet-xmms2.c
	applet-xmms2.h
)

add_library(${PACKAGE_MUSICPLAYER} SHARED ${cd-musicPlayer_LIB_SRCS})

########### compil ###############
add_definitions (-DMY_APPLET_SHARE_DATA_DIR="${musicplayerdatadir}")
add_definitions (-DMY_APPLET_PREVIEW_FILE="preview.jpg")
add_definitions (-DMY_APPLET_CONF_FILE="musicPlayer.conf")
add_definitions (-DMY_APPLET_USER_DATA_DIR="musicPlayer")
add_definitions (-DMY_APPLET_VERSION="${VERSION_MUSICPLAYER}")
add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="${GETTEXT_MUSICPLAYER}")
add_definitions (-DMY_APPLET_DOCK_VERSION="${dock_version}")
add_definitions (-DMY_APPLET_ICON_FILE="icon.png")

include_directories (
	${PACKAGE_INCLUDE_DIRS})

link_directories (
	${PACKAGE_LIBRARY_DIRS})

target_link_libraries (${PACKAGE_MUSICPLAYER}
	${PACKAGE_LIBRARIES})

########### install files ###############

install(TARGETS ${PACKAGE_MUSICPLAYER} DESTINATION ${pluginsdir})


#original Makefile.am contents follow:

# #dnl Process this file with automake to produce Makefile.in
#
#lib_LTLIBRARIES = 
#	libcd-musicPlayer.la
#
#
#libcd_musicPlayer_la_SOURCES = 
#	applet-struct.h
#	applet-init.c
#	applet-init.h
#	applet-config.c
#	applet-config.h
#	applet-cover.c
#	applet-cover.h
#	applet-notifications.c
#	applet-notifications.h
#	applet-dbus.c
#	applet-dbus.h
#	applet-draw.c
#	applet-draw.h
#	applet-musicplayer.c
#	applet-musicplayer.h
#	3dcover-draw.c
#	3dcover-draw.h
#	applet-mpris.c
#	applet-mpris.h
#	applet-amarok2.c
#	applet-amarok2.h
#	applet-amazon.c
#	applet-amazon.h
#	applet-banshee.c
#	applet-banshee.h
#	applet-exaile.c
#	applet-exaile.h
#	applet-exaile3.c
#	applet-exaile3.h
#	applet-listen.c
#	applet-listen.h
#	applet-quodlibet.c
#	applet-quodlibet.h
#	applet-rhythmbox.c
#	applet-rhythmbox.h
#	applet-songbird.c
#	applet-songbird.h
#	applet-xmms.c
#	applet-xmms.h
#	applet-audacious.c
#	applet-audacious.h
#	applet-xmms2.c
#	applet-xmms2.h
#
#
#dock_version = `pkg-config --modversion cairo-dock`
#
#libcd_musicPlayer_la_CFLAGS = 
#	-I$(top_srcdir)/src
#	$(PACKAGE_CFLAGS)
#	-DMY_APPLET_SHARE_DATA_DIR=""${musicplayerdatadir)""
#	-DMY_APPLET_PREVIEW_FILE=""preview.jpg""
#	-DMY_APPLET_CONF_FILE=""musicPlayer.conf""
#	-DMY_APPLET_USER_DATA_DIR=""musicPlayer""
#	-DMY_APPLET_VERSION=""${VERSION_MUSICPLAYER)""
#	-DMY_APPLET_GETTEXT_DOMAIN=""${GETTEXT_MUSICPLAYER)""
#	-DMY_APPLET_DOCK_VERSION=""${dock_version)""
#	-DMY_APPLET_ICON_FILE=""icon.png""
#	-std=c99
#	-Werror-implicit-function-declaration
##	-g -ggdb -W -Wall
#
#
#libcd_musicPlayer_la_LIBADD = 
#	$(PACKAGE_LIBS) -lm
#
#
#libcd_musicPlayer_la_LDFLAGS =
#	-avoid-version -module