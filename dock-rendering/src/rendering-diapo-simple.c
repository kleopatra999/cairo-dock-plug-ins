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

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include <cairo.h>

#include "rendering-diapo-simple.h"

extern gdouble  my_diapo_simple_max_size;
extern gint     my_diapo_simple_iconGapX;
extern gint     my_diapo_simple_iconGapY;
extern gdouble  my_diapo_simple_fScaleMax;
extern gint     my_diapo_simple_sinW;
extern gboolean my_diapo_simple_lineaire;
extern gboolean  my_diapo_simple_wide_grid;

extern gdouble  my_diapo_simple_color_frame_start[4];
extern gdouble  my_diapo_simple_color_frame_stop[4];
extern gboolean my_diapo_simple_fade2bottom;
extern gboolean my_diapo_simple_fade2right;
extern gint    my_diapo_simple_arrowWidth;
extern gint    my_diapo_simple_arrowHeight;
//extern gdouble  my_diapo_simple_arrowShift;
extern gint    my_diapo_simple_lineWidth;
extern gint    my_diapo_simple_radius;
extern gdouble  my_diapo_simple_color_border_line[4];
extern gboolean my_diapo_simple_draw_background;
extern gboolean my_diapo_simple_display_all_labels;

extern gdouble my_diapo_simple_color_scrollbar_line[4];
extern gdouble my_diapo_simple_color_scrollbar_inside[4];
extern gdouble my_diapo_simple_color_grip[4];

const gint iSeparatorHeight = 30;  // height added by a separator.
const gint X_BORDER_SPACE = 40;  // espace laisse de chaque cote pour eviter de sortir trop facilement (et pour laisser de la place pour les etiquettes).
const gint ARROW_TIP = 5;  // pour gerer la pointe de la fleche.
const double fArrowHeight = 14, fScrollbarWidth = 10, fScrollbarArrowGap = 4, fScrollbarRadius = 5;
const double fScrollbarIconGap = 10;
/// On considere qu'on a my_diapo_simple_iconGapX entre chaque icone horizontalement, et my_diapo_simple_iconGapX/2 entre les icones et les bords (pour aerer un peu plus le dessin). Idem verticalement. X_BORDER_SPACE est la pour empecher que les icones debordent de la fenetre au zoom.

typedef struct {
	gint nRowsX;
	gint nRowsY;
	gint iNbSeparators;  // number of drawn separators.
	gint iDeltaHeight;  // hauteur scrollable, en pixels
	gint iScrollOffset;  // hauteur scrollee, en pixels, positive.
	gboolean bDraggingScrollbar;  // si le clic est couramment enfonce sur la scrollbar.
	guint iSidPressEvent;  // sid du clic
	guint iSidReleaseEvent;  // sid du relachement du clic
	gint iClickY;  // hauteur ou on a clique, en coordonnees container.
	gint iClickOffset;  // hauteur scrollee au moment du clic
	gint iDeltaIconX;
	gint iArrowShift;
	} CDSlideData;

static void cd_rendering_render_diapo_simple (cairo_t *pCairoContext, CairoDock *pDock);


  ////////////////////////
 /// SCROLL CALLBACKS ///
////////////////////////

static void _set_scroll (CairoDock *pDock, int iOffsetY)
{
	//g_print ("%s (%d)\n", __func__, iOffsetY);
	CDSlideData *pData = pDock->pRendererData;
	g_return_if_fail (pData != NULL);
	
	pData->iScrollOffset = MAX (0, MIN (iOffsetY, pData->iDeltaHeight));
	cairo_dock_calculate_dock_icons (pDock);
	gtk_widget_queue_draw (pDock->container.pWidget);
}
static gboolean _add_scroll (CairoDock *pDock, int iDeltaOffsetY)
{
	//g_print ("%s (%d)\n", __func__, iDeltaOffsetY);
	CDSlideData *pData = pDock->pRendererData;
	g_return_val_if_fail (pData != NULL, FALSE);
	
	if (iDeltaOffsetY < 0)
	{
		if (pData->iScrollOffset <= 0)
			return FALSE;
	}
	else
	{
		if (pData->iScrollOffset >= pData->iDeltaHeight)
			return FALSE;
	}
	_set_scroll (pDock, pData->iScrollOffset + iDeltaOffsetY);
	return TRUE;
}

static gboolean _cd_slide_on_scroll (gpointer data, Icon *pClickedIcon, CairoDock *pDock, int iDirection)
{
	CDSlideData *pData = pDock->pRendererData;
	g_return_val_if_fail (pData != NULL, CAIRO_DOCK_LET_PASS_NOTIFICATION);
	if (pData->iDeltaHeight == 0)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	gboolean bScrolled = _add_scroll (pDock, iDirection == 1 ? pDock->iMaxIconHeight : - pDock->iMaxIconHeight);
	return (bScrolled ? CAIRO_DOCK_INTERCEPT_NOTIFICATION : CAIRO_DOCK_LET_PASS_NOTIFICATION);
}
static gboolean _cd_slide_on_click (gpointer data, Icon *pClickedIcon, CairoDock *pDock, guint iButtonState)
{
	CDSlideData *pData = pDock->pRendererData;
	g_return_val_if_fail (pData != NULL, CAIRO_DOCK_LET_PASS_NOTIFICATION);
	if (pData->iDeltaHeight == 0)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	if (pData->bDraggingScrollbar)
		return CAIRO_DOCK_INTERCEPT_NOTIFICATION;
	else
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}
gboolean _cd_slide_on_press_button (GtkWidget* pWidget, GdkEventButton* pButton, CairoDock *pDock)
{
	CDSlideData *pData = pDock->pRendererData;
	g_return_val_if_fail (pData != NULL, FALSE);
	if (pData->iDeltaHeight == 0)
		return FALSE;
	
	if (pButton->type == GDK_BUTTON_PRESS && pButton->button == 1)
	{
		double x_arrow = pDock->iMaxDockWidth - X_BORDER_SPACE - fScrollbarWidth;
		int x, y;
		if (pDock->container.bIsHorizontal)
		{
			x = pButton->x;
			y = pButton->y;
		}
		else
		{
			x = pButton->y;
			y = pButton->x;
		}
		if (x > x_arrow)  // on a clique dans la zone de scroll.
		{
			//g_print ("click (y=%d, scroll=%d)\n", (int) y, pData->iScrollOffset);
			
			// on regarde sur quoi on clic.
			double y_arrow_top, y_arrow_bottom;
			if (pDock->container.bDirectionUp)
			{
				y_arrow_bottom = pDock->iMaxDockHeight - (my_diapo_simple_arrowHeight + ARROW_TIP + my_diapo_simple_lineWidth + my_diapo_simple_radius);
				y_arrow_top = my_diapo_simple_lineWidth + my_diapo_simple_radius;
			}
			else
			{
				y_arrow_bottom = pDock->iMaxDockHeight - my_diapo_simple_lineWidth - my_diapo_simple_radius;
				y_arrow_top = my_diapo_simple_arrowHeight + ARROW_TIP + my_diapo_simple_lineWidth + my_diapo_simple_radius;
			}
			if (y > y_arrow_top - fScrollbarArrowGap/2 && y < y_arrow_top + fArrowHeight + fScrollbarArrowGap/2)  // bouton haut
			{
				_set_scroll (pDock, 0);
			}
			else if (y < y_arrow_bottom + fScrollbarArrowGap/2 && y > y_arrow_bottom - fArrowHeight - fScrollbarArrowGap/2)  // bouton bas
			{
				_set_scroll (pDock, pData->iDeltaHeight);
			}
			else  // scrollbar
			{
				pData->bDraggingScrollbar = TRUE;
				pData->iClickY = y;
				pData->iClickOffset = pData->iScrollOffset;
			}
		}
	}
	else if (GDK_BUTTON_RELEASE)
	{
		//g_print ("release\n");
		pData->bDraggingScrollbar = FALSE;
	}
	return FALSE;
}
static gboolean _cd_slide_on_mouse_moved (gpointer data, CairoDock *pDock, gboolean *bStartAnimation)
{
	CDSlideData *pData = pDock->pRendererData;
	g_return_val_if_fail (pData != NULL, CAIRO_DOCK_LET_PASS_NOTIFICATION);
	if (pData->iDeltaHeight == 0)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	if (pData->bDraggingScrollbar)
	{
		//g_print ("scroll on motion (y=%d)\n", pDock->container.iMouseY);
		
		double y_arrow_top, y_arrow_bottom;
		if (pDock->container.bDirectionUp)
		{
			y_arrow_bottom = my_diapo_simple_arrowHeight + ARROW_TIP + my_diapo_simple_lineWidth + my_diapo_simple_radius;
			y_arrow_top = pDock->iMaxDockHeight - my_diapo_simple_lineWidth - my_diapo_simple_radius;
		}
		else
		{
			y_arrow_top = pDock->iMaxDockHeight - (my_diapo_simple_arrowHeight + ARROW_TIP + my_diapo_simple_lineWidth + my_diapo_simple_radius);
			y_arrow_bottom = my_diapo_simple_lineWidth + my_diapo_simple_radius;
		}
		double fFrameHeight = pDock->iMaxDockHeight- (my_diapo_simple_arrowHeight + ARROW_TIP + my_diapo_simple_lineWidth);  // hauteur du cadre avec les rayons et sans la pointe.
		double fGripHeight = fFrameHeight / (fFrameHeight + pData->iDeltaHeight) * (y_arrow_top - y_arrow_bottom - 2*(fArrowHeight+fScrollbarArrowGap));
		double ygrip = (double) pData->iScrollOffset / pData->iDeltaHeight * (y_arrow_top - y_arrow_bottom - 2*(fArrowHeight+fScrollbarArrowGap) - fGripHeight);
		
		int delta = pDock->container.iMouseY - pData->iClickY;
		_set_scroll (pDock, (pData->iClickOffset + (double)delta / (y_arrow_top - y_arrow_bottom - 2*(fArrowHeight+fScrollbarArrowGap) - fGripHeight) * pData->iDeltaHeight));
		return CAIRO_DOCK_INTERCEPT_NOTIFICATION;
	}
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}
gboolean cd_slide_on_leave (gpointer data, CairoDock *pDock, gboolean *bStartAnimation)
{
	CDSlideData *pData = pDock->pRendererData;
	//g_return_val_if_fail (pData != NULL, CAIRO_DOCK_LET_PASS_NOTIFICATION);
	if (pData == NULL || ! pDock->pRenderer || pDock->pRenderer->render != cd_rendering_render_diapo_simple)  // pas nous
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	//g_print (" LEAVE (%d)\n", pData->bDraggingScrollbar);
	
	return (pData->bDraggingScrollbar ? CAIRO_DOCK_INTERCEPT_NOTIFICATION : CAIRO_DOCK_LET_PASS_NOTIFICATION);
}


  ////////////////////////
 /// GRID COMPUTATION ///
////////////////////////

