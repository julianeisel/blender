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

/** \file blender/editors/space_layers/layers_ops.c
 *  \ingroup splayers
 */

#include <stdlib.h>

#include "BKE_context.h"
#include "BKE_depsgraph.h"
#include "BKE_layer.h"
#include "BKE_object.h"

#include "BLI_alloca.h"
#include "BLI_compiler_attrs.h"
#include "BLI_utildefines.h"
#include "BLI_ghash.h"
#include "BLI_listbase.h"

#include "DNA_windowmanager_types.h"

#include "ED_object.h"
#include "ED_screen.h"

#include "RNA_access.h"
#include "RNA_define.h"

#include "WM_api.h"
#include "WM_types.h"

#include "layers_intern.h" /* own include */


static int layer_add_invoke(bContext *C, wmOperator *UNUSED(op), const wmEvent *UNUSED(event))
{
	SpaceLayers *slayer = CTX_wm_space_layers(C);
	LayerTreeItem *new_item = NULL;

	if (slayer->act_tree->type == LAYER_TREETYPE_OBJECT) {
		new_item = layers_object_add(slayer->act_tree, NULL);
	}
	else {
		BLI_assert(0);
	}
	layers_tile_add(slayer, new_item);

	WM_event_add_notifier(C, NC_SCENE | ND_LAYER, NULL);

	return OPERATOR_FINISHED;
}

static void LAYERS_OT_layer_add(wmOperatorType *ot)
{
	/* identifiers */
	ot->name = "Add Layer";
	ot->idname = "LAYERS_OT_layer_add";
	ot->description = "Add a new layer to the layer list";

	/* api callbacks */
	ot->invoke = layer_add_invoke;
	ot->poll = ED_operator_layers_active;

	/* flags */
	ot->flag = OPTYPE_REGISTER | OPTYPE_UNDO;
}

enum {
	LAYER_DELETE_LAYER_ONLY,
	LAYER_DELETE_WITH_CONTENT,
};

static void layers_remove_layer_objects(bContext *C, SpaceLayers *slayer, LayerTreeItem *litem)
{
	LayerTypeObject *oblayer = (LayerTypeObject *)litem;
	struct Main *bmain = CTX_data_main(C);
	Scene *scene = CTX_data_scene(C);

	ListBase remlist = {NULL};
	const unsigned int tot_bases = BLI_ghash_size(oblayer->basehash);
	LinkData *base_links = BLI_array_alloca(base_links, tot_bases);
	GHashIterator gh_iter;
	int i;

	GHASH_ITER_INDEX(gh_iter, oblayer->basehash, i) {
		Base *base = BLI_ghashIterator_getValue(&gh_iter);
		base_links[i].data = base;
		BLI_addhead(&remlist, &base_links[i]);
	}
	BLI_assert(tot_bases == i);

	for (LinkData *base_link = remlist.first, *baselink_next; base_link; base_link = baselink_next) {
		Base *base = base_link->data;
		/* remove object from other layers */
		/* XXX bases could have info about the layers they are in, then
		 * we could avoid loop in loop and do this all on BKE_ level */
		GHASH_ITER(gh_iter, slayer->tiles) {
			BKE_objectlayer_base_unassign(base, BLI_ghashIterator_getKey(&gh_iter));
		}
		ED_base_object_free_and_unlink(bmain, scene, base);

		baselink_next = base_link->next;
		BLI_remlink(&remlist, base_link);
	}
	BLI_assert(BLI_listbase_is_empty(&remlist));

	DAG_relations_tag_update(bmain);
}

static int layer_remove_exec(bContext *C, wmOperator *op)
{
	SpaceLayers *slayer = CTX_wm_space_layers(C);
	ListBase remlist = {NULL};
	const int rem_type = RNA_enum_get(op->ptr, "type");

	/* First iterate over tiles. Ghash iterator doesn't allow removing items
	 * while iterating, so temporarily store selected items in a list */
	GHashIterator gh_iter;
	GHASH_ITER(gh_iter, slayer->tiles) {
		LayerTile *tile = BLI_ghashIterator_getValue(&gh_iter);
		if (tile->flag & LAYERTILE_SELECTED) {
			LinkData *tile_link = BLI_genericNodeN(tile);
			BLI_addhead(&remlist, tile_link);
		}
	}
	/* Now, delete all items in the list (and content if needed). */
	for (LinkData *tile_link = remlist.first, *next_link; tile_link; tile_link = next_link) {
		LayerTile *tile = tile_link->data;
		LayerTreeItem *litem = tile->litem;

		/* delete layer content */
		if (rem_type == LAYER_DELETE_WITH_CONTENT) {
			switch (slayer->act_tree->type) {
				case LAYER_TREETYPE_OBJECT:
					layers_remove_layer_objects(C, slayer, litem);
					break;
			}
		}

		layers_tile_remove(slayer, tile, true);
		BKE_layeritem_remove(litem, true);

		next_link = tile_link->next;
		BLI_freelinkN(&remlist, tile_link);
	}
	BLI_assert(BLI_listbase_is_empty(&remlist));

	WM_event_add_notifier(C, NC_SCENE | ND_LAYER, NULL);

	return OPERATOR_FINISHED;
}

