/****************************************************************************
* Copyright (C) 2018 Zarklord
*
* This file is part of BetterCells.
*
* BetterCells is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with BetterCells.  If not, see <http://www.gnu.org/licenses/>.
****************************************************************************/

#pragma once

#include <Spore\Internal.h>
#include <Spore\Detouring.h>
#include <EASTL\vector.h>
#include <EASTL\list.h>

#define DetouredMethodStaticFixed(name, returnType, newName, parameters) typedef returnType (* name##_original_t)(parameters); \
	static name##_original_t name##_original; \
	static returnType newName (parameters);

class BetterCells {
	public:
		static bool Initialize();
		static long AttachDetours();
};

#pragma DisableOptimization
DetouredMethodStaticFixed(CellEditorRemoveNewEffect, void, DetouredCellEditorRemoveNewEffect,
						  PARAMS(void));
#pragma DisableOptimization
DetouredMethodStaticFixed(CellEditorSetUnlockedPartList, void, DetouredCellEditorSetUnlockedPartList,
						  PARAMS(uint32_t * CellDataList));

struct CellPartListKey {
	uint32_t unlockID;
	uint32_t partHash;
};

static eastl::vector<CellPartListKey> partlist{};
static eastl::list<uint32_t> idList{};
static int partListSize;
static bool wasConfiged = false;