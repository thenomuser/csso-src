//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"


#if defined( CLIENT_DLL )

	#define CWeaponMP9 C_WeaponMP9
	#include "c_cs_player.h"

#else

	#include "cs_player.h"

#endif


class CWeaponMP9 : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponMP9, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponMP9();

	virtual void Spawn();
	virtual void PrimaryAttack();
	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_MP9; }

private:

	CWeaponMP9( const CWeaponMP9 & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponMP9, DT_WeaponMP9 )

BEGIN_NETWORK_TABLE( CWeaponMP9, DT_WeaponMP9 )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponMP9 )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_mp9, CWeaponMP9 );
#ifdef GAME_DLL
LINK_ENTITY_TO_CLASS( weapon_tmp, CWeaponMP9 ); // for backwards compatibility
#endif
PRECACHE_WEAPON_REGISTER( weapon_mp9 );


CWeaponMP9::CWeaponMP9()
{
}

void CWeaponMP9::PrimaryAttack( void )
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	if ( !CSBaseGunFire( GetCSWpnData().m_flCycleTime[m_weaponMode], Primary_Mode ) )
		return;
}

void CWeaponMP9::Spawn()
{
	SetClassname( "weapon_mp9" ); // for backwards compatibility
	BaseClass::Spawn();
}