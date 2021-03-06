
########### sources ###############

SET(cd-terminal_LIB_SRCS
	terminal-init.c
	terminal-init.h
	terminal-config.c
	terminal-config.h
	terminal-menu-functions.c
	terminal-menu-functions.h
	terminal-callbacks.c
	terminal-callbacks.h
	terminal-widget.c
	terminal-widget.h
	terminal-struct.h
)

add_library(${PACKAGE_TERMINAL} SHARED ${cd-terminal_LIB_SRCS})

########### compil ###############
add_definitions (-DMY_APPLET_SHARE_DATA_DIR="${terminaldatadir}")
add_definitions (-DMY_APPLET_PREVIEW_FILE="preview.jpg")
add_definitions (-DMY_APPLET_CONF_FILE="terminal.conf")
add_definitions (-DMY_APPLET_USER_DATA_DIR="terminal")
add_definitions (-DMY_APPLET_VERSION="${VERSION_TERMINAL}")
add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="${GETTEXT_TERMINAL}")
add_definitions (-DMY_APPLET_DOCK_VERSION="${dock_version}")
add_definitions (-DMY_APPLET_ICON_FILE="icon.png")
add_definitions (-DVTE_MAJOR=${VTE_MAJOR})
add_definitions (-DVTE_MINOR=${VTE_MINOR})

include_directories (
	${PACKAGE_INCLUDE_DIRS}
	${VTE_INCLUDE_DIRS})

link_directories (
	${PACKAGE_LIBRARY_DIRS}
	${VTE_LIBRARY_DIRS})

target_link_libraries (${PACKAGE_TERMINAL}
	${PACKAGE_LIBRARIES}
	${VTE_LIBRARIES})

########### install files ###############

install(TARGETS ${PACKAGE_TERMINAL} DESTINATION ${pluginsdir})


#original Makefile.am contents follow:

# #dnl Process this file with automake to produce Makefile.in
#
#lib_LTLIBRARIES = 
#	libcd-terminal.la
#
#
#libcd_terminal_la_SOURCES = 
#	terminal-init.c
#	terminal-init.h
#	terminal-config.c
#	terminal-config.h
#	terminal-menu-functions.c
#	terminal-menu-functions.h
#	terminal-callbacks.c
#	terminal-callbacks.h
#	terminal-widget.c
#	terminal-widget.h
#	terminal-struct.h
#
#dock_version = `pkg-config --modversion cairo-dock`
#
#libcd_terminal_la_CFLAGS = 
#	-I$(top_srcdir)/src
#	$(PACKAGE_CFLAGS)
#	$(VTE_CFLAGS)
#	-DMY_APPLET_SHARE_DATA_DIR=""${terminaldatadir)""
#	-DMY_APPLET_PREVIEW_FILE=""preview.jpg""
#	-DMY_APPLET_CONF_FILE=""terminal.conf""
#	-DMY_APPLET_USER_DATA_DIR=""terminal""
#	-DMY_APPLET_VERSION=""${VERSION_TERMINAL)""
#	-DMY_APPLET_GETTEXT_DOMAIN=""${GETTEXT_TERMINAL)""
#	-DMY_APPLET_DOCK_VERSION=""${dock_version)""
#	-DMY_APPLET_ICON_FILE=""icon.png""
#	-std=c99
#	-Werror-implicit-function-declaration
##	-g -ggdb -W -Wall  -W -Werror
#
#
#libcd_terminal_la_LIBADD = 
#	$(PACKAGE_LIBS) -lm 
#	$(VTE_LIBS)
#
#
#libcd_terminal_la_LDFLAGS =
#	-avoid-version -module