static guint _cd_rendering_diapo_simple_guess_grid (GList *pIconList, guint *nRowX, guint *nRowY, guint *nSep)
{
	// first count the number of icons and separators
	int nc=0, nl=0;  // nb columns and lines
	int ncmax=0, ncmin=999999;  // min/max number of columns = smallest/largest line
	
	guint count = 0;  // number of drawn icons (= not a separator)
	guint nb_sep = 0;  // number of separators.
	Icon *icon;
	GList *ic;
	for (ic = pIconList; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		if (CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (icon))
		{
			nb_sep ++;
			ncmax = MAX (ncmax, nc);
			ncmin = MIN (ncmin, nc);
			nc = 0;  // new line
		}
		else
		{
			count ++;
			nc ++;
		}
	}
	ncmax = MAX (ncmax, nc);
	ncmin = MIN (ncmin, nc);
	if (nc == 0 && nl > 0)  // if we ended on an empty line, ignore it.
	{
		nb_sep --;
	}
	//g_print (" %d icons, %d separator(s), min/max line=%d/%d\n", count, nb_sep, ncmin, ncmax);
	
	// then compute the best layout
	if (count == 0)  // if no icons, stop here.
	{
		*nRowX = 0;
		*nRowY = 0;
		*nSep = 0;
		return 0;
	}
	else if (nb_sep == 0)  // if no separator, the grid is obvious.
	{
		if (my_diapo_simple_wide_grid)
		{
			*nRowX = ceil(sqrt(count));
			*nRowY = ceil(((double) count) / *nRowX);
		}
		else
		{
			*nRowY = ceil(sqrt(count));
			*nRowX = ceil(((double) count) / *nRowY);
		}
	}
	else  // iterate on the possible layouts.
	{
		int iSurface, iSurfaceMin = count * count;
		int delta_lc, delta_lc_min = count;  // delta line/column
		int nx=0, ny=0;  // result
		int nclim, ncols=0;
		for (nclim = MIN (ncmin, floor(sqrt(count))); nclim <= ncmax; nclim ++)  // if no separator, we'll have sqrt(count) columns; since each separator adds a line, and we want as many columns as lines, we can iterate from this value.
		{
			nl = 1;
			nc = 0;
			ncols = 0;
			for (ic = pIconList; ic != NULL; ic = ic->next)
			{
				icon = ic->data;
				if (CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (icon))  // new line
				{
					if (nc > 0)
					{
						ncols = MAX (ncols, nc);
						nc = 0;
						nl ++;
					}
				}
				else
				{
					nc ++;
					if (nc == nclim)  // end of line reached, start a new line.
					{
						ncols = MAX (ncols, nc);
						nc = 0;
						nl ++;
					}
				}
			}
			if (nc == 0 && nl > 0)  // if we ended on an empty line, ignore it.
			{
				nl --;
			}

			if (my_diapo_simple_wide_grid && ncols >= nl
			|| ! my_diapo_simple_wide_grid && ncols <= nl)
			{
				delta_lc = abs (ncols - nl);
				iSurface = ncols * nl;
				if (delta_lc < delta_lc_min
				|| (delta_lc == delta_lc_min && iSurface < iSurfaceMin))  // we want to have something roughly square, and which minimizes the lost space.
				{
					delta_lc_min = delta_lc;
					iSurfaceMin = iSurface;
					nx = ncols;
					ny = nl;
				}
			}
		}
		*nRowX = nx;
		*nRowY = ny;
	}
	
	/**
	// Calcul du nombre de lignes (nY) / colonnes (nX) :
	guint count = 0;
	Icon *icon;
	GList *ic;
	for (ic = pIconList; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		if (! CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (icon))
			count ++;
	}
	
	if (count == 0)
	{
		*nRowX = 0;
		*nRowY = 0;
	}
	else if (my_diapo_simple_wide_grid)
	{
		*nRowX = ceil(sqrt(count));
		*nRowY = ceil(((double) count) / *nRowX);
	}
	else
	{
		*nRowY = ceil(sqrt(count));
		*nRowX = ceil(((double) count) / *nRowY);
	}*/
	*nSep = nb_sep;
	return count;
}

static void cd_rendering_calculate_max_dock_size_diapo_simple (CairoDock *pDock)
{
	// On calcule la configuration de la grille sans contrainte.
	double srx = my_diapo_simple_max_size;  // screen ratio hori
	double sry = MIN (1., my_diapo_simple_max_size * 1.2);  // screen ratio verti
	guint nRowsX = 0;  // nb colonnes.
	guint nRowsY = 0;  // nb lignes.
	guint nIcones = 0;  // nb icones.
	guint iNbSeparators = 0;  // nb separators
	int iDeltaHeight = 0;  // hauteur ne pouvant rentrer dans le dock.
	int iMaxIconWidth = 0;
	int iDockWidth, iDockHeight;  // dimension dock.
	int Ws = g_desktopGeometry.iXScreenWidth[CAIRO_DOCK_HORIZONTAL];  // dimensions ecran.
	int Hs = g_desktopGeometry.iXScreenHeight[CAIRO_DOCK_HORIZONTAL];
	nIcones = _cd_rendering_diapo_simple_guess_grid (pDock->icons, &nRowsX, &nRowsY, &iNbSeparators);
	//g_print ("nIcones : %d\n", nIcones);
	
	if (! pDock->container.bIsHorizontal)
	{
		int tmp = nRowsX;
		nRowsX = nRowsY;
		nRowsY = tmp;
	}
	// On calcule la taille de l'affichage avec contrainte taille ecran.
	if (nIcones != 0)
	{
		// on calcule la largeur avec contrainte, ce qui donne aussi le nombre de lignes.
		iMaxIconWidth = ((Icon*)pDock->icons->data)->fWidth;  // approximation un peu bof.
		iDockWidth = nRowsX * (iMaxIconWidth + my_diapo_simple_iconGapX) + 2*X_BORDER_SPACE;
		int iMaxWidth = Ws * (Ws > Hs ? srx : sry);
		if (iDockWidth > iMaxWidth)
		{
			nRowsX = (iMaxWidth - 2*X_BORDER_SPACE) / (iMaxIconWidth + my_diapo_simple_iconGapX);
			nRowsY = ceil((double) nIcones / nRowsX);
			iDockWidth = nRowsX * (iMaxIconWidth + my_diapo_simple_iconGapX) + 2*X_BORDER_SPACE;
			//g_print ("%d -> %d\n", iMaxWidth, iDockWidth);
		}
		
		// on calcule la hauteur avec contrainte, ce qui donne aussi la marge de defilement.
		int iSingleLineHeight = pDock->iMaxIconHeight * pDock->container.fRatio * my_diapo_simple_fScaleMax +  // les icones des bords zooment
			myIconsParam.iLabelSize +  // le texte des icones de la 1ere ligne
			my_diapo_simple_lineWidth + // les demi-lignes du haut et du bas
			my_diapo_simple_arrowHeight + ARROW_TIP +  // la fleche etendue
			iNbSeparators * iSeparatorHeight;  // les separateurs
		int iOneLineHeight = pDock->iMaxIconHeight * pDock->container.fRatio + my_diapo_simple_iconGapY;
		
		iDockHeight = (nRowsY - 1) * iOneLineHeight + iSingleLineHeight;
		int iMaxHeight = Hs * (Ws > Hs ? sry : srx);
		if (iDockHeight > iMaxHeight)
		{
			nRowsY = (iMaxHeight - iSingleLineHeight) / iOneLineHeight + 1;
			if (Ws > Hs && nRowsY > nRowsX)  // on evite d'avoir un sous-dock plus haut que large si l'ecran est ausi comme ca, ca rend mieux.
				nRowsY = MIN (nRowsY, MAX (5, nRowsX));  // tout de meme, au moins 5 lignes.
			int iMaxDockHeight0 = iDockHeight;
			iDockHeight = (nRowsY - 1) * iOneLineHeight + iSingleLineHeight;
			iDeltaHeight = iMaxDockHeight0 - iDockHeight;
			//g_print ("%d -> %d\n", iMaxHeight, iDockHeight);
		}
	}
	else
	{
		iDockWidth = X_BORDER_SPACE * 2 + my_diapo_simple_arrowWidth + 10;
		iDockHeight = my_diapo_simple_lineWidth + my_diapo_simple_arrowHeight + ARROW_TIP + 10;
	}
	if (! pDock->container.bIsHorizontal)
	{
		int tmp = nRowsX;
		nRowsX = nRowsY;
		nRowsY = tmp;
	}
	
	CDSlideData *pData = pDock->pRendererData;
	if (pData == NULL)
	{
		pData = g_new0 (CDSlideData, 1);
		pDock->pRendererData = pData;
		cairo_dock_register_notification_on_object (CAIRO_CONTAINER (pDock), NOTIFICATION_SCROLL_ICON, (CairoDockNotificationFunc) _cd_slide_on_scroll, CAIRO_DOCK_RUN_AFTER, NULL);
		cairo_dock_register_notification_on_object (CAIRO_CONTAINER (pDock), NOTIFICATION_CLICK_ICON, (CairoDockNotificationFunc) _cd_slide_on_click, CAIRO_DOCK_RUN_FIRST, NULL);
		cairo_dock_register_notification_on_object (CAIRO_CONTAINER (pDock), NOTIFICATION_MOUSE_MOVED, (CairoDockNotificationFunc) _cd_slide_on_mouse_moved, CAIRO_DOCK_RUN_AFTER, NULL);
		pData->iSidPressEvent = g_signal_connect (G_OBJECT (pDock->container.pWidget),
			"button-press-event",
			G_CALLBACK (_cd_slide_on_press_button),
			pDock);  // car les notification de clic en provenance du dock sont emises lors du relachement du bouton.
		pData->iSidReleaseEvent = g_signal_connect (G_OBJECT (pDock->container.pWidget),
			"button-release-event",
			G_CALLBACK (_cd_slide_on_press_button),
			pDock);
	}
	pData->nRowsX = nRowsX;
	pData->nRowsY = nRowsY;
	pData->iNbSeparators = iNbSeparators;
	pData->iDeltaHeight = iDeltaHeight;
	if (iDeltaHeight != 0)
	{
		int iScrollMargin = iMaxIconWidth * (my_diapo_simple_fScaleMax - 1) / 2
			+ fScrollbarIconGap
			+ fScrollbarWidth;  // donc a droite on a : derniere icone en taille max + demi-gapx + gap + scrollbar + X_BORDER_SPACE
		iDockWidth += iScrollMargin;
	}
	
	// taille du dock
	//if (pDock->container.bIsHorizontal)
	{
		pDock->iMaxDockWidth = iDockWidth;
		pDock->iMaxDockHeight = iDockHeight;
	}
	/*else  // pareil, sauf la fleche qui sera sur les cotes.
	{
		pDock->iMaxDockWidth = iDockHeight - (my_diapo_simple_arrowHeight + ARROW_TIP);
		pDock->iMaxDockHeight = iDockWidth + my_diapo_simple_arrowHeight + ARROW_TIP;
	}*/
	pDock->iMinDockWidth = pDock->iMaxDockWidth - 2*X_BORDER_SPACE;
	pDock->iMinDockHeight = pDock->iMaxDockHeight;
	// pas de decorations.
	pDock->iDecorationsHeight = 0;
	pDock->iDecorationsWidth  = 0;
	// On affecte ca aussi au cas ou.
	pDock->fFlatDockWidth = pDock->iMaxDockWidth;
	pDock->fMagnitudeMax = my_diapo_simple_fScaleMax / (1+myIconsParam.fAmplitude);
	
	pDock->iActiveWidth = pDock->iMaxDockWidth;
	pDock->iActiveHeight = pDock->iMaxDockHeight;
}


  /////////////////////////
 /// ICONS COMPUTATION ///
/////////////////////////

