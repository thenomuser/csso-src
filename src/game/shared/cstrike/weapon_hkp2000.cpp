//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbase.h"
#include "fx_cs_shared.h"
#include "npcevent.h"


#if defined( CLIENT_DLL )

#define CWeaponHKP2000 C_WeaponHKP2000
#include "c_cs_player.h"

#else

#include "cs_player.h"

#endif


class CWeaponHKP2000 : public CWeaponCSBase
{
public:
	DECLARE_CLASS( CWeaponHKP2000, CWeaponCSBase );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CWeaponHKP2000();

	virtual void Spawn();

	virtual void PrimaryAttack();
	virtual bool Deploy();

	virtual bool Reload();
	virtual void WeaponIdle();

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_HKP2000; }

private:
	CWeaponHKP2000( const CWeaponHKP2000 & );

	float m_flLastFire;
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponHKP2000, DT_WeaponHKP2000 )

BEGIN_NETWORK_TABLE( CWeaponHKP2000, DT_WeaponHKP2000 )
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CWeaponHKP2000 )
DEFINE_FIELD( m_flLastFire, FIELD_FLOAT ),
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_hkp2000, CWeaponHKP2000 );
PRECACHE_WEAPON_REGISTER( weapon_hkp2000 );



CWeaponHKP2000::CWeaponHKP2000()
{
	m_flLastFire = gpGlobals->curtime;
}


void CWeaponHKP2000::Spawn()
{
	BaseClass::Spawn();

	//m_iDefaultAmmo = 12;

	//FallInit();// get ready to fall down.
}


bool CWeaponHKP2000::Deploy()
{
	return BaseClass::Deploy();
}

void CWeaponHKP2000::PrimaryAttack()
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	m_flLastFire = gpGlobals->curtime;
	
	if (m_iClip1 <= 0)
	{
		if ( m_bFireOnEmpty )
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.1f;
			m_bFireOnEmpty = false;
		}

		return;
	}

	pPlayer->m_iShotsFired++;

	m_iClip1--;
	
	 pPlayer->DoMuzzleFlash();
	//SetPlayerShieldAnim();

	SendWeaponAnim( ACT_VM_PRIMARYATTACK );

	// player "shoot" animation
	pPlayer->SetAnimation( PLAYER_ATTACK1 );
		
	// Aiming
	FX_FireBullets(
		pPlayer->entindex(),
		pPlayer->Weapon_ShootPosition(),
		pPlayer->GetFinalAimAngle(),
		GetWeaponID(),
		Primary_Mode,
		CBaseEntity::GetPredictionRandomSeed() & 255,
		GetInaccuracy(),
		GetSpread());
	
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + GetCSWpnData().m_flCycleTime[m_weaponMode];

	if (!m_iClip1 && pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) <= 0)
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", false, 0);
	}

	SetWeaponIdleTime( gpGlobals->curtime + 2 );

	//ResetPlayerShieldAnim();

	// update accuracy
	m_fAccuracyPenalty += GetCSWpnData().m_fInaccuracyImpulseFire[Primary_Mode];

	// table driven recoil
	Recoil( m_weaponMode );

	m_flRecoilIndex += 1.0f;
}


bool CWeaponHKP2000::Reload()
{
	return DefaultPistolReload();
}

void CWeaponHKP2000::WeaponIdle()
{
	if ( m_flTimeWeaponIdle > gpGlobals->curtime )
		return;

	// only idle if the slid isn't back
	if ( m_iClip1 != 0 )
	{
		SetWeaponIdleTime( gpGlobals->curtime + 6.0 );
	}
}
