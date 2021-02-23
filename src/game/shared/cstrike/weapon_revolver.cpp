//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h" 
#include "decals.h" 
#include "cbase.h" 
#include "shake.h" 
#include "weapon_csbasegun.h"
#include "fx_cs_shared.h"


#if defined( CLIENT_DLL )

	#define CRevolver C_Revolver
	#include "c_cs_player.h"

#else

	#include "cs_player.h"

#endif



class CRevolver : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CRevolver, CWeaponCSBase );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CRevolver();

	virtual void PrimaryAttack();
	virtual void SecondaryAttack();
	virtual bool Deploy();
	virtual bool Reload();

	virtual bool IsRevolver() const { return true; }

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_REVOLVER; }

private:
	CRevolver( const CRevolver & );
};



IMPLEMENT_NETWORKCLASS_ALIASED( Revolver, DT_WeaponRevolver )

BEGIN_NETWORK_TABLE( CRevolver, DT_WeaponRevolver )
END_NETWORK_TABLE()

#if defined CLIENT_DLL
BEGIN_PREDICTION_DATA( CRevolver )
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_revolver, CRevolver );
PRECACHE_WEAPON_REGISTER( weapon_revolver );



CRevolver::CRevolver()
{
}


void CRevolver::PrimaryAttack()
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( pPlayer == NULL )
	{
		Assert(pPlayer != NULL);
		return;
	}

	float flCycleTime = GetCSWpnData().m_flCycleTime[m_weaponMode];
	m_weaponMode = Primary_Mode;
	UpdateAccuracyPenalty();

	if ( !CSBaseGunFire( flCycleTime, m_weaponMode ) )								// <--	'PEW PEW' HAPPENS HERE
		return;

	m_flNextPrimaryAttack = gpGlobals->curtime + GetCSWpnData().m_flCycleTime[m_weaponMode];
}

void CRevolver::SecondaryAttack()
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( pPlayer == NULL )
	{
		Assert(pPlayer != NULL);
		return;
	}

	if ( m_flNextSecondaryAttack < gpGlobals->curtime )
	{
		m_weaponMode = Secondary_Mode;
		float flCycleTimeAlt = GetCSWpnData().m_flCycleTime[m_weaponMode];
		UpdateAccuracyPenalty();
		
		if ( !CSBaseGunFire( flCycleTimeAlt, m_weaponMode ) )								// <--	'PEW PEW' HAPPENS HERE
			return;

		return;
	}

	m_flNextPrimaryAttack = gpGlobals->curtime + GetCSWpnData().m_flCycleTime[m_weaponMode];
}

bool CRevolver::Deploy()
{
	m_weaponMode = Secondary_Mode;
	return BaseClass::Deploy();
}

bool CRevolver::Reload()
{
	if ( !DefaultPistolReload() )
		return false;

	return true;
}