static Icon* _cd_rendering_calculate_icons_for_diapo_simple (CairoDock *pDock, gint nRowsX, gint nRowsY, gint Mx, gint My)
{
	CDSlideData *pData = pDock->pRendererData;  // non nul
	double fScrollOffset = (pDock->container.bDirectionUp ? - pData->iScrollOffset : pData->iScrollOffset);
	gboolean bIsHorizontal = pDock->container.bIsHorizontal;
	
	// On calcule la position de base pour toutes les icones
	int iOneLineHeight = pDock->iMaxIconHeight * pDock->container.fRatio + my_diapo_simple_iconGapY;
	int iOffsetY;
	if (pDock->container.bDirectionUp)
		iOffsetY = .5 * pDock->iMaxIconHeight * pDock->container.fRatio * (my_diapo_simple_fScaleMax - 1) +  // les icones de la 1ere ligne zooment
			(bIsHorizontal ? myIconsParam.iLabelSize : .5*myIconsParam.iLabelSize) +  // le texte des icones de la 1ere ligne
			.5 * my_diapo_simple_lineWidth +  // demi-ligne du haut;
			fScrollOffset;
	else
		iOffsetY = .5 * pDock->iMaxIconHeight * pDock->container.fRatio * (my_diapo_simple_fScaleMax - 1) +  // les icones de la 1ere ligne zooment
				.5 * my_diapo_simple_lineWidth +  // demi-ligne du bas;
				(bIsHorizontal ? 0 : myIconsParam.iLabelSize/2) +
				fScrollOffset;
	double fFoldingX = (pDock->fFoldingFactor > .2 ? (pDock->fFoldingFactor - .2) / .8 : 0.);  // placement de 1 a 0.2
	double fFoldingY = (pDock->fFoldingFactor > .5 ? (pDock->fFoldingFactor - .5) / .5 : 0.);  // placement de 1 a 0.5
	double fFoldingScale = (pDock->fFoldingFactor > .7 ? (1 - pDock->fFoldingFactor) / .3 : 1.);
	Icon* icon;
	GList* ic, *pointed_ic=NULL;
	int x=0, y=0;  // index of the current icon on the grid.
	int X, Y;
	double sep_offset[2] = {0, 0};
	if (! pDock->container.bDirectionUp)
	{
		sep_offset[!bIsHorizontal] = - iSeparatorHeight * pData->iNbSeparators;
	}
	for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		if (CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (icon))
		{
			if (x > 0)
			{
				x = 0;  // go to next line.
				y ++;
			}
			
			icon->fWidthFactor = ((pDock->container.bDirectionUp?pDock->container.iWidth - 2*X_BORDER_SPACE - my_diapo_simple_iconGapX:pDock->container.iHeight - my_diapo_simple_arrowHeight - ARROW_TIP)) * (1 - fFoldingX);  // we'll use it as the width of the line.
			
			icon->fX = ((pDock->container.bIsHorizontal?pDock->container.iWidth:pDock->container.iHeight) - icon->fWidthFactor) / 2 + sep_offset[CAIRO_DOCK_HORIZONTAL];
			if (pDock->container.bDirectionUp)
				icon->fY = iOffsetY + (pDock->iMaxIconHeight + my_diapo_simple_iconGapY) * y + iSeparatorHeight + sep_offset[CAIRO_DOCK_VERTICAL]  // top of the icon on the next line.
				-  myIconsParam.iLabelSize - (my_diapo_simple_iconGapY + iSeparatorHeight - myIconsParam.iLabelSize)/2 - iSeparatorHeight/2;  // place the separator between the top of the label of the next line, and the bottom of the icon on the previous line.
			else
			{
				icon->fY = (pDock->container.bIsHorizontal?pDock->container.iHeight:pDock->container.iWidth) - iOffsetY - (nRowsY - y) * (pDock->iMaxIconHeight + my_diapo_simple_iconGapY) + sep_offset[CAIRO_DOCK_VERTICAL]
				+ (my_diapo_simple_iconGapY + iSeparatorHeight - myIconsParam.iLabelSize)/2 - iSeparatorHeight/2;
			}
			
			// on tient compte du depliage.
			icon->fDrawX = icon->fX;
			icon->fDrawY = icon->fY + (pDock->container.bDirectionUp ?
				pDock->container.iHeight - (my_diapo_simple_arrowHeight + ARROW_TIP + icon->fDrawY) :
				- icon->fDrawY) * fFoldingY;
			
			icon->fScale = 1.;
			
			sep_offset[!bIsHorizontal] += iSeparatorHeight;
		}
		else
		{
			if (bIsHorizontal)
			{
				X = x;
				Y = y;
			}
			else
			{
				X = y;
				Y = x;
			}
			// on en deduit la position au repos.
			icon->fX = X_BORDER_SPACE + .5*my_diapo_simple_iconGapX + (icon->fWidth + my_diapo_simple_iconGapX) * X + sep_offset[CAIRO_DOCK_HORIZONTAL];
			if (pDock->container.bDirectionUp)
				icon->fY = iOffsetY + (pDock->iMaxIconHeight + my_diapo_simple_iconGapY) * Y + sep_offset[CAIRO_DOCK_VERTICAL];
			else
			{
				icon->fY = pDock->container.iHeight - iOffsetY - icon->fHeight - (nRowsY - 1 - Y) * (pDock->iMaxIconHeight + my_diapo_simple_iconGapY) + sep_offset[CAIRO_DOCK_VERTICAL];
			}

			// on en deduit le zoom par rapport a la position de la souris.
			gdouble distanceE = sqrt ((icon->fX + icon->fWidth/2 - Mx) * (icon->fX + icon->fWidth/2 - Mx) + (icon->fY + icon->fHeight/2 - My) * (icon->fY + icon->fHeight/2 - My));
			if (my_diapo_simple_lineaire)
			{
				gdouble eloignementMax = my_diapo_simple_sinW;  // 3. * (icon->fWidth + icon->fHeight)  / 2
				icon->fScale = MAX (1., my_diapo_simple_fScaleMax + (1. - my_diapo_simple_fScaleMax) * distanceE / eloignementMax);
				icon->fPhase = 0.;
			}
			else
			{
				icon->fPhase = distanceE * G_PI / my_diapo_simple_sinW + G_PI / 2.;
				if (icon->fPhase < 0)
					icon->fPhase = 0;
				else if (icon->fPhase > G_PI)
					icon->fPhase = G_PI;
				icon->fScale = 1. + (my_diapo_simple_fScaleMax - 1.) * sin (icon->fPhase);
			}

			// on tient compte du zoom (zoom centre).
			icon->fXMin = icon->fXMax = icon->fXAtRest =  // Ca on s'en sert pas encore
			icon->fDrawX = icon->fX + icon->fWidth  * (1. - icon->fScale) / 2;
			icon->fDrawY = icon->fY + icon->fHeight * (1. - icon->fScale) / 2;

			// on tient compte du depliage.
			icon->fDrawX -= (icon->fDrawX - pDock->container.iWidth/2) * fFoldingX;
			icon->fDrawY = icon->fDrawY + (pDock->container.bDirectionUp ?
				pDock->container.iHeight - (my_diapo_simple_arrowHeight + ARROW_TIP + icon->fDrawY) :
				- icon->fDrawY) * fFoldingY;

			// On affecte tous les parametres qui n'ont pas été défini précédement
			icon->fPhase = 0.;
			icon->fOrientation = 0.;  // 2. * G_PI * pDock->fFoldingFactor;
			icon->fWidthFactor = icon->fHeightFactor = fFoldingScale;

			x ++;
			if (x >= nRowsX)
			{
				x = 0;
				y ++;
			}
		}
		icon->fAlpha = (pDock->fFoldingFactor > .7 ? (1 - pDock->fFoldingFactor) / (1 - .7) : 1.);  // apparition de 1 a 0.7
		
		///g_print (" +%s %d;%d\n", icon->cName, x, y);
		// On regarde si l'icone est pointee.
		if((Mx > icon->fX - .5*my_diapo_simple_iconGapX) && 
			(My > icon->fY - .5*my_diapo_simple_iconGapY) &&
			(Mx < icon->fX + icon->fWidth  + .5*my_diapo_simple_iconGapX) &&
			(My < icon->fY + icon->fHeight + .5*my_diapo_simple_iconGapY))
		{
			icon->bPointed = TRUE;
			pointed_ic = ic;
		}
		else
		{
			icon->bPointed = FALSE;
		}
	}
	return (pointed_ic == NULL ? NULL : pointed_ic->data);
}
static void _cd_rendering_check_if_mouse_inside_diapo_simple (CairoDock *pDock)
{
	if ((pDock->container.iMouseX < 0) || (pDock->container.iMouseX > pDock->iMaxDockWidth - 1) || (pDock->container.iMouseY < 0) || (pDock->container.iMouseY > pDock->iMaxDockHeight - 0))
	{
		pDock->iMousePositionType = CAIRO_DOCK_MOUSE_OUTSIDE;
	}
	else  // on fait simple.
	{
		pDock->iMousePositionType = CAIRO_DOCK_MOUSE_INSIDE;
	}
}
#define make_icon_avoid_mouse(icon, sens) do { \
	cairo_dock_mark_icon_as_avoiding_mouse (icon);\
	icon->fAlpha = 0.75;\
	if (myIconsParam.fAmplitude != 0)\
		icon->fDrawX += icon->fWidth / 2 * (icon->fScale - 1) / myIconsParam.fAmplitude * sens; } while (0)
///TODO: make it work...
static inline gboolean _check_can_drop (CairoDock *pDock, CairoDockIconGroup iGroup, double fMargin)
{
	gboolean bUndefined = TRUE;
	gboolean bCanDrop = FALSE;
	Icon *icon;
	GList *ic;
	for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		if (icon->bPointed)
		{
			if (pDock->container.iMouseX < icon->fDrawX + icon->fWidth * icon->fScale * fMargin)  // on est a gauche.  // fDrawXAtRest
			{
				GList *prev_ic = ic->prev;
				Icon *prev_icon = (prev_ic ? prev_ic->data : NULL);
				if (icon->iGroup == iGroup
				|| (prev_icon && prev_icon->iGroup == iGroup) )
				{
					make_icon_avoid_mouse (icon, 1);
					if (prev_icon)
						make_icon_avoid_mouse (prev_icon, -1);
					//g_print ("%s> <%s\n", prev_icon->cName, icon->cName);
					bCanDrop = TRUE;
					bUndefined = FALSE;
				}
			}
			else if (pDock->container.iMouseX > icon->fDrawX + icon->fWidth * icon->fScale * (1 - fMargin))  // on est a droite.  // fDrawXAtRest
			{
				GList *next_ic = ic->next;
				Icon *next_icon = (next_ic ? next_ic->data : NULL);
				if (icon->iGroup == iGroup
				|| (next_icon && next_icon->iGroup == iGroup) )
				{
					make_icon_avoid_mouse (icon, -1);
					if (next_icon)
						make_icon_avoid_mouse (next_icon, 1);
					bCanDrop = TRUE;
					bUndefined = FALSE;
				}
				ic = ic->next;  // on la saute pour ne pas la de-marquer.
				if (ic == NULL)
					break;
			}
			else  // on the icon
			{
				bCanDrop = FALSE;
				bUndefined = FALSE;
			}
		}
		else
			cairo_dock_stop_marking_icon_as_avoiding_mouse (icon);
	}
	
	if (bUndefined)  // no pointed icon, so we are either on the side, on between icons.
	{
		bCanDrop = (pDock->container.iMouseX > X_BORDER_SPACE && pDock->container.iMouseX < pDock->container.iWidth - X_BORDER_SPACE);  // no drop on the side.
	}
	return bCanDrop;
}
static void _cd_rendering_check_can_drop (CairoDock *pDock)
{
	if (! pDock->bIsDragging)  // not dragging, so no drop possible.
	{
		pDock->bCanDrop = FALSE;
	}
	else if (pDock->icons == NULL)  // dragging but no icons, so drop always possible.
	{
		pDock->bCanDrop = TRUE;
	}
	else  // dragging and some icons.
	{
		pDock->bCanDrop = _check_can_drop (pDock, pDock->iAvoidingMouseIconType, pDock->fAvoidingMouseMargin);
	}
}
Icon *cd_rendering_calculate_icons_diapo_simple (CairoDock *pDock)
{
	if (pDock->icons == NULL)
		return NULL;
	CDSlideData *pData = pDock->pRendererData;
	g_return_val_if_fail (pData != NULL, NULL);
	
	// On recupere la configuration de la grille
	gint nRowsX = pData->nRowsX;
	gint nRowsY = pData->nRowsY;
	if (nRowsX == 0 || nRowsY == 0)  // maybe a dock with no displayed icons (like 1 separator); anyway the grid is not valid, so discard.
		return NULL;
	
	// On calcule les parametres des icones
	Icon *pPointedIcon = _cd_rendering_calculate_icons_for_diapo_simple (pDock, nRowsX, nRowsY, pDock->container.iMouseX, pDock->container.iMouseY);
	
	_cd_rendering_check_if_mouse_inside_diapo_simple (pDock);
	
	_cd_rendering_check_can_drop (pDock);
	
	return pPointedIcon;
}


  /////////////////////
 /// CAIRO DRAWING ///
