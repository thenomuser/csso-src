//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbase.h"
#include "fx_cs_shared.h"


#if defined( CLIENT_DLL )

	#define CWeaponP250 C_WeaponP250
	#include "c_cs_player.h"

#else

	#include "cs_player.h"

#endif


class CWeaponP250 : public CWeaponCSBase
{
public:
	DECLARE_CLASS( CWeaponP250, CWeaponCSBase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponP250();

	virtual void Spawn();

	virtual void PrimaryAttack();
	virtual bool Deploy();

	virtual bool Reload();
	virtual void WeaponIdle();

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_P250; }

private:
	
	CWeaponP250( const CWeaponP250 & );

	float m_flLastFire;
};

#if defined CLIENT_DLL
BEGIN_PREDICTION_DATA( CWeaponP250 )
	DEFINE_FIELD( m_flLastFire, FIELD_FLOAT ),
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_p250, CWeaponP250 );
#ifdef GAME_DLL
LINK_ENTITY_TO_CLASS( weapon_p228, CWeaponP250 ); // for backwards compatibility
#endif
PRECACHE_WEAPON_REGISTER( weapon_p250 );



CWeaponP250::CWeaponP250()
{
	m_flLastFire = gpGlobals->curtime;
}


void CWeaponP250::Spawn()
{
	SetClassname( "weapon_p250" ); // for backwards compatibility
	
	BaseClass::Spawn();
}


bool CWeaponP250::Deploy()
{
	return BaseClass::Deploy();
}

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponP250, DT_WeaponP250 )

BEGIN_NETWORK_TABLE( CWeaponP250, DT_WeaponP250 )
END_NETWORK_TABLE()

void CWeaponP250::PrimaryAttack( void )
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

	// table driven recoil
	Recoil( m_weaponMode );

	m_flRecoilIndex += 1.0f;
}


bool CWeaponP250::Reload()
{
	return DefaultPistolReload();
}

void CWeaponP250::WeaponIdle()
{
	if (m_flTimeWeaponIdle > gpGlobals->curtime)
		return;

	// only idle if the slid isn't back
	if (m_iClip1 != 0)
	{	
		SetWeaponIdleTime( gpGlobals->curtime + 3.0 ) ;
		SendWeaponAnim( ACT_VM_IDLE );
	}
}