static void LAYERS_OT_remove(wmOperatorType *ot)
{
	static EnumPropertyItem prop_layers_delete_types[] = {
		{LAYER_DELETE_LAYER_ONLY,   "LAYER_ONLY",   0, "Only Layer",   "Delete layer(s), keept its content"},
		{LAYER_DELETE_WITH_CONTENT, "WITH_CONTENT", 0, "With Content", "Delete layer(s) and its content"},
		{0, NULL, 0, NULL, NULL}
	};

	/* identifiers */
	ot->name = "Remove Layers";
	ot->idname = "LAYERS_OT_remove";
	ot->description = "Remove selected layers";

	/* api callbacks */
	ot->invoke = WM_menu_invoke;
	ot->exec = layer_remove_exec;
	ot->poll = ED_operator_layers_active;

	/* flags */
	ot->flag = OPTYPE_REGISTER | OPTYPE_UNDO;

	ot->prop = RNA_def_enum(ot->srna, "type", prop_layers_delete_types, LAYER_DELETE_LAYER_ONLY,
	                        "Type", "Method used for deleting layers");
}

typedef struct {
	SpaceLayers *slayer;
	LayerTreeItem *group;
} GroupAddSelectedData;

static bool layer_group_add_selected_cb(LayerTreeItem *litem, void *customdata)
{
	GroupAddSelectedData *gadata = customdata;
	LayerTile *tile = BLI_ghash_lookup(gadata->slayer->tiles, litem);

	if (tile->flag & LAYERTILE_SELECTED) {
		BKE_layeritem_group_assign(gadata->group, litem);
	}

	return true;
}

static int layer_group_add_invoke(bContext *C, wmOperator *UNUSED(op), const wmEvent *UNUSED(event))
{
	Scene *scene = CTX_data_scene(C);
	SpaceLayers *slayer = CTX_wm_space_layers(C);

	LayerTreeItem *new_group = layers_group_add(scene->object_layers, NULL);
	layers_tile_add(slayer, new_group);

	/* Add selected items to group */
	GroupAddSelectedData gadata = {slayer, new_group};
	BKE_layertree_iterate(slayer->act_tree, layer_group_add_selected_cb, &gadata, true);

	WM_event_add_notifier(C, NC_SCENE | ND_LAYER, NULL);
	return OPERATOR_FINISHED;
}

static void LAYERS_OT_group_add(wmOperatorType *ot)
{
	/* identifiers */
	ot->name = "Add Layer Group";
	ot->idname = "LAYERS_OT_group_add";
	ot->description = "Add a new layer group to the layer list and place selected elements inside of it";

	/* api callbacks */
	ot->invoke = layer_group_add_invoke;
	ot->poll = ED_operator_layers_active;

	/* flags */
	ot->flag = OPTYPE_REGISTER | OPTYPE_UNDO;
}

static int layer_rename_invoke(bContext *C, wmOperator *UNUSED(op), const wmEvent *event)
{
	SpaceLayers *slayer = CTX_wm_space_layers(C);
	ARegion *ar = CTX_wm_region(C);
	LayerTile *tile = layers_tile_find_at_coordinate(slayer, ar, event->mval, NULL);
	if (tile) {
		tile->flag |= LAYERTILE_RENAME;

		ED_region_tag_redraw(ar);
		return OPERATOR_FINISHED;
	}
	return OPERATOR_CANCELLED;
}

static void LAYERS_OT_layer_rename(wmOperatorType *ot)
{
	/* identifiers */
	ot->name = "Rename Layer";
	ot->idname = "LAYERS_OT_layer_rename";
	ot->description = "Rename the layer under the cursor";

	/* api callbacks */
	ot->invoke = layer_rename_invoke;
	ot->poll = ED_operator_layers_active;

	/* flags */
	ot->flag = OPTYPE_REGISTER | OPTYPE_UNDO;
}

BLI_INLINE void layer_selection_set(SpaceLayers *slayer, LayerTile *tile, const int tile_idx, const bool enable)
{
	if (enable) {
		(tile->flag |= LAYERTILE_SELECTED);
		slayer->last_selected = tile_idx;
	}
	else {
		tile->flag &= ~LAYERTILE_SELECTED;
	}
}

