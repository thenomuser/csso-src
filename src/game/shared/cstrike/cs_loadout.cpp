#include "cbase.h"
#include "cs_loadout.h"
#include "cs_shareddefs.h"

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
#ifdef CLIENT_DLL
const char* CCSLoadout::GetWeaponFromSlot( LoadoutSlot_t slot )
#else
const char* CCSLoadout::GetWeaponFromSlot( const edict_t *edict, LoadoutSlot_t slot )
#endif
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
			value = atoi( engine->GetClientConVarValue( engine->IndexOfEdict( edict ), WeaponLoadout[i].m_szCommand ) );
#endif
			return (value > 0) ? WeaponLoadout[i].m_szSecondWeapon : WeaponLoadout[i].m_szFirstWeapon;
		}
	}

	return NULL;
}