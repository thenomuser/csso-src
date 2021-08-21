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
//

//skins loadout
//CT
ConVar skin_p250_ct( "skin_p250_ct", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select p250 skin", true, 0, true, 41);
ConVar skin_ssg08_ct( "skin_ssg08_ct", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select ssg08 skin", true, 0, true, 26);
ConVar skin_xm1014_ct( "skin_xm1014_ct", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select xm1014 skin", true, 0, true, 29);
ConVar skin_aug( "skin_aug", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select aug skin", true, 0, true, 32);
ConVar skin_elite_ct( "skin_elite_ct", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select elite skin", true, 0, true, 29);
ConVar skin_fiveseven( "skin_fiveseven", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select fiveseven skin", true, 0, true, 30);
ConVar skin_ump45_ct( "skin_ump45_ct", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select ump45 skin", true, 0, true, 31);
ConVar skin_scar20( "skin_scar20", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select scar20 skin", true, 0, true, 24);
ConVar skin_famas( "skin_famas", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select famas skin", true, 0, true, 26);
ConVar skin_usp_silencer( "skin_usp_silencer", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select usp skin", true, 0, true, 28);
ConVar skin_awp_ct( "skin_awp_ct", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select awp skin", true, 0, true, 34);
ConVar skin_mp5sd_ct( "skin_mp5sd_ct", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select mp5sd skin", true, 0, true, 12);
ConVar skin_m249_ct( "skin_m249_ct", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select m249 skin", true, 0, true, 17);
ConVar skin_nova_ct( "skin_nova_ct", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select nova skin", true, 0, true, 32);
ConVar skin_m4a1( "skin_m4a1", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select m4a1 skin", true, 0, true, 29);
ConVar skin_mp9( "skin_mp9", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select mp9 skin", true, 0, true, 29);
ConVar skin_deagle_ct( "skin_deagle_ct", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select deagle skin", true, 0, true, 32);
ConVar skin_p90_ct( "skin_p90_ct", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select p90 skin", true, 0, true, 35);
ConVar skin_hkp2000( "skin_hkp2000", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select hkp2000 skin", true, 0, true, 27);
ConVar skin_m4a4( "skin_m4a4", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select m4a4 skin", true, 0, true, 34);
ConVar skin_revolver_ct( "skin_revolver_ct", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select revolver skin", true, 0, true, 16);
ConVar skin_cz75_ct( "skin_cz75_ct", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select cz75 skin", true, 0, true, 29);
ConVar skin_mag7( "skin_mag7", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select mag7 skin", true, 0, true, 27);
ConVar skin_negev_ct( "skin_negev_ct", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select negev skin", true, 0, true, 20);
ConVar skin_mp7_ct( "skin_mp7_ct", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select mp7 skin", true, 0, true, 29);
ConVar skin_bizon_ct( "skin_bizon_ct", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select bizon skin", true, 0, true, 29);

//knifes CT
ConVar skin_knife_ct_css( "skin_knife_ct_css", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select Clasic Knife Skin", true, 0, true, 13);
ConVar skin_knife_ct_karambit( "skin_knife_ct_karambit", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select Karambit Knife Skin", true, 0, true, 34);
ConVar skin_knife_ct_flip( "skin_knife_ct_flip", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select Flip Knife Skin", true, 0, true, 34);
ConVar skin_knife_ct_bayonet( "skin_knife_ct_bayonet", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select Bayonet Knife Skin", true, 0, true, 35);
ConVar skin_knife_ct_m9bayonet( "skin_knife_ct_m9bayonet", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select M9 Bayonet Knife Skin", true, 0, true, 35);
ConVar skin_knife_ct_butterfly( "skin_knife_ct_butterfly", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select Butterfly Knife Skin", true, 0, true, 24);
ConVar skin_knife_ct_gut( "skin_knife_ct_gut", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select Gut Knife Skin", true, 0, true, 34);
ConVar skin_knife_ct_huntsman( "skin_knife_ct_huntsman", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select Huntsman Knife Skin", true, 0, true, 24);
ConVar skin_knife_ct_falchion( "skin_knife_ct_falchion", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select Falchion Knife Skin", true, 0, true, 24);
ConVar skin_knife_ct_bowie( "skin_knife_ct_bowie", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select Bowie Knife Skin", true, 0, true, 24);
ConVar skin_knife_ct_survival( "skin_knife_ct_survival", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select Survival Knife Skin", true, 0, true, 12);
ConVar skin_knife_ct_paracord( "skin_knife_ct_paracord", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select Paracord Knife Skin", true, 0, true, 12);
ConVar skin_knife_ct_navaja( "skin_knife_ct_navaja", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select Navaja Knife Skin", true, 0, true, 24);
ConVar skin_knife_ct_nomad( "skin_knife_ct_nomad", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select Nomad Knife Skin", true, 0, true, 12);
ConVar skin_knife_ct_skeleton( "skin_knife_ct_skeleton", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select Skeleton Knife Skin", true, 0, true, 12);
ConVar skin_knife_ct_stiletto( "skin_knife_ct_stiletto", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select Stiletto Knife Skin", true, 0, true, 24);
ConVar skin_knife_ct_ursus( "skin_knife_ct_ursus", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select Ursus Knife Skin", true, 0, true, 24);
ConVar skin_knife_ct_talon( "skin_knife_ct_talon", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select Talon Knife Skin", true, 0, true, 24);
ConVar skin_knife_ct_shadowdaggers( "skin_knife_ct_shadowdaggers", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select Shadow Daggers Skin", true, 0, true, 24);

//knifes T
ConVar skin_knife_t_css( "skin_knife_t_css", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select Clasic Knife Skin", true, 0, true, 13);
ConVar skin_knife_t_karambit( "skin_knife_t_karambit", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select Karambit Knife Skin", true, 0, true, 34);
ConVar skin_knife_t_flip( "skin_knife_t_flip", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select Flip Knife Skin", true, 0, true, 34);
ConVar skin_knife_t_bayonet( "skin_knife_t_bayonet", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select Bayonet Knife Skin", true, 0, true, 35);
ConVar skin_knife_t_m9bayonet( "skin_knife_t_m9bayonet", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select M9 Bayonet Knife Skin", true, 0, true, 35);
ConVar skin_knife_t_butterfly( "skin_knife_t_butterfly", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select Butterfly Knife Skin", true, 0, true, 24);
ConVar skin_knife_t_gut( "skin_knife_t_gut", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select Gut Knife Skin", true, 0, true, 34);
ConVar skin_knife_t_huntsman( "skin_knife_t_huntsman", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select Huntsman Knife Skin", true, 0, true, 24);
ConVar skin_knife_t_falchion( "skin_knife_t_falchion", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select Falchion Knife Skin", true, 0, true, 24);
ConVar skin_knife_t_bowie( "skin_knife_t_bowie", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select Bowie Knife Skin", true, 0, true, 24);
ConVar skin_knife_t_survival( "skin_knife_t_survival", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select Survival Knife Skin", true, 0, true, 12);
ConVar skin_knife_t_paracord( "skin_knife_t_paracord", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select Paracord Knife Skin", true, 0, true, 12);
ConVar skin_knife_t_navaja( "skin_knife_t_navaja", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select Navaja Knife Skin", true, 0, true, 24);
ConVar skin_knife_t_nomad( "skin_knife_t_nomad", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select Nomad Knife Skin", true, 0, true, 12);
ConVar skin_knife_t_skeleton( "skin_knife_t_skeleton", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select Skeleton Knife Skin", true, 0, true, 12);
ConVar skin_knife_t_stiletto( "skin_knife_t_stiletto", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select Stiletto Knife Skin", true, 0, true, 24);
ConVar skin_knife_t_ursus( "skin_knife_t_ursus", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select Ursus Knife Skin", true, 0, true, 24);
ConVar skin_knife_t_talon( "skin_knife_t_talon", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select Talon Knife Skin", true, 0, true, 24);
ConVar skin_knife_t_shadowdaggers( "skin_knife_t_shadowdaggers", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select Shadow Daggers Skin", true, 0, true, 24);

//terrorist
ConVar skin_p250_t( "skin_p250_t", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select p250 skin", true, 0, true, 41);
ConVar skin_glock( "skin_glock", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select glock skin", true, 0, true, 34);
ConVar skin_ssg08_t( "skin_ssg08_t", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select ssg08 skin", true, 0, true, 26);
ConVar skin_xm1014_t( "skin_xm1014_t", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select xm1014 skin", true, 0, true, 29);
ConVar skin_mac10( "skin_mac10", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select mac10 skin", true, 0, true, 36);
ConVar skin_elite_t( "skin_elite_t", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select elite skin", true, 0, true, 29);
ConVar skin_ump45_t( "skin_ump45_t", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select ump45 skin", true, 0, true, 31);
ConVar skin_galilar( "skin_galilar", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select galilar skin", true, 0, true, 30);
ConVar skin_awp_t( "skin_awp_t", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select awp skin", true, 0, true, 34);
ConVar skin_mp5sd_t( "skin_mp5sd_t", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select mp5sd skin", true, 0, true, 12);
ConVar skin_m249_t( "skin_m249_t", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select m249 skin", true, 0, true, 17);
ConVar skin_nova_t( "skin_nova_t", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select nova skin", true, 0, true, 32);
ConVar skin_g3sg1( "skin_g3sg1", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select g3sg1 skin", true, 0, true, 25);
ConVar skin_deagle_t( "skin_deagle_t", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select deagle skin", true, 0, true, 32);
ConVar skin_sg556( "skin_sg556", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select sg556 skin", true, 0, true, 29);
ConVar skin_ak47( "skin_ak47", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select ak47 skin", true, 0, true, 38);
ConVar skin_p90_t( "skin_p90_t", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select p90 skin", true, 0, true, 35);
ConVar skin_tec9( "skin_tec9", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select tec9 skin", true, 0, true, 35);
ConVar skin_revolver_t( "skin_revolver_t", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select revolver skin", true, 0, true, 16);
ConVar skin_cz75_t( "skin_cz75_t", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select cz75 skin", true, 0, true, 29);
ConVar skin_sawedoff( "skin_sawedoff", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select sawedoff skin", true, 0, true, 28);
ConVar skin_negev_t( "skin_negev_t", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select negev skin", true, 0, true, 20);
ConVar skin_mp7_t( "skin_mp7_t", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select mp7 skin", true, 0, true, 29);
ConVar skin_bizon_t( "skin_bizon_t", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "select bizon skin", true, 0, true, 29);
#endif
//

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

LoadoutSlot_t CCSLoadout::GetSlotFromWeapon( int team, const char* weaponName )
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
			slot = (team == TEAM_CT) ? SLOT_MP7_CT : SLOT_MP7_T;
		}
		if ( slot == SLOT_DEAGLE_CT || slot == SLOT_DEAGLE_T )
		{
			slot = (team == TEAM_CT) ? SLOT_DEAGLE_CT : SLOT_DEAGLE_T;
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
			value = pPlayer->m_iLoadoutSlotKnifeWeaponCT + 2;
			break;
		case TEAM_TERRORIST:
			value = pPlayer->m_iLoadoutSlotKnifeWeaponT + 2;
			break;
		default:
			break;
	}

	return value;
}

int CCSLoadout::GetSkinsForPlayer( CCSPlayer* pPlayer)
{
	int value = 0;
#ifdef CLIENT_DLL
	if ( !pPlayer )
		return 0;
	if ( pPlayer->IsBotOrControllingBot() )
		return 0;
#else

	if ( !pPlayer )
		return 0;
	if ( pPlayer->IsBotOrControllingBot() )
		return 0;

	CWeaponCSBase *pCSWeapon = ( CWeaponCSBase* )pPlayer->GetActiveWeapon();
	int iWeaponId = pCSWeapon->GetCSWeaponID();


	if (pPlayer->GetTeamNumber() == TEAM_CT)
	{
		switch ( iWeaponId )
		{
			case WEAPON_P250:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_p250_ct" ));
				break;
			case WEAPON_SSG08:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_ssg08_ct" ));
				break;
			case WEAPON_XM1014:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_xm1014_ct" ));
				break;
			case WEAPON_AUG:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_aug" ));
				break;
			case WEAPON_ELITE:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_elite_ct" ));
				break;
			case WEAPON_FIVESEVEN:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_fiveseven" ));
				break;
			case WEAPON_UMP45:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_ump45_ct" ));
				break;
			case WEAPON_SCAR20:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_scar20" ));
				break;
			case WEAPON_FAMAS:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_famas" ));
				break;
			case WEAPON_USP:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_usp_silencer" ));
				break;
			case WEAPON_AWP:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_awp_ct" ));
				break;
			case WEAPON_MP5SD:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_mp5sd_ct" ));
				break;
			case WEAPON_M249:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_m249_ct" ));
				break;
			case WEAPON_NOVA:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_nova_ct" ));
				break;
			case WEAPON_M4A1:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_m4a1" ));
				break;
			case WEAPON_MP9:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_mp9" ));
				break;
			case WEAPON_DEAGLE:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_deagle_ct" ));
				break;
			case WEAPON_P90:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_p90_ct" ));
				break;
			case WEAPON_HKP2000:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_hkp2000" ));
				break;
			case WEAPON_M4A4:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_m4a4" ));
				break;
			case WEAPON_REVOLVER:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_revolver_ct" ));
				break;
			case WEAPON_CZ75:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_cz75_ct" ));
				break;
			case WEAPON_MAG7:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_mag7" ));
				break;
			case WEAPON_NEGEV:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_negev_ct" ));
				break;
			case WEAPON_MP7:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_mp7_ct" ));
				break;
			case WEAPON_BIZON:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_bizon_ct" ));
				break;

			//knifes

			case WEAPON_KNIFE_CSS:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_knife_ct_css" ));
				break;
			case WEAPON_KNIFE_KARAMBIT:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_knife_ct_karambit" ));
				break;
			case WEAPON_KNIFE_FLIP:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_knife_ct_flip" ));
				break;
			case WEAPON_KNIFE_BAYONET:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_knife_ct_bayonet" ));
				break;
			case WEAPON_KNIFE_M9_BAYONET:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_knife_ct_m9bayonet" ));
				break;
			case WEAPON_KNIFE_BUTTERFLY:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_knife_ct_butterfly" ));
				break;
			case WEAPON_KNIFE_GUT:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_knife_ct_gut" ));
				break;
			case WEAPON_KNIFE_TACTICAL:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_knife_ct_huntsman" ));
				break;
			case WEAPON_KNIFE_FALCHION:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_knife_ct_falchion" ));
				break;
			case WEAPON_KNIFE_SURVIVAL_BOWIE:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_knife_ct_bowie" ));
				break;
			case WEAPON_KNIFE_CANIS:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_knife_ct_survival" ));
				break;
			case WEAPON_KNIFE_CORD:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_knife_ct_paracord" ));
				break;
			case WEAPON_KNIFE_GYPSY:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_knife_ct_navaja" ));
				break;
			case WEAPON_KNIFE_OUTDOOR:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_knife_ct_nomad" ));
				break;
			case WEAPON_KNIFE_SKELETON:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_knife_ct_skeleton" ));
				break;
			case WEAPON_KNIFE_STILETTO:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_knife_ct_stiletto" ));
				break;
			case WEAPON_KNIFE_URSUS:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_knife_ct_ursus" ));
				break;
			case WEAPON_KNIFE_WIDOWMAKER:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_knife_ct_widowmaker" ));
				break;
			case WEAPON_KNIFE_PUSH:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_knife_ct_push" ));
				break;

			default:
				value = 0;
				break;
		}
	}
	else if (pPlayer->GetTeamNumber() == TEAM_TERRORIST)
	{
		switch ( iWeaponId )
		{
			case WEAPON_P250:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_p250_t" ));
				break;
			case WEAPON_GLOCK:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_glock" ));
				break;
			case WEAPON_SSG08:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_ssg08_t" ));
				break;
			case WEAPON_XM1014:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_xm1014_t" ));
				break;
			case WEAPON_MAC10:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_mac10" ));
				break;
			case WEAPON_ELITE:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_elite_t" ));
				break;
			case WEAPON_UMP45:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_ump45_t" ));
				break;
			case WEAPON_GALILAR:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_galilar" ));
				break;
			case WEAPON_AWP:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_awp_t" ));
				break;
			case WEAPON_MP5SD:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_mp5sd_t" ));
				break;
			case WEAPON_M249:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_m249_t" ));
				break;
			case WEAPON_NOVA:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_nova_t" ));
				break;
			case WEAPON_G3SG1:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_g3sg1" ));
				break;
			case WEAPON_DEAGLE:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_deagle_t" ));
				break;
			case WEAPON_SG556:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_sg556" ));
				break;
			case WEAPON_AK47:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_ak47" ));
				break;
			case WEAPON_P90:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_p90_t" ));
				break;
			case WEAPON_TEC9:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_tec9" ));
				break;
			case WEAPON_REVOLVER:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_revolver_t" ));
				break;
			case WEAPON_CZ75:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_cz75_t" ));
				break;
			case WEAPON_SAWEDOFF:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_sawedoff" ));
				break;
			case WEAPON_NEGEV:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_negev_t" ));
				break;
			case WEAPON_MP7:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_mp7_t" ));
				break;
			case WEAPON_BIZON:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_bizon_t" ));
				break;

			//knifes

			case WEAPON_KNIFE_CSS:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_knife_t_css" ));
				break;
			case WEAPON_KNIFE_KARAMBIT:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_knife_t_karambit" ));
				break;
			case WEAPON_KNIFE_FLIP:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_knife_t_flip" ));
				break;
			case WEAPON_KNIFE_BAYONET:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_knife_t_bayonet" ));
				break;
			case WEAPON_KNIFE_M9_BAYONET:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_knife_t_m9bayonet" ));
				break;
			case WEAPON_KNIFE_BUTTERFLY:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_knife_t_butterfly" ));
				break;
			case WEAPON_KNIFE_GUT:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_knife_t_gut" ));
				break;
			case WEAPON_KNIFE_TACTICAL:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_knife_t_huntsman" ));
				break;
			case WEAPON_KNIFE_FALCHION:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_knife_t_falchion" ));
				break;
			case WEAPON_KNIFE_SURVIVAL_BOWIE:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_knife_t_bowie" ));
				break;
			case WEAPON_KNIFE_CANIS:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_knife_t_survival" ));
				break;
			case WEAPON_KNIFE_CORD:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_knife_t_paracord" ));
				break;
			case WEAPON_KNIFE_GYPSY:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_knife_t_navaja" ));
				break;
			case WEAPON_KNIFE_OUTDOOR:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_knife_t_nomad" ));
				break;
			case WEAPON_KNIFE_SKELETON:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_knife_t_skeleton" ));
				break;
			case WEAPON_KNIFE_STILETTO:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_knife_t_stiletto" ));
				break;
			case WEAPON_KNIFE_URSUS:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_knife_t_ursus" ));
				break;
			case WEAPON_KNIFE_WIDOWMAKER:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_knife_t_widowmaker" ));
				break;
			case WEAPON_KNIFE_PUSH:
				value = atoi( engine->GetClientConVarValue( pPlayer->entindex(), "skin_knife_t_push" ));
				break;

			default:
				value = 0;
				break;
		}
	}
#endif
	return value;
}
