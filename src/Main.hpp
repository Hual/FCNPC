/* =========================================

		FCNPC - Fully Controllable NPC
			----------------------

	- File: Main.hpp
	- Author(s): OrMisicL

  =========================================*/

// OS includes
#if defined(WIN32)
	#include <snprintf/snprintf.h>

	#pragma warning(disable : 4201 4206 4127 4100 4305 4127)
	#define STDCALL __stdcall
	#define THISCALL __thiscall
	#define FASTCALL __fastcall
	#define SN_TARGET_PS2
#elif defined(LINUX)
	#include <pthread.h>
	#include <unistd.h>
	#include <sys/mman.h>
	#include <fopen_s/fopen_s.h>

	#define BOOL    int32_t
	#define DWORD   uint32_t
	#define BYTE    uint8_t
	#define WORD    uint16_t
	#define TRUE    (1)
	#define FALSE   (0)
	#define STDCALL
	#define THISCALL
	#define FASTCALL
	#define MAX_PATH 260
#endif

// Standard includes
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <cmath>
#include <cstring>
#include <ctime>
#include <vector>
#include <algorithm>
#include <array>
#include <queue>
#include <sstream>
#include <btBulletDynamicsCommon.h>
// Library includes
#include <sdk/plugin.h>
#include <sdk.hpp>
#include <bitstream.hpp>
#include <strlcpy/strlcpy.h>
// ExceptionHandler includes
#include "CExceptionHandler.hpp"
// Utils includes
#include "CUtils.hpp"
#include "Common.h"
#include "CVector.h"
// Nodes includes
#include "CPlayback.hpp"
#include "CNode.hpp"
// Server includes
#include "CAddress.hpp"
#include "CServer.hpp"
#include "CHooks.hpp"
#include "CVehicleInfo.hpp"
#include "CWeaponInfo.hpp"
#include "CAnimationInfo.hpp"
#include "CMovePath.hpp"
#include "CRecordManager.hpp"
// SAMP includes
#include "CSAMPRakPeer.hpp"
#include "CFunctions.hpp"
#include "Structs.h"
#include "RPCs.h"
// SA includes
#include "CMaths.hpp"

// Managers includes
#include "CPlayerManager.hpp"
#include "CNodeManager.hpp"
#include "CCallbackManager.hpp"
// Entity includes
#include "CPlayerData.hpp"
// Scripting includes
#include "Natives.hpp"

// externals
extern CServer      *pServer;
extern CNetGameWrapper     *pNetGame;
#ifndef OMP_WRAPPER
extern CNetGame     *pNetGameSAMP;
extern void         **ppPluginData;
#ifdef SAMP_03DL
extern CArtInfo     *pArtInfo;
#endif
extern void         *pConsole;
extern void         *pRakServer;
#endif
extern char         szSampVersion[64];
extern char         szSampClient[64];

extern bool colInit;
extern bool colDataLoaded;
