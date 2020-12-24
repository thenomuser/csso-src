//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbase.h"
#include "gamerules.h"
#include "cs_gamerules.h"
#include "npcevent.h"
#include "engine/IEngineSound.h"
#include "weapon_basecsgrenade.h"
#include "in_buttons.h"	
#include "datacache/imdlcache.h"

#include "cs_shareddefs.h"

#ifdef CLIENT_DLL

	#include "c_cs_player.h"

#else

	#include "cs_player.h"
	#include "items.h"
	#include "cs_gamestats.h"

#endif


#define GRENADE_TIMER	1.5f //Seconds
#define GRENADE_SECONDARY_DAMPENING 0.3f
#define GRENADE_SECONDARY_LOWER 12.0f
#define GRENADE_SECONDARY_TRANSITION 1.3f
#define GRENADE_SECONDARY_INTERP 2.0f


IMPLEMENT_NETWORKCLASS_ALIASED( BaseCSGrenade, DT_BaseCSGrenade )

BEGIN_NETWORK_TABLE(CBaseCSGrenade, DT_BaseCSGrenade)

#ifndef CLIENT_DLL
	SendPropBool( SENDINFO(m_bRedraw) ),
	SendPropBool( SENDINFO(m_bIsHeldByPlayer) ),
	SendPropBool( SENDINFO(m_bPinPulled) ),
	SendPropFloat( SENDINFO(m_fThrowTime), 0, SPROP_NOSCALE ),
	SendPropBool( SENDINFO( m_bLoopingSoundPlaying ) ),
	SendPropFloat( SENDINFO(m_flThrowStrength), 0, SPROP_NOSCALE ),
#else
	RecvPropBool( RECVINFO(m_bRedraw) ),
	RecvPropBool( RECVINFO(m_bIsHeldByPlayer) ),
	RecvPropBool( RECVINFO(m_bPinPulled) ),
	RecvPropFloat( RECVINFO(m_fThrowTime) ),
	RecvPropBool( RECVINFO( m_bLoopingSoundPlaying ) ),
	RecvPropFloat( RECVINFO(m_flThrowStrength) ),
#endif

END_NETWORK_TABLE()

#if defined CLIENT_DLL
BEGIN_PREDICTION_DATA( CBaseCSGrenade )
	DEFINE_PRED_FIELD( m_bRedraw, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bRedraw, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_flThrowStrength, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_basecsgrenade, CBaseCSGrenade );

#ifndef CLIENT_DLL
ConVar sv_ignoregrenaderadio( "sv_ignoregrenaderadio", "0", 0, "Turn off Fire in the hole messages" );
#endif

