//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"


#if defined( CLIENT_DLL )

	#define CWeaponSCAR20 C_WeaponSCAR20
	#include "c_cs_player.h"

#else

	#include "cs_player.h"
	#include "KeyValues.h"

#endif


class CWeaponSCAR20 : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponSCAR20, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponSCAR20();

	virtual void Spawn();
	virtual void SecondaryAttack();
	virtual void PrimaryAttack();
	virtual bool Reload();
	virtual bool Deploy();

	virtual float GetMaxSpeed() const;

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_SCAR20; }


private:
	CWeaponSCAR20( const CWeaponSCAR20 & );

	float m_flLastFire;
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponSCAR20, DT_WeaponSCAR20 )

BEGIN_NETWORK_TABLE( CWeaponSCAR20, DT_WeaponSCAR20 )
END_NETWORK_TABLE()

#if defined CLIENT_DLL
BEGIN_PREDICTION_DATA( CWeaponSCAR20 )
	DEFINE_FIELD( m_flLastFire, FIELD_FLOAT ),
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_scar20, CWeaponSCAR20 );
#ifdef GAME_DLL
LINK_ENTITY_TO_CLASS( weapon_sg550, CWeaponSCAR20 ); // for backwards compatibility
#endif
PRECACHE_WEAPON_REGISTER( weapon_scar20 );



CWeaponSCAR20::CWeaponSCAR20()
{
	m_flLastFire = gpGlobals->curtime;
}

void CWeaponSCAR20::Spawn()
{
	SetClassname( "weapon_scar20" ); // for backwards compatibility
	BaseClass::Spawn();
}


void CWeaponSCAR20::SecondaryAttack()
{
	const float kZoomTime = 0.10f;

	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	if (pPlayer->GetFOV() == pPlayer->GetDefaultFOV())
	{
		pPlayer->SetFOV( pPlayer, 40, kZoomTime );
		m_weaponMode = Secondary_Mode;
		m_fAccuracyPenalty += GetCSWpnData().m_fInaccuracyAltSwitch;
	}
	else if (pPlayer->GetFOV() == 40)
	{
		pPlayer->SetFOV( pPlayer, 15, kZoomTime );
		m_weaponMode = Secondary_Mode;
	}
	else if (pPlayer->GetFOV() == 15)
	{
		pPlayer->SetFOV( pPlayer, pPlayer->GetDefaultFOV(), kZoomTime );
		m_weaponMode = Primary_Mode;
	}


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
		GetPlayerOwner()->EmitSound( "Default.Zoom" ); // zoom sound.
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

	m_flNextSecondaryAttack = gpGlobals->curtime + 0.3f;
	m_zoomFullyActiveTime = gpGlobals->curtime + 0.3; // The worst zoom time from above.  
}

void CWeaponSCAR20::PrimaryAttack()
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	m_flLastFire = gpGlobals->curtime;

	if ( !CSBaseGunFire( GetCSWpnData().m_flCycleTime[m_weaponMode], m_weaponMode ) )
		return;
}

bool CWeaponSCAR20::Reload()
{
	bool ret = BaseClass::Reload();
	
	m_weaponMode = Primary_Mode;
	
	return ret;
}

bool CWeaponSCAR20::Deploy()
{
	bool ret = BaseClass::Deploy();
	
	m_weaponMode = Primary_Mode;
	
	return ret;
}

float CWeaponSCAR20::GetMaxSpeed() const
{
	CCSPlayer *pPlayer = GetPlayerOwner();

	if ( !pPlayer || pPlayer->GetFOV() == 90 )
		return BaseClass::GetMaxSpeed();
	else
		return 150; // zoomed in
}