/////////////////////

static void _cairo_dock_draw_frame_horizontal_for_diapo_simple (cairo_t *pCairoContext, CairoDock *pDock)
{
	CDSlideData *pData = pDock->pRendererData;  // non nul
	int iArrowShift = pData->iArrowShift;
	int iDeltaIconX = pData->iDeltaIconX;
	
	gdouble fFrameWidth  = pDock->iMaxDockWidth - 2 * X_BORDER_SPACE;
	gdouble fFrameHeight = pDock->iMaxDockHeight - (my_diapo_simple_arrowHeight + ARROW_TIP + my_diapo_simple_lineWidth);
	gdouble fDockOffsetX = X_BORDER_SPACE;
	gdouble fDockOffsetY = (pDock->container.bDirectionUp ? .5*my_diapo_simple_lineWidth : my_diapo_simple_arrowHeight + ARROW_TIP);
	
	cairo_move_to (pCairoContext, fDockOffsetX, fDockOffsetY);

	//HautGauche -> HautDroit
	if(pDock->container.bDirectionUp)
	{
		cairo_rel_line_to (pCairoContext, fFrameWidth, 0);
	}
	else
	{
		//On fait la fleche
		cairo_rel_line_to (pCairoContext,  fFrameWidth/2 - my_diapo_simple_arrowWidth/2 + iArrowShift, 0);                //     _
		cairo_rel_line_to (pCairoContext, + my_diapo_simple_arrowWidth/2 - iArrowShift + iDeltaIconX, -my_diapo_simple_arrowHeight);       //  \.
		cairo_rel_line_to (pCairoContext, + my_diapo_simple_arrowWidth/2 + iArrowShift - iDeltaIconX, +my_diapo_simple_arrowHeight);        //    /----
		cairo_rel_line_to (pCairoContext, (fFrameWidth/2 - my_diapo_simple_arrowWidth/2 - iArrowShift) , 0);               // _
	}
	//\_________________ Coin haut droit.
	cairo_rel_curve_to (pCairoContext,
		0, 0,
		my_diapo_simple_radius, 0,
		my_diapo_simple_radius, my_diapo_simple_radius );
	
	//HautDroit -> BasDroit
	cairo_rel_line_to (pCairoContext, 0, fFrameHeight + my_diapo_simple_lineWidth - my_diapo_simple_radius *  2 );
	//\_________________ Coin bas droit.
	cairo_rel_curve_to (pCairoContext,
		0, 0,
		0 , my_diapo_simple_radius,
		-my_diapo_simple_radius , my_diapo_simple_radius);
	
	//BasDroit -> BasGauche
	if(!pDock->container.bDirectionUp)
	{
		cairo_rel_line_to (pCairoContext, - fFrameWidth , 0);
	}
	else
	{
		//On fait la fleche
		cairo_rel_line_to (pCairoContext, - fFrameWidth/2 + my_diapo_simple_arrowWidth/2 + iArrowShift, 0);                //     _
		cairo_rel_line_to (pCairoContext, - my_diapo_simple_arrowWidth/2 - iArrowShift + iDeltaIconX, +my_diapo_simple_arrowHeight);       //  \.
		cairo_rel_line_to (pCairoContext, - my_diapo_simple_arrowWidth/2 + iArrowShift - iDeltaIconX, -my_diapo_simple_arrowHeight);        //    /
		cairo_rel_line_to (pCairoContext, - fFrameWidth/2 + my_diapo_simple_arrowWidth/2 - iArrowShift , 0);               // _
	}
	//\_________________ Coin bas gauche.
	cairo_rel_curve_to (pCairoContext,
		0, 0,
		-my_diapo_simple_radius, 0,
		-my_diapo_simple_radius, -my_diapo_simple_radius);
	
	//BasGauche -> HautGauche
	cairo_rel_line_to (pCairoContext, 0, - fFrameHeight - my_diapo_simple_lineWidth + my_diapo_simple_radius * 2);
	//\_________________ Coin haut gauche.
	cairo_rel_curve_to (pCairoContext,
		0, 0,
		0 , -my_diapo_simple_radius ,
		my_diapo_simple_radius, -my_diapo_simple_radius);

}
static void _cairo_dock_draw_frame_vertical_for_diapo_simple (cairo_t *pCairoContext, CairoDock *pDock)
{
	CDSlideData *pData = pDock->pRendererData;  // non nul
	int iArrowShift = pData->iArrowShift;
	int iDeltaIconX = pData->iDeltaIconX;
	
	gdouble fFrameWidth  = pDock->iMaxDockWidth - 2 * X_BORDER_SPACE;
	gdouble fFrameHeight = pDock->iMaxDockHeight - (my_diapo_simple_arrowHeight + ARROW_TIP + my_diapo_simple_lineWidth);
	gdouble fDockOffsetX = X_BORDER_SPACE;
	gdouble fDockOffsetY = (pDock->container.bDirectionUp ? .5*my_diapo_simple_lineWidth : my_diapo_simple_arrowHeight + ARROW_TIP);
	
	cairo_move_to (pCairoContext, fDockOffsetY, fDockOffsetX);

	if(pDock->container.bDirectionUp)
	{
		cairo_rel_line_to (pCairoContext, 0, fFrameWidth);
	}
	else
	{
		cairo_rel_line_to (pCairoContext, 0,  fFrameWidth/2 - my_diapo_simple_arrowWidth/2 + iArrowShift);                //     _
		cairo_rel_line_to (pCairoContext, -my_diapo_simple_arrowHeight, + my_diapo_simple_arrowWidth/2 - iArrowShift + iDeltaIconX);       //  \.
		cairo_rel_line_to (pCairoContext, +my_diapo_simple_arrowHeight, + my_diapo_simple_arrowWidth/2 + iArrowShift - iDeltaIconX);        //    /
		cairo_rel_line_to (pCairoContext, 0, (fFrameWidth/2 - my_diapo_simple_arrowWidth/2 - iArrowShift));               // _
	}
	//\_________________ Coin haut droit.
	cairo_rel_curve_to (pCairoContext,
		0, 0,
		0, my_diapo_simple_radius,
		my_diapo_simple_radius, my_diapo_simple_radius);
	cairo_rel_line_to (pCairoContext, fFrameHeight + my_diapo_simple_lineWidth - my_diapo_simple_radius * 2, 0);
	//\_________________ Coin bas droit.
	cairo_rel_curve_to (pCairoContext,
			0, 0,
			my_diapo_simple_radius, 0,
			my_diapo_simple_radius, -my_diapo_simple_radius);
	if(!pDock->container.bDirectionUp)
	{
		cairo_rel_line_to (pCairoContext, 0, - fFrameWidth);
	}
	else
	{
		//On fait la fleche
		cairo_rel_line_to (pCairoContext, 0, - fFrameWidth/2 + my_diapo_simple_arrowWidth/2 + iArrowShift);                //     _
		cairo_rel_line_to (pCairoContext, +my_diapo_simple_arrowHeight, - my_diapo_simple_arrowWidth/2 - iArrowShift + iDeltaIconX);       //  \.
		cairo_rel_line_to (pCairoContext, -my_diapo_simple_arrowHeight, - my_diapo_simple_arrowWidth/2 + iArrowShift - iDeltaIconX);        //    /
		cairo_rel_line_to (pCairoContext, 0, - fFrameWidth/2 + my_diapo_simple_arrowWidth/2 - iArrowShift);               // _
	}
	
	//\_________________ Coin bas gauche.
	cairo_rel_curve_to (pCairoContext,
		0, 0,
		0, -my_diapo_simple_radius,
		-my_diapo_simple_radius, -my_diapo_simple_radius);
	cairo_rel_line_to (pCairoContext, - fFrameHeight - my_diapo_simple_lineWidth + my_diapo_simple_radius * 2, 0);
	//\_________________ Coin haut gauche.
	cairo_rel_curve_to (pCairoContext,
		0, 0,
		-my_diapo_simple_radius, 0,
		-my_diapo_simple_radius, my_diapo_simple_radius);
}
static void _cairo_dock_draw_frame_for_diapo_simple (cairo_t *pCairoContext, CairoDock *pDock)
{
	if (pDock->container.bIsHorizontal)
		_cairo_dock_draw_frame_horizontal_for_diapo_simple (pCairoContext, pDock);
	else
		_cairo_dock_draw_frame_vertical_for_diapo_simple (pCairoContext, pDock);
}

static void cairo_dock_render_decorations_in_frame_for_diapo_simple (cairo_t *pCairoContext, CairoDock *pDock, double fAlpha)
{
	// On se fait un beau pattern degrade :
	cairo_pattern_t *mon_super_pattern;
	mon_super_pattern = cairo_pattern_create_linear (0.0,
		0.0,
		my_diapo_simple_fade2right  ? pDock->iMaxDockWidth  : 0.0, // Y'aurait surement des calculs complexes a faire mais 
		my_diapo_simple_fade2bottom ? pDock->iMaxDockHeight : 0.0);     //  a quelques pixels pres pour un degrade : OSEF !
			
	cairo_pattern_add_color_stop_rgba (mon_super_pattern, 0, 
		my_diapo_simple_color_frame_start[0],
		my_diapo_simple_color_frame_start[1],
		my_diapo_simple_color_frame_start[2],
		my_diapo_simple_color_frame_start[3] * fAlpha);  // transparent -> opaque au depliage.
		
	cairo_pattern_add_color_stop_rgba (mon_super_pattern, 1, 
		my_diapo_simple_color_frame_stop[0],
		my_diapo_simple_color_frame_stop[1],
		my_diapo_simple_color_frame_stop[2],
		my_diapo_simple_color_frame_stop[3] * fAlpha);
	cairo_set_source (pCairoContext, mon_super_pattern);
	
	//On remplit le contexte en le preservant -> pourquoi ?  ----> parce qu'on va tracer le contour plus tard ;-)
	cairo_fill_preserve (pCairoContext);
	cairo_pattern_destroy (mon_super_pattern);
}

static void _render_separator (Icon *icon, CairoDock *pDock, cairo_t *pCairoContext)
{
	cairo_pattern_t *mon_super_pattern;
	mon_super_pattern = cairo_pattern_create_linear (icon->fDrawX,
		0.0,
		icon->fDrawX + icon->fWidthFactor,
		0.0);
	
	cairo_pattern_add_color_stop_rgba (mon_super_pattern, 0.0, 
		my_diapo_simple_color_border_line[0],
		my_diapo_simple_color_border_line[1],
		my_diapo_simple_color_border_line[2],
		0.);
	
	cairo_pattern_add_color_stop_rgba (mon_super_pattern, .25, 
		my_diapo_simple_color_border_line[0],
		my_diapo_simple_color_border_line[1],
		my_diapo_simple_color_border_line[2],
		my_diapo_simple_color_border_line[3] * icon->fAlpha);
	
	cairo_pattern_add_color_stop_rgba (mon_super_pattern, .75, 
		my_diapo_simple_color_border_line[0],
		my_diapo_simple_color_border_line[1],
		my_diapo_simple_color_border_line[2],
		my_diapo_simple_color_border_line[3] * icon->fAlpha);
	
	cairo_pattern_add_color_stop_rgba (mon_super_pattern, 1.0, 
		my_diapo_simple_color_border_line[0],
		my_diapo_simple_color_border_line[1],
		my_diapo_simple_color_border_line[2],
		0.);
	
	cairo_set_source (pCairoContext, mon_super_pattern);
	
	cairo_set_line_width (pCairoContext, my_diapo_simple_lineWidth);  /// TODO: essayer avec une valeur constante = 1...
	
	cairo_move_to (pCairoContext, icon->fDrawX, icon->fDrawY + iSeparatorHeight/2);
	cairo_rel_line_to (pCairoContext, icon->fWidthFactor, 0.);
	
	cairo_stroke (pCairoContext);
	cairo_pattern_destroy (mon_super_pattern);
}

