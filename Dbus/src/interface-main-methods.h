/**
* This file is a part of the Cairo-Dock project
*
* Copyright : (C) see the 'copyright' file.
* E-mail    : see the 'copyright' file.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 3
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef __APPLET_MAIN_METHODS__
#define  __APPLET_MAIN_METHODS__

#include <cairo-dock.h>
#include "applet-struct.h"


gboolean cd_dbus_main_show_desklet(dbusMainObject *dbusMainObject, gboolean *widgetLayer, GError **error);

gboolean cd_dbus_main_reboot(dbusMainObject *dbusMainObject, GError **error);

gboolean cd_dbus_main_quit (dbusMainObject *dbusMainObject, GError **error);

gboolean cd_dbus_main_show_dock (dbusMainObject *dbusMainObject, gboolean bShow, GError **error);

gboolean cd_dbus_main_reload_module (dbusMainObject *dbusMainObject, const gchar *cModuleName, GError **error);

gboolean cd_dbus_main_activate_module (dbusMainObject *dbusMainObject, const gchar *cModuleName, gboolean bActivate, GError **error);

gboolean cd_dbus_main_load_launcher_from_file (dbusMainObject *dbusMainObject, const gchar *cDesktopFile, GError **error);

gboolean cd_dbus_main_create_launcher_from_scratch (dbusMainObject *dbusMainObject, const gchar *cIconFile, const gchar *cLabel, const gchar *cCommand, const gchar *cParentDockName, GError **error);

gboolean cd_dbus_main_reload_launcher (dbusMainObject *dbusMainObject, const gchar *cDesktopFile, GError **error);

gboolean cd_dbus_main_remove_launcher (dbusMainObject *dbusMainObject, const gchar *cDesktopFile, GError **error);


gboolean cd_dbus_main_set_quick_info (dbusMainObject *dbusMainObject, const gchar *cQuickInfo, const gchar *cIconName, const gchar *cIconCommand, const gchar *cModuleName, GError **error);

gboolean cd_dbus_main_set_label (dbusMainObject *dbusMainObject, const gchar *cLabel, const gchar *cIconName, const gchar *cIconCommand, const gchar *cModuleName, GError **error);

gboolean cd_dbus_main_set_icon (dbusMainObject *dbusMainObject, const gchar *cImage, const gchar *cIconName, const gchar *cIconCommand, const gchar *cModuleName, GError **error);

gboolean cd_dbus_main_set_emblem (dbusMainObject *pDbusCallback, const gchar *cImage, gint iPosition, const gchar *cIconName, const gchar *cIconCommand, const gchar *cModuleName, GError **error);

gboolean cd_dbus_main_animate (dbusMainObject *dbusMainObject, const gchar *cAnimation, gint iNbRounds, const gchar *cIconName, const gchar *cIconCommand, const gchar *cModuleName, GError **error);

gboolean cd_dbus_main_show_dialog (dbusMainObject *dbusMainObject, const gchar *message, gint iDuration, const gchar *cIconName, const gchar *cIconCommand, const gchar *cModuleName, GError **error);


gboolean cd_dbus_main_register_new_module (dbusMainObject *pDbusCallback, const gchar *cModuleName, const gchar *cDescription, const gchar *cAuthor, const gchar *cVersion, gint iCategory, const gchar *cShareDataDir, GError **error);

gboolean cd_dbus_main_unregister_module (dbusMainObject *dbusMainObject, const gchar *cModuleName, GError **error);


#endif