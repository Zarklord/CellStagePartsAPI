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

#include "stdafx.h"
#include "BetterCells.h"
#include <Spore\Hash.h>
#include <Spore\Properties.h>

namespace BetterCells {
	eastl::vector<CellPartListKey> partlist{};
	eastl::list<uint32_t> idList{};
	int partListSize;
	bool wasConfiged = false;

	bool Initialize() {
		partListSize = 0;
		int groupID = id("cell_editor_part_list");
		eastl::vector<uint32_t> instanceList{};
		App::IPropManager::Get()->GetAllListIDs(groupID, instanceList);
		if (instanceList.size() == 0) {
			//the bundled package wasnt included and no other mod provided a list for us to use so lets default it to whats in the executable.
			return false;
		}
		for (eastl_size_t i = 0; i < instanceList.size(); i++) {
			PropertyListPtr propList;
			PropManager.GetPropertyList(instanceList[i], groupID, propList);

			//load the two lists that define these cellpartlist's
			size_t UnlockIDCount = 0;
			size_t PartHashCount = 0;
			int* UnlockIDList = nullptr;
			ResourceKey* PartHashList = nullptr;
			App::Property::GetArrayInt32(propList.get(), id("partUnlockID"), UnlockIDCount, UnlockIDList);
			App::Property::GetArrayKey(propList.get(), id("partName"), PartHashCount, PartHashList);

			//lets skip over any files with unbalanced lists
			if (UnlockIDCount != PartHashCount) continue;
			partListSize += UnlockIDCount;
			for (size_t j = 0; j < UnlockIDCount; j++) {
				//until we add more unlockID's lets lock them to the avaliable values...
				uint32_t unlockID = max(0x2, min(UnlockIDList[j], 0xB));
				partlist.emplace_back(CellPartListKey{unlockID, PartHashList[j].instanceID});
				idList.emplace_back(unlockID);
			}
		}
		wasConfiged = true;
		idList.unique();
		return true;
	}

	long AttachDetours() {
		long result = 0;
		result |= EnterCellEditor_detour::attach(Address(ModAPI::ChooseAddress(0xE50130, 0xE4FAA0)));
		result |= LeaveCellEditor_detour::attach(Address(ModAPI::ChooseAddress(0xE50220, 0xE4FB90)));

		return result;
	}
}

#define CELLPARTSFOLDERHASH 0x40616000
void BetterCells::EnterCellEditor_detour::DETOUR(uint32_t* CellDataList) {
	uint32_t* ClassPtr;
	__asm mov ClassPtr, EDI; //the this* is stored in EDI, pull it out before it gets clobbered
	if (!BetterCells::wasConfiged) {
		//restore the this* to EDI before calling the original function
		__asm mov EDI, ClassPtr;
		return original_function(CellDataList);
	}
	CALL(Address(ModAPI::ChooseAddress(0x5976E0, 0x597A20)), void, Args(uint32_t*), Args(ClassPtr));
	CALL(Address(ModAPI::ChooseAddress(0x599100, 0x599440)), void, Args(uint32_t*, int, int, int), Args(ClassPtr, 0, 0, CELLPARTSFOLDERHASH));
	*(byte*)(ClassPtr + 0xC) = 0;
	for (auto i : BetterCells::partlist) {
		CALL(Address(ModAPI::ChooseAddress(0x598A70, 0x598DB0)), void, Args(uint32_t*, int, int, int, unsigned int, int, int, int, int, int),
			Args(ClassPtr, i.partHash, 0, 0, 0, 0, 0, 0, 0, 0));
		uint32_t isUnlockedValue = CellDataList[i.unlockID];
		if (isUnlockedValue == 1) {
			CALL(Address(ModAPI::ChooseAddress(0x596A60, 0x596DA0)), void, Args(uint32_t*, int, int, int), Args(ClassPtr, i.partHash, CELLPARTSFOLDERHASH, 0));
		}
		else if (isUnlockedValue == 0) {
			CALL(Address(ModAPI::ChooseAddress(0x596AD0, 0x596E10)), void, Args(uint32_t*, int, int), Args(ClassPtr, i.partHash, CELLPARTSFOLDERHASH));
		}
	}
	CALL(Address(ModAPI::ChooseAddress(0x594010, 0x5942E0)), void, Args(uint32_t*), Args(ClassPtr));
	for (auto i : BetterCells::partlist) {
		uint32_t isUnlockedValue = CellDataList[i.unlockID];
		if (isUnlockedValue == 2) {
			CALL(Address(ModAPI::ChooseAddress(0x596A60, 0x596DA0)), void, Args(uint32_t*, int, int, int), Args(ClassPtr, i.partHash, CELLPARTSFOLDERHASH, 0));
		}
	}
}

void BetterCells::LeaveCellEditor_detour::DETOUR() {
	uint32_t* ClassPtr;
	__asm mov ClassPtr, EAX; //the this* is stored in EAX, pull it out before it gets clobbered
	if (!BetterCells::wasConfiged) {
		//restore the this* to EAX before calling the original function
		__asm mov EAX, ClassPtr;
		original_function();
	}
	//2 means display the "new" part effect, 1 just means unlocked.
	for (auto i : BetterCells::idList) {
		if (ClassPtr[i] == 2) ClassPtr[i] = 1;
	}
}