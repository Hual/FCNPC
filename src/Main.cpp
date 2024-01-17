/* =========================================

		FCNPC - Fully Controllable NPC
			----------------------

	- File: Main.cpp
	- Author(s): OrMisicL

  =========================================*/

#include "Main.hpp"
#include "CAddress.hpp"

#include <Server/Components/Pawn/pawn.hpp>

// Globals
logprintf_t  logprintf;
extern const void  *pAMXFunctions;
CServer      *pServer;
bool         bServerInit = false;
DWORD        dwStartTick;
CNetGameWrapper *pNetGame;
#ifndef OMP_WRAPPER
void         **ppPluginData;
CNetGame     *pNetGameSAMP;
#ifdef SAMP_03DL
CArtInfo     *pArtInfo;
#endif
void         *pConsole = NULL;
void         *pRakServer = NULL;
#else
ICore *pCore;
IPlayerPool *pPlayerPool;
IPawnComponent *pPawn = nullptr;
int pMaxPlayers = 0;

#endif
char         szSampClient[64];
char         szSampVersion[64];

// ColAndreas stuff
bool colInit = false;
bool colDataLoaded = false;

#ifndef OMP_WRAPPER

PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports()
{
	return SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES | SUPPORTS_PROCESS_TICK;
}

PLUGIN_EXPORT bool PLUGIN_CALL Load(void **ppData)
{
	ppPluginData = ppData;
	// Get the AMX functions pointer
	pAMXFunctions = ppData[PLUGIN_DATA_AMX_EXPORTS];
	// Get the logprintf function address
	logprintf = reinterpret_cast<logprintf_t>(ppData[PLUGIN_DATA_LOGPRINTF]);
	// Check server version
	eSAMPVersion version;

	if ((DWORD)0x9999 == CAddress::FindNetVersion()) {
		version = CRMP_VERSION_037_R2;
		strlcpy(szSampVersion, "0.3.7 R2", sizeof(szSampVersion));
		strlcpy(szSampClient, "CR-MP", sizeof(szSampClient));
	} else if ((DWORD)logprintf == CAddress::FUNC_Logprintf_037_R2) {
		version = SAMP_VERSION_037_R2;
		strlcpy(szSampVersion, "0.3.7 R2", sizeof(szSampVersion));
		strlcpy(szSampClient, "SA-MP", sizeof(szSampClient));
	} else if ((DWORD)logprintf == CAddress::FUNC_Logprintf_03DL_R1) {
		version = SAMP_VERSION_03DL_R1;
		strlcpy(szSampVersion, "0.3.DL R1", sizeof(szSampVersion));
		strlcpy(szSampClient, "SA-MP", sizeof(szSampClient));
	} else {
		version = SAMP_VERSION_UNKNOWN;
		strlcpy(szSampVersion, "Unknown", sizeof(szSampVersion));
		strlcpy(szSampClient, "SA-MP", sizeof(szSampClient));
	}

	// Print the loading message
	logprintf("");
	logprintf("-------------------------------------------------");
	logprintf("     FCNPC - Fully Controllable NPC v" PLUGIN_VERSION "");
	logprintf("            " SYSTEM_NAME " %s %s", szSampClient, szSampVersion);
	logprintf("           " __DATE__" at " __TIME__);
	logprintf("");
	logprintf("  Author:       OrMisicL (2013 - 2015)");
	logprintf("  Continued by: ziggi    (2016 - present)");
	logprintf("");
	logprintf("  See full credits in the README.md file");
	logprintf("-------------------------------------------------");
	logprintf("");
	logprintf("Loading...");
#ifdef SAMP_03DL
	if (version != SAMP_VERSION_03DL_R1 && version != SAMP_VERSION_UNKNOWN) {
		logprintf("[FCNPC] Error: Loading failed. This server requires the FCNPC version of the plugin, instead of the FCNPC-DL version.");
		return false;
	}
#else
	if (version == SAMP_VERSION_03DL_R1) {
		logprintf("[FCNPC] Error: Loading failed. This server requires the FCNPC-DL version of the plugin, instead of the FCNPC version.");
		return false;
	}
#endif
	// Install the exception handler
	CExceptionHandler::Install();
	// Initialize linux tick count
#if defined(LINUX)
	LoadTickCount();
#endif
	// Create the server instance
	pServer = new CServer(version);

	return true;
}

