#include "simulation/rations.c"

bool wdmDisableRelease = true;

//=========================================================================================
//Events
//=========================================================================================

void wdmEvent_EncounterCreate()
{
	float dltTime = GetEventData();
	float playerShipX = GetEventData();
	float playerShipZ = GetEventData();
	float playerShipAY = GetEventData();
	//Save player ship position
	worldMap.playerShipX = playerShipX;
	worldMap.playerShipZ = playerShipZ;
	worldMap.playerShipAY = playerShipAY;
	string currentIsland = wdmGetCurrentIsland();
	if(currentIsland!=WDM_NONE_ISLAND && CheckAttribute(worldMap.islands, currentIsland)) // PB: Prevent error.log entries
	{
		wdmLoginToSea.island = worldMap.islands.(currentIsland).name;
		float ix = MakeFloat(worldMap.islands.(currentIsland).position.rx);
		float iz = MakeFloat(worldMap.islands.(currentIsland).position.rz);
		worldMap.playerShipDispX = (playerShipX - ix);
		worldMap.playerShipDispZ = (playerShipZ - iz);
	}else{
		worldMap.playerShipDispX = 0.0;
		worldMap.playerShipDispZ = 0.0;
	}
	//Skip encounters
	if(CheckAttribute(worldMap, "noenc") != 0)
	{
		if(worldMap.noenc == "true") return;
	}

	//Generate encounters
	if (GENERATE_STORMS) {
		wdmStormGen(dltTime, playerShipX, playerShipZ, playerShipAY);
	}

	wdmShipEncounter(dltTime, playerShipX, playerShipZ, playerShipAY);
	wdmQuestShipEncounter(dltTime, playerShipX, playerShipZ, playerShipAY);
}

// KK -->
void PlayLandHo()
{
	if (ENABLE_EXTRA_SOUNDS == 0) return;
	if (actLoadFlag == 1) return;
	if (!CheckAttribute(&worldMap, "outsideIsland")) return;
	DeleteAttribute(&worldMap, "outsideIsland");
	PlaySound("#land_ho");
}
// <-- KK

void wdmEvent_PlayerInStorm()
{
	float playerShipX = GetEventData();
	float playerShipZ = GetEventData();
	float playerShipAY = GetEventData();
	int stormIndex = GetEventData();
	wdmDisableTornadoGen = false;
	wdmReloadToSea();
}

void wdmEvent_ShipEncounter()
{
	wdmDisableTornadoGen = true;
	float playerShipX = GetEventData();
	float playerShipZ = GetEventData();
	float playerShipAY = GetEventData();
	//Trace("Player ship hit in ship encounter with index " + eshipIndex);

	int encounterId = GetEventData();
	ref pchar = GetMainCharacter();
	if (CheckAttribute(pchar, "SkipEshipIndex") && pchar.SkipEshipIndex == encounterId) return;
	pchar.eshipIndex = encounterId;

	if(CheckAttribute(worldMap, "evwin") != 0)
	{
		if(worldMap.evwin == "true")
		{
			wdmMessage("Sails on horizon!# Pirate ships ahead.", "Engage", "Skip");
			return;
		}
	}

	// boal -->
//changed by MAXIMUS: for new "Sail Ho" and DirectSail-Mod -->
	CalculateEncInfoData();
/*	ref messenger = DirectSailMessenger("sail_ho");
	messenger.SailHo = true;
	messenger.Dialog.Filename = "Enc_WorldMap_dialog.c";
	Log_SetActiveAction("Nothing");
	BLI_RefreshCommandMenu();
	BLI_DisableShow();
	SelfDialog(messenger);*/
	LaunchBoalMapScreen();
//changed by MAXIMUS: for new "Sail Ho" and DirectSail-Mod <--
	//wdmReloadToSea();
	// boal <--

}

void wdmEvent_DeleteShipEncounter()
{
	if(wdmDisableRelease == true)
	{
		int type = GetEventData();
		if(type >= 0) ReleaseMapEncounter(type);
	}
}

void wdmEvent_EventWindowChangeSelectoin()
{
	int select = GetEventData();
}

void wdmEvent_EventWindowSelect()
{
	int result = GetEventData();

	//Trace("Event window select is " + result);

	if(result == 0) wdmReloadToSea();
}

void wdmEvent_WaitMenuChangeSelectoin()
{
	int selectItem = GetEventData();
}

void wdmEvent_WaitMenuSelect()
{
	int selectItem = GetEventData();

	//Trace("Wait menu select is " + selectItem);
}

void wdmEvent_UpdateDate()
{
	Environment.date.day = worldMap.date.day;
	Environment.date.month = worldMap.date.month;	
	Environment.date.year = worldMap.date.year;
	Environment.date.hour = worldMap.date.hour;
	Environment.date.min = worldMap.date.min;
	Environment.date.sec = worldMap.date.sec;
	Environment.time = GetEventData();
}



#event_handler("NextDay", "wdmNextDayUpdate");
void wdmNextDayUpdate()
{
	// NK -->
	ref PChar = GetMainCharacter();
	if(!CheckAttribute(PChar,"articles")) {PChar.articles = 0;}
	if(!CheckAttribute(PChar,"LastPayMonth")) {PChar.LastPayMonth = GetDataMonth();} // PB: NOT at the beginning of the game
	/*if( sti(PChar.LastPayMonth) != GetDataMonth() )
	{
		if( GetDataDay() >= 7 && !sti(PChar.articles) )
		{
			wdmLockReload = true;
			PChar.LastPayMonth = GetDataMonth();
			PostEvent("Event_CheckMoneyData", 0);
		}
	}*/ //This is in daily updates already
	/*DailyCrewUpdate();
	// NK <--
	UpdateAllTowns(false); // NK
	DailyShipBerthingUpdate(); 		// added after build 11 by KAM
	UpdateAllItemTraders(false); // NK itemtrade 05-04-02 do all trader update.
	UpdateMoonInfo(LogsToggle == LOG_VERBOSE); // added by Levis*/
	DoDailyUpdates("all"); //Changed to this by levis to take everything in account
}


/*#event_handler("Event_CheckMoneyData", "wdmCheckMoneyData");
void wdmCheckMoneyData()
{
	trace("wdmCheckMoneyData on day: "+ worldMap.date.day);
	LaunchSalaryScreen();
	wdmLockReload = false;
}*/ //in daily updates already

#event_handler("WorldMap_IsSkipEnable", "wdmIsSkipEnable");
bool wdmSkipReturnBool = false;
bool wdmIsSkipEnable()
{
	wdmSkipReturnBool = false;
	if(IsCharacterPerkOn(GetMainCharacter(), "SailingProfessional"))
	{
		if(rand(100) <= 50)
		{
			wdmSkipReturnBool = true;
		}
	}
	return wdmSkipReturnBool;
}

#event_handler("WorldMap_GetMoral", "wdmGetMorale");
float wdmGetMorale()
{
	ref pchar = GetMainCharacter();
	return stf(pchar.Ship.Crew.Morale);
}

#event_handler("WorldMap_GetFood", "wdmGetFood");
float wdmGetFood()
{
	ref pchar = GetMainCharacter();
	return CalculateFoodRations(pchar);
}

#event_handler("WorldMap_GetRum", "wdmGetRum");
float wdmGetRum()
{
	ref pchar = GetMainCharacter();
	return CalculateRumRations(pchar);
}
