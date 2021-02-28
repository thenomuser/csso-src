//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbase.h"
#include "fx_cs_shared.h"


#if defined( CLIENT_DLL )

	#define CWeaponGlock C_WeaponGlock
	#include "c_cs_player.h"

#else

	#include "cs_player.h"

#endif


class CWeaponGlock : public CWeaponCSBase
{
public:
	DECLARE_CLASS( CWeaponGlock, CWeaponCSBase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponGlock();

	virtual void Spawn();

	virtual void PrimaryAttack();
	virtual void SecondaryAttack();
	virtual bool Deploy();

	virtual void ItemPostFrame();

	void GlockFire( float fSpread, bool bFireBurst );
	void FireRemaining( float fSpread );
	
	virtual bool Reload();

	virtual void WeaponIdle();

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_GLOCK; }

private:
	
	CWeaponGlock( const CWeaponGlock & );

	CNetworkVar( bool, m_bBurstMode );
	CNetworkVar( int, m_iBurstShotsRemaining );	// used to keep track of the shots fired during the Glock18 burst fire mode.
	float	m_fNextBurstShot;					// time to shoot the next bullet in burst fire mode
	float	m_flLastFire;
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponGlock, DT_WeaponGlock )

BEGIN_NETWORK_TABLE( CWeaponGlock, DT_WeaponGlock )
	#ifdef CLIENT_DLL
		RecvPropBool( RECVINFO( m_bBurstMode ) ),
		RecvPropInt( RECVINFO( m_iBurstShotsRemaining ) ),
	#else
		SendPropBool( SENDINFO( m_bBurstMode ) ),
		SendPropInt( SENDINFO( m_iBurstShotsRemaining ) ),
	#endif
END_NETWORK_TABLE()

#if defined(CLIENT_DLL)
BEGIN_PREDICTION_DATA( CWeaponGlock )
	DEFINE_FIELD( m_flLastFire, FIELD_FLOAT ),
	DEFINE_PRED_FIELD( m_iBurstShotsRemaining, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
 	DEFINE_PRED_FIELD( m_fNextBurstShot, FIELD_FLOAT, 0 ),
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_glock, CWeaponGlock );
PRECACHE_WEAPON_REGISTER( weapon_glock );

const float kGlockBurstCycleTime = 0.06f;

CWeaponGlock::CWeaponGlock()
{
	m_bBurstMode = false;
	m_flLastFire = gpGlobals->curtime;
	m_iBurstShotsRemaining = 0;
	m_fNextBurstShot = 0.0f;
}


void CWeaponGlock::Spawn( )
{
	BaseClass::Spawn();

	m_bBurstMode = false;
	m_iBurstShotsRemaining = 0;
	m_fNextBurstShot = 0.0f;
}

bool CWeaponGlock::Deploy( )
{
	m_iBurstShotsRemaining = 0;
	m_fNextBurstShot = 0.0f;

	return BaseClass::Deploy();
}

void CWeaponGlock::SecondaryAttack()
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	if ( m_bBurstMode )
	{
		ClientPrint( pPlayer, HUD_PRINTCENTER, "#Switch_To_SemiAuto" );
		m_bBurstMode = false;
		m_weaponMode = Primary_Mode;
	}
	else
	{
		ClientPrint( pPlayer, HUD_PRINTCENTER, "#Switch_To_BurstFire" );
		m_bBurstMode = true;
		m_weaponMode = Secondary_Mode;
	}

#ifndef CLIENT_DLL
	pPlayer->EmitSound( "Weapon.AutoSemiAutoSwitch" );
#endif
	
	m_flNextSecondaryAttack = gpGlobals->curtime + 0.3;
}

