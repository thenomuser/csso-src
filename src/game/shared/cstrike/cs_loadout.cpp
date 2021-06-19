//========= Copyright PiMoNFeeD, CS:SO, All rights reserved. ==================//
//
// Purpose: player loadout
//
//=============================================================================//

#include "cbase.h"
#include "cs_loadout.h"
#include "cs_shareddefs.h"
#ifdef CLIENT_DLL
#include "c_cs_player.h"
#else
#include "cs_player.h"
#endif

#ifdef CLIENT_DLL
ConVar loadout_slot_m4_weapon( "loadout_slot_m4_weapon", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Which weapon to use in M4 slot.\n 0 - M4A4\n 1 - M4A1-S", true, 0, true, 1 );
ConVar loadout_slot_hkp2000_weapon( "loadout_slot_hkp2000_weapon", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Which weapon to use in HKP2000 slot.\n 0 - HKP2000\n 1 - USP-S", true, 0, true, 1 );
ConVar loadout_slot_knife_weapon_ct( "loadout_slot_knife_weapon_ct", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Which weapon to use in knife slot for CTs.\n 0 - Default CT knife\n 1 - CS:S knife\n 2 - Karambit\n 3 - Flip\n 4 - Bayonet\n 5 - M9 Bayonet\n 6 - Butterfly\n 7 - Gut\n 8 - Huntsman\n 9 - Falchion\n 10 - Bowie\n 11 - Survival\n 12 - Paracord\n 13 - Navaja\n 14 - Nomad\n 15 - Skeleton\n 16 - Stiletto\n 17 - Ursus\n 18 - Talon", true, 0, true, MAX_KNIVES );
ConVar loadout_slot_knife_weapon_t( "loadout_slot_knife_weapon_t", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Which weapon to use in knife slot for Ts.\n 0 - Default T knife\n 1 - CS:S knife\n 2 - Karambit\n 3 - Flip\n 4 - Bayonet\n 5 - M9 Bayonet\n 6 - Butterfly\n 7 - Gut\n 8 - Huntsman\n 9 - Falchion\n 10 - Bowie\n 11 - Survival\n 12 - Paracord\n 13 - Navaja\n 14 - Nomad\n 15 - Skeleton\n 16 - Stiletto\n 17 - Ursus\n 18 - Talon", true, 0, true, MAX_KNIVES );
ConVar loadout_slot_fiveseven_weapon( "loadout_slot_fiveseven_weapon", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Which weapon to use in Five-SeveN slot.\n 0 - Five-SeveN\n 1 - CZ-75", true, 0, true, 1 );
ConVar loadout_slot_tec9_weapon( "loadout_slot_tec9_weapon", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Which weapon to use in Tec-9 slot.\n 0 - Tec-9\n 1 - CZ-75", true, 0, true, 1 );
ConVar loadout_slot_mp7_weapon_ct( "loadout_slot_mp7_weapon_ct", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Which weapon to use in MP7 slot for CTs.\n 0 - MP7\n 1 - MP5SD", true, 0, true, 1 );
ConVar loadout_slot_mp7_weapon_t( "loadout_slot_mp7_weapon_t", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Which weapon to use in MP7 slot for Ts.\n 0 - MP7\n 1 - MP5SD", true, 0, true, 1 );
ConVar loadout_slot_deagle_weapon_ct( "loadout_slot_deagle_weapon_ct", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Which weapon to use in Deagle slot for CTs.\n 0 - Deagle\n 1 - R8 Revolver", true, 0, true, 1 );
ConVar loadout_slot_deagle_weapon_t( "loadout_slot_deagle_weapon_t", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Which weapon to use in Deagle slot for Ts.\n 0 - Deagle\n 1 - R8 Revolver", true, 0, true, 1 );
ConVar loadout_slot_agent_ct( "loadout_slot_agent_ct", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Which agent to use for CTs.", true, 0, true, MAX_AGENTS_CT );
ConVar loadout_slot_agent_t( "loadout_slot_agent_t", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Which agent to use for Ts.", true, 0, true, MAX_AGENTS_T );
ConVar loadout_slot_gloves_ct( "loadout_slot_gloves_ct", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Which gloves to use for CTs.", true, 0, true, MAX_GLOVES );
ConVar loadout_slot_gloves_t( "loadout_slot_gloves_t", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Which gloves to use for Ts.", true, 0, true, MAX_GLOVES );
ConVar loadout_stattrak( "loadout_stattrak", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Enable or disable StatTrak on weapons.", true, 0, true, 1 );
#endif

CCSLoadout*	g_pCSLoadout = NULL;
CCSLoadout::CCSLoadout() : CAutoGameSystemPerFrame("CCSLoadout")
{
	Assert( !g_pCSLoadout );
	g_pCSLoadout = this;
}
CCSLoadout::~CCSLoadout()
{
	Assert( g_pCSLoadout == this );
	g_pCSLoadout = NULL;
}

CLoadout WeaponLoadout[]
{
	{	SLOT_M4,		"loadout_slot_m4_weapon",			"m4a4",			"m4a1_silencer"	},
	{	SLOT_HKP2000,	"loadout_slot_hkp2000_weapon",		"hkp2000",		"usp_silencer"	},
	{	SLOT_FIVESEVEN,	"loadout_slot_fiveseven_weapon",	"fiveseven",	"cz75"			},
	{	SLOT_TEC9,		"loadout_slot_tec9_weapon",			"tec9",			"cz75"			},
	{	SLOT_MP7_CT,	"loadout_slot_mp7_weapon_ct",		"mp7",			"mp5sd"			},
	{	SLOT_MP7_T,		"loadout_slot_mp7_weapon_t",		"mp7",			"mp5sd"			},
	{	SLOT_DEAGLE_CT,	"loadout_slot_deagle_weapon_ct",	"deagle",		"revolver"		},
	{	SLOT_DEAGLE_T,	"loadout_slot_deagle_weapon_t",		"deagle",		"revolver"		},
};

LoadoutSlot_t CCSLoadout::GetSlotFromWeapon( CBasePlayer* pPlayer, const char* weaponName )
{
	LoadoutSlot_t slot = SLOT_NONE;

	for ( int i = 0; i < ARRAYSIZE( WeaponLoadout ); i++ )
	{
		if ( Q_strcmp( WeaponLoadout[i].m_szFirstWeapon, weaponName ) == 0 )
			slot = WeaponLoadout[i].m_iLoadoutSlot;
		else if ( Q_strcmp( WeaponLoadout[i].m_szSecondWeapon, weaponName ) == 0 )
			slot = WeaponLoadout[i].m_iLoadoutSlot;

		if ( slot == SLOT_MP7_CT || slot == SLOT_MP7_T )
		{
			if ( pPlayer )
				slot = (pPlayer->GetTeamNumber() == TEAM_CT) ? SLOT_MP7_CT : SLOT_MP7_T;
		}
		if ( slot == SLOT_DEAGLE_CT || slot == SLOT_DEAGLE_T )
		{
			if ( pPlayer )
				slot = (pPlayer->GetTeamNumber() == TEAM_CT) ? SLOT_DEAGLE_CT : SLOT_DEAGLE_T;
		}

		if ( slot != SLOT_NONE )
			break;
	}
	return slot;
}
const char* CCSLoadout::GetWeaponFromSlot( CBasePlayer* pPlayer, LoadoutSlot_t slot )
{
	for ( int i = 0; i < ARRAYSIZE( WeaponLoadout ); i++ )
	{
		if ( WeaponLoadout[i].m_iLoadoutSlot == slot )
		{
			int value = 0;
#ifdef CLIENT_DLL
			ConVarRef convar( WeaponLoadout[i].m_szCommand );
			if (convar.IsValid())
				value = convar.GetInt();
#else
			value = atoi( engine->GetClientConVarValue( engine->IndexOfEdict( pPlayer->edict() ), WeaponLoadout[i].m_szCommand ) );
#endif
			return (value > 0) ? WeaponLoadout[i].m_szSecondWeapon : WeaponLoadout[i].m_szFirstWeapon;
		}
	}

	return NULL;
}

bool CCSLoadout::HasGlovesSet( CCSPlayer* pPlayer, int team )
{
	if ( !pPlayer )
		return false;

	if ( pPlayer->IsBotOrControllingBot() )
		return false;

	int value = 0;
	switch ( team )
	{
		case TEAM_CT:
			value = pPlayer->m_iLoadoutSlotGlovesCT;
			break;
		case TEAM_TERRORIST:
			value = pPlayer->m_iLoadoutSlotGlovesT;
			break;
		default:
			break;
	}

	return (value > 0) ? true : false;
}

int CCSLoadout::GetGlovesForPlayer( CCSPlayer* pPlayer, int team )
{
	if ( !pPlayer )
		return 0;

	if ( pPlayer->IsBotOrControllingBot() )
		return 0;

	int value = 0;
	switch ( team )
	{
		case TEAM_CT:
			value = pPlayer->m_iLoadoutSlotGlovesCT;
			break;
		case TEAM_TERRORIST:
			value = pPlayer->m_iLoadoutSlotGlovesT;
			break;
		default:
			break;
	}

	return value;
}

bool CCSLoadout::HasKnifeSet( CCSPlayer* pPlayer, int team )
{
	if ( !pPlayer )
		return false;

	if ( pPlayer->IsBotOrControllingBot() )
		return false;

	int value = 0;
	switch ( team )
	{
		case TEAM_CT:
			value = pPlayer->m_iLoadoutSlotKnifeWeaponCT;
			break;
		case TEAM_TERRORIST:
			value = pPlayer->m_iLoadoutSlotKnifeWeaponT;
			break;
		default:
			break;
	}

	return (value > 0) ? true : false;
}

int CCSLoadout::GetKnifeForPlayer( CCSPlayer* pPlayer, int team )
{
	if ( !pPlayer )
		return 0;

	if ( pPlayer->IsBotOrControllingBot() )
		return 0;

	int value = 0;
	switch ( team )
	{
		case TEAM_CT:
			value = pPlayer->m_iLoadoutSlotKnifeWeaponCT + 1;
			break;
		case TEAM_TERRORIST:
			value = pPlayer->m_iLoadoutSlotKnifeWeaponT + 1;
			break;
		default:
			break;
	}

	return value;
}