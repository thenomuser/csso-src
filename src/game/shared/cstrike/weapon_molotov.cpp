//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbase.h"
#include "gamerules.h"
#include "npcevent.h"
#include "engine/IEngineSound.h"
#include "weapon_molotov.h"

#ifdef CLIENT_DLL
	#include "c_cs_player.h"
#else
	#include "cs_player.h"
	#include "items.h"
	#include "molotov_projectile.h"
	#include "effects/inferno.h"
#endif

// NOTE: This has to be the last file included!
#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED( MolotovGrenade, DT_MolotovGrenade )

BEGIN_NETWORK_TABLE( CMolotovGrenade, DT_MolotovGrenade )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CMolotovGrenade )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_molotov, CMolotovGrenade );
PRECACHE_REGISTER( weapon_molotov );

#if !defined( CLIENT_DLL )

BEGIN_DATADESC( CMolotovGrenade )
END_DATADESC()

void CMolotovGrenade::EmitGrenade( Vector vecSrc, QAngle vecAngles, Vector vecVel, AngularImpulse angImpulse, CBasePlayer *pPlayer )
{
	// [mlowrance] were throwing the grenade, be sure to remove flame sound effect
	SetLoopingSoundPlaying( false );
	StopSound( "Molotov.IdleLoop" ); 
	//DevMsg( 1, "---------->Stopping Molotov.IdleLoop 2\n" );
	CMolotovProjectile::Create( vecSrc, vecAngles, vecVel, angImpulse, pPlayer, false );
}

void CMolotovGrenade::Precache( void )
{
	BaseClass::Precache();
	
	PrecacheScriptSound( "Molotov.IdleLoop" );
	PrecacheParticleSystem( "weapon_molotov_held" );
}

#else // GAME_DLL

void CMolotovGrenade::UpdateParticles( void )
{
	// FIXME: This is bogus; we need to make the particle property have particle system types:
	// first person, third person, owner, and logic in the particle property to know whether
	// to render a given system given these rules and knowledge of what mode the owner is in
	C_CSPlayer *pPlayer = ToCSPlayer( GetOwner() );
	if ( !pPlayer )
		return;

	//int nRenderFlags = 0;

	CWeaponCSBase *pCSWeapon = (CWeaponCSBase*)pPlayer->GetActiveWeapon();
	if ( !pCSWeapon )
		return;

	if ( pCSWeapon->GetCSWeaponID() == WEAPON_MOLOTOV )
	{
		if ( m_molotovParticleEffect.IsValid() )
		{
			//		m_molotovParticleEffect->SetDormant( pPlayer->GetPlayerAnimState()->ShouldHideWeapon() ); // ShouldHideWeapon is a Terror Codebase function, not CStrike15
			m_molotovParticleEffect->SetDormant( !pCSWeapon->IsVisible() ); // Is the weapon Hidden?
		}

		if ( m_bPinPulled )
		{
			if ( !m_molotovParticleEffect() )
			{
				// TEST: [mlowrance] This is to test for attachment.
				int iAttachment = pCSWeapon->LookupAttachment( "Wick" );

				if ( iAttachment >= 0 )
				{
					// FIXME: Precache 'Wick' attachment index
					m_molotovParticleEffect = pCSWeapon->ParticleProp()->Create( "weapon_molotov_held", PATTACH_POINT_FOLLOW, iAttachment );
					EmitSound( "Molotov.IdleLoop" );
					SetLoopingSoundPlaying( true );

					//DevMsg( 1, "++++++++++>Playing Molotov.IdleLoop 1\n" );
				}
			}
		}
	}
	else
	{
		if ( m_molotovParticleEffect.IsValid() )
		{
			StopSound( "Molotov.IdleLoop" );
			//DevMsg( 1, "---------->Stopping Molotov.IdleLoop 1\n" );
			m_molotovParticleEffect->StopEmission( false, false );
			m_molotovParticleEffect->SetRemoveFlag();
			m_molotovParticleEffect = NULL;
		}
	}
}

