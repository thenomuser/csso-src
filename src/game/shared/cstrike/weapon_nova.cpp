//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbase.h"
#include "fx_cs_shared.h"


#if defined( CLIENT_DLL )

	#define CWeaponNova C_WeaponNova
	#include "c_cs_player.h"

#else

	#include "cs_player.h"
	#include "te_shotgun_shot.h"

#endif


class CWeaponNova : public CWeaponCSBase
{
public:
	DECLARE_CLASS( CWeaponNova, CWeaponCSBase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponNova();

	virtual void Spawn();

	virtual void PrimaryAttack();
	virtual bool Reload();
	virtual void WeaponIdle();


	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_NOVA; }

private:

	CWeaponNova( const CWeaponNova & );

	float m_flPumpTime;
	CNetworkVar( int, m_reloadState );

};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponNova, DT_WeaponNova )

BEGIN_NETWORK_TABLE( CWeaponNova, DT_WeaponNova )
#ifdef CLIENT_DLL
	RecvPropInt( RECVINFO( m_reloadState ) )
#else
	SendPropInt( SENDINFO( m_reloadState ), 2, SPROP_UNSIGNED )
#endif
END_NETWORK_TABLE()

#if defined(CLIENT_DLL)
BEGIN_PREDICTION_DATA( CWeaponNova )
DEFINE_PRED_FIELD( m_reloadState, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_nova, CWeaponNova );
#ifdef GAME_DLL
LINK_ENTITY_TO_CLASS( weapon_m3, CWeaponNova );
#endif
PRECACHE_WEAPON_REGISTER( weapon_nova );



CWeaponNova::CWeaponNova()
{
	m_flPumpTime = 0;
	m_reloadState = 0;
}

void CWeaponNova::Spawn()
{
	SetClassname( "weapon_nova" ); // for backwards compatibility
	BaseClass::Spawn();
}

void CWeaponNova::PrimaryAttack()
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	float flCycleTime = GetCSWpnData().m_flCycleTime[m_weaponMode];

	// don't fire underwater
	if (pPlayer->GetWaterLevel() == 3)
	{
		PlayEmptySound( );
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.15;
		return;
	}

	// Out of ammo?
	if ( m_iClip1 <= 0 )
	{
		Reload();
		if ( m_iClip1 == 0 )
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.2;
		}

		return;
	}

	SendWeaponAnim( ACT_VM_PRIMARYATTACK );

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
		GetSpread(),
		flCurAttack );

	if (!m_iClip1 && pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) <= 0)
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", false, 0);
	}

	if (m_iClip1 != 0)
		m_flPumpTime = gpGlobals->curtime + 0.5;

	if (m_iClip1 != 0)
		SetWeaponIdleTime( gpGlobals->curtime + 2.5 );
	else
		SetWeaponIdleTime( gpGlobals->curtime + 0.875 );
	m_reloadState = 0;

	// update accuracy
	m_fAccuracyPenalty += GetCSWpnData().m_fInaccuracyImpulseFire[Primary_Mode];

	// table driven recoil
	Recoil( Primary_Mode );

	m_flRecoilIndex += 1.0f;
}


bool CWeaponNova::Reload()
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return false;

	if ( GetReserveAmmoCount( AMMO_POSITION_PRIMARY ) <= 0 || m_iClip1 == GetMaxClip1() )
		return true;

	// don't reload until recoil is done
	if (m_flNextPrimaryAttack > gpGlobals->curtime)
		return true;
		
	// check to see if we're ready to reload
	if (m_reloadState == 0)
	{
		pPlayer->SetAnimation( PLAYER_RELOAD );

		SendWeaponAnim( ACT_SHOTGUN_RELOAD_START );
		m_reloadState = 1;
		pPlayer->m_flNextAttack = gpGlobals->curtime + 0.5;
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.5;
		m_flNextSecondaryAttack = gpGlobals->curtime + 0.5;
		SetWeaponIdleTime( gpGlobals->curtime + 0.5 );

#ifdef GAME_DLL
		pPlayer->DoAnimationEvent( PLAYERANIMEVENT_RELOAD_START );
#endif

		return true;
	}
	else if (m_reloadState == 1)
	{
		if (m_flTimeWeaponIdle > gpGlobals->curtime)
			return true;
		// was waiting for gun to move to side
		m_reloadState = 2;

		SendWeaponAnim( ACT_VM_RELOAD );
		SetWeaponIdleTime( gpGlobals->curtime + 0.5 );
#ifdef GAME_DLL
		if ( m_iClip1 == 7 )
		{
			pPlayer->DoAnimationEvent( PLAYERANIMEVENT_RELOAD_END );
		}
		else
		{
			pPlayer->DoAnimationEvent( PLAYERANIMEVENT_RELOAD_LOOP );
		}
#endif
	}
	else
	{
		// Add them to the clip
		m_iClip1 += 1;
		
#ifdef GAME_DLL
		SendReloadEvents();
#endif

		GiveReserveAmmo( AMMO_POSITION_PRIMARY, -1, true );
		m_reloadState = 1;
	}

	return true;
}


void CWeaponNova::WeaponIdle()
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	if (m_flPumpTime && m_flPumpTime < gpGlobals->curtime)
	{
		// play pumping sound
		m_flPumpTime = 0;
	}

	if (m_flTimeWeaponIdle < gpGlobals->curtime)
	{
		if ( m_iClip1 == 0 && m_reloadState == 0 && GetReserveAmmoCount( AMMO_POSITION_PRIMARY ) )
		{
			Reload();
		}
		else if (m_reloadState != 0)
		{
			if ( m_iClip1 != 8 && GetReserveAmmoCount( AMMO_POSITION_PRIMARY ) )
			{
				Reload();
			}
			else
			{
				// reload debounce has timed out
				//MIKETODO: shotgun anims
				SendWeaponAnim( ACT_SHOTGUN_RELOAD_FINISH );
				
				// play cocking sound
				m_reloadState = 0;
				SetWeaponIdleTime( gpGlobals->curtime + 1.5 );
			}
		}
		else
		{
			SendWeaponAnim( ACT_VM_IDLE );
		}
	}
}
