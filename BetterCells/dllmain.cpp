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

// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

#include <ModAPI\MainUtilities.h>

#include "BetterCells.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	long error;
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:

		// This line is always necessary
		ModAPI::ModAPIUtils::InitModAPI();

		/* Insert code here */

		/* 
		Note: You cannot use any Spore function inside this method. 
		If you want to add any initialization methods, do it with:

		bool myFunction() {...}

		And then, inside the DllMain function:

		ModAPI::ModAPIUtils::AddInitFunction(&myFunction);

		Things you can do in the DllMain method:
		 - Add init functions
		 - Add UI event listeners
		 - Detour functions
		*/

		ModAPI::ModAPIUtils::AddInitFunction(BetterCells::Initialize); 
		
		PrepareDetours(hModule);
		// It is recommended to attach the detoured methods in specialised methods in the class
		BetterCells::AttachDetours();
		error = SendDetours();

		//WARNING: INTENTIONAL FALL THROUGH TO NEXT CASE
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		//WARNING: INTENTIONAL FALL THROUGH TO NEXT CASE
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}