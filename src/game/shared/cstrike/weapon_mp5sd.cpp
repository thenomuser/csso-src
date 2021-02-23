//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"


#if defined( CLIENT_DLL )

	#define CWeaponMP5SD C_WeaponMP5SD
	#include "c_cs_player.h"

#else

	#include "cs_player.h"

#endif


class CWeaponMP5SD : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponMP5SD, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponMP5SD();

	virtual void Spawn();
	virtual void PrimaryAttack();
	virtual bool Deploy();
	virtual bool Reload();

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_MP5SD; }
	virtual bool IsSilenced( void ) const				{ return true; }


private:
	CWeaponMP5SD( const CWeaponMP5SD & );

	void DoFireEffects( void );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponMP5SD, DT_WeaponMP5SD )

BEGIN_NETWORK_TABLE( CWeaponMP5SD, DT_WeaponMP5SD )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponMP5SD )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_mp5sd, CWeaponMP5SD );
#ifdef GAME_DLL
LINK_ENTITY_TO_CLASS( weapon_mp5navy, CWeaponMP5SD ); // for backwards compatibility
#endif
PRECACHE_WEAPON_REGISTER( weapon_mp5sd );



CWeaponMP5SD::CWeaponMP5SD()
{
}

void CWeaponMP5SD::Spawn()
{
	SetClassname( "weapon_mp5sd" ); // for backwards compatibility
	BaseClass::Spawn();
}


bool CWeaponMP5SD::Deploy()
{
	return BaseClass::Deploy();
}

bool CWeaponMP5SD::Reload()
{
	return BaseClass::Reload();
}

void CWeaponMP5SD::PrimaryAttack( void )
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	if ( !CSBaseGunFire( GetCSWpnData().m_flCycleTime[m_weaponMode], Primary_Mode ) )
		return;
}

void CWeaponMP5SD::DoFireEffects( void )
{
	// MP5SD is silenced, so do nothing
}