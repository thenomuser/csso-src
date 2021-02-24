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

	#define CWeaponUSP C_WeaponUSP
	#include "c_cs_player.h"

#else

	#include "cs_player.h"

#endif


class CWeaponUSP : public CWeaponCSBase
{
public:
	DECLARE_CLASS( CWeaponUSP, CWeaponCSBase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponUSP();

	virtual void Spawn();

	virtual void PrimaryAttack();
	virtual void SecondaryAttack();
	virtual bool Deploy();

	virtual bool Reload();

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_USP; }

	// return true if this weapon has a silencer equipped
	virtual bool IsSilenced( void ) const				{ return m_bSilencerOn; }
	virtual void SetSilencer( bool silencer );

	virtual Activity GetDeployActivity( void );

#ifndef CLIENT_DLL
	virtual void Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
#endif

private:
	CWeaponUSP( const CWeaponUSP & );

	CNetworkVar( bool, m_bSilencerOn );
	float m_flLastFire;
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponUSP, DT_WeaponUSP )

BEGIN_NETWORK_TABLE( CWeaponUSP, DT_WeaponUSP )
#ifdef CLIENT_DLL
	RecvPropBool( RECVINFO( m_bSilencerOn ) ),
#else
	SendPropBool( SENDINFO( m_bSilencerOn ) ),
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CWeaponUSP )
	DEFINE_PRED_FIELD( m_bSilencerOn, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_FIELD( m_flLastFire, FIELD_FLOAT ),
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_usp_silencer, CWeaponUSP );
#ifdef GAME_DLL
LINK_ENTITY_TO_CLASS( weapon_usp, CWeaponUSP ); // for backwards compatibility
#endif
PRECACHE_WEAPON_REGISTER( weapon_usp_silencer );



CWeaponUSP::CWeaponUSP()
{
	m_flLastFire = gpGlobals->curtime;
	m_bSilencerOn = true;
	m_flDoneSwitchingSilencer = 0.0f;
}


void CWeaponUSP::Spawn()
{
	SetClassname( "weapon_usp_silencer" ); // for backwards compatibility
	BaseClass::Spawn();

	//m_iDefaultAmmo = 12;
	m_bSilencerOn = true;
	m_weaponMode = Secondary_Mode;
	m_flDoneSwitchingSilencer = 0.0f;

	//FallInit();// get ready to fall down.
}


bool CWeaponUSP::Deploy()
{
	m_flDoneSwitchingSilencer = 0.0f;

	return BaseClass::Deploy();
}

Activity CWeaponUSP::GetDeployActivity( void )
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

void CWeaponUSP::SecondaryAttack()
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

void CWeaponUSP::PrimaryAttack()
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	float flCycleTime =  GetCSWpnData().m_flCycleTime[m_weaponMode];

	m_flLastFire = gpGlobals->curtime;

	if (m_iClip1 <= 0)
	{
		if ( m_bFireOnEmpty )
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.2;
			m_bFireOnEmpty = false;
		}

		return;
	}

	pPlayer->m_iShotsFired++;

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + flCycleTime;

	m_iClip1--;

	SendWeaponAnim( ACT_VM_PRIMARYATTACK );

	// player "shoot" animation
	pPlayer->SetAnimation( PLAYER_ATTACK1 );
	
	FX_FireBullets(
		pPlayer->entindex(),
		pPlayer->Weapon_ShootPosition(),
		pPlayer->GetFinalAimAngle(),
		GetWeaponID(),
		m_weaponMode,
		CBaseEntity::GetPredictionRandomSeed() & 255,
		GetInaccuracy(),
		GetSpread());

	if (!m_iClip1 && pPlayer->GetAmmoCount( GetPrimaryAmmoType() ) <= 0)
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", false, 0);
	}
 
	SetWeaponIdleTime( gpGlobals->curtime + 2 );

	// update accuracy
	m_fAccuracyPenalty += GetCSWpnData().m_fInaccuracyImpulseFire[m_weaponMode];

	// table driven recoil
	Recoil( m_weaponMode );

	m_flRecoilIndex += 1.0f;
}


bool CWeaponUSP::Reload()
{
	return DefaultPistolReload();
}


#ifndef CLIENT_DLL
void CWeaponUSP::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
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

void CWeaponUSP::SetSilencer( bool silencer )
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