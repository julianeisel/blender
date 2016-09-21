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
 * The Original Code is Copyright (C) 2014 Blender Foundation.
 * All rights reserved.
 *
 * Contributor(s): Blender Foundation
 *
 * ***** END GPL LICENSE BLOCK *****
 */

/** \file blender/windowmanager/manipulators/intern/wm_manipulator.c
 *  \ingroup wm
 */

#include "BKE_context.h"

#include "BLI_listbase.h"
#include "BLI_math.h"
#include "BLI_path_util.h"
#include "BLI_string.h"

#include "DNA_manipulator_types.h"

#include "ED_screen.h"
#include "ED_view3d.h"

#include "GL/glew.h"

#include "MEM_guardedalloc.h"

#include "RNA_access.h"

#include "WM_api.h"
#include "WM_types.h"

/* own includes */
#include "wm_manipulator_wmapi.h"
#include "wm_manipulator_intern.h"

/**
 * Main draw call for ManipulatorGeometryInfo data
 */
void wm_manipulator_geometryinfo_draw(ManipulatorGeometryInfo *info, const bool select)
{
	GLuint buf[3];

	const bool use_lighting = !select && ((U.manipulator_flag & V3D_SHADED_MANIPULATORS) != 0);

	if (use_lighting)
		glGenBuffers(3, buf);
	else
		glGenBuffers(2, buf);

	glEnableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, buf[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * info->nverts, info->verts, GL_STATIC_DRAW);
	glVertexPointer(3, GL_FLOAT, 0, NULL);

	if (use_lighting) {
		glEnableClientState(GL_NORMAL_ARRAY);
		glBindBuffer(GL_ARRAY_BUFFER, buf[2]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * info->nverts, info->normals, GL_STATIC_DRAW);
		glNormalPointer(GL_FLOAT, 0, NULL);
		glShadeModel(GL_SMOOTH);
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * (3 * info->ntris), info->indices, GL_STATIC_DRAW);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	glDrawElements(GL_TRIANGLES, info->ntris * 3, GL_UNSIGNED_SHORT, NULL);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glDisableClientState(GL_VERTEX_ARRAY);

	if (use_lighting) {
		glDisableClientState(GL_NORMAL_ARRAY);
		glShadeModel(GL_FLAT);
		glDeleteBuffers(3, buf);
	}
	else {
		glDeleteBuffers(2, buf);
	}
}

/* Still unused */
wmManipulator *WM_manipulator_new(
        void (*draw)(const bContext *C, wmManipulator *customdata),
        void (*render_3d_intersection)(const bContext *C, wmManipulator *customdata, int selectionbase),
        int  (*intersect)(bContext *C, const wmEvent *event, wmManipulator *manipulator),
        int  (*handler)(bContext *C, const wmEvent *event, wmManipulator *manipulator, const int flag))
{
	wmManipulator *manipulator = MEM_callocN(sizeof(wmManipulator), "manipulator");

	manipulator->draw = draw;
	manipulator->handler = handler;
	manipulator->intersect = intersect;
	manipulator->render_3d_intersection = render_3d_intersection;

	/* XXX */
	fix_linking_manipulator_arrow();
	fix_linking_manipulator_arrow2d();
	fix_linking_manipulator_cage();
	fix_linking_manipulator_dial();
	fix_linking_manipulator_facemap();
	fix_linking_manipulator_primitive();

	return manipulator;
}

/**
 * Assign an idname that is unique in \a mgroup to \a manipulator.
 *
 * \param rawname: Name used as basis to define final unique idname.
 */
static void manipulator_unique_idname_set(wmManipulatorGroup *mgroup, wmManipulator *manipulator, const char *rawname)
{
	if (mgroup->type->idname[0]) {
		BLI_snprintf(manipulator->idname, sizeof(manipulator->idname), "%s_%s", mgroup->type->idname, rawname);
	}
	else {
		BLI_strncpy(manipulator->idname, rawname, sizeof(manipulator->idname));
	}

	/* ensure name is unique, append '.001', '.002', etc if not */
	BLI_uniquename(&mgroup->manipulators, manipulator, "Manipulator", '.',
	               offsetof(wmManipulator, idname), sizeof(manipulator->idname));
}

/**
 * Register \a manipulator.
 *
 * \param name  name used to create a unique idname for \a manipulator in \a mgroup
 */
