//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"
#include "npcevent.h"

#if defined( CLIENT_DLL )

	#define CWeaponM4A4 C_WeaponM4A4
	#include "c_cs_player.h"

#else

	#include "cs_player.h"

#endif


class CWeaponM4A4 : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponM4A4, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();
	
	CWeaponM4A4();

	virtual void Spawn();
	virtual void Precache();

	virtual void PrimaryAttack();
	virtual bool Deploy();
	virtual bool Reload();
	virtual void WeaponIdle();

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_M4A4; }

private:

	CWeaponM4A4( const CWeaponM4A4 & );

	bool m_inPrecache;
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponM4A4, DT_WeaponM4A4 )

BEGIN_NETWORK_TABLE( CWeaponM4A4, DT_WeaponM4A4 )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponM4A4 )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_m4a4, CWeaponM4A4 );
PRECACHE_WEAPON_REGISTER( weapon_m4a4 );



CWeaponM4A4::CWeaponM4A4()
{
	m_inPrecache = false;
}


void CWeaponM4A4::Spawn()
{
	BaseClass::Spawn();

	m_bDelayFire = true;
}


void CWeaponM4A4::Precache()
{
	m_inPrecache = true;
	BaseClass::Precache();

	m_inPrecache = false;
}

bool CWeaponM4A4::Deploy()
{
	bool ret = BaseClass::Deploy();

	m_bDelayFire = true;

	return ret;
}

void CWeaponM4A4::PrimaryAttack()
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	if ( !CSBaseGunFire( GetCSWpnData().m_flCycleTime, m_weaponMode ) )
		return;

	pPlayer = GetPlayerOwner();

	// CSBaseGunFire can kill us, forcing us to drop our weapon, if we shoot something that explodes
	if ( !pPlayer )
		return;

	DoFireEffects();

	if (pPlayer->GetAbsVelocity().Length2D() > 5)
		pPlayer->KickBack (1.0, 0.45, 0.28, 0.045, 3.75, 3, 7);
	else if ( !FBitSet( pPlayer->GetFlags(), FL_ONGROUND ) )
		pPlayer->KickBack (1.2, 0.5, 0.23, 0.15, 5.5, 3.5, 6);
	else if ( FBitSet( pPlayer->GetFlags(), FL_DUCKING ) )
		pPlayer->KickBack (0.6, 0.3, 0.2, 0.0125, 3.25, 2, 7);
	else
		pPlayer->KickBack (0.65, 0.35, 0.25, 0.015, 3.5, 2.25, 7);
}

bool CWeaponM4A4::Reload()
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return false;

	if ( GetReserveAmmoCount( AMMO_POSITION_PRIMARY ) <= 0 )
		return false;

	int iResult = DefaultReload( GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD );

	if ( !iResult )
		return false;

	pPlayer->SetAnimation( PLAYER_RELOAD );

	if ( (iResult) && (pPlayer->GetFOV() != pPlayer->GetDefaultFOV()) )
	{
		pPlayer->SetFOV( pPlayer, pPlayer->GetDefaultFOV() );
	}

	m_flAccuracy = 0.2;
	pPlayer->m_iShotsFired = 0;
	m_bDelayFire = false;
	return true;
}

void CWeaponM4A4::WeaponIdle()
{
	if (m_flTimeWeaponIdle > gpGlobals->curtime)
		return;

	// only idle if the slid isn't back
	if ( m_iClip1 != 0 )
	{
		SetWeaponIdleTime( gpGlobals->curtime + GetCSWpnData().m_flIdleInterval );
		SendWeaponAnim( ACT_VM_IDLE );
	}
}
