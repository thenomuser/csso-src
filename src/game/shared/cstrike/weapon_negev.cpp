//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"


#if defined( CLIENT_DLL )

	#define CWeaponNegev C_WeaponNegev
	#include "c_cs_player.h"

#else

	#include "cs_player.h"

#endif


class CWeaponNegev : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponNegev, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponNegev();

	virtual void PrimaryAttack();

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_NEGEV; }


private:
	CWeaponNegev( const CWeaponNegev & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponNegev, DT_WeaponNegev )

BEGIN_NETWORK_TABLE( CWeaponNegev, DT_WeaponNegev )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponNegev )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_negev, CWeaponNegev );
PRECACHE_WEAPON_REGISTER( weapon_negev );



CWeaponNegev::CWeaponNegev()
{
}

void CWeaponNegev::PrimaryAttack( void )
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	if ( !CSBaseGunFire( GetCSWpnData().m_flCycleTime[m_weaponMode], Primary_Mode ) )
		return;
}