typedef struct LayerSelectData {
	/* input variables */
	SpaceLayers *slayer;
	int from, to; /* from must be smaller than two, or both -1 */
	bool enable;

	/* helper variable */
	int idx;
} LayerSelectData;

static bool layer_select_cb(LayerTreeItem *litem, void *customdata)
{
	LayerSelectData *sdata = customdata;
	LayerTile *tile = BLI_ghash_lookup(sdata->slayer->tiles, litem);

	BLI_assert((sdata->from == -1 && sdata->to == -1) || sdata->from < sdata->to);

	if ((sdata->from == -1) || (sdata->idx >= sdata->from && sdata->idx <= sdata->to)) {
		layer_selection_set(sdata->slayer, tile, sdata->idx, sdata->enable);
	}
	sdata->idx++;

	return true;
}

/**
 * Change the selection state of all layer tiles.
 * \param enable: If true, tiles are selected, else they are deselected.
 */
static void layers_selection_set_all(SpaceLayers *slayer, const bool enable)
{
	LayerSelectData sdata = {slayer, -1, -1, enable};
	BKE_layertree_iterate(slayer->act_tree, layer_select_cb, &sdata, true);
}

/**
 * Select everything within the range of \a from to \a to.
 * \return if anything got selected. Nothing is selected if from == to or one of them is < 0.
 */
static bool layers_select_fill(SpaceLayers *slayer, const int from, const int to)
{
	const int min = MIN2(from, to);
	const int max = MAX2(from, to);
	LayerSelectData sdata = {slayer, min, max, true};

	if (min < 0 || min == max)
		return false;

	BKE_layertree_iterate(slayer->act_tree, layer_select_cb, &sdata, true);

	return true;
}

static int layer_select_invoke(bContext *C, wmOperator *op, const wmEvent *event)
{
	SpaceLayers *slayer = CTX_wm_space_layers(C);
	ARegion *ar = CTX_wm_region(C);
	const bool extend = RNA_boolean_get(op->ptr, "extend");
	const bool deselect = RNA_boolean_get(op->ptr, "deselect");
	const bool toggle = RNA_boolean_get(op->ptr, "toggle");
	const bool fill = RNA_boolean_get(op->ptr, "fill");

	int tile_idx;
	LayerTile *tile = layers_tile_find_at_coordinate(slayer, ar, event->mval, &tile_idx);

	/* little helper for setting/unsetting selection flag */
#define TILE_SET_SELECTION(enable) layer_selection_set(slayer, tile, tile_idx, enable);

	if (tile) {
		/* deselect all, but only if extend, deselect and toggle are false */
		if (((extend == deselect) == toggle) == false) {
			layers_selection_set_all(slayer, false);
		}
		if (extend) {
			if (fill && layers_select_fill(slayer, slayer->last_selected, tile_idx)) {
				/* skip */
			}
			else {
				TILE_SET_SELECTION(true);
			}
		}
		else if (deselect) {
			TILE_SET_SELECTION(false);
		}
		else {
			if (!(tile->flag & LAYERTILE_SELECTED)) {
				TILE_SET_SELECTION(true);
			}
			else if (toggle) {
				TILE_SET_SELECTION(false);
			}
		}

#undef TILE_SET_SELECTION

		ED_region_tag_redraw(ar);
		return OPERATOR_FINISHED;
	}
	return OPERATOR_CANCELLED;
}

static void LAYERS_OT_select(wmOperatorType *ot)
{
	/* identifiers */
	ot->name = "Select Layer";
	ot->idname = "LAYERS_OT_select";
	ot->description = "Select/activate the layer under the cursor";

	/* api callbacks */
	ot->invoke = layer_select_invoke;
	ot->poll = ED_operator_layers_active;

	WM_operator_properties_mouse_select(ot);
	PropertyRNA *prop = RNA_def_boolean(ot->srna, "fill", false, "Fill", "Select everything beginning "
	                                                                     "with the last selection");
	RNA_def_property_flag(prop, PROP_SKIP_SAVE);
}

static int layer_select_all_toggle_invoke(bContext *C, wmOperator *UNUSED(op), const wmEvent *UNUSED(event))
{
	SpaceLayers *slayer = CTX_wm_space_layers(C);

	/* if a tile was found we deselect all, else we select all */
	layers_selection_set_all(slayer, !layers_any_selected(slayer, slayer->act_tree));
	ED_region_tag_redraw(CTX_wm_region(C));

	return OPERATOR_FINISHED;
}