CBaseCSGrenade::CBaseCSGrenade()
{
	m_bRedraw = false;
	m_bIsHeldByPlayer = false;
	m_bPinPulled = false;
	m_fThrowTime = 0;
	m_bLoopingSoundPlaying = false;
	m_flThrowStrength = 1.0f;
	m_flThrowStrengthClientSmooth = 1.0f;

#ifndef CLIENT_DLL
	m_bHasEmittedProjectile = false;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseCSGrenade::Precache()
{
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBaseCSGrenade::Deploy()
{
	m_bRedraw = false;
	m_bIsHeldByPlayer = true;
	m_bPinPulled = false;
	m_flThrowStrength = 1.0f;
	m_flThrowStrengthClientSmooth = 1.0f;
	m_fThrowTime = 0;

#ifndef CLIENT_DLL
	// if we're officially out of grenades, ditch this weapon
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return false;

	if( pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0 )
	{
		pPlayer->Weapon_Drop( this, NULL, NULL );
		UTIL_Remove(this);
		return false;
	}
#endif

	return BaseClass::Deploy();
}

#ifdef CLIENT_DLL
int CBaseCSGrenade::DrawModel( int flags )
{
	//hide the grenade that's in the player's hand while playing grenade throwing animations
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( pPlayer )
	{
		if ( !pPlayer->m_bUseNewAnimstate && pPlayer->m_PlayerAnimState && pPlayer->m_PlayerAnimState->ShouldHideGrenadeDuringThrow() )
		{
			return 0;
		}
	}

	return BaseClass::DrawModel( flags );
}
#endif

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBaseCSGrenade::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	m_bRedraw = false;
	m_bPinPulled = false; // when this is holstered make sure the pin isn’t pulled.
	m_flThrowStrength = 1.0f;
	m_flThrowStrengthClientSmooth = 1.0f;
	m_fThrowTime = 0;

#ifndef CLIENT_DLL
	// If they attempt to switch weapons before the throw animation is done, 
	// allow it, but kill the weapon if we have to.
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return false;

	if( pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0 )
	{
		CBaseCombatCharacter *pOwner = (CBaseCombatCharacter *)pPlayer;
		pOwner->Weapon_Drop( this );
		UTIL_Remove(this);
	}
#endif

	return BaseClass::Holster( pSwitchingTo );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseCSGrenade::PrimaryAttack()
{
	if ( !m_bIsHeldByPlayer || m_bPinPulled || m_fThrowTime > 0.0f )
		return;

	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer || pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
		return;

#ifndef CLIENT_DLL
	pPlayer->DoAnimationEvent( PLAYERANIMEVENT_GRENADE_PULL_PIN );
#endif

	// The pull pin animation has to finish, then we wait until they aren't holding the primary
	// attack button, then throw the grenade.
	SendWeaponAnim( ACT_VM_PULLPIN );
	m_bPinPulled = true;

	// Don't let weapon idle interfere in the middle of a throw!
	MDLCACHE_CRITICAL_SECTION();
	SetWeaponIdleTime( gpGlobals->curtime + SequenceDuration() );

	m_flNextPrimaryAttack	= gpGlobals->curtime + SequenceDuration();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseCSGrenade::SecondaryAttack()
{
	if ( !m_bPinPulled )
	{
		m_flThrowStrength = 0.0f;
		m_flThrowStrengthClientSmooth = 0.0f;
	}

	if ( CSGameRules()->IsFreezePeriod() )	// Don't let Brian molotov the team during freezetime
		return;

	PrimaryAttack();
	return;

	/*if ( m_bRedraw )
		return;

	CCSPlayer *pPlayer = GetPlayerOwner();
	
	if ( pPlayer == NULL )
		return;

	//See if we're ducking
	if ( pPlayer->GetFlags() & FL_DUCKING )
	{
		//Send the weapon animation
		SendWeaponAnim( ACT_VM_SECONDARYATTACK );
	}
	else
	{
		//Send the weapon animation
		SendWeaponAnim( ACT_VM_HAULBACK );
	}

	// Don't let weapon idle interfere in the middle of a throw!
	SetWeaponIdleTime( gpGlobals->curtime + SequenceDuration() );

	m_flNextSecondaryAttack	= gpGlobals->curtime + SequenceDuration();*/
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBaseCSGrenade::Reload()
{
	if ( ( m_bRedraw ) && ( m_flNextPrimaryAttack <= gpGlobals->curtime ) && ( m_flNextSecondaryAttack <= gpGlobals->curtime ) )
	{
		//Redraw the weapon
		SendWeaponAnim( ACT_VM_DRAW );

		//Update our times
		m_flNextPrimaryAttack	= gpGlobals->curtime + SequenceDuration();
		m_flNextSecondaryAttack	= gpGlobals->curtime + SequenceDuration();

		SetWeaponIdleTime( gpGlobals->curtime + SequenceDuration() );
		
		//Mark this as done
	//	m_bRedraw = false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPicker - 
//-----------------------------------------------------------------------------
void CBaseCSGrenade::OnPickedUp( CBaseCombatCharacter *pNewOwner )
{
	BaseClass::OnPickedUp( pNewOwner );

#if !defined( CLIENT_DLL )
	if ( pNewOwner )
	{
		m_bIsHeldByPlayer = true;
	}
#endif
}

float CBaseCSGrenade::ApproachThrownStrength()
{
	m_flThrowStrengthClientSmooth = Approach(
		m_flThrowStrength,
		m_flThrowStrengthClientSmooth,
		gpGlobals->frametime * GRENADE_SECONDARY_INTERP
		);
	return m_flThrowStrengthClientSmooth;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseCSGrenade::ItemPostFrame()
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	CBaseViewModel *vm = pPlayer->GetViewModel( m_nViewModelIndex );
	if ( !vm )
		return;

	bool bPrimaryHeld = (pPlayer->m_nButtons & IN_ATTACK) != 0;
	bool bSecondaryHeld = (pPlayer->m_nButtons & IN_ATTACK2) != 0;

	if ( m_bPinPulled && (bPrimaryHeld || bSecondaryHeld) )
	{
		float flIdealThrowStrength = 0.5f;

		if ( bPrimaryHeld )
			flIdealThrowStrength += 0.5f;

		if ( bSecondaryHeld )
			flIdealThrowStrength -= 0.5f;

		m_flThrowStrength = Approach( flIdealThrowStrength, m_flThrowStrength, gpGlobals->frametime * GRENADE_SECONDARY_TRANSITION );
	}

	// If they let go of the fire button, they want to throw the grenade.
	if ( m_bPinPulled && !(bPrimaryHeld) && !(bSecondaryHeld) )
	{
		if ( IsThrownUnderhand() )
			pPlayer->DoAnimationEvent( PLAYERANIMEVENT_THROW_GRENADE_UNDERHAND );
		else
			pPlayer->DoAnimationEvent( PLAYERANIMEVENT_THROW_GRENADE );

		StartGrenadeThrow();
		
		MDLCACHE_CRITICAL_SECTION();
		m_bPinPulled = false;
		if ( IsThrownUnderhand() )
		{
			SendWeaponAnim( ACT_VM_RELEASE );
		}
		else
		{
			SendWeaponAnim( ACT_VM_THROW );
		}
		SetWeaponIdleTime( gpGlobals->curtime + SequenceDuration() );

		m_flNextPrimaryAttack	= gpGlobals->curtime + SequenceDuration(); // we're still throwing, so reset our next primary attack

#ifndef CLIENT_DLL
		IGameEvent * event = gameeventmanager->CreateEvent( "weapon_fire" );
		if( event )
		{
			const char *weaponName = STRING( m_iClassname );
			if ( strncmp( weaponName, "weapon_", 7 ) == 0 )
			{
				weaponName += 7;
			}

			event->SetInt( "userid", pPlayer->GetUserID() );
			event->SetString( "weapon", weaponName );
			event->SetBool( "silenced", false );
			gameeventmanager->FireEvent( event );
		}
#endif
	}
	else if ((m_fThrowTime > 0) && (m_fThrowTime < gpGlobals->curtime))
	{
		// only decrement our ammo when we actually create the projectile
		DecrementAmmo( pPlayer );

		ThrowGrenade();
	}
	else if ( !m_bIsHeldByPlayer )
	{
		// Has the throw animation finished playing
		if( m_flTimeWeaponIdle < gpGlobals->curtime )
		{
#ifdef GAME_DLL
			// if we're officially out of grenades, ditch this weapon
			if( pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0 )
			{
				pPlayer->Weapon_Drop( this, NULL, NULL );
				UTIL_Remove(this);
			}
			else
			{
				pPlayer->SwitchToNextBestWeapon( this );
			}
#endif
			return;	//don't animate this grenade any more!
		}	
	}
	else if( !m_bRedraw )
	{
		BaseClass::ItemPostFrame();
	}
}



#ifdef CLIENT_DLL

	void CBaseCSGrenade::DecrementAmmo( CBaseCombatCharacter *pOwner )
	{
	}

	void CBaseCSGrenade::DropGrenade()
	{
		m_bRedraw = true;
		m_bIsHeldByPlayer = false;
		m_fThrowTime = 0.0f;
	}

	void CBaseCSGrenade::ThrowGrenade()
	{
		m_bRedraw = true;
		m_bIsHeldByPlayer = false;
		m_fThrowTime = 0.0f;
	}

	void CBaseCSGrenade::StartGrenadeThrow()
	{
		m_fThrowTime = gpGlobals->curtime + 0.1f;

		CBroadcastRecipientFilter filter;
		CSoundParameters params;
		if ( GetParametersForSound( GetShootSound( SINGLE ), params, NULL ) )
		{
			//CPASAttenuationFilter filter( this );
			EmitSound( filter, entindex(), GetShootSound( SINGLE )); 
		}
	}

#else

	BEGIN_DATADESC( CBaseCSGrenade )
		DEFINE_FIELD( m_bRedraw, FIELD_BOOLEAN ),
	END_DATADESC()

	int CBaseCSGrenade::CapabilitiesGet()
	{
		return bits_CAP_WEAPON_RANGE_ATTACK1; 
	}

	//-----------------------------------------------------------------------------
	// Purpose: 
	// Input  : *pOwner - 
	//-----------------------------------------------------------------------------
	void CBaseCSGrenade::DecrementAmmo( CBaseCombatCharacter *pOwner )
	{
		pOwner->RemoveAmmo( 1, m_iPrimaryAmmoType );
	}

	void CBaseCSGrenade::StartGrenadeThrow()
	{
		m_fThrowTime = gpGlobals->curtime + 0.1f;
	}

	void CBaseCSGrenade::ThrowGrenade()
	{
		CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
		if ( !pPlayer )
		{
			Assert( false );
			return;
		}

		QAngle angThrow = pPlayer->LocalEyeAngles();

		Vector vForward, vRight, vUp;

		if ( angThrow.x < 0 )
		{
			angThrow.x += 360; // make sure we have a positive angle from LocalEyeAngles()
		}

		if ( angThrow.x < 90 )
			angThrow.x = -10 + angThrow.x * ((90 + 10) / 90.0);
		else
		{
			angThrow.x = 360.0f - angThrow.x;
			angThrow.x = -10 + angThrow.x * -((90 - 10) / 90.0);
		}

		const float kBaseVelocity = GetCSWpnData().m_fThrowVelocity;
		float flVel = clamp( ( kBaseVelocity * 0.9f ), 15, 750 );

		if (flVel > 750)
			flVel = 750;

		//clamp the throw strength ranges just to be sure
		float flClampedThrowStrength = m_flThrowStrength;
		flClampedThrowStrength = clamp( flClampedThrowStrength, 0.0f, 1.0f );

		flVel *= Lerp( flClampedThrowStrength, GRENADE_SECONDARY_DAMPENING, 1.0f );

		AngleVectors( angThrow, &vForward, &vRight, &vUp );

		Vector vecSrc = pPlayer->GetAbsOrigin() + pPlayer->GetViewOffset();

		vecSrc += Vector( 0, 0, Lerp( flClampedThrowStrength, -GRENADE_SECONDARY_LOWER, 0.0f ) );

		// We want to throw the grenade from 16 units out.  But that can cause problems if we're facing
		// a thin wall.  Do a hull trace to be safe.
		trace_t trace;
		Vector mins( -2, -2, -2 );
		Vector maxs(  2,  2,  2 );
		UTIL_TraceHull( vecSrc, vecSrc + vForward * 16, mins, maxs, MASK_SOLID | CONTENTS_GRENADECLIP, pPlayer, COLLISION_GROUP_NONE, &trace );
		vecSrc = trace.endpos;

		Vector vecThrow = vForward * flVel + (pPlayer->GetAbsVelocity() * 1.25);

		EmitGrenade( vecSrc, vec3_angle, vecThrow, AngularImpulse(600,random->RandomInt(-1200,1200),0), pPlayer );

		m_bHasEmittedProjectile = true; // Flag the grenade weapon as having emitted a projectile. The 'grenade' is now flying away from the player, so we don't want to drop *this* grenade on death (that'll make a duplicate)

		m_bRedraw = true;
		m_bIsHeldByPlayer = false;
		m_fThrowTime = 0.0f;

		CCSPlayer *pCSPlayer = ToCSPlayer( pPlayer );

		if( pCSPlayer )
		{
			int iWeaponId = GetWeaponID();

			if ( !sv_ignoregrenaderadio.GetBool() )
			{
				switch ( iWeaponId )
				{
					case WEAPON_HEGRENADE:
					{
						pCSPlayer->Radio( "Radio.FireInTheHole", "#Cstrike_TitlesTXT_Fire_in_the_hole" );
						break;
					}
					case WEAPON_FLASHBANG:
					{
						pCSPlayer->Radio( "Radio.Flashbang", "#Cstrike_TitlesTXT_Flashbang_in_the_hole" );
						break;
					}
					case WEAPON_SMOKEGRENADE:
					{
						pCSPlayer->Radio( "Radio.Smoke", "#Cstrike_TitlesTXT_Smoke_in_the_hole" );
						break;
					}
					case WEAPON_MOLOTOV:
					{
						pCSPlayer->Radio( "Radio.Molotov", "#Cstrike_TitlesTXT_Molotov_in_the_hole" );
						break;
					}
					case WEAPON_INCGRENADE:
					{
						pCSPlayer->Radio( "Radio.Incendiary", "#Cstrike_TitlesTXT_Incendiary_in_the_hole" );
						break;
					}
					case WEAPON_DECOY:
					{
						pCSPlayer->Radio( "Radio.Decoy", "#Cstrike_TitlesTXT_Decoy_in_the_hole" );
						break;
					}
				}
			}
			CCS_GameStats.IncrementStat(pCSPlayer, CSSTAT_GRENADES_THROWN, 1);
		}
	}

	void CBaseCSGrenade::DropGrenade()
	{
		CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
		if ( !pPlayer )
		{
			Assert( false );
			return;
		}

		Vector vForward;
		pPlayer->EyeVectors( &vForward );
		Vector vecSrc = pPlayer->GetAbsOrigin() + pPlayer->GetViewOffset() + vForward * 16; 

		Vector vecVel = pPlayer->GetAbsVelocity();

		EmitGrenade( vecSrc, vec3_angle, vecVel, AngularImpulse(600,random->RandomInt(-1200,1200),0), pPlayer );

		CCSPlayer *pCSPlayer = ToCSPlayer( pPlayer );

		if( pCSPlayer )
		{
			CCS_GameStats.IncrementStat(pCSPlayer, CSSTAT_GRENADES_THROWN, 1);
		}

		m_bRedraw = true;
		m_bIsHeldByPlayer = false;
		m_fThrowTime = 0.0f;
	}

	void CBaseCSGrenade::EmitGrenade( Vector vecSrc, QAngle vecAngles, Vector vecVel, AngularImpulse angImpulse, CBasePlayer *pPlayer )
	{
		Assert( 0 && "CBaseCSGrenade::EmitGrenade should not be called. Make sure to implement this in your subclass!\n" );
	}

	bool CBaseCSGrenade::AllowsAutoSwitchFrom( void ) const
	{
		return !m_bPinPulled;
	}

#endif