PLUGIN_EXPORT void PLUGIN_CALL Unload()
{
	// Reset the unintizalized flag
	bServerInit = false;
	// Delete the server instance
	SAFE_DELETE(pServer);

	logprintf("[FCNPC] Info: Plugin unloaded.");
}

#else

struct FCNPCComponent : public INetworkComponent, CoreEventHandler, PawnEventHandler {
	PROVIDE_UID(0x5875446ED0C4D445)

public:
	StringView componentName() const { return "FCNPC"; }
	SemanticVersion componentVersion() const { return SemanticVersion(0, 0, 1); }

	void onLoad(ICore *c) override {
		pCore = c;
		pPlayerPool = &pCore->getPlayers();
		logprintf = &PluginLogprintf;

		strlcpy(szSampVersion, "0.3.DL R1", sizeof(szSampVersion));
		strlcpy(szSampClient, "open.mp", sizeof(szSampClient));

		// Print the loading message
		logprintf("");
		logprintf("-------------------------------------------------");
		logprintf("     FCNPC - Fully Controllable NPC v" PLUGIN_VERSION "");
		logprintf("            " SYSTEM_NAME " %s %s", szSampClient, szSampVersion);
		logprintf("           " __DATE__" at " __TIME__);
		logprintf("");
		logprintf("  Author:       OrMisicL (2013 - 2015)");
		logprintf("  Continued by: ziggi    (2016 - present)");
		logprintf("  open.mp version by: Hual");
		logprintf("");
		logprintf("  See full credits in the README.md file");
		logprintf("-------------------------------------------------");
		logprintf("");
		logprintf("Loading...");

		// Install the exception handler
		CExceptionHandler::Install();
		// Initialize linux tick count
#if defined(LINUX)
		LoadTickCount();
#endif
		// Create the server instance
		pServer = new CServer(pCore->getVersion().major);

	}

	INetwork *getNetwork() override {
		// TODO
		return nullptr;
	}

	void onInit(IComponentList *components) override {
		pPawn = components->queryComponent<IPawnComponent>();
		if (!pPawn) {
			pCore->logLn(LogLevel::Error, "No PAWN component, FCNPC can't load");
		}

		pCore->getEventDispatcher().addEventHandler(this);
		pPawn->getEventDispatcher().addEventHandler(this);
	}

	void onReady() override {
		pAMXFunctions = pPawn->getAmxFunctions().data();
		pMaxPlayers = *pCore->getConfig().getInt("max_players");
	}

	void onTick(Microseconds elapsed, TimePoint now) override;

	void onFree(IComponent *component) override {
		if (component == pPawn) {
			pPawn = nullptr;
		}
	}

	void free() override {
		delete this;
	}

	void reset() override {
		pServer->GetPlayerManager()->ResetAllPlayers();
	}

	// Pawn
	void onAmxLoad(IPawnScript &script) override;
	void onAmxUnload(IPawnScript &script) override;

	// Printing
	static void PluginLogprintf(const char *szFormat, ...) {
		va_list args;
		va_start(args, szFormat);
		pCore->vprintLn(szFormat, args);
		va_end(args);
	}
};

COMPONENT_ENTRY_POINT()
{
	return new FCNPCComponent();
}

#endif

