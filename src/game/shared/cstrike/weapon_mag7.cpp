//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"
#include "fx_cs_shared.h"


#if defined( CLIENT_DLL )

	#define CWeaponMAG7 C_WeaponMAG7
	#include "c_cs_player.h"

#else

	#include "cs_player.h"
	#include "te_shotgun_shot.h"

#endif


class CWeaponMAG7 : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponMAG7, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponMAG7();

	virtual void PrimaryAttack();

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_MAG7; }

private:

	CWeaponMAG7( const CWeaponMAG7 & );

};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponMAG7, DT_WeaponMAG7 )

BEGIN_NETWORK_TABLE( CWeaponMAG7, DT_WeaponMAG7 )
END_NETWORK_TABLE()

#if defined(CLIENT_DLL)
BEGIN_PREDICTION_DATA( CWeaponMAG7 )
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_mag7, CWeaponMAG7 );
PRECACHE_WEAPON_REGISTER( weapon_mag7 );


CWeaponMAG7::CWeaponMAG7()
{
}

void CWeaponMAG7::PrimaryAttack()
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	float flCycleTime = GetCSWpnData().m_flCycleTime[m_weaponMode];

	// don't fire underwater
	if ( pPlayer->GetWaterLevel() == 3 )
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.15;
		return;
	}

	if ( m_iClip1 <= 0 )
	{
		Reload();

		if ( m_iClip1 == 0 )
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.25;
		}

		return;
	}

	SendWeaponAnim( ACT_VM_PRIMARYATTACK );

	//pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	//pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	m_iClip1--;
	pPlayer->DoMuzzleFlash();

	// player "shoot" animation
	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	// Dispatch the FX right away with full accuracy.
	float flCurAttack = CalculateNextAttackTime( flCycleTime );
	FX_FireBullets(
		pPlayer->entindex(),
		pPlayer->Weapon_ShootPosition(),
		pPlayer->GetFinalAimAngle(),
		GetWeaponID(),
		Primary_Mode,
		CBaseEntity::GetPredictionRandomSeed() & 255, // wrap it for network traffic so it's the same between client and server
		GetInaccuracy(),
		GetSpread(), // flSpread
		flCurAttack );

	if ( !m_iClip1 && pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate( "!HEV_AMO0", false, 0 );
	}

	if ( m_iClip1 != 0 )
		SetWeaponIdleTime( gpGlobals->curtime + 2.5 );
	else
		SetWeaponIdleTime( gpGlobals->curtime + 0.25 );

	// update accuracy
	m_fAccuracyPenalty += GetCSWpnData().m_fInaccuracyImpulseFire[Primary_Mode];

	// table driven recoil
	Recoil( m_weaponMode );

	m_flRecoilIndex += 1.0f;
}