bool wm_manipulator_register(wmManipulatorGroup *mgroup, wmManipulator *manipulator, const char *name)
{
	const float col_default[4] = {1.0f, 1.0f, 1.0f, 1.0f};

	manipulator_unique_idname_set(mgroup, manipulator, name);

	manipulator->user_scale = 1.0f;
	manipulator->line_width = 1.0f;

	/* defaults */
	copy_v4_v4(manipulator->col, col_default);
	copy_v4_v4(manipulator->col_hi, col_default);

	/* create at least one property for interaction */
	if (manipulator->max_prop == 0) {
		manipulator->max_prop = 1;
	}

	manipulator->props = MEM_callocN(sizeof(PropertyRNA *) * manipulator->max_prop, "manipulator->props");
	manipulator->ptr = MEM_callocN(sizeof(PointerRNA) * manipulator->max_prop, "manipulator->ptr");

	BLI_addtail(&mgroup->manipulators, manipulator);
	return true;
}

/**
 * Free \a manipulator and unlink from \a manipulatorlist.
 * \a manipulatorlist is allowed to be NULL.
 */
void WM_manipulator_delete(ListBase *manipulatorlist, wmManipulatorMap *mmap, wmManipulator *manipulator, bContext *C)
{
	if (manipulator->flag & WM_MANIPULATOR_HIGHLIGHT) {
		wm_manipulatormap_set_highlighted_manipulator(mmap, C, NULL, 0);
	}
	if (manipulator->flag & WM_MANIPULATOR_ACTIVE) {
		wm_manipulatormap_set_active_manipulator(mmap, C, NULL, NULL);
	}
	if (manipulator->flag & WM_MANIPULATOR_SELECTED) {
		wm_manipulator_deselect(mmap, manipulator);
	}

	if (manipulator->opptr.data) {
		WM_operator_properties_free(&manipulator->opptr);
	}
	MEM_freeN(manipulator->props);
	MEM_freeN(manipulator->ptr);

	if (manipulatorlist)
		BLI_remlink(manipulatorlist, manipulator);
	MEM_freeN(manipulator);
}

wmManipulatorGroup *wm_manipulator_group_find(const wmManipulatorMap *mmap, wmManipulator *manipulator)
{
	for (wmManipulatorGroup *mgroup = mmap->manipulator_groups.first; mgroup; mgroup = mgroup->next) {
		for (wmManipulator *man_iter = mgroup->manipulators.first; man_iter; man_iter = man_iter->next) {
			if (man_iter == manipulator) {
				return mgroup;
			}
		}
	}

	return NULL;
}


/* -------------------------------------------------------------------- */
/** \name Manipulator Creation API
 *
 * API for defining data on manipulator creation.
 *
 * \{ */

void WM_manipulator_set_property(wmManipulator *manipulator, const int slot, PointerRNA *ptr, const char *propname)
{
	if (slot < 0 || slot >= manipulator->max_prop) {
		fprintf(stderr, "invalid index %d when binding property for manipulator type %s\n", slot, manipulator->idname);
		return;
	}

	/* if manipulator evokes an operator we cannot use it for property manipulation */
	manipulator->opname = NULL;
	manipulator->ptr[slot] = *ptr;
	manipulator->props[slot] = RNA_struct_find_property(ptr, propname);

	if (manipulator->prop_data_update)
		manipulator->prop_data_update(manipulator, slot);
}

PointerRNA *WM_manipulator_set_operator(wmManipulator *manipulator, const char *opname)
{
	wmOperatorType *ot = WM_operatortype_find(opname, 0);

	if (ot) {
		manipulator->opname = opname;

		if (manipulator->opptr.data) {
			WM_operator_properties_free(&manipulator->opptr);
		}
		WM_operator_properties_create_ptr(&manipulator->opptr, ot);

		return &manipulator->opptr;
	}
	else {
		fprintf(stderr, "Error binding operator to manipulator: operator %s not found!\n", opname);
	}

	return NULL;
}

/**
 * \brief Set manipulator select callback.
 *
 * Callback is called when manipulator gets selected/deselected.
 */
void WM_manipulator_set_func_select(wmManipulator *manipulator, wmManipulatorSelectFunc select)
{
	manipulator->flag |= WM_MANIPULATOR_SELECTABLE;
	manipulator->select = select;
}

void WM_manipulator_set_origin(wmManipulator *manipulator, const float origin[3])
{
	copy_v3_v3(manipulator->origin, origin);
}

void WM_manipulator_set_offset(wmManipulator *manipulator, const float offset[3])
{
	copy_v3_v3(manipulator->offset, offset);
}