static void cd_rendering_render_diapo_simple (cairo_t *pCairoContext, CairoDock *pDock)
{
	CDSlideData *pData = pDock->pRendererData;
	g_return_if_fail (pData != NULL);
	
	double fAlpha = (pDock->fFoldingFactor < .3 ? (.3 - pDock->fFoldingFactor) / .3 : 0.);  // apparition du cadre de 0.3 a 0
	
	double fDeltaIconX = 0.;
	
	if (my_diapo_simple_draw_background)
	{
		//\____________________ On trace le cadre.
		cairo_save (pCairoContext);
		_cairo_dock_draw_frame_for_diapo_simple (pCairoContext, pDock);
		
		//\____________________ On dessine les decorations dedans.
		cairo_dock_render_decorations_in_frame_for_diapo_simple (pCairoContext, pDock, fAlpha);

		//\____________________ On dessine le cadre.
		if (my_diapo_simple_lineWidth != 0 && my_diapo_simple_color_border_line[3] != 0 && fAlpha != 0)
		{
			cairo_set_line_width (pCairoContext,  my_diapo_simple_lineWidth);
			cairo_set_source_rgba (pCairoContext,
				my_diapo_simple_color_border_line[0],
				my_diapo_simple_color_border_line[1],
				my_diapo_simple_color_border_line[2],
				my_diapo_simple_color_border_line[3] * fAlpha);
			cairo_stroke (pCairoContext);
		}
		else
			cairo_new_path (pCairoContext);
		cairo_restore (pCairoContext);
	}
	
	if (pDock->icons == NULL)
		return;
	
	//\____________________ On dessine la ficelle qui les joint.
	//TODO Rendre joli !
	if (myIconsParam.iStringLineWidth > 0)
		cairo_dock_draw_string (pCairoContext, pDock, myIconsParam.iStringLineWidth, TRUE, TRUE);
	
	//\____________________ On dessine les barres de defilement.
	if (pData->iDeltaHeight != 0)
	{
		cairo_save (pCairoContext);
		cairo_set_line_width (pCairoContext, 2.);
		
		if (! pDock->container.bIsHorizontal)
		{
			cairo_translate (pCairoContext, pDock->container.iHeight/2, pDock->container.iWidth/2);
			cairo_rotate (pCairoContext, G_PI/2);
			cairo_scale (pCairoContext, 1., -1.);
			cairo_translate (pCairoContext, -pDock->container.iWidth/2, -pDock->container.iHeight/2);
		}
		
		double x_arrow = pDock->iMaxDockWidth - X_BORDER_SPACE - fScrollbarWidth/2;  // pointe de la fleche.
		double y_arrow_top, y_arrow_bottom;
		if (pDock->container.bDirectionUp)
		{
			y_arrow_bottom = pDock->iMaxDockHeight - (my_diapo_simple_arrowHeight + ARROW_TIP + my_diapo_simple_lineWidth + my_diapo_simple_radius);
			y_arrow_top = my_diapo_simple_lineWidth + my_diapo_simple_radius;
		}
		else
		{
			y_arrow_top = my_diapo_simple_arrowHeight + ARROW_TIP + my_diapo_simple_lineWidth + my_diapo_simple_radius;
			y_arrow_bottom = pDock->iMaxDockHeight - my_diapo_simple_lineWidth - my_diapo_simple_radius;
		}
		if (pData->iScrollOffset != 0)  // fleche vers le haut.
		{
			cairo_move_to (pCairoContext, x_arrow, y_arrow_top);
			cairo_rel_line_to (pCairoContext, fScrollbarWidth/2, fArrowHeight);
			cairo_rel_line_to (pCairoContext, -fScrollbarWidth, 0.);
			cairo_close_path (pCairoContext);
			
			cairo_set_source_rgba (pCairoContext, my_diapo_simple_color_scrollbar_inside[0], my_diapo_simple_color_scrollbar_inside[1], my_diapo_simple_color_scrollbar_inside[2], my_diapo_simple_color_scrollbar_inside[3] * fAlpha);
			cairo_fill_preserve (pCairoContext);
			
			cairo_set_source_rgba (pCairoContext, my_diapo_simple_color_scrollbar_line[0], my_diapo_simple_color_scrollbar_line[1], my_diapo_simple_color_scrollbar_line[2], my_diapo_simple_color_scrollbar_line[3] * fAlpha);
			cairo_stroke (pCairoContext);
		}
		if (pData->iScrollOffset != pData->iDeltaHeight)  // fleche vers le bas.
		{
			cairo_move_to (pCairoContext, x_arrow, y_arrow_bottom);
			cairo_rel_line_to (pCairoContext, fScrollbarWidth/2, - fArrowHeight);
			cairo_rel_line_to (pCairoContext, -fScrollbarWidth, 0.);
			cairo_close_path (pCairoContext);
			
			cairo_set_source_rgba (pCairoContext, my_diapo_simple_color_scrollbar_inside[0], my_diapo_simple_color_scrollbar_inside[1], my_diapo_simple_color_scrollbar_inside[2], my_diapo_simple_color_scrollbar_inside[3] * fAlpha);
			cairo_fill_preserve (pCairoContext);
			
			cairo_set_source_rgba (pCairoContext, my_diapo_simple_color_scrollbar_line[0], my_diapo_simple_color_scrollbar_line[1], my_diapo_simple_color_scrollbar_line[2], my_diapo_simple_color_scrollbar_line[3] * fAlpha);
			cairo_stroke (pCairoContext);
		}
		// scrollbar outline
		cairo_move_to (pCairoContext, x_arrow - fScrollbarWidth/2, y_arrow_top + fArrowHeight + fScrollbarArrowGap);
		cairo_rel_line_to (pCairoContext, fScrollbarWidth, 0.);
		cairo_rel_line_to (pCairoContext, 0., y_arrow_bottom - y_arrow_top - 2*(fArrowHeight+fScrollbarArrowGap));
		cairo_rel_line_to (pCairoContext, -fScrollbarWidth, 0.);
		cairo_close_path (pCairoContext);
		
		cairo_set_source_rgba (pCairoContext, my_diapo_simple_color_scrollbar_inside[0], my_diapo_simple_color_scrollbar_inside[1], my_diapo_simple_color_scrollbar_inside[2], my_diapo_simple_color_scrollbar_inside[3] * fAlpha);
		cairo_fill_preserve (pCairoContext);
		
		cairo_set_source_rgba (pCairoContext, my_diapo_simple_color_scrollbar_line[0], my_diapo_simple_color_scrollbar_line[1], my_diapo_simple_color_scrollbar_line[2], my_diapo_simple_color_scrollbar_line[3] * fAlpha);
		cairo_stroke (pCairoContext);
		// grip
		double fFrameHeight = pDock->iMaxDockHeight - (my_diapo_simple_arrowHeight + ARROW_TIP + my_diapo_simple_lineWidth);  // hauteur du cadre avec les rayons et sans la pointe.
		double fGripHeight = fFrameHeight / (fFrameHeight + pData->iDeltaHeight) * (y_arrow_bottom - y_arrow_top - 2*(fArrowHeight+fScrollbarArrowGap));
		double ygrip = (double) pData->iScrollOffset / pData->iDeltaHeight * (y_arrow_bottom - y_arrow_top - 2*(fArrowHeight+fScrollbarArrowGap) - fGripHeight);
		cairo_set_source_rgba (pCairoContext, my_diapo_simple_color_grip[0], my_diapo_simple_color_grip[1], my_diapo_simple_color_grip[2], my_diapo_simple_color_grip[3] * fAlpha);
		cairo_move_to (pCairoContext, x_arrow - fScrollbarWidth/2 + 1, y_arrow_top + fArrowHeight + fScrollbarArrowGap + ygrip);
		cairo_rel_line_to (pCairoContext, fScrollbarWidth - 2, 0.);
		cairo_rel_line_to (pCairoContext, 0., fGripHeight);
		cairo_rel_line_to (pCairoContext, - (fScrollbarWidth - 2), 0.);
		cairo_fill (pCairoContext);
		
		cairo_restore (pCairoContext);
	}
	
	//\____________________ On dessine les icones avec leurs etiquettes.
	// on determine la 1ere icone a tracer : l'icone suivant l'icone pointee.
	GList *pFirstDrawnElement = cairo_dock_get_first_drawn_element_linear (pDock->icons);
	if (pFirstDrawnElement == NULL)
		return;
	
	//clip pour le scroll
	if (pData->iDeltaHeight != 0) // on fait un clip pour les icones qui debordent.
	{
		int h = my_diapo_simple_arrowHeight + ARROW_TIP + my_diapo_simple_lineWidth;
		if (pDock->container.bIsHorizontal)
		{
			cairo_rectangle (pCairoContext,
				0.,
				(pDock->container.bDirectionUp ? my_diapo_simple_lineWidth : h),  // top left corner.
				pDock->container.iWidth,
				pDock->container.iHeight - h - my_diapo_simple_lineWidth);
		}
		else
		{
			cairo_rectangle (pCairoContext,
				(pDock->container.bDirectionUp ? my_diapo_simple_lineWidth : h),  // top left corner.
				0.,
				pDock->container.iHeight - h - my_diapo_simple_lineWidth,
				pDock->container.iWidth);
		}
		cairo_clip (pCairoContext);
	}
	
	// on dessine les icones, l'icone pointee en dernier.
	Icon *icon;
	GList *ic = pFirstDrawnElement;
	do
	{
		icon = ic->data;
		if (CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (icon))
		{
			_render_separator (icon, pDock, pCairoContext);
			
			ic = cairo_dock_get_next_element (ic, pDock->icons);
			continue;
		}
		
		cairo_save (pCairoContext);
		cairo_dock_render_one_icon (icon, pDock, pCairoContext, 1., FALSE);
		cairo_restore (pCairoContext);
		
//////////////////////////////////////////////////////////////////////////////////////// On affiche le texte !
		gdouble zoom;
		if(icon->pTextBuffer != NULL && (my_diapo_simple_display_all_labels || icon->bPointed))
		{
			double fAlpha = (pDock->fFoldingFactor > .5 ? (1 - pDock->fFoldingFactor) / .5 : 1.);
			cairo_save (pCairoContext);
			
			double fOffsetX = -icon->iTextWidth/2 + icon->fWidthFactor * icon->fWidth * icon->fScale / 2;
			if (fOffsetX < 0)
				fOffsetX = 0;
			else if (0 + fOffsetX + icon->iTextWidth > pDock->container.iWidth)
				fOffsetX = pDock->container.iWidth - icon->iTextWidth - 0;
			
			if (icon->iTextWidth > icon->fWidth + my_diapo_simple_iconGapX && ! icon->bPointed)
			{
				if (pDock->container.bIsHorizontal)
				{
					cairo_translate (pCairoContext,
						floor (icon->fDrawX - my_diapo_simple_iconGapX/2),
						floor (icon->fDrawY - icon->iTextHeight));
				}
				else
				{
					cairo_translate (pCairoContext,
						floor (icon->fDrawY - my_diapo_simple_iconGapX/2),
						floor (icon->fDrawX - icon->iTextHeight));
				}
				cairo_set_source_surface (pCairoContext,
					icon->pTextBuffer,
					fOffsetX,
					0.);
				
				cairo_pattern_t *pGradationPattern = cairo_pattern_create_linear (0.,
					0.,
					icon->fWidth * 1 + my_diapo_simple_iconGapX,
					0.);
				cairo_pattern_set_extend (pGradationPattern, icon->bPointed ? CAIRO_EXTEND_PAD : CAIRO_EXTEND_NONE);
				cairo_pattern_add_color_stop_rgba (pGradationPattern,
					0.,
					0.,
					0.,
					0.,
					fAlpha);
				cairo_pattern_add_color_stop_rgba (pGradationPattern,
					0.75,
					0.,
					0.,
					0.,
					fAlpha);
				cairo_pattern_add_color_stop_rgba (pGradationPattern,
					1.,
					0.,
					0.,
					0.,
					0*MIN (0.2, fAlpha/2));
				cairo_mask (pCairoContext, pGradationPattern);
				cairo_pattern_destroy (pGradationPattern);
			}
			else  // le texte tient dans l'icone.
			{
				if (pDock->container.bIsHorizontal)
				{
					fOffsetX = icon->fDrawX + (icon->fWidth * icon->fScale - icon->iTextWidth) / 2;
					if (fOffsetX < 0)
						fOffsetX = 0;
					else if (fOffsetX + icon->iTextWidth > pDock->container.iWidth)
						fOffsetX = pDock->container.iWidth - icon->iTextWidth;
					cairo_translate (pCairoContext,
						floor (fOffsetX),
						floor (icon->fDrawY - icon->iTextHeight));
				}
				else
				{
					fOffsetX = icon->fDrawY + (icon->fWidth * icon->fScale - icon->iTextWidth) / 2;
					if (fOffsetX < 0)
						fOffsetX = 0;
					else if (fOffsetX + icon->iTextWidth > pDock->container.iHeight)
						fOffsetX = pDock->container.iHeight - icon->iTextWidth;
					cairo_translate (pCairoContext,
						floor (fOffsetX),
						floor (icon->fDrawX - icon->iTextHeight));
				}
				cairo_set_source_surface (pCairoContext,
					icon->pTextBuffer,
					0.,
					0.);
				cairo_paint_with_alpha (pCairoContext, fAlpha);
			}
			cairo_restore (pCairoContext);
		}
		
		ic = cairo_dock_get_next_element (ic, pDock->icons);
	}
	while (ic != pFirstDrawnElement);
}


  //////////////////////
 /// OPENGL DRAWING ///
