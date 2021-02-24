//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbase.h"
#include "fx_cs_shared.h"

#if defined( CLIENT_DLL )

	#define CWeaponTec9 C_WeaponTec9
	#include "c_cs_player.h"

#else

	#include "cs_player.h"

#endif


class CWeaponTec9 : public CWeaponCSBase
{
public:
	DECLARE_CLASS( CWeaponTec9, CWeaponCSBase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponTec9();

	virtual void Spawn();

	virtual void PrimaryAttack();
	virtual void SecondaryAttack();
	virtual bool Deploy();

	virtual bool Reload();

	virtual void WeaponIdle();

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_TEC9; }

private:
	
	CWeaponTec9( const CWeaponTec9 & );
	
	float m_flLastFire;
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponTec9, DT_WeaponTec9 )

BEGIN_NETWORK_TABLE( CWeaponTec9, DT_WeaponTec9 )
END_NETWORK_TABLE()

#if defined CLIENT_DLL
BEGIN_PREDICTION_DATA( CWeaponTec9 )
	DEFINE_FIELD( m_flLastFire, FIELD_FLOAT ),
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_tec9, CWeaponTec9 );
PRECACHE_WEAPON_REGISTER( weapon_tec9 );



CWeaponTec9::CWeaponTec9()
{
	m_flLastFire = gpGlobals->curtime;
}


void CWeaponTec9::Spawn()
{
	BaseClass::Spawn();
}

bool CWeaponTec9::Deploy()
{
	return BaseClass::Deploy();
}

void CWeaponTec9::PrimaryAttack()
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

	// table driven recoil
	Recoil( m_weaponMode );

	m_flRecoilIndex += 1.0f;
}


void CWeaponTec9::SecondaryAttack()
{
}


bool CWeaponTec9::Reload()
{
	return DefaultPistolReload();
}

void CWeaponTec9::WeaponIdle()
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
