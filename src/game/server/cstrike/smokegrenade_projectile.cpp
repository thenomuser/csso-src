//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "smokegrenade_projectile.h"
#include "sendproxy.h"
#include "particle_smokegrenade.h"
#include "cs_player.h"
#include "KeyValues.h"
#include "bot_manager.h"
#include "weapon_csbase.h"
#include "effects/inferno.h"

#define GRENADE_MODEL "models/Weapons/w_eq_smokegrenade_thrown.mdl"


LINK_ENTITY_TO_CLASS( smokegrenade_projectile, CSmokeGrenadeProjectile );
PRECACHE_WEAPON_REGISTER( smokegrenade_projectile );

BEGIN_DATADESC( CSmokeGrenadeProjectile )
	DEFINE_THINKFUNC( Think_Detonate ),
	DEFINE_THINKFUNC( Think_Fade ),
	DEFINE_THINKFUNC( Think_Remove )
END_DATADESC()


CSmokeGrenadeProjectile* CSmokeGrenadeProjectile::Create( 
	const Vector &position, 
	const QAngle &angles, 
	const Vector &velocity, 
	const AngularImpulse &angVelocity, 
	CBaseCombatCharacter *pOwner )
{
	CSmokeGrenadeProjectile *pGrenade = (CSmokeGrenadeProjectile*)CBaseEntity::Create( "smokegrenade_projectile", position, angles, pOwner );
	
	// Set the timer for 1 second less than requested. We're going to issue a SOUND_DANGER
	// one second before detonation.
	pGrenade->SetTimer( 1.5 );
	pGrenade->SetAbsVelocity( velocity );
	pGrenade->SetupInitialTransmittedGrenadeVelocity( velocity );
	pGrenade->SetThrower( pOwner );
	pGrenade->SetGravity( 0.55 );
	pGrenade->SetFriction( 0.7 );
	pGrenade->m_flDamage = 100;
	pGrenade->ChangeTeam( pOwner->GetTeamNumber() );
	pGrenade->ApplyLocalAngularVelocityImpulse( angVelocity );	
	pGrenade->SetTouch( &CBaseGrenade::BounceTouch );

	pGrenade->SetGravity( BaseClass::GetGrenadeGravity() );
	pGrenade->SetFriction( BaseClass::GetGrenadeFriction() );
	pGrenade->SetElasticity( BaseClass::GetGrenadeElasticity() );
	pGrenade->m_bDidSmokeEffect = false;
	pGrenade->m_flLastBounce = 0;

	pGrenade->m_pWeaponInfo = GetWeaponInfo( WEAPON_SMOKEGRENADE );

	return pGrenade;
}


void CSmokeGrenadeProjectile::SetTimer( float timer )
{
	SetThink( &CSmokeGrenadeProjectile::Think_Detonate );
	SetNextThink( gpGlobals->curtime + timer );

	TheBots->SetGrenadeRadius( this, 0.0f );
}

void CSmokeGrenadeProjectile::Think_Detonate()
{
	if ( GetAbsVelocity().Length() > 0.1 )
	{
		// Still moving. Don't detonate yet.
		SetNextThink( gpGlobals->curtime + 0.2 );
		return;
	}

	SmokeDetonate();
}

void CSmokeGrenadeProjectile::SmokeDetonate( void )
{
	TheBots->SetGrenadeRadius( this, SmokeGrenadeRadius );

	// Ok, we've stopped rolling or whatever. Now detonate.
	ParticleSmokeGrenade *pGren = (ParticleSmokeGrenade*)CBaseEntity::Create( PARTICLESMOKEGRENADE_ENTITYNAME, GetAbsOrigin(), QAngle(0,0,0), NULL );
	if ( pGren )
	{
		pGren->FillVolume();
		pGren->SetFadeTime( 15, 20 );
		pGren->SetAbsOrigin( GetAbsOrigin() );

		//tell the hostages about the smoke!
		CBaseEntity *pEntity = NULL;
		variant_t var;	//send the location of the smoke?
		var.SetVector3D( GetAbsOrigin() );
		while ( ( pEntity = gEntList.FindEntityByClassname( pEntity, "hostage_entity" ) ) != NULL)
		{
			//send to hostages that have a resonable chance of being in it while its still smoking
			if( (GetAbsOrigin() - pEntity->GetAbsOrigin()).Length() < 1000 )
				pEntity->AcceptInput( "smokegrenade", this, this, var, 0 );
		}

		// tell the bots a smoke grenade has exploded
		CCSPlayer *player = ToCSPlayer(GetThrower());
		if ( player )
		{
			IGameEvent * event = gameeventmanager->CreateEvent( "smokegrenade_detonate" );
			if ( event )
			{
				event->SetInt( "userid", player->GetUserID() );
				event->SetFloat( "x", GetAbsOrigin().x );
				event->SetFloat( "y", GetAbsOrigin().y );
				event->SetFloat( "z", GetAbsOrigin().z );
				gameeventmanager->FireEvent( event );
			}
		}
	}

	m_hSmokeEffect = pGren;
	m_bDidSmokeEffect = true;

	EmitSound( "BaseSmokeEffect.Sound" );

	m_nRenderMode = kRenderTransColor;
	SetNextThink( gpGlobals->curtime + 12.5f );
	SetThink( &CSmokeGrenadeProjectile::Think_Fade );
}

