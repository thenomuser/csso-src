//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbase.h"
#include "fx_cs_shared.h"

#if defined( CLIENT_DLL )

	#define CWeaponFiveSeven C_WeaponFiveSeven
	#include "c_cs_player.h"

#else

	#include "cs_player.h"

#endif


class CWeaponFiveSeven : public CWeaponCSBase
{
public:
	DECLARE_CLASS( CWeaponFiveSeven, CWeaponCSBase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponFiveSeven();

	virtual void Spawn();

	virtual void PrimaryAttack();
	virtual void SecondaryAttack();
	virtual bool Deploy();

	virtual bool Reload();

	virtual void WeaponIdle();

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_FIVESEVEN; }

private:
	
	CWeaponFiveSeven( const CWeaponFiveSeven & );
	
	float m_flLastFire;
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponFiveSeven, DT_WeaponFiveSeven )

BEGIN_NETWORK_TABLE( CWeaponFiveSeven, DT_WeaponFiveSeven )
END_NETWORK_TABLE()

#if defined CLIENT_DLL
BEGIN_PREDICTION_DATA( CWeaponFiveSeven )
	DEFINE_FIELD( m_flLastFire, FIELD_FLOAT ),
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_fiveseven, CWeaponFiveSeven );
PRECACHE_WEAPON_REGISTER( weapon_fiveseven );



CWeaponFiveSeven::CWeaponFiveSeven()
{
	m_flLastFire = gpGlobals->curtime;
}


void CWeaponFiveSeven::Spawn( )
{
	BaseClass::Spawn();
}

bool CWeaponFiveSeven::Deploy()
{
	return BaseClass::Deploy();
}

void CWeaponFiveSeven::PrimaryAttack()
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

	SendWeaponAnim( ACT_VM_PRIMARYATTACK );
	// player "shoot" animation
	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	
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

	if (!m_iClip1 && pPlayer->GetAmmoCount( GetPrimaryAmmoType() ) <= 0)
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", false, 0);
	}

	SetWeaponIdleTime( gpGlobals->curtime + 2 );

	// update accuracy
	m_fAccuracyPenalty += GetCSWpnData().m_fInaccuracyImpulseFire[Primary_Mode];

	// table driven recoil
	Recoil( m_weaponMode );

	m_flRecoilIndex += 1.0f;
}


void CWeaponFiveSeven::SecondaryAttack() 
{
}


bool CWeaponFiveSeven::Reload()
{
	return DefaultPistolReload();
}

void CWeaponFiveSeven::WeaponIdle()
{
	if (m_flTimeWeaponIdle > gpGlobals->curtime)
		return;

	// only idle if the slid isn't back
	if (m_iClip1 != 0)
	{	
		SetWeaponIdleTime( gpGlobals->curtime + 4 );
		SendWeaponAnim( ACT_VM_IDLE );
	}
}
