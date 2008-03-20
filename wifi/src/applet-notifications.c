#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-wifi.h"

CD_APPLET_INCLUDE_MY_VARS

extern AppletConfig myConfig;
extern AppletData myData;


CD_APPLET_ABOUT (D_("This is the wifi applet\n made by ChAnGFu for Cairo-Dock"))


CD_APPLET_ON_CLICK_BEGIN
	cd_wifi_launch_measure();
CD_APPLET_ON_CLICK_END


static void _wifi_recheck_wireless_extension (GtkMenuItem *menu_item, gpointer *data) {
		if (myData.iSidTimer != 0) {
		  g_source_remove (myData.iSidTimer);
		  myData.iSidTimer = 0;
	  }
	  myConfig.iCheckInterval = myConfig.dCheckInterval;
	  cd_wifi_launch_measure();
}

CD_APPLET_ON_BUILD_MENU_BEGIN
	CD_APPLET_ADD_SUB_MENU ("Wifi", pSubMenu, CD_APPLET_MY_MENU)
		if (myData.isWirelessDevice == 0) {
	    CD_APPLET_ADD_IN_MENU (D_("Check for Wireless Extension"), _wifi_recheck_wireless_extension, pSubMenu)
	  }
		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu)
CD_APPLET_ON_BUILD_MENU_END
