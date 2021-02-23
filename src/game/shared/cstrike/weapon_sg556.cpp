//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"


#if defined( CLIENT_DLL )

	#define CWeaponSG556 C_WeaponSG556
	#include "c_cs_player.h"

#else

	#include "cs_player.h"

#endif


class CWeaponSG556 : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponSG556, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponSG556();

	virtual void Spawn();

	//virtual void SecondaryAttack();
	virtual void PrimaryAttack();

	virtual float GetMaxSpeed() const;
	virtual bool Reload();
	virtual bool Deploy();

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_SG556; }

#ifdef CLIENT_DLL
	virtual bool	HideViewModelWhenZoomed( void ) { return false; }
#endif

private:

	CWeaponSG556( const CWeaponSG556 & );

};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponSG556, DT_WeaponSG556 )

BEGIN_NETWORK_TABLE( CWeaponSG556, DT_WeaponSG556 )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponSG556 )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_sg556, CWeaponSG556 );
#ifdef GAME_DLL
LINK_ENTITY_TO_CLASS( weapon_sg552, CWeaponSG556 ); // for backwards compatibility
#endif
PRECACHE_WEAPON_REGISTER( weapon_sg556 );



CWeaponSG556::CWeaponSG556()
{
}

void CWeaponSG556::Spawn()
{
	SetClassname( "weapon_sg556" ); // for backwards compatibility
	BaseClass::Spawn();
}
/*
void CWeaponSG556::SecondaryAttack()
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
				GetPlayerOwner()->EmitSound( "Weapon_sg556.ZoomIn" );
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
				GetPlayerOwner()->EmitSound( "Weapon_sg556.ZoomOut" );
			}
		}
	}
#else

	if (pPlayer->GetFOV() == pPlayer->GetDefaultFOV())
	{
		pPlayer->SetFOV( pPlayer, 55, 0.2f );
		m_weaponMode = Secondary_Mode;
	}
	else if (pPlayer->GetFOV() == 55)
	{
		pPlayer->SetFOV( pPlayer, 0, 0.15f );
		m_weaponMode = Secondary_Mode;
	}
	else 
	{
		//FIXME: This seems wrong
		pPlayer->SetFOV( pPlayer, pPlayer->GetDefaultFOV() );
		m_weaponMode = Primary_Mode;
	}
#endif

	m_flNextSecondaryAttack = gpGlobals->curtime + 0.3;
}
*/
void CWeaponSG556::PrimaryAttack()
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


float CWeaponSG556::GetMaxSpeed() const
{
	CCSPlayer *pPlayer = GetPlayerOwner();

	if ( !pPlayer || pPlayer->GetFOV() == pPlayer->GetDefaultFOV() )
		return BaseClass::GetMaxSpeed();
	else
		return 200; // zoomed in.
}	


bool CWeaponSG556::Reload()
{
	m_weaponMode = Primary_Mode;
	return BaseClass::Reload();
}

bool CWeaponSG556::Deploy()
{
	m_weaponMode = Primary_Mode;
	return BaseClass::Deploy();
}
