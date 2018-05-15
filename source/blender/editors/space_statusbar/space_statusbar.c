/*
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * ***** END GPL LICENSE BLOCK *****
 */

/** \file blender/editors/space_statusbar/space_statusbar.c
 *  \ingroup spstatusbar
 */


#include <string.h>
#include <stdio.h>

#include "MEM_guardedalloc.h"

#include "BLI_blenlib.h"

#include "BKE_context.h"
#include "BKE_screen.h"

#include "ED_screen.h"
#include "ED_space_api.h"

#include "UI_view2d.h"

#include "WM_api.h"
#include "WM_types.h"


/* ******************** default callbacks for statusbar space ********************  */

static SpaceLink *statusbar_new(const ScrArea *UNUSED(area), const Scene *UNUSED(scene))
{
	ARegion *ar;
	SpaceStatusBar *sstatusbar;

	sstatusbar = MEM_callocN(sizeof(*sstatusbar), "init statusbar");
	sstatusbar->spacetype = SPACE_STATUSBAR;

	/* main regions */
	ar = MEM_callocN(sizeof(*ar), "header region for statusbar");
	BLI_addtail(&sstatusbar->regionbase, ar);
	ar->regiontype = RGN_TYPE_HEADER;

	return (SpaceLink *)sstatusbar;
}

/* not spacelink itself */
static void statusbar_free(SpaceLink *UNUSED(sl))
{

}


/* spacetype; init callback */
static void statusbar_init(struct wmWindowManager *UNUSED(wm), ScrArea *UNUSED(sa))
{

}

static SpaceLink *statusbar_duplicate(SpaceLink *sl)
{
	SpaceStatusBar *sstatusbarn = MEM_dupallocN(sl);

	/* clear or remove stuff from old */

	return (SpaceLink *)sstatusbarn;
}



/* add handlers, stuff you only do once or on area/region changes */
static void statusbar_header_region_init(wmWindowManager *wm, ARegion *region)
{
	wmKeyMap *keymap;

	UI_view2d_region_reinit(&region->v2d, V2D_COMMONVIEW_HEADER, region->winx, region->winy);

	keymap = WM_keymap_find(wm->defaultconf, "View2D Buttons List", 0, 0);
	WM_event_add_keymap_handler(&region->handlers, keymap);
}

static void statusbar_operatortypes(void)
{

}

static void statusbar_keymap(struct wmKeyConfig *UNUSED(keyconf))
{

}

static void statusbar_header_region_listener(bScreen *UNUSED(sc), ScrArea *UNUSED(sa), ARegion *ar,
                                           wmNotifier *wmn, const Scene *UNUSED(scene))
{
	/* context changes */
	switch (wmn->category) {
	}
	/* TODO */
	ED_region_tag_redraw(ar);
}

/* only called once, from space/spacetypes.c */
void ED_spacetype_statusbar(void)
{
	SpaceType *st = MEM_callocN(sizeof(*st), "spacetype statusbar");
	ARegionType *art;

	st->spaceid = SPACE_STATUSBAR;
	strncpy(st->name, "Status Bar", BKE_ST_MAXNAME);

	st->new = statusbar_new;
	st->free = statusbar_free;
	st->init = statusbar_init;
	st->duplicate = statusbar_duplicate;
	st->operatortypes = statusbar_operatortypes;
	st->keymap = statusbar_keymap;

	/* regions: header window */
	art = MEM_callocN(sizeof(*art), "spacetype statusbar header region");
	art->regionid = RGN_TYPE_HEADER;
	art->init = statusbar_header_region_init;
	art->layout = ED_region_header_layout;
	art->draw = ED_region_header_draw;
	art->listener = statusbar_header_region_listener;
	art->keymapflag = ED_KEYMAP_UI | ED_KEYMAP_VIEW2D | ED_KEYMAP_HEADER;
	BLI_addhead(&st->regiontypes, art);

	BKE_spacetype_register(st);
}