// Fade the projectile out over time before making it disappear
void CSmokeGrenadeProjectile::Think_Fade()
{
	SetNextThink( gpGlobals->curtime );

	byte a = GetRenderColor().a;
	a -= 1;
	SetRenderColorA( a );

	if ( !a )
	{
		//RemoveGrenadeFromLists();
		SetNextThink( gpGlobals->curtime + 1.0 );
		SetThink( &CSmokeGrenadeProjectile::Think_Remove );	// Spit out smoke for 10 seconds.
	}
}


void CSmokeGrenadeProjectile::Think_Remove()
{
	SetMoveType( MOVETYPE_NONE );
	UTIL_Remove( this );
}

//Implement this so we never call the base class,
//but this should never be called either.
void CSmokeGrenadeProjectile::Detonate( void )
{
	Assert(!"Smoke grenade handles its own detonation");
}


void CSmokeGrenadeProjectile::Spawn()
{
	SetModel( GRENADE_MODEL );
	BaseClass::Spawn();

	SetThrownBodygroup();
}


void CSmokeGrenadeProjectile::Precache()
{
	PrecacheModel( GRENADE_MODEL );
	PrecacheScriptSound( "BaseSmokeEffect.Sound" );
	PrecacheScriptSound( "SmokeGrenade.Bounce" );
	BaseClass::Precache();
}


void CSmokeGrenadeProjectile::OnBounced( void )
{
	if ( m_flLastBounce >= ( gpGlobals->curtime - 3*gpGlobals->interval_per_tick ) )
		return;

	m_flLastBounce = gpGlobals->curtime;

	//
	// if the smoke grenade is above ground, trace down to the ground and see where it would end up?
	//
	Vector posDropSmoke = GetAbsOrigin();
	trace_t trSmokeTrace;
	UTIL_TraceLine( posDropSmoke, posDropSmoke - Vector( 0, 0, SmokeGrenadeRadius ), ( MASK_PLAYERSOLID & ~CONTENTS_PLAYERCLIP ),
		this, COLLISION_GROUP_PROJECTILE, &trSmokeTrace );
	if ( !trSmokeTrace.startsolid )
	{
		if ( trSmokeTrace.fraction >= 1.0f )
			return;	// this smoke cannot drop enough to cause extinguish

		if ( trSmokeTrace.fraction > 0.001f )
			posDropSmoke = trSmokeTrace.endpos;
	}

	//
	// See if it touches any inferno?
	//
	const int maxEnts = 64;
	CBaseEntity *list[ maxEnts ];
	int count = UTIL_EntitiesInSphere( list, maxEnts, GetAbsOrigin(), 512, FL_ONFIRE );
	for( int i=0; i<count; ++i )
	{
		if (list[i] == NULL || list[i] == this)
			continue;

		CInferno* pInferno = dynamic_cast<CInferno*>( list[i] );

		if ( pInferno && pInferno->BShouldExtinguishSmokeGrenadeBounce( this, posDropSmoke ) )
		{
			if ( posDropSmoke != GetAbsOrigin() )
			{
				const QAngle qAngOriginZero = vec3_angle;
				const Vector vVelocityZero = vec3_origin;
				Teleport( &posDropSmoke, &qAngOriginZero, &vVelocityZero );
			}

			SmokeDetonate();
			break;
		}
	}
}

void CSmokeGrenadeProjectile::BounceSound( void )
{
	if ( !m_bDidSmokeEffect )
	{
		EmitSound( "SmokeGrenade.Bounce" );
	}
}