// Natives table
AMX_NATIVE_INFO PluginNatives[] = {
	{ "FCNPC_GetPluginVersion", CNatives::FCNPC_GetPluginVersion },
	{ "FCNPC_SetUpdateRate", CNatives::FCNPC_SetUpdateRate },
	{ "FCNPC_GetUpdateRate", CNatives::FCNPC_GetUpdateRate },
	{ "FCNPC_SetTickRate", CNatives::FCNPC_SetTickRate },
	{ "FCNPC_GetTickRate", CNatives::FCNPC_GetTickRate },
	{ "FCNPC_UseMoveMode", CNatives::FCNPC_UseMoveMode },
	{ "FCNPC_IsMoveModeUsed", CNatives::FCNPC_IsMoveModeUsed },
	{ "FCNPC_UseMovePathfinding", CNatives::FCNPC_UseMovePathfinding },
	{ "FCNPC_IsMovePathfindingUsed", CNatives::FCNPC_IsMovePathfindingUsed },
	{ "FCNPC_TriggerWeaponShot", CNatives::FCNPC_TriggerWeaponShot },
	{ "FCNPC_UseCrashLog", CNatives::FCNPC_UseCrashLog },
	{ "FCNPC_IsCrashLogUsed", CNatives::FCNPC_IsCrashLogUsed },

	{ "FCNPC_Create", CNatives::FCNPC_Create },
	{ "FCNPC_Destroy", CNatives::FCNPC_Destroy },
	{ "FCNPC_Spawn", CNatives::FCNPC_Spawn },
	{ "FCNPC_Respawn", CNatives::FCNPC_Respawn },
	{ "FCNPC_IsSpawned", CNatives::FCNPC_IsSpawned },
	{ "FCNPC_Kill", CNatives::FCNPC_Kill },
	{ "FCNPC_IsDead", CNatives::FCNPC_IsDead },
	{ "FCNPC_IsValid", CNatives::FCNPC_IsValid },
	{ "FCNPC_IsStreamedIn", CNatives::FCNPC_IsStreamedIn },
	{ "FCNPC_IsStreamedInForAnyone", CNatives::FCNPC_IsStreamedInForAnyone },
	{ "FCNPC_GetValidArray", CNatives::FCNPC_GetValidArray },

	{ "FCNPC_SetPosition", CNatives::FCNPC_SetPosition },
	{ "FCNPC_GivePosition", CNatives::FCNPC_GivePosition },
	{ "FCNPC_GetPosition", CNatives::FCNPC_GetPosition },
	{ "FCNPC_SetQuaternion", CNatives::FCNPC_SetQuaternion },
	{ "FCNPC_GiveQuaternion", CNatives::FCNPC_GiveQuaternion },
	{ "FCNPC_GetQuaternion", CNatives::FCNPC_GetQuaternion },
	{ "FCNPC_SetAngle", CNatives::FCNPC_SetAngle },
	{ "FCNPC_GiveAngle", CNatives::FCNPC_GiveAngle },
	{ "FCNPC_SetAngleToPos", CNatives::FCNPC_SetAngleToPos },
	{ "FCNPC_SetAngleToPlayer", CNatives::FCNPC_SetAngleToPlayer },
	{ "FCNPC_GetAngle", CNatives::FCNPC_GetAngle },
	{ "FCNPC_SetVelocity", CNatives::FCNPC_SetVelocity },
	{ "FCNPC_GiveVelocity", CNatives::FCNPC_GiveVelocity },
	{ "FCNPC_GetVelocity", CNatives::FCNPC_GetVelocity },
	{ "FCNPC_SetSpeed", CNatives::FCNPC_SetSpeed },
	{ "FCNPC_GetSpeed", CNatives::FCNPC_GetSpeed },
	{ "FCNPC_SetInterior", CNatives::FCNPC_SetInterior },
	{ "FCNPC_GetInterior", CNatives::FCNPC_GetInterior },
	{ "FCNPC_SetVirtualWorld", CNatives::FCNPC_SetVirtualWorld },
	{ "FCNPC_GetVirtualWorld", CNatives::FCNPC_GetVirtualWorld },

	{ "FCNPC_SetHealth", CNatives::FCNPC_SetHealth },
	{ "FCNPC_GiveHealth", CNatives::FCNPC_GiveHealth },
	{ "FCNPC_GetHealth", CNatives::FCNPC_GetHealth },
	{ "FCNPC_SetArmour", CNatives::FCNPC_SetArmour },
	{ "FCNPC_GiveArmour", CNatives::FCNPC_GiveArmour },
	{ "FCNPC_GetArmour", CNatives::FCNPC_GetArmour },

	{ "FCNPC_SetInvulnerable", CNatives::FCNPC_SetInvulnerable },
	{ "FCNPC_IsInvulnerable", CNatives::FCNPC_IsInvulnerable },

	{ "FCNPC_SetSkin", CNatives::FCNPC_SetSkin },
	{ "FCNPC_GetSkin", CNatives::FCNPC_GetSkin },

	{ "FCNPC_SetWeapon", CNatives::FCNPC_SetWeapon },
	{ "FCNPC_GetWeapon", CNatives::FCNPC_GetWeapon },
	{ "FCNPC_SetAmmo", CNatives::FCNPC_SetAmmo },
	{ "FCNPC_GiveAmmo", CNatives::FCNPC_SetAmmo },
	{ "FCNPC_GetAmmo", CNatives::FCNPC_GetAmmo },
	{ "FCNPC_SetAmmoInClip", CNatives::FCNPC_SetAmmoInClip },
	{ "FCNPC_GiveAmmoInClip", CNatives::FCNPC_SetAmmoInClip },
	{ "FCNPC_GetAmmoInClip", CNatives::FCNPC_GetAmmoInClip },
	{ "FCNPC_SetWeaponSkillLevel", CNatives::FCNPC_SetWeaponSkillLevel },
	{ "FCNPC_GiveWeaponSkillLevel", CNatives::FCNPC_GiveWeaponSkillLevel },
	{ "FCNPC_GetWeaponSkillLevel", CNatives::FCNPC_GetWeaponSkillLevel },
	{ "FCNPC_SetWeaponState", CNatives::FCNPC_SetWeaponState },
	{ "FCNPC_GetWeaponState", CNatives::FCNPC_GetWeaponState },

	{ "FCNPC_SetWeaponReloadTime", CNatives::FCNPC_SetWeaponReloadTime },
	{ "FCNPC_GetWeaponReloadTime", CNatives::FCNPC_GetWeaponReloadTime },
	{ "FCNPC_GetWeaponActualReloadTime", CNatives::FCNPC_GetWeaponActualReloadTime },
	{ "FCNPC_SetWeaponShootTime", CNatives::FCNPC_SetWeaponShootTime },
	{ "FCNPC_GetWeaponShootTime", CNatives::FCNPC_GetWeaponShootTime },
	{ "FCNPC_SetWeaponClipSize", CNatives::FCNPC_SetWeaponClipSize },
	{ "FCNPC_GetWeaponClipSize", CNatives::FCNPC_GetWeaponClipSize },
	{ "FCNPC_GetWeaponActualClipSize", CNatives::FCNPC_GetWeaponActualClipSize },
	{ "FCNPC_SetWeaponAccuracy", CNatives::FCNPC_SetWeaponAccuracy },
	{ "FCNPC_GetWeaponAccuracy", CNatives::FCNPC_GetWeaponAccuracy },

	{ "FCNPC_SetWeaponInfo", CNatives::FCNPC_SetWeaponInfo },
	{ "FCNPC_GetWeaponInfo", CNatives::FCNPC_GetWeaponInfo },
	{ "FCNPC_SetWeaponDefaultInfo", CNatives::FCNPC_SetWeaponDefaultInfo },
	{ "FCNPC_GetWeaponDefaultInfo", CNatives::FCNPC_GetWeaponDefaultInfo },

	{ "FCNPC_SetKeys", CNatives::FCNPC_SetKeys },
	{ "FCNPC_GetKeys", CNatives::FCNPC_GetKeys },

	{ "FCNPC_SetSpecialAction", CNatives::FCNPC_SetSpecialAction },
	{ "FCNPC_GetSpecialAction", CNatives::FCNPC_GetSpecialAction },

	{ "FCNPC_SetAnimation", CNatives::FCNPC_SetAnimation },
	{ "FCNPC_SetAnimationByName", CNatives::FCNPC_SetAnimationByName },
	{ "FCNPC_GetAnimation", CNatives::FCNPC_GetAnimation },
	{ "FCNPC_ResetAnimation", CNatives::FCNPC_ResetAnimation },
	{ "FCNPC_ApplyAnimation", CNatives::FCNPC_ApplyAnimation },
	{ "FCNPC_ClearAnimations", CNatives::FCNPC_ClearAnimations },

	{ "FCNPC_SetFightingStyle", CNatives::FCNPC_SetFightingStyle },
	{ "FCNPC_GetFightingStyle", CNatives::FCNPC_GetFightingStyle },

	{ "FCNPC_UseReloading", CNatives::FCNPC_UseReloading },
	{ "FCNPC_IsReloadingUsed", CNatives::FCNPC_IsReloadingUsed },
	{ "FCNPC_UseInfiniteAmmo", CNatives::FCNPC_UseInfiniteAmmo },
	{ "FCNPC_IsInfiniteAmmoUsed", CNatives::FCNPC_IsInfiniteAmmoUsed },

	{ "FCNPC_GoTo", CNatives::FCNPC_GoTo },
	{ "FCNPC_GoToPlayer", CNatives::FCNPC_GoToPlayer },
	{ "FCNPC_Stop", CNatives::FCNPC_Stop },
	{ "FCNPC_IsMoving", CNatives::FCNPC_IsMoving },
	{ "FCNPC_IsMovingAtPlayer", CNatives::FCNPC_IsMovingAtPlayer },
	{ "FCNPC_GetDestinationPoint", CNatives::FCNPC_GetDestinationPoint },

	{ "FCNPC_AimAt", CNatives::FCNPC_AimAt },
	{ "FCNPC_AimAtPlayer", CNatives::FCNPC_AimAtPlayer },
	{ "FCNPC_StopAim", CNatives::FCNPC_StopAim },
	{ "FCNPC_MeleeAttack", CNatives::FCNPC_MeleeAttack },
	{ "FCNPC_StopAttack", CNatives::FCNPC_StopAttack },
	{ "FCNPC_IsAttacking", CNatives::FCNPC_IsAttacking },
	{ "FCNPC_IsAiming", CNatives::FCNPC_IsAiming },
	{ "FCNPC_IsAimingAtPlayer", CNatives::FCNPC_IsAimingAtPlayer },
	{ "FCNPC_GetAimingPlayer", CNatives::FCNPC_GetAimingPlayer },
	{ "FCNPC_IsShooting", CNatives::FCNPC_IsShooting },
	{ "FCNPC_IsReloading", CNatives::FCNPC_IsReloading },
	{ "FCNPC_GetClosestEntityInBetween", CNatives::FCNPC_GetClosestEntityInBetween },

	{ "FCNPC_EnterVehicle", CNatives::FCNPC_EnterVehicle },
	{ "FCNPC_ExitVehicle", CNatives::FCNPC_ExitVehicle },
	{ "FCNPC_PutInVehicle", CNatives::FCNPC_PutInVehicle },
	{ "FCNPC_RemoveFromVehicle", CNatives::FCNPC_RemoveFromVehicle },
	{ "FCNPC_GetVehicleID", CNatives::FCNPC_GetVehicleID },
	{ "FCNPC_GetVehicleSeat", CNatives::FCNPC_GetVehicleSeat },
	{ "FCNPC_UseVehicleSiren", CNatives::FCNPC_UseVehicleSiren },
	{ "FCNPC_IsVehicleSirenUsed", CNatives::FCNPC_IsVehicleSirenUsed },
	{ "FCNPC_SetVehicleHealth", CNatives::FCNPC_SetVehicleHealth },
	{ "FCNPC_GetVehicleHealth", CNatives::FCNPC_GetVehicleHealth },
	{ "FCNPC_SetVehicleHydraThrusters", CNatives::FCNPC_SetVehicleHydraThrusters },
	{ "FCNPC_GetVehicleHydraThrusters", CNatives::FCNPC_GetVehicleHydraThrusters },
	{ "FCNPC_SetVehicleGearState", CNatives::FCNPC_SetVehicleGearState },
	{ "FCNPC_GetVehicleGearState", CNatives::FCNPC_GetVehicleGearState },
	{ "FCNPC_SetVehicleTrainSpeed", CNatives::FCNPC_SetVehicleTrainSpeed },
	{ "FCNPC_GetVehicleTrainSpeed", CNatives::FCNPC_GetVehicleTrainSpeed },

	{ "FCNPC_SetSurfingOffsets", CNatives::FCNPC_SetSurfingOffsets },
	{ "FCNPC_GiveSurfingOffsets", CNatives::FCNPC_GiveSurfingOffsets },
	{ "FCNPC_GetSurfingOffsets", CNatives::FCNPC_GetSurfingOffsets },
	{ "FCNPC_SetSurfingVehicle", CNatives::FCNPC_SetSurfingVehicle },
	{ "FCNPC_GetSurfingVehicle", CNatives::FCNPC_GetSurfingVehicle },
	{ "FCNPC_SetSurfingObject", CNatives::FCNPC_SetSurfingObject },
	{ "FCNPC_GetSurfingObject", CNatives::FCNPC_GetSurfingObject },
	{ "FCNPC_SetSurfingPlayerObject", CNatives::FCNPC_SetSurfingPlayerObject },
	{ "FCNPC_GetSurfingPlayerObject", CNatives::FCNPC_GetSurfingPlayerObject },
	{ "FCNPC_StopSurfing", CNatives::FCNPC_StopSurfing },

	{ "FCNPC_StartPlayingPlayback", CNatives::FCNPC_StartPlayingPlayback },
	{ "FCNPC_StopPlayingPlayback", CNatives::FCNPC_StopPlayingPlayback },
	{ "FCNPC_PausePlayingPlayback", CNatives::FCNPC_PausePlayingPlayback },
	{ "FCNPC_ResumePlayingPlayback", CNatives::FCNPC_ResumePlayingPlayback },
	{ "FCNPC_LoadPlayingPlayback", CNatives::FCNPC_LoadPlayingPlayback },
	{ "FCNPC_UnloadPlayingPlayback", CNatives::FCNPC_UnloadPlayingPlayback },
	{ "FCNPC_SetPlayingPlaybackPath", CNatives::FCNPC_SetPlayingPlaybackPath },
	{ "FCNPC_GetPlayingPlaybackPath", CNatives::FCNPC_GetPlayingPlaybackPath },

	{ "FCNPC_OpenNode", CNatives::FCNPC_OpenNode },
	{ "FCNPC_CloseNode", CNatives::FCNPC_CloseNode },
	{ "FCNPC_IsNodeOpen", CNatives::FCNPC_IsNodeOpen },
	{ "FCNPC_GetNodePointPosition", CNatives::FCNPC_GetNodePointPosition },
	{ "FCNPC_GetNodeType", CNatives::FCNPC_GetNodeType },
	{ "FCNPC_SetNodePoint", CNatives::FCNPC_SetNodePoint },
	{ "FCNPC_GetNodePointCount", CNatives::FCNPC_GetNodePointCount },
	{ "FCNPC_GetNodeInfo", CNatives::FCNPC_GetNodeInfo },
	{ "FCNPC_PlayNode", CNatives::FCNPC_PlayNode },
	{ "FCNPC_StopPlayingNode", CNatives::FCNPC_StopPlayingNode },
	{ "FCNPC_PausePlayingNode", CNatives::FCNPC_PausePlayingNode },
	{ "FCNPC_ResumePlayingNode", CNatives::FCNPC_ResumePlayingNode },
	{ "FCNPC_IsPlayingNode", CNatives::FCNPC_IsPlayingNode },
	{ "FCNPC_IsPlayingNodePaused", CNatives::FCNPC_IsPlayingNodePaused },

	{ "FCNPC_CreateMovePath", CNatives::FCNPC_CreateMovePath },
	{ "FCNPC_DestroyMovePath", CNatives::FCNPC_DestroyMovePath },
	{ "FCNPC_IsValidMovePath", CNatives::FCNPC_IsValidMovePath },
	{ "FCNPC_AddPointToMovePath", CNatives::FCNPC_AddPointToMovePath },
	{ "FCNPC_AddPointsToMovePath", CNatives::FCNPC_AddPointsToMovePath },
	{ "FCNPC_AddPointsToMovePath2", CNatives::FCNPC_AddPointsToMovePath2 },
	{ "FCNPC_RemovePointFromMovePath", CNatives::FCNPC_RemovePointFromMovePath },
	{ "FCNPC_IsValidMovePathPoint", CNatives::FCNPC_IsValidMovePathPoint },
	{ "FCNPC_GetMovePathPoint", CNatives::FCNPC_GetMovePathPoint },
	{ "FCNPC_GetNumberMovePathPoint", CNatives::FCNPC_GetNumberMovePathPoint },
	{ "FCNPC_GoByMovePath", CNatives::FCNPC_GoByMovePath },

	{ "FCNPC_SetMoveMode", CNatives::FCNPC_SetMoveMode },
	{ "FCNPC_GetMoveMode", CNatives::FCNPC_GetMoveMode },
	{ "FCNPC_SetMinHeightPosCall", CNatives::FCNPC_SetMinHeightPosCall },
	{ "FCNPC_GetMinHeightPosCall", CNatives::FCNPC_GetMinHeightPosCall },

	// nah
	{ "FCNPC_ShowInTabListForPlayer", CNatives::FCNPC_ShowInTabListForPlayer },
	{ "FCNPC_HideInTabListForPlayer", CNatives::FCNPC_HideInTabListForPlayer },

	{ 0, 0 }
};

