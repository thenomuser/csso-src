//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"


#if defined( CLIENT_DLL )

	#define CWeaponBizon C_WeaponBizon
	#include "c_cs_player.h"

#else

	#include "cs_player.h"

#endif


class CWeaponBizon : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponBizon, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponBizon();

	virtual void PrimaryAttack();

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_BIZON; }


private:
	CWeaponBizon( const CWeaponBizon & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponBizon, DT_WeaponBizon )

BEGIN_NETWORK_TABLE( CWeaponBizon, DT_WeaponBizon )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponBizon )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_bizon, CWeaponBizon );
PRECACHE_WEAPON_REGISTER( weapon_bizon );



CWeaponBizon::CWeaponBizon()
{
}

void CWeaponBizon::PrimaryAttack( void )
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	if ( !CSBaseGunFire( GetCSWpnData().m_flCycleTime[m_weaponMode], Primary_Mode ) )
		return;
}