static void LAYERS_OT_select_all_toggle(wmOperatorType *ot)
{
	/* identifiers */
	ot->name = "(De)select All Layers";
	ot->idname = "LAYERS_OT_select_all_toggle";
	ot->description = "Select or deselect all layers";

	/* api callbacks */
	ot->invoke = layer_select_all_toggle_invoke;
	ot->poll = ED_operator_layers_active;
}

static int layer_objects_assign_invoke(bContext *C, wmOperator *UNUSED(op), const wmEvent *UNUSED(event))
{
	Scene *scene = CTX_data_scene(C);
	SpaceLayers *slayer = CTX_wm_space_layers(C);
	int tot_sel = 0;

	/* Count selected tiles (could be cached to avoid extra loop). */
	GHashIterator gh_iter;
	GHASH_ITER(gh_iter, slayer->tiles) {
		LayerTile *tile = BLI_ghashIterator_getValue(&gh_iter);
		if (tile->flag & LAYERTILE_SELECTED) {
			tot_sel++;
		}
	}
	/* Collect selected items in an array */
	LayerTreeItem **litems = BLI_array_alloca(litems, tot_sel);
	int i = 0;
	GHASH_ITER(gh_iter, slayer->tiles) {
		LayerTile *tile = BLI_ghashIterator_getValue(&gh_iter);
		if (tile->flag & LAYERTILE_SELECTED) {
			litems[i] = tile->litem;
			i++;
		}
	}

	for (Base *base = scene->base.first; base; base = base->next) {
		if (base->flag & SELECT) {
			/* Only iterate over selected items */
			for (i = 0; i < tot_sel; i++) {
				BKE_objectlayer_base_assign(base, litems[i]);
			}
		}
	}

	WM_event_add_notifier(C, NC_SCENE | ND_LAYER_CONTENT, NULL);

	return OPERATOR_FINISHED;
}

static void LAYERS_OT_objects_assign(wmOperatorType *ot)
{
	/* identifiers */
	ot->name = "Assign Objects";
	ot->idname = "LAYERS_OT_objects_assign";
	ot->description = "Assign selected objects to selected layers";

	/* api callbacks */
	ot->invoke = layer_objects_assign_invoke;
	ot->poll = ED_operator_layers_active;
}


/* ************************** registration - operator types **********************************/

void layers_operatortypes(void)
{
	/* organization */
	WM_operatortype_append(LAYERS_OT_layer_add);
	WM_operatortype_append(LAYERS_OT_group_add);
	WM_operatortype_append(LAYERS_OT_remove);
	WM_operatortype_append(LAYERS_OT_layer_rename);

	/* states (activating selecting, highlighting) */
	WM_operatortype_append(LAYERS_OT_select);
	WM_operatortype_append(LAYERS_OT_select_all_toggle);

	WM_operatortype_append(LAYERS_OT_objects_assign);
}

void layers_keymap(wmKeyConfig *keyconf)
{
	wmKeyMap *keymap = WM_keymap_find(keyconf, "Layer Manager", SPACE_LAYERS, 0);
	wmKeyMapItem *kmi;

	/* selection */
	WM_keymap_add_item(keymap, "LAYERS_OT_select", LEFTMOUSE, KM_CLICK, 0, 0);
	kmi = WM_keymap_add_item(keymap, "LAYERS_OT_select", LEFTMOUSE, KM_CLICK, KM_SHIFT, 0);
	RNA_boolean_set(kmi->ptr, "toggle", true);
	kmi = WM_keymap_add_item(keymap, "LAYERS_OT_select", LEFTMOUSE, KM_CLICK, KM_CTRL | KM_SHIFT, 0);
	RNA_boolean_set(kmi->ptr, "extend", true);
	RNA_boolean_set(kmi->ptr, "fill", true);
	WM_keymap_add_item(keymap, "LAYERS_OT_select_all_toggle", AKEY, KM_PRESS, 0, 0);

	WM_keymap_add_item(keymap, "LAYERS_OT_layer_rename", LEFTMOUSE, KM_DBL_CLICK, 0, 0);
	WM_keymap_add_item(keymap, "LAYERS_OT_layer_rename", LEFTMOUSE, KM_PRESS, KM_CTRL, 0);

	WM_keymap_add_item(keymap, "LAYERS_OT_remove", XKEY, KM_PRESS, 0, 0);
	WM_keymap_add_item(keymap, "LAYERS_OT_remove", DELKEY, KM_PRESS, 0, 0);

	WM_keymap_add_item(keymap, "LAYERS_OT_layer_add", NKEY, KM_PRESS, KM_CTRL, 0);
	WM_keymap_add_item(keymap, "LAYERS_OT_group_add", GKEY, KM_PRESS, KM_CTRL, 0);

	WM_keymap_add_item(keymap, "LAYERS_OT_objects_assign", MKEY, KM_PRESS, 0, 0);
}
