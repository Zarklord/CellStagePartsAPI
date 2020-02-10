/****************************************************************************
* Copyright (C) 2018, 2019 Zarklord
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

#include <EASTL\fixed_vector.h>
#include <EASTL\list.h>
#include <Spore\ModAPI.h>

namespace BetterCells {
	struct CellPartListKey {
		uint32_t unlockID;
		uint32_t partinstanceID;
	};

	extern eastl::vector<CellPartListKey> partlist;
	extern eastl::list<uint32_t> idList;
	extern int partListSize;
	extern bool wasConfiged;

	bool Initialize();
	long AttachDetours();

	static_detour(EnterCellEditor_detour, void(uint32_t*)) {};
	static_detour(LeaveCellEditor_detour, void()) {};
}