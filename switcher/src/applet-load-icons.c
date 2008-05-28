/************************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

************************************************************************************/
#include <string.h>
#include <glib/gstdio.h>
#include <cairo-dock.h>
#include <glib.h>

#include "applet-struct.h"
#include "applet-load-icons.h"
#include "applet-read-data.h"
#include "applet-draw.h"
#include "applet-init.h"

CD_APPLET_INCLUDE_MY_VARS

extern AppletConfig myConfig;
extern AppletData myData;

static GList *s_pIconList = NULL;





static GList * _load_icons (void)
{	
	
	myData.pIconList = NULL;
	Icon *pIcon;	
	int i;

cd_switcher_get_current_desktop (&myData.switcher.ScreenCurrentSizes, &myData.switcher.ScreenCurrentNums);

for (i = 0; i < myData.switcher.iNbViewportX; i ++)
	{
cd_message ("  myData.switcher.iNbViewportX : %d",myData.switcher.iNbViewportX);
	pIcon = g_new0 (Icon, 1);
	if (myData.switcher.ScreenCurrentNums == i)
		{
	pIcon->acName = g_strdup_printf ("Courant %d",i+1);
	pIcon->acFileName = g_strdup_printf ("%s/.cairo-dock/current_theme/plug-ins/switcher/default.png", g_getenv ("HOME"));
	pIcon->cQuickInfo = g_strdup_printf ("%d",i+1);
	pIcon->fScale = 1.;
	pIcon->fAlpha = 1.;
	pIcon->fWidthFactor = 1.;
	pIcon->fHeightFactor = 1.;
	pIcon->acCommand = g_strdup ("none");
	pIcon->cParentDockName = g_strdup (myIcon->acName);
}
else
		{
	pIcon->acName = g_strdup_printf ("Bureau %d",i+1);
	pIcon->acFileName = g_strdup_printf ("%s/.cairo-dock/current_theme/plug-ins/switcher/default.png", g_getenv ("HOME"));
	pIcon->cQuickInfo = g_strdup_printf ("%d",i+1);
	pIcon->fScale = 1.;
	pIcon->fAlpha = 1.;
	pIcon->fWidthFactor = 1.;
	pIcon->fHeightFactor = 1.;
	pIcon->acCommand = g_strdup ("none");
	pIcon->cParentDockName = g_strdup (myIcon->acName);
	}
	myData.pIconList = g_list_append (myData.pIconList, pIcon);
//_add_icon (i);



      }

	
	return myData.pIconList;

}

gboolean cd_switcher_timer (gpointer data)
{
	cd_switcher_launch_measure ();
	return TRUE;
}


static gboolean _cd_switcher_check_for_redraw (gpointer data)
{
	cd_switcher_get_current_desktop (&myData.switcher.ScreenCurrentSizes, &myData.switcher.ScreenCurrentNums);
		//\_______________________ On recharge l'icone principale.
		if (myConfig.bCurrentView)
		{
			
if (myIcon->pSubDock != NULL)
{
g_list_foreach (myIcon->pSubDock->icons, (GFunc) cairo_dock_free_icon, NULL);
			g_list_free (myIcon->pSubDock->icons);
			myIcon->pSubDock->icons = NULL;

cairo_dock_destroy_dock (myIcon->pSubDock, myIcon->acName, NULL, NULL);
					myIcon->pSubDock = NULL;

					}
				
						cd_message ("SWITCHER : chargement de l'icone Switcher sans sous dock");

_cd_switcher_check_for_redraw_cairo(NULL);

}
else
{
	
		//\_______________________ On cree la liste des icones de prevision.
		GList *pIconList = _load_icons ();
		
		//\_______________________ On efface l'ancienne liste.

		if (myIcon->pSubDock != NULL)
		{
			g_list_foreach (myIcon->pSubDock->icons, (GFunc) cairo_dock_free_icon, NULL);
			g_list_free (myIcon->pSubDock->icons);
			myIcon->pSubDock->icons = NULL;
		}
		
		//\_______________________ On charge la nouvelle liste.
		if (myDock != NULL)  
		{
			if (myIcon->pSubDock == NULL)
			{	
				
					if (pIconList != NULL) 
					{
					
					cd_message ("SWITCHER : creation du sous-dock Switcher");
					myIcon->pSubDock = cairo_dock_create_subdock_from_scratch (pIconList, myIcon->acName, myDock);
					cairo_dock_set_renderer (myIcon->pSubDock, myConfig.cRenderer);
					cairo_dock_update_dock_size (myIcon->pSubDock);
					}
				
			}
			else  // on a deja notre sous-dock, on remplace juste ses icones.
			{
				cd_message ("SWITCHER : rechargement du sous-dock Switcher");
				if (pIconList == NULL)  // inutile de le garder.
				{
					cairo_dock_destroy_dock (myIcon->pSubDock, myIcon->acName, NULL, NULL);
					myIcon->pSubDock = NULL;
				}
				else
				{
					myIcon->pSubDock->icons = pIconList;
					cairo_dock_load_buffers_in_one_dock (myIcon->pSubDock);
					cairo_dock_update_dock_size (myIcon->pSubDock);
				}
			
			}
		}
		else
		{
			if (myIcon->pSubDock != NULL)

			{
				cairo_dock_destroy_dock (myIcon->pSubDock, myIcon->acName, NULL, NULL);
				myIcon->pSubDock = NULL;
			}
			myData.pDeskletIconList = pIconList;
			myDesklet->icons = pIconList;
			myData.iNbIcons = g_list_length (myData.pDeskletIconList);
			GList* ic;
			Icon *icon;
			cairo_t *pCairoContext = cairo_dock_create_context_from_window (myContainer);
			for (ic = pIconList; ic != NULL; ic = ic->next)
			{
				icon = ic->data;
				if (myConfig.bDesklet3D)
				{
					icon->fWidth = 0;
					icon->fHeight = 0;
				}
				else
				{
					icon->fWidth = MAX (1, .2 * myDesklet->iWidth - g_iconTextDescription.iSize);
					icon->fHeight = MAX (1, .2 * myDesklet->iHeight - g_iconTextDescription.iSize);
				}
				cairo_dock_fill_icon_buffers (icon, pCairoContext, 1, myContainer->bIsHorizontal, myConfig.bDesklet3D, myContainer->bDirectionUp);
				myData.iMaxIconWidth = MAX (myData.iMaxIconWidth, icon->fWidth);
				}

			}

	if (myData.LoadAfterCompiz != 0)//\_______________________ On Tue le Timer.
{
cd_message ("SWITCHER : timer à 0 ");
		g_source_remove (myData.LoadAfterCompiz);
	myData.LoadAfterCompiz = 0;
}
	//}
}
if (myConfig.bDisplayNumDesk)
			{

				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%d", myData.switcher.ScreenCurrentNums+1)
	cd_message ("SWITCHER : chargement de quick info %d ", myData.switcher.ScreenCurrentNums+1);

			}
			else
			{

cd_message ("SWITCHER : chargement de quick info NULL");
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF (NULL)

	
			}

CD_APPLET_REDRAW_MY_ICON

}

void cd_switcher_launch_measure (void)
{
//cd_switcher_grab_and_draw_icon();
_cd_switcher_check_for_redraw (NULL);
	
}
