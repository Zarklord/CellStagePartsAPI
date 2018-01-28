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

// BetterCells.cpp : Defines the exported functions for the DLL application.

#include "stdafx.h"
#include "BetterCells.h"
#include <Spore\Hash.h>
#include <Spore\App\PropertyList.h>
#include <Spore\App\IPropManager.h>
#include <Spore\ResourceKey.h>


bool BetterCells::Initialize() {
	if (!partlist) {
		partlist = new eastl::vector<CellPartListKey>;
		if (!partlist) {
			return false;
		}
		partListSize = 0;
		int groupID = Hash::FNV("cell_editor_part_list");
		eastl::vector<uint32_t> instanceList = eastl::vector<uint32_t>{};
		App::IPropManager::Get()->GetAllListIDs(groupID, instanceList);
		if (instanceList.size() == 0) {
			//the bundled package wasnt included and no other mod provided a list for us to use so lets default it to whats in the executable.
			return false;
		}
		for (eastl_size_t i = 0; i < instanceList.size(); i++) {
			App::PropertyList::Pointer pProp = nullptr;  // this is just an intrusive_ptr
			App::IPropManager::Get()->GetPropertyList(instanceList[i], groupID, pProp);

			//load the two lists that define these cellpartlist's
			size_t UnlockIDCount = 0;
			int * UnlockIDList = nullptr;
			size_t PartHashCount = 0;
			ResourceKey * PartHashList = nullptr;
			App::Property::GetArrayInt32(pProp.get(), Hash::FNV("partUnlockID"), UnlockIDCount, UnlockIDList);
			App::Property::GetArrayKey(pProp.get(), Hash::FNV("partName"), PartHashCount, PartHashList);

			//lets skip over any files with unbalanced lists
			if (UnlockIDCount != PartHashCount) continue;
			partListSize += UnlockIDCount;
			for (size_t j = 0; j < UnlockIDCount; j++) {
				//until we add more unlockID's lets lock them to the avaliable values...
				int unlockID = UnlockIDList[j];
				if (unlockID > 0xB) {
					unlockID = 0xB;
				} else if (unlockID < 0x2) {
					unlockID = 0x2;
				}
				partlist->emplace_back(CellPartListKey{unlockID, PartHashList[j].mnInstanceID});
			}
		}
	}
	HANDLE hProcess = GetCurrentProcess();
	//Detouring the method failed spectacularly so instead we replace a couple bytes with our own cell part array
	//this replaces where it loads the pointer to the part array
	int toWrite = (int)partlist->data() + 0x4;
	WriteProcessMemory(hProcess, (LPVOID)GetAddress(0xE50153, 0x0, 0xE4FAC3), &toWrite, 0x4, 0);
	WriteProcessMemory(hProcess, (LPVOID)GetAddress(0xE501D2, 0x0, 0xE4FB42), &toWrite, 0x4, 0);
	//this replaces where it does a compare to see if its at the end of the array
	//NOTE: the cmp it does is a memory location based check, not a number based check.
	toWrite = (int)partlist->data() + ((sizeof(CellPartListKey) * partListSize) + 0x4);
	WriteProcessMemory(hProcess, (LPVOID)GetAddress(0xE501C4, 0x0, 0xE4FB34), &toWrite, 0x4, 0);
	WriteProcessMemory(hProcess, (LPVOID)GetAddress(0xE5020C, 0x0, 0xE4FB7C), &toWrite, 0x4, 0);
	return true;
}