//////////////////////

#define DELTA_ROUND_DEGREE 5
#define _recopy_prev_color(pColorTab, i) memcpy (&pColorTab[4*i], &pColorTab[4*(i-1)], 4*sizeof (GLfloat));
#define _copy_color(pColorTab, i, fAlpha, c) do { \
	pColorTab[4*i]   = c[0];\
	pColorTab[4*i+1] = c[1];\
	pColorTab[4*i+2] = c[2];\
	pColorTab[4*i+3] = c[3] * fAlpha; } while (0)
/*#define _copy_mean_color(pColorTab, i, fAlpha, c1, c2, f) do { \
	pColorTab[4*i]   = c1[0]*f + c2[0]*(1-f);\
	pColorTab[4*i+1] = c1[1]*f + c2[1]*(1-f);\
	pColorTab[4*i+2] = c1[2]*f + c2[2]*(1-f);\
	pColorTab[4*i+3] = (c1[3]*f + c2[3]*(1-f)) * fAlpha; } while (0)*/
static void cd_add_arrow_to_path (CairoDockGLPath *pPath, double fFrameWidth, CDSlideData *pData)
{
	int iArrowShift = pData->iArrowShift;
	int iDeltaIconX = pData->iDeltaIconX;
	double w = fFrameWidth / 2;
	double aw = my_diapo_simple_arrowWidth/2;
	double ah = my_diapo_simple_arrowHeight;
	/**double xa = my_diapo_simple_arrowShift * (w - aw);  // abscisse de l'extremite de la pointe.
	cairo_dock_gl_path_rel_line_to (pPath, w + xa - aw, 0.);  // pointe.
	cairo_dock_gl_path_rel_line_to (pPath, aw, -ah);
	cairo_dock_gl_path_rel_line_to (pPath, aw, ah);*/
	cairo_dock_gl_path_rel_line_to (pPath, w - aw + iArrowShift, 0.);  // pointe.
	cairo_dock_gl_path_rel_line_to (pPath, aw - iArrowShift + iDeltaIconX, -ah);
	cairo_dock_gl_path_rel_line_to (pPath, aw + iArrowShift - iDeltaIconX, ah);
}
static CairoDockGLPath *cd_generate_frame_path_without_arrow (double fFrameWidth, double fTotalHeight, double fRadius)
{
	static CairoDockGLPath *pPath = NULL;
	double fTotalWidth = fFrameWidth + 2 * fRadius;
	double fFrameHeight = MAX (0, fTotalHeight - 2 * fRadius);
	double w = fFrameWidth / 2;
	double h = fFrameHeight / 2;
	double r = fRadius;
	
	int iNbPoins1Round = 90/DELTA_ROUND_DEGREE;
	if (pPath == NULL)
		pPath = cairo_dock_new_gl_path ((iNbPoins1Round+1)*4+1+3, w, -h-r, fTotalWidth, fTotalHeight);  // on commence au coin haut droit pour avoir une bonne triangulation du polygone, et en raisonnant par rapport au centre du rectangle.
	else
		cairo_dock_gl_path_move_to (pPath, w, -h-r);
	
	cairo_dock_gl_path_arc (pPath, iNbPoins1Round,  w, -h, r, -G_PI/2, +G_PI/2);  // coin bas droit.
	
	cairo_dock_gl_path_arc (pPath, iNbPoins1Round, w, h, r, 0.,     +G_PI/2);  // coin haut droit.
	
	cairo_dock_gl_path_arc (pPath, iNbPoins1Round, -w,  h, r, G_PI/2,  +G_PI/2);  // coin haut gauche.
	
	cairo_dock_gl_path_arc (pPath, iNbPoins1Round, -w, -h, r, G_PI,    +G_PI/2);  // coin bas gauche.
	
	return pPath;
}

static CairoDockGLPath *cd_generate_arrow_path (double fFrameWidth, double fTotalHeight, CDSlideData *pData)
{
	static CairoDockGLPath *pPath = NULL;
	int iArrowShift = pData->iArrowShift;
	int iDeltaIconX = pData->iDeltaIconX;
	double w = fFrameWidth / 2;
	double aw = my_diapo_simple_arrowWidth/2;
	double ah = my_diapo_simple_arrowHeight;
	/**double xa = my_diapo_simple_arrowShift * (w - aw);  // abscisse de l'extremite de la pointe.
	if (pPath == NULL)
		pPath = cairo_dock_new_gl_path (3, xa - aw, -fTotalHeight/2, 0., 0.);
	else
		cairo_dock_gl_path_move_to (pPath, xa - aw, -fTotalHeight/2);
	cairo_dock_gl_path_rel_line_to (pPath, aw, -ah);
	cairo_dock_gl_path_rel_line_to (pPath, aw, ah);*/
	
	if (pPath == NULL)
		pPath = cairo_dock_new_gl_path (3, iArrowShift - aw, -fTotalHeight/2, 0., 0.);
	else
		cairo_dock_gl_path_move_to (pPath, iArrowShift - aw, -fTotalHeight/2);
	
	cairo_dock_gl_path_rel_line_to (pPath, aw - iArrowShift + iDeltaIconX, -ah);
	cairo_dock_gl_path_rel_line_to (pPath, aw + iArrowShift - iDeltaIconX, ah);
	
	return pPath;
}

static const GLfloat *cd_generate_color_tab (double fAlpha, GLfloat *pMiddleBottomColor)
{
	static GLfloat *pColorTab = NULL;
	int iNbPoins1Round = 90/DELTA_ROUND_DEGREE;
	if (pColorTab == NULL)
		pColorTab = g_new (GLfloat, ((iNbPoins1Round+1)*4+1) * 4);
	
	double *pTopRightColor, *pTopLeftColor, *pBottomLeftColor, *pBottomRightColor;
	double pMeanColor[4] = {(my_diapo_simple_color_frame_start[0] + my_diapo_simple_color_frame_stop[0])/2,
		(my_diapo_simple_color_frame_start[1] + my_diapo_simple_color_frame_stop[1])/2,
		(my_diapo_simple_color_frame_start[2] + my_diapo_simple_color_frame_stop[2])/2,
		(my_diapo_simple_color_frame_start[3] + my_diapo_simple_color_frame_stop[3])/2};
	pTopLeftColor = my_diapo_simple_color_frame_start;
	if (my_diapo_simple_fade2bottom || my_diapo_simple_fade2right)
	{
		pBottomRightColor = my_diapo_simple_color_frame_stop;
		if (my_diapo_simple_fade2bottom && my_diapo_simple_fade2right)
		{
			pBottomLeftColor = pMeanColor;
			pTopRightColor = pMeanColor;
		}
		else if (my_diapo_simple_fade2bottom)
		{
			pBottomLeftColor = my_diapo_simple_color_frame_stop;
			pTopRightColor = my_diapo_simple_color_frame_start;
		}
		else
		{
			pBottomLeftColor = my_diapo_simple_color_frame_start;
			pTopRightColor = my_diapo_simple_color_frame_stop;
		}
	}
	else
	{
		pBottomRightColor = my_diapo_simple_color_frame_start;
		pBottomLeftColor = my_diapo_simple_color_frame_start;
		pTopRightColor = my_diapo_simple_color_frame_start;
	}
	
	pMiddleBottomColor[0] = (pBottomRightColor[0] + pBottomLeftColor[0])/2;
	pMiddleBottomColor[1] = (pBottomRightColor[1] + pBottomLeftColor[1])/2;
	pMiddleBottomColor[2] = (pBottomRightColor[2] + pBottomLeftColor[2])/2;
	pMiddleBottomColor[3] = (pBottomRightColor[3] + pBottomLeftColor[3])/2;
	
	int i=0, j;
	_copy_color (pColorTab, i, fAlpha, pBottomRightColor);
	i ++;
	
	for (j = 0; j < iNbPoins1Round; j ++, i ++)  // coin bas droit.
	{
		_copy_color (pColorTab, i, fAlpha, pBottomRightColor);
	}
	
	for (j = 0; j < iNbPoins1Round; j ++, i ++)  // coin haut droit.
	{
		_copy_color (pColorTab, i, fAlpha, pTopRightColor);
	}
	
	for (j = 0; j < iNbPoins1Round; j ++, i ++)  // coin haut gauche.
	{
		_copy_color (pColorTab, i, fAlpha, pTopLeftColor);
	}
	
	for (j = 0; j < iNbPoins1Round; j ++, i ++)  // coin bas gauche.
	{
		_copy_color (pColorTab, i, fAlpha, pBottomLeftColor);
	}
	
	return pColorTab;
}

