/* =========================================

		FCNPC - Fully Controllable NPC
			----------------------

	- File: SAMPFunctions.hpp
	- Author(s): OrMisicL

  =========================================*/


#ifndef SAMPFUNCTIONS_H
#define SAMPFUNCTIONS_H

#ifndef OMP_WRAPPER
struct Packet;
struct PlayerId;

// Functions definitions
typedef void (*ClientJoin_RPC_t)(RPCParameters *parameters);

typedef void (THISCALL *CPlayerPool__DeletePlayer_t)(void *pPlayerPool, WORD wPlayerId, BYTE byteReason);
typedef void (THISCALL *CPlayer__SpawnForWorld_t)(void *pPlayer);
typedef void (THISCALL *CPlayer__Kill_t)(void *pPlayer, BYTE byteReason, WORD wKillerId);
typedef void (THISCALL *CPlayer__EnterVehicle_t)(void *pPlayer, WORD wVehicleId, BYTE byteSeatId);
typedef void (THISCALL *CPlayer__ExitVehicle_t)(void *pPlayer, WORD wVehicleId);
typedef int (THISCALL *CConsole__GetIntVariable_t)(void *pConfig, const char *szKey);
typedef bool (THISCALL *RakNet__Send_t)(void* ppRakServer, RakNet::BitStream* parameters, PacketPriority priority, PacketReliability reliability, unsigned orderingChannel, PlayerID playerId, bool broadcast);
typedef bool (THISCALL *RakNet__RPC_t)(void* ppRakServer, int* uniqueID, RakNet::BitStream* parameters, PacketPriority priority, PacketReliability reliability, unsigned orderingChannel, PlayerID playerId, bool broadcast, bool shiftTimestamp);
typedef Packet* (THISCALL *RakNet__Receive_t)(void* ppRakServer);
typedef PlayerID(THISCALL *RakNet__GetPlayerIDFromIndex_t)(void* ppRakServer, int index);
typedef int (THISCALL *RakNet__GetIndexFromPlayerID_t)(void* ppRakServer, PlayerID playerId);

typedef CVector *( *GetVehicleModelInfo_t)(int iModelID, int iInfoType);
#endif

class CFunctions
{
public:
	static void Initialize();
	static void PreInitialize();

	static WORD GetFreePlayerSlot();
	static WORD NewPlayer(char *szName);
	static void DeletePlayer(WORD wPlayerId);
	static void SpawnPlayer(CPlayer *pPlayer);
	static void KillPlayer(CPlayer *pPlayer, BYTE byteReason, WORD wKillerId);
	static void PlayerEnterVehicle(CPlayer *pPlayer, WORD wVehicleId, BYTE byteSeatId);
	static void PlayerExitVehicle(CPlayer *pPlayer, WORD wVehicleId);
	static CVector GetVehicleModelInfoEx(CVehicle* pVehicle, int iInfoType);
	static WORD GetMaxPlayers();
	static WORD GetMaxNPC();

#ifdef SAMP_03DL
	static int GetSkinBaseID(DWORD dwSkinId);
#endif

	static void GlobalRPC(int* szUniqueID, RakNet::BitStream* bsParams, WORD wExcludePlayerId = INVALID_PLAYER_ID, char PacketStream = 2);
	static void AddedPlayersRPC(int* szUniqueID, RakNet::BitStream* bsParams, WORD wPlayerId, char PacketStream = 2);
	static void AddedVehicleRPC(int* szUniqueID, RakNet::BitStream* bsParams, WORD wVehicleId, WORD wExcludePlayerId = INVALID_PLAYER_ID, char PacketStream = 2);
	static void PlayerRPC(int* szUniqueID, RakNet::BitStream* bsParams, WORD wPlayerId, char PacketStream = 2);

	static void GlobalPacket(RakNet::BitStream* bsParams);
	static void PlayerPacket(RakNet::BitStream* bsParams, WORD wPlayerId);

#ifndef OMP_WRAPPER
	static PlayerID GetPlayerIDFromIndex(int index);
	static int GetIndexFromPlayerID(PlayerID playerId);
#endif

	static void PlayerShoot(WORD wPlayerId, WORD wHitId, BYTE byteHitType, BYTE byteWeaponId, const CVector &vecPoint, const CVector &vecOffsetFrom, bool bIsHit, int iMode, BYTE byteBetweenCheckFlags);
	static WORD GetClosestEntityInBetween(const CVector &vecHitOrigin, const CVector &vecHitTarget, float fRange, int iMode, BYTE byteBetweenCheckFlags, WORD wPlayerId, WORD wTargetId, BYTE &byteEntityType, WORD &wPlayerObjectOwnerId, CVector &vecHitMap);

	static int32_t RayCastLine(const CVector &vecStart, const CVector &vecEnd, CVector *vecResult);

#ifndef OMP_WRAPPER
	// Functions
	static ClientJoin_RPC_t                 pfn__ClientJoin_RPC;
	static CPlayerPool__DeletePlayer_t      pfn__CPlayerPool__DeletePlayer;
	static CPlayer__SpawnForWorld_t         pfn__CPlayer__SpawnForWorld;
	static CPlayer__Kill_t                  pfn__CPlayer__Kill;
	static CPlayer__EnterVehicle_t          pfn__CPlayer__EnterVehicle;
	static CPlayer__ExitVehicle_t           pfn__CPlayer__ExitVehicle;
	static CConsole__GetIntVariable_t       pfn__CConsole__GetIntVariable;
	static GetVehicleModelInfo_t            pfn__GetVehicleModelInfo;
	static RakNet__Send_t                   pfn__RakNet__Send;
	static RakNet__RPC_t                    pfn__RakNet__RPC;
	static RakNet__Receive_t                pfn__RakNet__Receive;
	static RakNet__GetPlayerIDFromIndex_t   pfn__RakNet__GetPlayerIDFromIndex;
	static RakNet__GetIndexFromPlayerID_t   pfn__RakNet__GetIndexFromPlayerID;
#endif

private:
	static WORD GetClosestPlayerInBetween(const CVector &vecHitOrigin, const CVector &vecHitTarget, float fRange, float &fDistance, WORD wPlayerId, WORD wTargetId);
	static WORD GetClosestNPCInBetween(const CVector &vecHitOrigin, const CVector &vecHitTarget, float fRange, float &fDistance, WORD wPlayerId, WORD wTargetId);
	static WORD GetClosestActorInBetween(const CVector &vecHitOrigin, const CVector &vecHitTarget, float fRange, float &fDistance);
	static WORD GetClosestVehicleInBetween(const CVector &vecHitOrigin, const CVector &vecHitTarget, float fRange, float &fDistance);
	static WORD GetClosestObjectInBetween(const CVector &vecHitOrigin, const CVector &vecHitTarget, float fRange, float &fDistance, int iMode);
	static WORD GetClosestPlayerObjectInBetween(const CVector &vecHitOrigin, const CVector &vecHitTarget, float fRange, float &fDistance, WORD wOwnerId, int iMode);
	static int32_t GetClosestMapPointInBetween(const CVector &vecHitOrigin, const CVector &vecHitTarget, float fRange, float &fDistance, CVector &vecHitMap, int iMode);

};

#endif
