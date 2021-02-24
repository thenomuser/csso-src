//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h" 
#include "decals.h" 
#include "cbase.h" 
#include "shake.h" 
#include "weapon_csbase.h"
#include "fx_cs_shared.h"


#if defined( CLIENT_DLL )

	#define CDEagle C_DEagle
	#include "c_cs_player.h"

#else

	#include "cs_player.h"

#endif



#define DEAGLE_WEIGHT   7
#define DEAGLE_MAX_CLIP 7

enum deagle_e {
	DEAGLE_IDLE1 = 0,
	DEAGLE_SHOOT1,
	DEAGLE_SHOOT2,
	DEAGLE_SHOOT_EMPTY,
	DEAGLE_RELOAD,	
	DEAGLE_DRAW,
};



class CDEagle : public CWeaponCSBase
{
public:
	DECLARE_CLASS( CDEagle, CWeaponCSBase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CDEagle();

	void Spawn();

	void PrimaryAttack();
	virtual bool Deploy();
	bool Reload();
	void WeaponIdle();
	void MakeBeam ();
	void BeamUpdate ();
	virtual bool UseDecrement() {return true;};

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_DEAGLE; }

public:
	float m_flLastFire;

private:
	CDEagle( const CDEagle & );
};



IMPLEMENT_NETWORKCLASS_ALIASED( DEagle, DT_WeaponDEagle )

BEGIN_NETWORK_TABLE( CDEagle, DT_WeaponDEagle )
END_NETWORK_TABLE()

#if defined CLIENT_DLL
BEGIN_PREDICTION_DATA( CDEagle )
	DEFINE_FIELD( m_flLastFire, FIELD_FLOAT ),
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_deagle, CDEagle );
PRECACHE_WEAPON_REGISTER( weapon_deagle );



CDEagle::CDEagle()
{
	m_flLastFire = gpGlobals->curtime;
}


void CDEagle::Spawn()
{
	BaseClass::Spawn();
}


bool CDEagle::Deploy()
{
	return BaseClass::Deploy();
}

void CDEagle::PrimaryAttack()
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

	if( m_iClip1 > 0 )
		SendWeaponAnim( ACT_VM_PRIMARYATTACK );
	else
		SendWeaponAnim( ACT_VM_DRYFIRE );

	//SetPlayerShieldAnim();
	
	// player "shoot" animation
	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	//pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
	//pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	FX_FireBullets(
		pPlayer->entindex(),
		pPlayer->Weapon_ShootPosition(),
		pPlayer->GetFinalAimAngle(),
		GetWeaponID(),
		Primary_Mode,
		CBaseEntity::GetPredictionRandomSeed() & 255,
		GetInaccuracy(),
		GetSpread());

	m_flNextPrimaryAttack = gpGlobals->curtime + GetCSWpnData().m_flCycleTime[m_weaponMode];

	if ( !m_iClip1 && pPlayer->GetAmmoCount( GetPrimaryAmmoType() ) <= 0 )
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", false, 0);
	}

	SetWeaponIdleTime( gpGlobals->curtime + 1.8 );

	// update accuracy
	m_fAccuracyPenalty += GetCSWpnData().m_fInaccuracyImpulseFire[Primary_Mode];

	// table driven recoil
	Recoil( m_weaponMode );

	m_flRecoilIndex += 1.0f;

	//ResetPlayerShieldAnim();
}


bool CDEagle::Reload()
{
	return DefaultPistolReload();
}

void CDEagle::WeaponIdle()
{
	if ( m_flTimeWeaponIdle > gpGlobals->curtime )
		return;

	SetWeaponIdleTime( gpGlobals->curtime + 20 );

	if (m_iClip1 != 0)
	{
		SendWeaponAnim( ACT_VM_IDLE );
	}

	//if ( FBitSet(m_iWeaponState, WPNSTATE_SHIELD_DRAWN) )
	//	 SendWeaponAnim( SHIELDGUN_DRAWN_IDLE, UseDecrement() ? 1:0 );
}