void CMolotovGrenade::Simulate()
{
	UpdateParticles();
	return BaseClass::Simulate();
}

//--------------------------------------------------------------------------------------------------------
void CMolotovGrenade::OnParticleEffectDeleted( CNewParticleEffect *pParticleEffect )
{
	if ( m_molotovParticleEffect() == pParticleEffect )
	{
		m_molotovParticleEffect = NULL;
	}
}

#endif // !CLIENT_DLL

void CMolotovGrenade::Drop(const Vector& vecVelocity)
{
	CBaseCSGrenade::Drop(vecVelocity);
	StopSound( "Molotov.IdleLoop" ); 
	SetLoopingSoundPlaying( false );
}


IMPLEMENT_NETWORKCLASS_ALIASED( IncendiaryGrenade, DT_IncendiaryGrenade )

BEGIN_NETWORK_TABLE( CIncendiaryGrenade, DT_IncendiaryGrenade )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CIncendiaryGrenade )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_incgrenade, CIncendiaryGrenade );
PRECACHE_REGISTER( weapon_incgrenade );

#if !defined( CLIENT_DLL )

BEGIN_DATADESC( CIncendiaryGrenade )
END_DATADESC()

void CIncendiaryGrenade::EmitGrenade( Vector vecSrc, QAngle vecAngles, Vector vecVel, AngularImpulse angImpulse, CBasePlayer *pPlayer )
{
	// [mlowrance] were throwing the grenade, be sure to remove flame sound effect
	//SetLoopingSoundPlaying( false );
	StopSound( "Molotov.IdleLoop" ); 
	//DevMsg( 1, "---------->Stopping Molotov.IdleLoop 2\n" );
	CMolotovProjectile::Create( vecSrc, vecAngles, vecVel, angImpulse, pPlayer, true );
}

void CIncendiaryGrenade::Precache( void )
{
	BaseClass::Precache();
	
	PrecacheScriptSound( "Molotov.IdleLoop" );
	PrecacheParticleSystem( "weapon_molotov_held" );
}

#else // GAME_DLL

void CIncendiaryGrenade::UpdateParticles( void )
{
	// FIXME: This is bogus; we need to make the particle property have particle system types:
	// first person, third person, owner, and logic in the particle property to know whether
	// to render a given system given these rules and knowledge of what mode the owner is in
	C_CSPlayer *pPlayer = ToCSPlayer( GetOwner() );
	if ( !pPlayer )
		return;

	//int nRenderFlags = 0;

	CWeaponCSBase *pCSWeapon = (CWeaponCSBase*)pPlayer->GetActiveWeapon();
	if ( !pCSWeapon )
		return;
	
	if ( m_molotovParticleEffect.IsValid() )
	{
		StopSound( "Molotov.IdleLoop" );
		//DevMsg( 1, "---------->Stopping Molotov.IdleLoop 1\n" );
		m_molotovParticleEffect->StopEmission( false, false );
		m_molotovParticleEffect->SetRemoveFlag();
		m_molotovParticleEffect = NULL;
	}
}

void CIncendiaryGrenade::Simulate()
{
	UpdateParticles();
	return BaseClass::Simulate();
}

//--------------------------------------------------------------------------------------------------------
void CIncendiaryGrenade::OnParticleEffectDeleted( CNewParticleEffect *pParticleEffect )
{
	if ( m_molotovParticleEffect() == pParticleEffect )
	{
		m_molotovParticleEffect = NULL;
	}
}

#endif // !CLIENT_DLL

void CIncendiaryGrenade::Drop(const Vector& vecVelocity)
{
	CBaseCSGrenade::Drop(vecVelocity);
	StopSound( "Molotov.IdleLoop" ); 
	SetLoopingSoundPlaying( false );
}
