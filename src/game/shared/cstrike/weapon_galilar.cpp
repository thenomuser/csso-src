//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"


#if defined( CLIENT_DLL )

	#define CWeaponGalilAR C_WeaponGalilAR
	#include "c_cs_player.h"

#else

	#include "cs_player.h"

#endif


class CWeaponGalilAR : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponGalilAR, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponGalilAR();

	virtual void Spawn();
	virtual void PrimaryAttack();

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_GALILAR; }

private:

	CWeaponGalilAR( const CWeaponGalilAR & );

	void GalilFire( float flSpread );

};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponGalilAR, DT_WeaponGalilAR )

BEGIN_NETWORK_TABLE( CWeaponGalilAR, DT_WeaponGalilAR )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponGalilAR )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_galilar, CWeaponGalilAR );
#ifdef GAME_DLL
LINK_ENTITY_TO_CLASS( weapon_galil, CWeaponGalilAR ); // for backwards compatibility
#endif
PRECACHE_WEAPON_REGISTER( weapon_galilar );



CWeaponGalilAR::CWeaponGalilAR()
{
}

void CWeaponGalilAR::PrimaryAttack()
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	// don't fire underwater
	if (pPlayer->GetWaterLevel() == 3)
	{
		PlayEmptySound( );
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.15;
		return;
	}
	
	if ( !CSBaseGunFire( GetCSWpnData().m_flCycleTime[m_weaponMode], Primary_Mode ) )
		return;
}

void CWeaponGalilAR::Spawn()
{
	SetClassname( "weapon_galilar" ); // for backwards compatibility
	BaseClass::Spawn();
}