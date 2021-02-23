//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"


#if defined( CLIENT_DLL )

	#define CWeaponSSG08 C_WeaponSSG08
	#include "c_cs_player.h"

#else

	#include "cs_player.h"
	#include "KeyValues.h"

#endif

const int cSSG08MidZoomFOV = 40;
const int cSSG08MaxZoomFOV = 15;


class CWeaponSSG08 : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponSSG08, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponSSG08();

	virtual void Spawn();

	virtual void PrimaryAttack();
	virtual void SecondaryAttack();

	virtual float GetMaxSpeed() const;
	virtual bool Reload();
	virtual bool Deploy();

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_SSG08; }


private:
	
	CWeaponSSG08( const CWeaponSSG08 & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponSSG08, DT_WeaponSSG08 )

BEGIN_NETWORK_TABLE( CWeaponSSG08, DT_WeaponSSG08 )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponSSG08 )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_ssg08, CWeaponSSG08 );
#ifdef GAME_DLL
LINK_ENTITY_TO_CLASS( weapon_scout, CWeaponSSG08 );
#endif
PRECACHE_WEAPON_REGISTER( weapon_ssg08 );



CWeaponSSG08::CWeaponSSG08()
{
}

void CWeaponSSG08::Spawn()
{
	SetClassname( "weapon_ssg08" ); // for backwards compatibility
	BaseClass::Spawn();
}

void CWeaponSSG08::SecondaryAttack()
{
	const float kZoomTime = 0.10f;

	CCSPlayer *pPlayer = GetPlayerOwner();
	if (pPlayer == NULL)
	{
		Assert(pPlayer != NULL);
		return;
	}

	if (pPlayer->GetFOV() == pPlayer->GetDefaultFOV())
	{
		pPlayer->SetFOV( pPlayer, cSSG08MidZoomFOV, kZoomTime );
		m_weaponMode = Secondary_Mode;
		m_fAccuracyPenalty += GetCSWpnData().m_fInaccuracyAltSwitch;
	}
	else if (pPlayer->GetFOV() == cSSG08MidZoomFOV)
	{
		pPlayer->SetFOV( pPlayer, cSSG08MaxZoomFOV, kZoomTime );
		m_weaponMode = Secondary_Mode;
	}
	else if (pPlayer->GetFOV() == cSSG08MaxZoomFOV)
	{
		pPlayer->SetFOV( pPlayer, pPlayer->GetDefaultFOV(), kZoomTime );
		m_weaponMode = Primary_Mode;
	}

	m_flNextSecondaryAttack = gpGlobals->curtime + 0.3f;   
	m_zoomFullyActiveTime = gpGlobals->curtime + 0.15; // The worst zoom time from above.  

#ifndef CLIENT_DLL
	// If this isn't guarded, the sound will be emitted twice, once by the server and once by the client.
	// Let the server play it since if only the client plays it, it's liable to get played twice cause of
	// a prediction error. joy.	
	
	//=============================================================================
	// HPE_BEGIN:
	// [tj] Playing this from the player so that we don't try to play the sound outside the level.
	//=============================================================================
	if ( GetPlayerOwner() )
	{
		GetPlayerOwner()->EmitSound( "Default.Zoom" ); // zoom sound
	}
	//=============================================================================
	// HPE_END
	//=============================================================================
	// let the bots hear the rifle zoom
	IGameEvent * event = gameeventmanager->CreateEvent( "weapon_zoom" );
	if( event )
	{
		event->SetInt( "userid", pPlayer->GetUserID() );
		gameeventmanager->FireEvent( event );
	}
#endif
}

void CWeaponSSG08::PrimaryAttack( void )
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if (pPlayer == NULL)
		return;

	if ( !CSBaseGunFire( GetCSWpnData().m_flCycleTime[m_weaponMode], m_weaponMode ) )
		return;

	if ( m_weaponMode == Secondary_Mode )
	{	
		float	midFOVdistance = fabs( pPlayer->GetFOV() - (float)cSSG08MidZoomFOV );
		float	farFOVdistance = fabs( pPlayer->GetFOV() - (float)cSSG08MaxZoomFOV );

		if ( midFOVdistance < farFOVdistance )
		{
			pPlayer->m_iLastZoom = cSSG08MidZoomFOV;
		}
		else
		{
			pPlayer->m_iLastZoom = cSSG08MaxZoomFOV;
		}
		
// 		#ifndef CLIENT_DLL
			pPlayer->m_bResumeZoom = true;
			pPlayer->SetFOV( pPlayer, pPlayer->GetDefaultFOV(), 0.05f );
			m_weaponMode = Primary_Mode;
// 		#endif
	}
}


float CWeaponSSG08::GetMaxSpeed() const
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if (pPlayer == NULL)
	{
		Assert(pPlayer != NULL);
		return BaseClass::GetMaxSpeed();
	}

	if ( pPlayer->GetFOV() == pPlayer->GetDefaultFOV() )
		return BaseClass::GetMaxSpeed();
	else
		return 220;	// zoomed in.
}


bool CWeaponSSG08::Reload()
{
	m_weaponMode = Primary_Mode;
	return BaseClass::Reload();

}

bool CWeaponSSG08::Deploy()
{
	m_weaponMode = Primary_Mode;
	return BaseClass::Deploy();
}
