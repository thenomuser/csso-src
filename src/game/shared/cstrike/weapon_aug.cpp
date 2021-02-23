//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"


#if defined( CLIENT_DLL )

	#define CWeaponAug C_WeaponAug
	#include "c_cs_player.h"

#else

	#include "cs_player.h"

#endif


class CWeaponAug : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponAug, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponAug();

	//virtual void SecondaryAttack();
	virtual void PrimaryAttack();

	virtual bool Reload();
	virtual bool Deploy();

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_AUG; }

#ifdef CLIENT_DLL
	virtual bool	HideViewModelWhenZoomed( void ) { return false; }
#endif

private:

	void AUGFire( float flSpread, bool bZoomed );
	
	CWeaponAug( const CWeaponAug & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponAug, DT_WeaponAug )

BEGIN_NETWORK_TABLE( CWeaponAug, DT_WeaponAug )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponAug )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_aug, CWeaponAug );
PRECACHE_WEAPON_REGISTER( weapon_aug );



CWeaponAug::CWeaponAug()
{
}
/*
void CWeaponAug::SecondaryAttack()
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

#if IRONSIGHT
	if ( pPlayer->GetFOV() == pPlayer->GetDefaultFOV() )
	{
		CIronSightController* pIronSightController = pPlayer->GetActiveCSWeapon()->GetIronSightController();
		if ( pIronSightController )
		{
			pPlayer->GetActiveCSWeapon()->UpdateIronSightController();
			pPlayer->SetFOV( pPlayer, pIronSightController->GetIronSightIdealFOV(), pIronSightController->GetIronSightPullUpDuration() );
			pIronSightController->SetState( IronSight_should_approach_sighted );

			//stop looking at weapon when going into ironsights
#ifndef CLIENT_DLL
			pPlayer->StopLookingAtWeapon();

			//force idle animation
			CBaseViewModel* pViewModel = pPlayer->GetViewModel();
			if ( pViewModel )
			{
				int nSequence = pViewModel->LookupSequence( "idle" );
				if ( nSequence != ACTIVITY_NOT_AVAILABLE )
				{
					pViewModel->ForceCycle( 0 );
					pViewModel->ResetSequence( nSequence );
				}
			}
#endif
			m_weaponMode = Secondary_Mode;
			if ( GetPlayerOwner() )
			{
				GetPlayerOwner()->EmitSound( "Weapon_AUG.ZoomIn" );
			}
		}
	}
	else
	{
		CIronSightController* pIronSightController = pPlayer->GetActiveCSWeapon()->GetIronSightController();
		if ( pIronSightController )
		{
			pPlayer->GetActiveCSWeapon()->UpdateIronSightController();
			int iFOV = pPlayer->GetDefaultFOV();
			pPlayer->SetFOV( pPlayer, iFOV, pIronSightController->GetIronSightPutDownDuration() );
			pIronSightController->SetState( IronSight_should_approach_unsighted );
			SendWeaponAnim( ACT_VM_FIDGET );
			m_weaponMode = Primary_Mode;
			if ( GetPlayerOwner() )
			{
				GetPlayerOwner()->EmitSound( "Weapon_AUG.ZoomOut" );
			}
		}
	}
#else

	if ( pPlayer->GetFOV() == pPlayer->GetDefaultFOV() )
	{
		pPlayer->SetFOV( pPlayer, 55, 0.2f );
		m_weaponMode = Secondary_Mode;
	}
	else if ( pPlayer->GetFOV() == 55 )
	{
		pPlayer->SetFOV( pPlayer, pPlayer->GetDefaultFOV(), 0.15f );
		m_weaponMode = Primary_Mode;
	}
	else 
	{
		pPlayer->SetFOV( pPlayer, pPlayer->GetDefaultFOV() );
		m_weaponMode = Primary_Mode;
	}
#endif

	m_flNextSecondaryAttack = gpGlobals->curtime + 0.3;
}
*/
void CWeaponAug::PrimaryAttack()
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	// bool bZoomed = pPlayer->GetFOV() < pPlayer->GetDefaultFOV();

	float flCycleTime = GetCSWpnData().m_flCycleTime[m_weaponMode];

	/*if ( bZoomed )
		flCycleTime = 0.135f;*/

	if ( !CSBaseGunFire( flCycleTime, m_weaponMode ) )
		return;
}


bool CWeaponAug::Reload()
{
	m_weaponMode = Primary_Mode;
	return BaseClass::Reload();
}

bool CWeaponAug::Deploy()
{
	m_weaponMode = Primary_Mode;
	return BaseClass::Deploy();
}
