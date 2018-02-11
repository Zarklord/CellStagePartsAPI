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
#include <EASTL\list.h>


bool BetterCells::Initialize() {
	partListSize = 0;
	int groupID = Hash::FNV("cell_editor_part_list");
	eastl::vector<uint32_t> instanceList{};
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
			uint32_t unlockID = UnlockIDList[j];
			if (unlockID > 0xB) {
				unlockID = 0xB;
			} else if (unlockID < 0x2) {
				unlockID = 0x2;
			}
			partlist.emplace_back(CellPartListKey{unlockID, PartHashList[j].mnInstanceID});
			idList.emplace_back(unlockID);
		}
	}
	wasConfiged = true;
	idList.unique();
}

long BetterCells::AttachDetours() {
	SetDetourAddress(CellEditorRemoveNewEffect, GetAddress(0xE50220, 0x0, 0xE4FB90));
	SetDetourAddress(CellEditorSetUnlockedPartList, GetAddress(0xE50130, 0x0, 0xE4FAA0));

	long result = AttachDetourFunctionStatic(CellEditorRemoveNewEffect_original, DetouredCellEditorRemoveNewEffect);
	result |= AttachDetourFunctionStatic(CellEditorSetUnlockedPartList_original, DetouredCellEditorSetUnlockedPartList);
	return result;
}

void DetouredCellEditorRemoveNewEffect() {
	//this is called after leaving the cell editor in cell stage.
	//the passed arguement wasn't put on the stack but instead is eax.
	uint32_t * PartAddress;
	__asm {
		mov PartAddress, EAX
	};
	//we have to do this since the compiler likes to do this check in the EAX register which is where the arguement is passed.
	if (!wasConfiged) {
		__asm mov EAX, PartAddress;
		return CallOriginalStatic(CellEditorRemoveNewEffect);
	}
	//2 means display the "new" part effect, 1 just means unlocked.
	for (auto i : idList) {
		if (PartAddress[i] == 2) PartAddress[i] = 1;
	}
}

#if EXECUTABLE_TYPE == SPORE_STANDARD
            #define ChooseAddress(addressStandard, addressSteam, addressSteamPatched) (addressStandard - 0x400000)
#elif EXECUTABLE_TYPE == SPORE_STEAM
    #if PATCHED_SPORE == 0 
        #define ChooseAddress(addressStandard, addressSteam, addressSteamPatched) (addressSteam - 0x400000)
    #else 
        #define ChooseAddress(addressStandard, addressSteam, addressSteamPatched) (addressSteamPatched - 0x400000)
    #endif
#endif

#define CELLPARTSFOLDERHASH 0x40616000
void DetouredCellEditorSetUnlockedPartList(uint32_t * CellDataList) {
	//this is called upon entering cell stage, and upon mating and entering the editor
	if (!wasConfiged) return CallOriginalStatic(CellEditorSetUnlockedPartList, CellDataList);

	//void function();
	__asm {
		mov ECX, EDI
		mov EAX, baseAddress
		add EAX, ChooseAddress(0x5976E0, 0x0, 0x597A20)
		call EAX
	};
	//void function(0, 0, CELLPARTSFOLDERHASH);
	__asm {
		push CELLPARTSFOLDERHASH
		push 0
		push 0
		mov ECX, EDI
		mov EAX, baseAddress
		add EAX, ChooseAddress(0x599100, 0x0, 0x599440)
		call EAX
	};
	__asm mov [EDI + 0xC], 0;
	for (auto i : partlist) {
		//void function(i.partHash, 0, 0, 0, 0, 0, 0, 0, 0);
		__asm {
			push 0
			push 0
			push 0
			push 0
			push 0
			push 0
			push 0
			push 0
			mov ECX, i.partHash
			push ECX
			mov ECX, EDI
			mov EAX, baseAddress
			add EAX, ChooseAddress(0x598A70, 0x0, 0x598DB0)
			call EAX
		};
		uint32_t isUnlockedValue = CellDataList[i.unlockID];
		if (isUnlockedValue == 1) {
			//void function(i.partHash, CELLPARTSFOLDERHASH, 0);
			__asm {
				push 0
				push CELLPARTSFOLDERHASH
				push i.partHash
				mov ECX, EDI
				mov EAX, baseAddress
				add EAX, ChooseAddress(0x596A60, 0x0, 0x596DA0)
				call EAX
			};
		} else if (isUnlockedValue == 0) {
			//void function(i.partHash, CELLPARTSFOLDERHASH);
			__asm {
				push CELLPARTSFOLDERHASH
				push i.partHash
				mov ECX, EDI
				mov EAX, baseAddress
				add EAX, ChooseAddress(0x596AD0, 0x0, 0x596E10)
				call EAX
			};
		}
	}
	//void function();
	__asm {
		mov ECX, EDI
		mov EAX, baseAddress
		add EAX, ChooseAddress(0x594010, 0x0, 0x5942E0)
		call EAX
	};
	for (auto i : partlist) {
		uint32_t isUnlockedValue = CellDataList[i.unlockID];
		if (isUnlockedValue == 2) {
			//void function(i.partHash, CELLPARTSFOLDERHASH, 0);
			__asm {
				push 0
				push CELLPARTSFOLDERHASH
				push i.partHash
				mov ECX, EDI
				mov EAX, baseAddress
				add EAX, ChooseAddress(0x596A60, 0x0, 0x596DA0)
				call EAX
			};
		}
	}
}
