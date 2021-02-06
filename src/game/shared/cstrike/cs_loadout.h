#ifndef CSLOADOUT_H
#define CSLOADOUT_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "baseplayer_shared.h"
#include "weapon_csbase.h"

#ifdef CLIENT_DLL
#define CCSLoadout C_CSLoadout
#define CBasePlayer C_BasePlayer
#define CCSPlayer C_CSPlayer
#endif

class CCSPlayer;

// Loadout slots
enum LoadoutSlot_t
{
	SLOT_NONE = 0,
	SLOT_M4,
	SLOT_HKP2000,
	SLOT_FIVESEVEN,
	SLOT_TEC9,
	SLOT_MP7_CT,
	SLOT_MP7_T,
	SLOT_DEAGLE_CT,
	SLOT_DEAGLE_T,
};

struct CLoadout
{
	LoadoutSlot_t	m_iLoadoutSlot;
	const char*		m_szCommand;
	const char*		m_szFirstWeapon;
	const char*		m_szSecondWeapon;
};

class CCSLoadout : public CAutoGameSystemPerFrame
{
public:
	DECLARE_CLASS_GAMEROOT( CCSLoadout, CAutoGameSystemPerFrame );

	virtual char const *Name() { return "CCSLoadout"; }

	CCSLoadout( void );
	~CCSLoadout( void );

public:
	// check if the given weapon is actually a knife
	bool			IsKnife( CSWeaponID weaponid ) { return (weaponid >= WEAPON_KNIFE) && (weaponid <= WEAPON_KNIFE + MAX_KNIVES + 1); }
	// get the loadout slot of the weapon
	LoadoutSlot_t	GetSlotFromWeapon( CBasePlayer* pPlayer, const char* weaponName );
	// get the weapon from a client's slot
	const char*		GetWeaponFromSlot( CBasePlayer* pPlayer, LoadoutSlot_t slot );
	
	bool			HasGlovesSet( CCSPlayer* pPlayer, int team );
	int				GetGlovesForPlayer( CCSPlayer* pPlayer, int team );
};


extern CCSLoadout *g_pCSLoadout;

inline CCSLoadout *CSLoadout()
{
	return g_pCSLoadout;
}

#endif