static void _render_separator_opengl (Icon *icon, CairoDock *pDock)
{
	glPushMatrix ();
	
	//glLoadIdentity ();
	glTranslatef (icon->fDrawX, floor (pDock->container.iHeight - icon->fDrawY - iSeparatorHeight/2) + .5 * ( my_diapo_simple_lineWidth&1), 0);
	
	glEnable (GL_BLEND);
	_cairo_dock_set_blend_alpha ();
	
	glPolygonMode (GL_FRONT, GL_LINE);
	glLineWidth (my_diapo_simple_lineWidth);
	
	glBegin(GL_LINE_STRIP);
	glColor4f (my_diapo_simple_color_border_line[0], my_diapo_simple_color_border_line[1], my_diapo_simple_color_border_line[2], 0);
	glVertex3f (0, 0, 0.);
	
	glColor4f (my_diapo_simple_color_border_line[0], my_diapo_simple_color_border_line[1], my_diapo_simple_color_border_line[2], my_diapo_simple_color_border_line[3]);
	glVertex3f (.25 * icon->fWidthFactor, 0, 0.);
	glVertex3f (.75 * icon->fWidthFactor, 0, 0.);
	
	glColor4f (my_diapo_simple_color_border_line[0], my_diapo_simple_color_border_line[1], my_diapo_simple_color_border_line[2], 0);
	glVertex3f (icon->fWidthFactor, 0, 0.);
	glEnd();
	
	glPopMatrix ();
}

#define N_ROUND 7
static void cd_rendering_render_diapo_simple_opengl (CairoDock *pDock)
{
	static CairoDockGLPath *pScrollPath = NULL;
	
	//\____________________ On initialise le cadre.
	CDSlideData *pData = pDock->pRendererData;
	g_return_if_fail (pData != NULL);
	
	int iNbVertex;
	GLfloat *pColorTab, *pVertexTab;
	
	double fRadius = my_diapo_simple_radius;
	double fFrameWidth  = pDock->iMaxDockWidth - 2*X_BORDER_SPACE;  // longueur du trait horizontal.
	double fFrameHeight = pDock->iMaxDockHeight - (my_diapo_simple_arrowHeight + ARROW_TIP + my_diapo_simple_lineWidth);  // hauteur du cadre avec les rayons et sans la pointe.
	double fDockOffsetX, fDockOffsetY;
	fDockOffsetX = X_BORDER_SPACE;
	fDockOffsetY = my_diapo_simple_arrowHeight+ARROW_TIP;
	
	//\_____________ On genere les coordonnees du contour.
	CairoDockGLPath *pFramePath = cd_generate_frame_path_without_arrow (fFrameWidth, fFrameHeight, fRadius);
	
	//\_____________ On remplit avec le fond.
	glPushMatrix ();
	double fAlpha = (pDock->fFoldingFactor < .3 ? (.3 - pDock->fFoldingFactor) / .3 : 0.);  // apparition du cadre de 0.3 a 0
	cairo_dock_set_container_orientation_opengl (CAIRO_CONTAINER (pDock));
	glTranslatef (fDockOffsetX + (fFrameWidth)/2,
		fDockOffsetY + fFrameHeight/2,
		0.);
	_cairo_dock_set_blend_alpha ();
	
	if (my_diapo_simple_draw_background)
	{
		// le cadre sans la pointe.
		GLfloat pBottomMiddleColor[4];
		const GLfloat *pColorTab = cd_generate_color_tab (fAlpha, pBottomMiddleColor);
		glEnableClientState (GL_COLOR_ARRAY);
		glColorPointer (4, GL_FLOAT, 0, pColorTab);
		cairo_dock_fill_gl_path (pFramePath, 0);
		glDisableClientState (GL_COLOR_ARRAY);
		
		// la pointe.
		CairoDockGLPath *pArrowPath = cd_generate_arrow_path (fFrameWidth, fFrameHeight, pData);
		glColor4f (pBottomMiddleColor[0], pBottomMiddleColor[1], pBottomMiddleColor[2], pBottomMiddleColor[3] * fAlpha);
		cairo_dock_fill_gl_path (pArrowPath, 0);
	}
	
	//\_____________ On trace le contour.
	if (my_diapo_simple_lineWidth != 0 && my_diapo_simple_color_border_line[3] != 0 && fAlpha != 0)
	{
		cd_add_arrow_to_path (pFramePath, fFrameWidth, pData);
		glLineWidth (my_diapo_simple_lineWidth);
		glColor4f (my_diapo_simple_color_border_line[0], my_diapo_simple_color_border_line[1], my_diapo_simple_color_border_line[2], my_diapo_simple_color_border_line[3] * fAlpha);
		cairo_dock_stroke_gl_path (pFramePath, TRUE);
	}
	glPopMatrix ();
	_cairo_dock_set_blend_over ();
	
	//\____________________ On dessine les barres de defilement.
	if (pData->iDeltaHeight != 0)
	{
		if (pScrollPath == NULL)
			pScrollPath = cairo_dock_new_gl_path (N_ROUND + 5, 0., 0., 0, 0);  // un arc + le sommet + la base
		glLineWidth (2.);
		glPushMatrix ();
		if (!pDock->container.bIsHorizontal)
		{
			glTranslatef (pDock->container.iHeight/2, pDock->container.iWidth/2, 0.);
			glRotatef (-90., 0., 0., 1.);
			glScalef (1., -1., 1.);  // comme si bDirectionUp
			glTranslatef (-pDock->container.iWidth/2, -pDock->container.iHeight/2, 0.);
		}
		double x_arrow = pDock->iMaxDockWidth - X_BORDER_SPACE - fScrollbarWidth/2;  // pointe de la fleche.
		double y_arrow_top, y_arrow_bottom;
		if (pDock->container.bDirectionUp)
		{
			y_arrow_bottom = my_diapo_simple_arrowHeight + ARROW_TIP + my_diapo_simple_lineWidth + my_diapo_simple_radius;
			y_arrow_top = pDock->iMaxDockHeight - my_diapo_simple_lineWidth - my_diapo_simple_radius;
		}
		else
		{
			y_arrow_top = pDock->iMaxDockHeight - (my_diapo_simple_arrowHeight + ARROW_TIP + my_diapo_simple_lineWidth + my_diapo_simple_radius);
			y_arrow_bottom = my_diapo_simple_lineWidth + my_diapo_simple_radius;
		}
		
		if (pData->iScrollOffset != 0)  // fleche vers le haut.
		{
			cairo_dock_gl_path_move_to (pScrollPath, x_arrow, y_arrow_top);
			cairo_dock_gl_path_rel_line_to (pScrollPath, fScrollbarWidth/2, -fArrowHeight);
			cairo_dock_gl_path_rel_line_to (pScrollPath, 0., -fScrollbarArrowGap);
			cairo_dock_gl_path_rel_simple_curve_to (pScrollPath, N_ROUND, 
				-fScrollbarWidth/2, 4*fScrollbarRadius/3,  // hauteur de controle pour atteindre la meme hauteur que l'arc de la barre.
				-fScrollbarWidth, 0.);
			cairo_dock_gl_path_rel_line_to (pScrollPath, 0., fScrollbarArrowGap);
			
			glColor4f (my_diapo_simple_color_scrollbar_inside[0], my_diapo_simple_color_scrollbar_inside[1], my_diapo_simple_color_scrollbar_inside[2], my_diapo_simple_color_scrollbar_inside[3] * fAlpha);
			cairo_dock_fill_gl_path (pScrollPath, 0);
			
			glColor4f (my_diapo_simple_color_scrollbar_line[0], my_diapo_simple_color_scrollbar_line[1], my_diapo_simple_color_scrollbar_line[2], my_diapo_simple_color_scrollbar_line[3] * fAlpha);
			cairo_dock_stroke_gl_path (pScrollPath, TRUE);  // TRUE <=> close
		}
		if (pData->iScrollOffset != pData->iDeltaHeight)  // fleche vers le bas.
		{
			cairo_dock_gl_path_move_to (pScrollPath, x_arrow, y_arrow_bottom);
			cairo_dock_gl_path_rel_line_to (pScrollPath, fScrollbarWidth/2, fArrowHeight);
			cairo_dock_gl_path_rel_line_to (pScrollPath, 0., fScrollbarArrowGap);
			cairo_dock_gl_path_rel_simple_curve_to (pScrollPath, N_ROUND, 
				-fScrollbarWidth/2, -(fScrollbarArrowGap + 4*fScrollbarRadius/3),
				-fScrollbarWidth, 0.);
			cairo_dock_gl_path_rel_line_to (pScrollPath, 0., -fScrollbarArrowGap);
			
			glColor4f (my_diapo_simple_color_scrollbar_inside[0], my_diapo_simple_color_scrollbar_inside[1], my_diapo_simple_color_scrollbar_inside[2], my_diapo_simple_color_scrollbar_inside[3] * fAlpha);
			cairo_dock_fill_gl_path (pScrollPath, 0);
			
			glColor4f (my_diapo_simple_color_scrollbar_line[0], my_diapo_simple_color_scrollbar_line[1], my_diapo_simple_color_scrollbar_line[2], my_diapo_simple_color_scrollbar_line[3] * fAlpha);
			cairo_dock_stroke_gl_path (pScrollPath, TRUE);  // TRUE <=> close
		}
		
		// scrollbar outline
		int iBarHeight = y_arrow_top - y_arrow_bottom - 2*(fArrowHeight+fScrollbarArrowGap);
		const CairoDockGLPath *pPath = cairo_dock_generate_rectangle_path (fScrollbarWidth - 2*fScrollbarRadius,
			iBarHeight,
			fScrollbarRadius, TRUE);
		glPushMatrix ();
		glTranslatef (x_arrow, (y_arrow_bottom + y_arrow_top)/2, 0.);
		
		glColor4f (my_diapo_simple_color_scrollbar_inside[0], my_diapo_simple_color_scrollbar_inside[1], my_diapo_simple_color_scrollbar_inside[2], my_diapo_simple_color_scrollbar_inside[3] * fAlpha);
		cairo_dock_fill_gl_path (pPath, 0);
		
		glColor4f (my_diapo_simple_color_scrollbar_line[0], my_diapo_simple_color_scrollbar_line[1], my_diapo_simple_color_scrollbar_line[2], my_diapo_simple_color_scrollbar_line[3] * fAlpha);
		cairo_dock_stroke_gl_path (pPath, TRUE);
		glPopMatrix ();
		
		// grip
		double fGripHeight = fFrameHeight / (fFrameHeight + pData->iDeltaHeight) * (y_arrow_top - y_arrow_bottom - 2*(fArrowHeight+fScrollbarArrowGap));
		if (fGripHeight < 2*fScrollbarRadius)
			fGripHeight = 2*fScrollbarRadius;
		double ygrip = (double) pData->iScrollOffset / pData->iDeltaHeight * (y_arrow_top - y_arrow_bottom - 2*(fArrowHeight+fScrollbarArrowGap) - fGripHeight);
		pPath = cairo_dock_generate_rectangle_path (fScrollbarWidth - 2*fScrollbarRadius - 2,
			fGripHeight,
			fScrollbarRadius, TRUE);
		glPushMatrix ();
		glTranslatef (x_arrow, y_arrow_top - (fArrowHeight+fScrollbarArrowGap) - ygrip - fGripHeight/2, 0.);
		
		glColor4f (my_diapo_simple_color_grip[0], my_diapo_simple_color_grip[1], my_diapo_simple_color_grip[2], my_diapo_simple_color_grip[3] * fAlpha);
		cairo_dock_fill_gl_path (pPath, 0);
		glPopMatrix ();

		glPopMatrix ();
	}
	
	if (pDock->icons == NULL)
		return ;
	
	//\____________________ On dessine la ficelle.
	if (myIconsParam.iStringLineWidth > 0)
		cairo_dock_draw_string_opengl (pDock, myIconsParam.iStringLineWidth, FALSE, FALSE);
	
	//\____________________ On dessine les icones.
	// on determine la 1ere icone a tracer : l'icone suivant l'icone pointee.
	GList *pFirstDrawnElement = cairo_dock_get_first_drawn_element_linear (pDock->icons);
	if (pFirstDrawnElement == NULL)
		return;
	
	// on dessine les icones, l'icone pointee en dernier.
	if (pData->iDeltaHeight != 0) // on fait un clip pour les icones qui debordent.
	{
		int h = my_diapo_simple_arrowHeight + ARROW_TIP + my_diapo_simple_lineWidth;
		glEnable (GL_SCISSOR_TEST);
		if (pDock->container.bIsHorizontal)
		{
			glScissor (0,
				(pDock->container.bDirectionUp ? h : my_diapo_simple_lineWidth),  // lower left corner of the scissor box.
				pDock->container.iWidth,
				pDock->container.iHeight - h - my_diapo_simple_lineWidth);
		}
		else
		{
			glScissor ((!pDock->container.bDirectionUp ? h : my_diapo_simple_lineWidth),  // lower left corner of the scissor box.
				my_diapo_simple_lineWidth,
				pDock->container.iHeight - h - my_diapo_simple_lineWidth,
				pDock->container.iWidth);
		}
	}
	
	Icon *icon;
	GList *ic = pFirstDrawnElement;
	do
	{
		icon = ic->data;
		if (CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (icon))
		{
			_render_separator_opengl (icon, pDock);
			ic = cairo_dock_get_next_element (ic, pDock->icons);
			continue;
		}
		
		cairo_dock_render_one_icon_opengl (icon, pDock, 1., FALSE);
		
		if (icon->iLabelTexture != 0 && (my_diapo_simple_display_all_labels || icon->bPointed))
		{
			glPushMatrix ();
			glLoadIdentity ();
			
			double fAlpha = (pDock->fFoldingFactor > .5 ? (1 - pDock->fFoldingFactor) / .5 : 1.);  // apparition du texte de 1 a 0.5
			
			double dx, dy = .5 * (icon->iTextHeight & 1);  // on decale la texture pour la coller sur la grille des coordonnees entieres.
			double u0 = 0., u1 = 1.;
			double fOffsetX = 0.;
			if (pDock->container.bIsHorizontal)
			{
				if (icon->bPointed)
				{
					_cairo_dock_set_alpha (fAlpha);
					if (icon->fDrawX + icon->fWidth/2 + icon->iTextWidth/2 > pDock->container.iWidth)
						fOffsetX = pDock->container.iWidth - (icon->fDrawX + icon->fWidth/2 + icon->iTextWidth/2);
					if (icon->fDrawX + icon->fWidth/2 - icon->iTextWidth/2 < 0)
						fOffsetX = icon->iTextWidth/2 - (icon->fDrawX + icon->fWidth/2);
				}
				else
				{
					_cairo_dock_set_alpha (fAlpha);
					///_cairo_dock_set_alpha (fAlpha * icon->fScale / my_diapo_simple_fScaleMax);
					double text_width = floor ((icon->fWidth + my_diapo_simple_iconGapX) / 2) * 2;  // make it even, so that we don't get blur when we draw with the gradation.
					if (icon->iTextWidth > text_width)
					{
						fOffsetX = 0.;
						u1 = text_width / icon->iTextWidth;
					}
				}
				dx = .5 * (((int)round (icon->iTextWidth * (u1 - u0))) & 1);
				
				glTranslatef (ceil (icon->fDrawX + icon->fScale * icon->fWidth/2 + fOffsetX) + dx,
					ceil (pDock->container.iHeight - icon->fDrawY + icon->iTextHeight / 2) + dy,
					0.);
			}
			else
			{
				if (icon->bPointed)
				{
					_cairo_dock_set_alpha (fAlpha);
					if (icon->fDrawY + icon->fHeight/2 + icon->iTextWidth/2 > pDock->container.iHeight)
						fOffsetX = pDock->container.iHeight - (icon->fDrawY + icon->fHeight/2 + icon->iTextWidth/2);
					if (icon->fDrawY + icon->fHeight/2 - icon->iTextWidth/2 < 0)
						fOffsetX = icon->iTextWidth/2 - (icon->fDrawY + icon->fHeight/2);
				}
				else
				{
					_cairo_dock_set_alpha (fAlpha);
					double text_width = (int)floor (icon->fWidth + 2 * myIconsParam.iLabelSize) / 2 * 2;  // see above.
					if (icon->iTextWidth > text_width)
					{
						fOffsetX = 0.;
						u1 = text_width / icon->iTextWidth;
					}
				}
				dx = .5 * (((int)round (icon->iTextWidth * (u1 - u0))) & 1);
				
				glTranslatef (ceil (icon->fDrawY + icon->fScale * icon->fHeight/2 + fOffsetX / 2) + dx,
					ceil (pDock->container.iWidth - icon->fDrawX + icon->iTextHeight / 2) + dy,
					0.);
			}
			_cairo_dock_enable_texture ();
			_cairo_dock_set_blend_alpha ();
			glBindTexture (GL_TEXTURE_2D, icon->iLabelTexture);
			
			double w = round (icon->iTextWidth * (u1 - u0)), h = icon->iTextHeight;
			if (u1 < .99)  // draw with an alpha gradation on the last part.
			{
				glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
				glBegin(GL_QUAD_STRIP);
				
				double a = .75; // 3/4 plain, 1/4 gradation
				a = (double) (floor ((-.5+a)*w)) / w + .5;
				glColor4f (1., 1., 1., 1.);
				glTexCoord2f(u0, 0); glVertex3f(-.5*w,  .5*h, 0.);  // top left
				glTexCoord2f(u0, 1); glVertex3f(-.5*w, -.5*h, 0.);  // bottom left
				
				glTexCoord2f(u1*a, 0); glVertex3f(  ((-.5+a)*w),  .5*h, 0.);  // top middle
				glTexCoord2f(u1*a, 1); glVertex3f(  ((-.5+a)*w),  -.5*h, 0.);  // bottom middle
				
				glColor4f (1., 1., 1., 0.);
				
				glTexCoord2f(u1, 0); glVertex3f( .5*w,  .5*h, 0.);  // top right
				glTexCoord2f(u1, 1); glVertex3f( .5*w, -.5*h, 0.);  // bottom right
				
				glEnd();
			}
			else
			{
				_cairo_dock_apply_current_texture_at_size (w, h);
			}
			_cairo_dock_disable_texture ();
			_cairo_dock_set_alpha (1.);
			
			glPopMatrix ();
		}
		
		ic = cairo_dock_get_next_element (ic, pDock->icons);
	}
	while (ic != pFirstDrawnElement);
	glDisable (GL_SCISSOR_TEST);
}


