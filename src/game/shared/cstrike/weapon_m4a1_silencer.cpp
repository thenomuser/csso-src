//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"
#include "npcevent.h"

#if defined( CLIENT_DLL )

	#define CWeaponM4A1 C_WeaponM4A1
	#include "c_cs_player.h"

#else

	#include "cs_player.h"

#endif


class CWeaponM4A1 : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponM4A1, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();
	
	CWeaponM4A1();

	virtual void Spawn();

	virtual void SecondaryAttack();
	virtual void PrimaryAttack();
	virtual bool Deploy();
	virtual bool Reload();
	virtual void WeaponIdle();

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_M4A1; }

	// return true if this weapon has a silencer equipped
	virtual bool IsSilenced( void ) const				{ return m_bSilencerOn; }
	virtual void SetSilencer( bool silencer );

	virtual Activity GetDeployActivity( void );

#ifndef CLIENT_DLL
	virtual void Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
#endif

private:

	CWeaponM4A1( const CWeaponM4A1 & );

	CNetworkVar( bool, m_bSilencerOn );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponM4A1, DT_WeaponM4A1 )

BEGIN_NETWORK_TABLE( CWeaponM4A1, DT_WeaponM4A1 )
	#ifdef CLIENT_DLL
		RecvPropBool( RECVINFO( m_bSilencerOn ) ),
	#else
		SendPropBool( SENDINFO( m_bSilencerOn ) ),
	#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponM4A1 )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_m4a1_silencer, CWeaponM4A1 );
#ifdef GAME_DLL
LINK_ENTITY_TO_CLASS( weapon_m4a1, CWeaponM4A1 ); // for backwards compatibility
#endif
PRECACHE_WEAPON_REGISTER( weapon_m4a1_silencer );



CWeaponM4A1::CWeaponM4A1()
{
	m_bSilencerOn = true;
	m_flDoneSwitchingSilencer = 0.0f;
}


void CWeaponM4A1::Spawn()
{
	SetClassname( "weapon_m4a1_silencer" ); // for backwards compatibility
	BaseClass::Spawn();

	m_bSilencerOn = true;
	m_weaponMode = Secondary_Mode;
	m_flDoneSwitchingSilencer = 0.0f;
	m_bDelayFire = true;
}

bool CWeaponM4A1::Deploy()
{
	bool ret = BaseClass::Deploy();

	m_flDoneSwitchingSilencer = 0.0f;
	m_bDelayFire = true;

	return ret;
}

Activity CWeaponM4A1::GetDeployActivity( void )
{
	if( IsSilenced() )
	{
		return ACT_VM_DRAW_SILENCED;
	}
	else
	{
		return ACT_VM_DRAW;
	}
}

void CWeaponM4A1::SecondaryAttack()
{
	CCSPlayer* pPlayer = GetPlayerOwner();

	if ( m_bSilencerOn )
	{
		SendWeaponAnim( ACT_VM_DETACH_SILENCER );
		pPlayer->DoAnimationEvent( PLAYERANIMEVENT_SILENCER_DETACH );
	}
	else
	{
		SendWeaponAnim( ACT_VM_ATTACH_SILENCER );
		pPlayer->DoAnimationEvent( PLAYERANIMEVENT_SILENCER_ATTACH );
	}

	m_flDoneSwitchingSilencer = gpGlobals->curtime + SequenceDuration();

	m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();
	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
	SetWeaponIdleTime( gpGlobals->curtime + SequenceDuration() );

	// SetWeaponModelIndex( GetWorldModel() );
}

void CWeaponM4A1::PrimaryAttack()
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	if ( !CSBaseGunFire( GetCSWpnData().m_flCycleTime[m_weaponMode], m_weaponMode ) )
		return;
}

bool CWeaponM4A1::Reload()
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

	if ((iResult) && (pPlayer->GetFOV() != pPlayer->GetDefaultFOV()))
	{
		pPlayer->SetFOV( pPlayer, pPlayer->GetDefaultFOV() );
	}

	pPlayer->m_iShotsFired = 0;
	m_bDelayFire = false;
	return true;
}


void CWeaponM4A1::WeaponIdle()
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


#ifndef CLIENT_DLL
void CWeaponM4A1::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	int nEvent = pEvent->event;

	if ( (pEvent->type & AE_TYPE_NEWEVENTSYSTEM) && (pEvent->type & AE_TYPE_SERVER) )
	{
		switch ( nEvent )
		{
			case AE_CL_ATTACH_SILENCER_COMPLETE:
			{
				m_bSilencerOn = true;
				m_weaponMode = Secondary_Mode;
				break;
			}
			case AE_CL_DETACH_SILENCER_COMPLETE:
			{
				m_bSilencerOn = false;
				m_weaponMode = Primary_Mode;
				break;
			}
			case AE_CL_SHOW_SILENCER:
			{
				if ( CBasePlayer *pOwner = ToBasePlayer( GetPlayerOwner() ) )
				{
					if ( CBaseViewModel *vm = pOwner->GetViewModel( m_nViewModelIndex ) )
						vm->SetBodygroup( vm->FindBodygroupByName( "silencer" ), 0 );
				}

				//world model
				SetBodygroup( FindBodygroupByName( "silencer" ), 0 );
				break;
			}
			case AE_CL_HIDE_SILENCER:
			{
				if ( CBasePlayer *pOwner = ToBasePlayer( GetPlayerOwner() ) )
				{
					if ( CBaseViewModel *vm = pOwner->GetViewModel( m_nViewModelIndex ) )
						vm->SetBodygroup( vm->FindBodygroupByName( "silencer" ), 1 );
				}

				//world model
				SetBodygroup( FindBodygroupByName( "silencer" ), 1 );
				break;
			}
		}
	}

	BaseClass::Operator_HandleAnimEvent( pEvent, pOperator );
}
#endif

void CWeaponM4A1::SetSilencer( bool silencer )
{
	m_bSilencerOn = silencer;
	m_weaponMode = silencer ? Secondary_Mode : Primary_Mode;

	// we need to update bodygroups as well
	if ( CBasePlayer* pOwner = ToBasePlayer( GetPlayerOwner() ) )
	{
		if ( CBaseViewModel* vm = pOwner->GetViewModel( m_nViewModelIndex ) )
			vm->SetBodygroup( vm->FindBodygroupByName( "silencer" ), silencer ? 0 : 1 );
	}

	//world model
	SetBodygroup( FindBodygroupByName( "silencer" ), silencer ? 0 : 1 );
}