#ifndef OMP_WRAPPER
PLUGIN_EXPORT int PLUGIN_CALL AmxLoad(AMX *pAMX)
{
#else

void FCNPCComponent::onAmxLoad(IPawnScript &script)
{
	AMX *pAMX = script.GetAMX();
#endif
	// Check for native usage
	AMX_HEADER *pAmxHeader = reinterpret_cast<AMX_HEADER *>(pAMX->base);
	AMX_FUNCSTUBNT *pAmxNativeTable = reinterpret_cast<AMX_FUNCSTUBNT *>(pAMX->base + pAmxHeader->natives);
	char *szName;
	bool bIsHaveNatives = false;
	int iNativesCount;
	amx_NumNatives(pAMX, &iNativesCount);

	for (int i = 0; i < iNativesCount; i++) {
		szName = reinterpret_cast<char *>(pAMX->base + pAmxNativeTable[i].nameofs);
		if (strstr(szName, "FCNPC_") != NULL) {
			bIsHaveNatives = true;
			break;
		}
	}

	// Check include version
	if (bIsHaveNatives) {
		int iIncludeVersion = 0;
		cell cellIndex;
		if (!amx_FindPubVar(pAMX, "FCNPC_IncludeVersion", &cellIndex)) {
			cell *pAddress = NULL;
			if (!amx_GetAddr(pAMX, cellIndex, &pAddress)) {
				iIncludeVersion = *pAddress;
			}
		}
		if (iIncludeVersion != INCLUDE_VERSION) {
			logprintf("[FCNPC] Error: Script initialization failed. Include file version does not match plugin version.");
			exit(0);
		}
	}

	// Initialize the server
	if (!bServerInit) {
		BYTE byteError = pServer->Initialize();
		if (byteError != ERROR_NO) {
			// Get the error
			char szError[64];
			CUtils::GetPluginError(byteError, szError, sizeof(szError));
			logprintf("[FCNPC] Error: Initialization failed. %s.", szError);
			exit(0);
		}
		// Initialize the starting tick
		dwStartTick = GetTickCount();
		// Set the initialized flag
		bServerInit = true;
	}

	// Register the AMX
	CCallbackManager::RegisterAMX(pAMX);

	// Register the plugin natives for the amx instance
#ifndef OMP_WRAPPER
	return
#endif
		amx_Register(pAMX, PluginNatives, -1);
}

#ifndef OMP_WRAPPER
PLUGIN_EXPORT int PLUGIN_CALL AmxUnload(AMX *pAMX)
{
#else
void FCNPCComponent::onAmxUnload(IPawnScript &script)
{
	AMX *pAMX = script.GetAMX();
#endif
	// Unregister the AMX
	CCallbackManager::UnregisterAMX(pAMX);
#ifndef OMP_WRAPPER
	return AMX_ERR_NONE;
#endif
}

#ifndef OMP_WRAPPER
PLUGIN_EXPORT void PLUGIN_CALL ProcessTick()
#else
void FCNPCComponent::onTick(Microseconds elapsed, TimePoint now)
#endif
{
	if (pServer) {
		pServer->Process();
	}
}

bool CNetGameWrapper::HasGameMode() {
#ifndef OMP_WRAPPER
	return pNetGameSAMP->pGameModePool != nullptr;
#else
	return pPawn->mainScript() != nullptr;
#endif
}

bool CNetGameWrapper::IsPlayerConnected(WORD wPlayerId) {
#ifndef OMP_WRAPPER
	return pNetGameSAMP->pPlayerPool->bIsPlayerConnected[wPlayerId] != 0
#else
	return pPlayerPool->get(wPlayerId) != nullptr;
#endif
}

const char *CNetGameWrapper::GetPlayerName(WORD wPlayerId) {
#ifndef OMP_WRAPPER
	return pNetGameSAMP->pPlayerPool->szName[id];
#else
	IPlayer *player = pPlayerPool->get(wPlayerId);
	if (!player) { return ""; }
	return player->getName().data();
#endif
}

DWORD CNetGameWrapper::GetPlayerPoolSize() {
#ifndef OMP_WRAPPER
	return pNetGameSAMP->pPlayerPool->dwPlayerPoolSize;
#else
	return pMaxPlayers;
#endif
}

CPlayer *CNetGameWrapper::GetPlayerAt(WORD wPlayerId) {
#ifndef OMP_WRAPPER
	return pNetGameSAMP->pPlayerPool->pPlayer[wPlayerId];
#else
	return pPlayerPool->get(wPlayerId);
#endif
}

CVector CNetGameWrapper::GetPlayerPos(WORD wPlayerId) {
#ifndef OMP_WRAPPER
	return pNetGameSAMP->pPlayerPool->pPlayer[wPlayerId]->vecPosition;
#else
	IPlayer *player = pPlayerPool->get(wPlayerId);
	if (!player) { return CVector(0.f, 0.f, 0.f); }
	return player->getPosition();
#endif
}