void cd_rendering_free_slide_data (CairoDock *pDock)
{
	CDSlideData *pData = pDock->pRendererData;
	if (pData != NULL)
	{
		cairo_dock_remove_notification_func_on_object (CAIRO_CONTAINER (pDock), NOTIFICATION_SCROLL_ICON, (CairoDockNotificationFunc) _cd_slide_on_scroll, NULL);
		cairo_dock_remove_notification_func_on_object (CAIRO_CONTAINER (pDock), NOTIFICATION_CLICK_ICON, (CairoDockNotificationFunc) _cd_slide_on_click, NULL);
		cairo_dock_remove_notification_func_on_object (CAIRO_CONTAINER (pDock), NOTIFICATION_MOUSE_MOVED, (CairoDockNotificationFunc) _cd_slide_on_mouse_moved, NULL);
		g_signal_handler_disconnect (pDock->container.pWidget, pData->iSidPressEvent);
		g_signal_handler_disconnect (pDock->container.pWidget, pData->iSidReleaseEvent);
		
		g_free (pData);
		pDock->pRendererData = NULL;
	}
}


void cd_rendering_set_subdock_position_slide (Icon *pPointedIcon, CairoDock *pDock)
{
	CairoDock *pSubDock = pPointedIcon->pSubDock;
	
	CDSlideData *pData = pSubDock->pRendererData;
	g_return_if_fail (pData != NULL);
	
	int iX = pPointedIcon->fXAtRest - (pDock->fFlatDockWidth - pDock->iMaxDockWidth) / 2 + pPointedIcon->fWidth / 2 + (pDock->iOffsetForExtend * (pDock->fAlign - .5) * 2);
	if (pSubDock->container.bIsHorizontal == pDock->container.bIsHorizontal)
	{
		pSubDock->fAlign = 0.5;
		pSubDock->iGapX = iX + pDock->container.iWindowPositionX - (pDock->container.bIsHorizontal ? pDock->iScreenOffsetX : pDock->iScreenOffsetY) - g_desktopGeometry.iScreenWidth[pDock->container.bIsHorizontal] / 2;  // ici les sous-dock ont un alignement egal a 0.5
		pSubDock->iGapY = pDock->iGapY + pDock->iActiveHeight;
	}
	else
	{
		pSubDock->fAlign = (pDock->container.bDirectionUp ? 1 : 0);
		pSubDock->iGapX = (pDock->iGapY + pDock->iActiveHeight) * (pDock->container.bDirectionUp ? -1 : 1);
		if (pDock->container.bDirectionUp)
			pSubDock->iGapY = g_desktopGeometry.iScreenWidth[pDock->container.bIsHorizontal] - (iX + pDock->container.iWindowPositionX - (pDock->container.bIsHorizontal ? pDock->iScreenOffsetX : pDock->iScreenOffsetY)) - pSubDock->iMaxDockHeight / 2;  // les sous-dock ont un alignement egal a 1.
		else
			pSubDock->iGapY = iX + pDock->container.iWindowPositionX - pSubDock->iMaxDockHeight / 2;  // les sous-dock ont un alignement egal a 0.
	}
	
	pData->iDeltaIconX = MIN (0, iX + pDock->container.iWindowPositionX - pSubDock->iMaxDockWidth/2);
	if (pData->iDeltaIconX == 0)
		pData->iDeltaIconX = MAX (0, iX + pDock->container.iWindowPositionX + pSubDock->iMaxDockWidth/2 - g_desktopGeometry.iScreenWidth[pDock->container.bIsHorizontal]);
	//g_print ("iDeltaIconX: %d\n", pData->iDeltaIconX);
	
	if (pData->iDeltaIconX != 0)  // il y'a un decalage, on va limiter la pente du cote le plus court de la pointe a 30 degres.
	{
		pData->iArrowShift = MAX (0, fabs (pData->iDeltaIconX) - my_diapo_simple_arrowHeight * .577 - my_diapo_simple_arrowWidth/2);  // tan(30)
		if (pData->iDeltaIconX < 0)
			pData->iArrowShift = - pData->iArrowShift;
		//g_print ("iArrowShift: %d\n", pData->iArrowShift);
	}
	else
		pData->iArrowShift = 0;
}


void cd_rendering_register_diapo_simple_renderer (const gchar *cRendererName)
{
	CairoDockRenderer *pRenderer = g_new0 (CairoDockRenderer, 1);
	// interface
	pRenderer->compute_size 		= cd_rendering_calculate_max_dock_size_diapo_simple;
	pRenderer->calculate_icons 		= cd_rendering_calculate_icons_diapo_simple;
	pRenderer->render 				= cd_rendering_render_diapo_simple;
	pRenderer->render_optimized 	= NULL;
	pRenderer->render_opengl 		= cd_rendering_render_diapo_simple_opengl;
	pRenderer->free_data 			= cd_rendering_free_slide_data;
	pRenderer->set_subdock_position = cd_rendering_set_subdock_position_slide;
	// parametres
	pRenderer->cReadmeFilePath 		= g_strdup (MY_APPLET_SHARE_DATA_DIR"/readme-diapo-simple-view");
	pRenderer->cPreviewFilePath 	= g_strdup (MY_APPLET_SHARE_DATA_DIR"/preview-diapo-simple.jpg");
	pRenderer->bUseReflect 			= FALSE;  // pas de reflections
	pRenderer->cDisplayedName 		= D_(cRendererName);
	
	cairo_dock_register_renderer (cRendererName, pRenderer);
}