void CWeaponGlock::PrimaryAttack()
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	float flCycleTime = m_bBurstMode ? 0.5f : GetCSWpnData().m_flCycleTime[Primary_Mode];

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

	// player "shoot" animation
	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	// non-silenced
	//pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	//pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

	FX_FireBullets( 
		pPlayer->entindex(),
		pPlayer->Weapon_ShootPosition(), 
		pPlayer->GetFinalAimAngle(), 
		GetWeaponID(),
		Primary_Mode,
		CBaseEntity::GetPredictionRandomSeed() & 255, // wrap it for network traffic so it's the same between client and server
		GetInaccuracy(),
		GetSpread(),
		gpGlobals->curtime);

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + flCycleTime;

	if (!m_iClip1 && pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) <= 0)
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", false, 0);
	}

	SetWeaponIdleTime( gpGlobals->curtime + 2.5f );

	if ( m_bBurstMode )
	{
		// Fire off the next two rounds
		m_fNextBurstShot = gpGlobals->curtime + kGlockBurstCycleTime;
		m_iBurstShotsRemaining = 2;
	}
	SendWeaponAnim( ACT_VM_PRIMARYATTACK );

	// update accuracy
	m_fAccuracyPenalty += GetCSWpnData().m_fInaccuracyImpulseFire[m_weaponMode];

	//ResetPlayerShieldAnim();

	// table driven recoil
	Recoil( m_weaponMode );

	m_flRecoilIndex += 1.0f;
}


// GOOSEMAN : FireRemaining used by Glock18

void CWeaponGlock::FireRemaining( float fSpread )
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		Error( "!pPlayer" );

	if ( m_iBurstShotsRemaining == 0 )
		return;

	if (m_iClip1 <= 0)
	{
		m_iClip1 = 0;
		m_iBurstShotsRemaining = 0;
		m_fNextBurstShot = 0.0f;
		return;
	}
	--m_iClip1;

	// TODO FIXME damage = 18, rangemode 0.9

	float fInaccuracy = GetInaccuracy();
	if ( weapon_accuracy_model.GetInt() == 1 )
		fInaccuracy = 0.05;
	
	FX_FireBullets( 
		pPlayer->entindex(),
		pPlayer->Weapon_ShootPosition(), 
		pPlayer->GetFinalAimAngle(), 
		GetWeaponID(),
		Secondary_Mode,
		CBaseEntity::GetPredictionRandomSeed() & 255, // wrap it for network traffic so it's the same between client and server
		fInaccuracy,
		GetSpread(),
		m_fNextBurstShot );

	SendWeaponAnim( ACT_VM_PRIMARYATTACK );
	
	pPlayer->SetAnimation( PLAYER_ATTACK1 );
	pPlayer->m_iShotsFired++;

	--m_iBurstShotsRemaining;

	if ( m_iBurstShotsRemaining > 0 )
		m_fNextBurstShot += kGlockBurstCycleTime;
	else
		m_fNextBurstShot = 0.0;

	// update accuracy
	m_fAccuracyPenalty += GetCSWpnData().m_fInaccuracyImpulseFire[Secondary_Mode];

	// table driven recoil
	Recoil( Secondary_Mode );

	m_flRecoilIndex += 1.0f;
}


void CWeaponGlock::ItemPostFrame()
{
	while ( m_iBurstShotsRemaining > 0 && gpGlobals->curtime >= m_fNextBurstShot )
	{
		if ( weapon_accuracy_model.GetInt() == 1 )
			FireRemaining(0.05f);
		else
			FireRemaining(GetSpread());
	}

	BaseClass::ItemPostFrame();
}


bool CWeaponGlock::Reload()
{
	if ( m_iBurstShotsRemaining != 0 )
		return true;

	return DefaultPistolReload();
}

void CWeaponGlock::WeaponIdle()
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	if (m_flTimeWeaponIdle > gpGlobals->curtime)
		return;

	if ( pPlayer->HasShield() )
	{
		SetWeaponIdleTime( gpGlobals->curtime + 20 );
				
		//MIKETODO: shields
		//if ( FBitSet(m_iWeaponState, WPNSTATE_SHIELD_DRAWN) )
		//	 SendWeaponAnim( GLOCK18_SHIELD_IDLE, UseDecrement() ? 1:0 );
	}
	else
	{
		// only idle if the slid isn't back
		if (m_iClip1 != 0)
		{
			SendWeaponAnim( ACT_VM_IDLE );
		}
	}
}
