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

	#define CWeaponCZ75 C_WeaponCZ75
	#include "c_cs_player.h"

#else

	#include "cs_player.h"

#endif


class CWeaponCZ75 : public CWeaponCSBase
{
public:
	DECLARE_CLASS( CWeaponCZ75, CWeaponCSBase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponCZ75();

	virtual void PrimaryAttack();

	virtual bool Reload();
	virtual Activity GetDeployActivity();

#ifndef CLIENT_DLL
	virtual void Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
#endif

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_CZ75; }

private:
	
	CWeaponCZ75( const CWeaponCZ75 & );

	Activity m_iReloadActivityIndex;
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponCZ75, DT_WeaponCZ75 )

BEGIN_NETWORK_TABLE( CWeaponCZ75, DT_WeaponCZ75 )
END_NETWORK_TABLE()

#if defined CLIENT_DLL
BEGIN_PREDICTION_DATA( CWeaponCZ75 )
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_cz75, CWeaponCZ75 );
PRECACHE_WEAPON_REGISTER( weapon_cz75 );



CWeaponCZ75::CWeaponCZ75()
{
}

void CWeaponCZ75::PrimaryAttack()
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

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

Activity CWeaponCZ75::GetDeployActivity()
{
	if ( GetReserveAmmoCount( AMMO_POSITION_PRIMARY ) <= 0 )
	{
		// just to be sure it will work
		m_iReloadActivityIndex = ACT_SECONDARY_VM_RELOAD;
		return ACT_VM_DRAW_EMPTY;
	}
	else
	{
		m_iReloadActivityIndex = ACT_VM_RELOAD;
		return ACT_VM_DRAW;
	}
}

bool CWeaponCZ75::Reload()
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return false;

	if ( GetReserveAmmoCount( AMMO_POSITION_PRIMARY ) <= 0 )
		return false;

	if ( !DefaultReload( GetCSWpnData().iDefaultClip1, 0, m_iReloadActivityIndex ) )
		return false;

	pPlayer->m_iShotsFired = 0;

	return true;
}

#ifndef CLIENT_DLL
void CWeaponCZ75::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	int nEvent = pEvent->event;

	if ( (pEvent->type & AE_TYPE_NEWEVENTSYSTEM) && (pEvent->type & AE_TYPE_SERVER) )
	{
		switch ( nEvent )
		{
			case AE_WPN_CZ_DUMP_CURRENT_MAG:
			{
				CCSPlayer *pCSPlayer = GetPlayerOwner();

				//m_iClip1 = 0;
				if ( CBaseViewModel *vm = pCSPlayer->GetViewModel( m_nViewModelIndex ) )
				{
					vm->SetBodygroup( vm->FindBodygroupByName( "front_mag" ), 1 );
					//world model
					/*CBaseWeaponWorldModel *pWorldModel = GetWeaponWorldModel();
					if ( pWorldModel )
					{
						pWorldModel->SetBodygroup( pWorldModel->FindBodygroupByName( "front_mag" ), 1 );
					}
					else*/
					{
						SetBodygroup( FindBodygroupByName( "front_mag" ), 1 );
					}

					//if the front mag is removed, all subsequent anims use the non-front mag reload
					m_iReloadActivityIndex = ACT_SECONDARY_VM_RELOAD;
				}
				break;
			}
			case AE_WPN_CZ_UPDATE_BODYGROUP:
			{
				CCSPlayer *pCSPlayer = GetPlayerOwner();

				int iGroupNum = ( GetReserveAmmoCount( AMMO_POSITION_PRIMARY ) <= 0 ) ? 1 : 0;

				if ( CBaseViewModel *vm = pCSPlayer->GetViewModel( m_nViewModelIndex ) )
				{
					vm->SetBodygroup( vm->FindBodygroupByName( "front_mag" ), iGroupNum );
					//world model
					/*CBaseWeaponWorldModel *pWorldModel = GetWeaponWorldModel();
					if ( pWorldModel )
					{
						pWorldModel->SetBodygroup( pWorldModel->FindBodygroupByName( "front_mag" ), iGroupNum );
					}
					else*/
					{
						SetBodygroup( FindBodygroupByName( "front_mag" ), iGroupNum );
					}

					//if the front mag is removed, all subsequent anims use the non-front mag reload
					m_iReloadActivityIndex = (iGroupNum == 0) ? ACT_VM_RELOAD : ACT_SECONDARY_VM_RELOAD;
				}
				break;
			}
		}
	}

	BaseClass::Operator_HandleAnimEvent( pEvent, pOperator );
}
#endif