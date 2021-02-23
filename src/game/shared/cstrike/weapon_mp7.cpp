//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"


#if defined( CLIENT_DLL )

	#define CWeaponMP7 C_WeaponMP7
	#include "c_cs_player.h"

#else

	#include "cs_player.h"

#endif


class CWeaponMP7 : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponMP7, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponMP7();

	virtual void PrimaryAttack();

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_MP7; }


private:
	CWeaponMP7( const CWeaponMP7 & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponMP7, DT_WeaponMP7 )

BEGIN_NETWORK_TABLE( CWeaponMP7, DT_WeaponMP7 )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponMP7 )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_mp7, CWeaponMP7 );
PRECACHE_WEAPON_REGISTER( weapon_mp7 );



CWeaponMP7::CWeaponMP7()
{
}

void CWeaponMP7::PrimaryAttack( void )
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	if ( !CSBaseGunFire( GetCSWpnData().m_flCycleTime[m_weaponMode], Primary_Mode ) )
		return;
}