
########### sources ###############

SET(cd-weather_LIB_SRCS
	applet-init.c
	applet-init.h
	applet-config.c
	applet-config.h
	applet-notifications.c
	applet-notifications.h
	applet-struct.h
	applet-read-data.c
	applet-read-data.h
	applet-load-icons.c
	applet-load-icons.h
)

add_library(${PACKAGE_WEATHER} SHARED ${cd-weather_LIB_SRCS})

########### compil ###############
add_definitions (-DMY_APPLET_SHARE_DATA_DIR="${weatherdatadir}")
add_definitions (-DMY_APPLET_PREVIEW_FILE="preview.jpg")
add_definitions (-DMY_APPLET_CONF_FILE="weather.conf")
add_definitions (-DMY_APPLET_USER_DATA_DIR="${weatheruserdirname}")
add_definitions (-DMY_APPLET_VERSION="${VERSION_WEATHER}")
add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="${GETTEXT_WEATHER}")
add_definitions (-DMY_APPLET_DOCK_VERSION="${dock_version}")
add_definitions (-DMY_APPLET_ICON_FILE="icon.png")
add_definitions (-DCD_APPLET_MULTI_INSTANCE="1")

include_directories (
	${PACKAGE_INCLUDE_DIRS})

link_directories (
	${PACKAGE_LIBRARY_DIRS})

target_link_libraries (${PACKAGE_WEATHER}
	${PACKAGE_LIBRARIES})

########### install files ###############

install(TARGETS ${PACKAGE_WEATHER} DESTINATION ${pluginsdir})


#original Makefile.am contents follow:

# #dnl Process this file with automake to produce Makefile.in
#
#lib_LTLIBRARIES = 
#	libcd-weather.la
#
#
#libcd_weather_la_SOURCES = 
#	applet-init.c
#	applet-init.h
#	applet-config.c
#	applet-config.h
#	applet-notifications.c
#	applet-notifications.h
#	applet-struct.h
#	applet-read-data.c
#	applet-read-data.h
#	applet-load-icons.c
#	applet-load-icons.h
#
#dock_version = `pkg-config --modversion cairo-dock`
#
#libcd_weather_la_CFLAGS = 
#	-I$(top_srcdir)/src
#	$(PACKAGE_CFLAGS)
#	-DMY_APPLET_SHARE_DATA_DIR=""${weatherdatadir)""
#	-DMY_APPLET_PREVIEW_FILE=""preview.jpg""
#	-DMY_APPLET_CONF_FILE=""weather.conf""
#	-DMY_APPLET_USER_DATA_DIR=""${weatheruserdirname)""
#	-DMY_APPLET_VERSION=""${VERSION_WEATHER)""
#	-DMY_APPLET_GETTEXT_DOMAIN=""${GETTEXT_WEATHER)""
#	-DMY_APPLET_DOCK_VERSION=""${dock_version)""
#	-DMY_APPLET_ICON_FILE=""icon.png""
#	-DCD_APPLET_MULTI_INSTANCE="1"
#	-std=c99
#	-Werror-implicit-function-declaration
##	-g -ggdb -W -Wall
#
#
#libcd_weather_la_LIBADD = 
#	$(PACKAGE_LIBS) -lm
#
#
#libcd_weather_la_LDFLAGS =
#	-avoid-version -module
