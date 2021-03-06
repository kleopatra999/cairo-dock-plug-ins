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

#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-clipboard.h"
#include "applet-config.h"


//\_________________ Here you have to get all your parameters from the conf file. Use the macros CD_CONFIG_GET_BOOLEAN, CD_CONFIG_GET_INTEGER, CD_CONFIG_GET_STRING, etc. myConfig has been reseted to 0 at this point. This function is called at the beginning of init and reload.
CD_APPLET_GET_CONFIG_BEGIN
	myConfig.iItemType = CD_CONFIG_GET_INTEGER ("Configuration", "item type");
	myConfig.bSeparateSelections = CD_CONFIG_GET_BOOLEAN ("Configuration", "separate");
	myConfig.iNbItems[CD_CLIPPER_CLIPBOARD] = CD_CONFIG_GET_INTEGER ("Configuration", "nb items");
	if (myConfig.bSeparateSelections)
	{
		if (myConfig.iItemType & CD_CLIPPER_PRIMARY)
			myConfig.iNbItems[CD_CLIPPER_PRIMARY] = CD_CONFIG_GET_INTEGER ("Configuration", "nb items2");
	}
	else
	{
		myConfig.iNbItems[CD_CLIPPER_BOTH] = myConfig.iNbItems[CD_CLIPPER_CLIPBOARD];
		myConfig.iNbItems[CD_CLIPPER_PRIMARY] = myConfig.iNbItems[CD_CLIPPER_CLIPBOARD];
	}
	myConfig.bPasteInClipboard = CD_CONFIG_GET_BOOLEAN ("Configuration", "paste clipboard");
	myConfig.bPasteInPrimary = CD_CONFIG_GET_BOOLEAN ("Configuration", "paste selection");
	myConfig.bEnableActions = CD_CONFIG_GET_BOOLEAN ("Configuration", "enable actions");
	myConfig.bMenuOnMouse = CD_CONFIG_GET_BOOLEAN ("Configuration", "menu on mouse");
	
	
	myConfig.bReplayAction = CD_CONFIG_GET_BOOLEAN ("Configuration", "replay action");
	myConfig.iActionMenuDuration = CD_CONFIG_GET_INTEGER ("Configuration", "action duration");
	
	myConfig.cShortcut = CD_CONFIG_GET_STRING ("Configuration", "shortkey");
	
	gsize length = 0;
	myConfig.pPersistentItems = CD_CONFIG_GET_STRING_LIST("Configuration", "persistent", &length);
	
	myConfig.bRememberItems = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "remember", FALSE);
	myConfig.cRememberedItems = CD_CONFIG_GET_STRING ("Configuration", "last items");  // on les recupere meme si on ne veut psa s'en souvenir, pour pouvoir effacer le contenu de la cle si on desactive l'option.
CD_APPLET_GET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myConfig. This one will be reseted to 0 at the end of this function. This function is called right before you get the applet's config, and when your applet is stopped, in the end.
CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.cShortcut);
	g_strfreev (myConfig.pPersistentItems);
	g_free (myConfig.cRememberedItems);
CD_APPLET_RESET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myData. This one will be reseted to 0 at the end of this function. This function is called when your applet is stopped, in the very end.
CD_APPLET_RESET_DATA_BEGIN
	g_list_foreach (myData.pItems, (GFunc) cd_clipper_free_item, NULL);
	g_list_free (myData.pItems);
	
	g_list_foreach (myData.pActions, (GFunc)cd_clipper_free_action, NULL);
	g_list_free (myData.pActions);
	
	if (myData.pActionMenu)
		gtk_widget_destroy (myData.pActionMenu);
CD_APPLET_RESET_DATA_END