void WM_manipulator_set_flag(wmManipulator *manipulator, const int flag, const bool enable)
{
	if (enable) {
		manipulator->flag |= flag;
	}
	else {
		manipulator->flag &= ~flag;
	}
}

void WM_manipulator_set_scale(wmManipulator *manipulator, const float scale)
{
	manipulator->user_scale = scale;
}

void WM_manipulator_set_line_width(wmManipulator *manipulator, const float line_width)
{
	manipulator->line_width = line_width;
}

/**
 * Set manipulator rgba colors.
 *
 * \param col  Normal state color.
 * \param col_hi  Highlighted state color.
 */
void WM_manipulator_set_colors(wmManipulator *manipulator, const float col[4], const float col_hi[4])
{
	copy_v4_v4(manipulator->col, col);
	copy_v4_v4(manipulator->col_hi, col_hi);
}

/** \} */ // Manipulator Creation API


/* -------------------------------------------------------------------- */

/**
 * Remove \a manipulator from selection.
 * Reallocates memory for selected manipulators so better not call for selecting multiple ones.
 *
 * \return if the selection has changed.
 */
bool wm_manipulator_deselect(wmManipulatorMap *mmap, wmManipulator *manipulator)
{
	if (!mmap->mmap_context.selected_manipulator)
		return false;

	wmManipulator ***sel = &mmap->mmap_context.selected_manipulator;
	int *tot_selected = &mmap->mmap_context.tot_selected;
	bool changed = false;

	/* caller should check! */
	BLI_assert(manipulator->flag & WM_MANIPULATOR_SELECTED);

	/* remove manipulator from selected_manipulators array */
	for (int i = 0; i < (*tot_selected); i++) {
		if ((*sel)[i] == manipulator) {
			for (int j = i; j < ((*tot_selected) - 1); j++) {
				(*sel)[j] = (*sel)[j + 1];
			}
			changed = true;
			break;
		}
	}

	/* update array data */
	if ((*tot_selected) <= 1) {
		wm_manipulatormap_selected_delete(mmap);
	}
	else {
		*sel = MEM_reallocN(*sel, sizeof(**sel) * (*tot_selected));
		(*tot_selected)--;
	}

	manipulator->flag &= ~WM_MANIPULATOR_SELECTED;
	return changed;
}

/**
 * Add \a manipulator to selection.
 * Reallocates memory for selected manipulators so better not call for selecting multiple ones.
 *
 * \return if the selection has changed.
 */
bool wm_manipulator_select(bContext *C, wmManipulatorMap *mmap, wmManipulator *manipulator)
{
	wmManipulator ***sel = &mmap->mmap_context.selected_manipulator;
	int *tot_selected = &mmap->mmap_context.tot_selected;

	if (!manipulator || (manipulator->flag & WM_MANIPULATOR_SELECTED))
		return false;

	(*tot_selected)++;

	*sel = MEM_reallocN(*sel, sizeof(wmManipulator *) * (*tot_selected));
	(*sel)[(*tot_selected) - 1] = manipulator;

	manipulator->flag |= WM_MANIPULATOR_SELECTED;
	if (manipulator->select) {
		manipulator->select(C, manipulator, SEL_SELECT);
	}
	wm_manipulatormap_set_highlighted_manipulator(mmap, C, manipulator, manipulator->highlighted_part);

	return true;
}

void wm_manipulator_calculate_scale(wmManipulator *manipulator, const bContext *C)
{
	const RegionView3D *rv3d = CTX_wm_region_view3d(C);
	float scale = 1.0f;

	if (manipulator->flag & WM_MANIPULATOR_SCALE_3D) {
		if (rv3d && (U.manipulator_flag & V3D_3D_MANIPULATORS) == 0) {
			if (manipulator->get_final_position) {
				float position[3];

				manipulator->get_final_position(manipulator, position);
				scale = ED_view3d_pixel_size(rv3d, position) * (float)U.manipulator_scale;
			}
			else {
				scale = ED_view3d_pixel_size(rv3d, manipulator->origin) * (float)U.manipulator_scale;
			}
		}
		else {
			scale = U.manipulator_scale * 0.02f;
		}
	}

	manipulator->scale = scale * manipulator->user_scale;
}

void wm_manipulator_update_prop_data(wmManipulator *manipulator)
{
	/* manipulator property might have been changed, so update manipulator */
	if (manipulator->props && manipulator->prop_data_update) {
		for (int i = 0; i < manipulator->max_prop; i++) {
			if (manipulator->props[i]) {
				manipulator->prop_data_update(manipulator, i);
			}
		}
	}
}

