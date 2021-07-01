//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "flashbang_projectile.h"
#include "shake.h"
#include "engine/IEngineSound.h"
#include "cs_player.h"
#include "dlight.h"
#include "KeyValues.h"
#include "weapon_csbase.h"
#include "collisionutils.h"
#include "particle_smokegrenade.h"
#include "smoke_fog_overlay_shared.h"
#include "cs_simple_hostage.h"

#define GRENADE_MODEL "models/weapons/w_eq_flashbang_thrown.mdl"


LINK_ENTITY_TO_CLASS( flashbang_projectile, CFlashbangProjectile );
PRECACHE_WEAPON_REGISTER( flashbang_projectile );

class CTraceFilterForFlashbang : public CTraceFilterNoPlayers
{
public:
	CTraceFilterForFlashbang( const IHandleEntity *passentity = NULL, int collisionGroup = COLLISION_GROUP_NONE )
		: CTraceFilterNoPlayers( passentity, collisionGroup )
	{
	}

	virtual bool ShouldHitEntity( IHandleEntity *pHandleEntity, int contentsMask )
	{

		CBaseEntity *pEnt = EntityFromEntityHandle(pHandleEntity);
		if ( pEnt )
		{
			// Hostages don't block flashbangs
			CHostage* pHostage = dynamic_cast<CHostage*>(pEnt);
			if ( pHostage )
				return false;

			// Weapons don't block flashbangs
			CWeaponCSBase* pWeapon = dynamic_cast< CWeaponCSBase* >( pEnt );
			CBaseGrenade* pGrenade = dynamic_cast< CBaseGrenade* > ( pEnt );
			if ( pWeapon || pGrenade )
				return false;
		}

		return CTraceFilterNoPlayers::ShouldHitEntity( pHandleEntity, contentsMask );
	}
};

float PercentageOfFlashForPlayer(CBaseEntity *player, Vector flashPos, CBaseEntity *pevInflictor)
{
	if (!(player->IsPlayer()))
	{
		// if this entity isn't a player, it's a hostage or some other entity, then don't bother with the expensive checks
		// that come below.
		return 0.0f;
	}

	const float FLASH_FRACTION = 0.167f;
	const float SIDE_OFFSET = 75.0f;

	Vector pos = player->EyePosition();
	Vector vecRight, vecUp;

	QAngle tempAngle;
	VectorAngles(player->EyePosition() - flashPos, tempAngle);
	AngleVectors(tempAngle, NULL, &vecRight, &vecUp);

	vecRight.NormalizeInPlace();
	vecUp.NormalizeInPlace();

	// Set up all the ray stuff.
	// We don't want to let other players block the flash bang so we use this custom filter.
	Ray_t ray;
	trace_t tr;
	CTraceFilterForFlashbang traceFilter( pevInflictor, COLLISION_GROUP_NONE );
	unsigned int FLASH_MASK = MASK_OPAQUE_AND_NPCS | CONTENTS_DEBRIS;

	// According to comment in IsNoDrawBrush in cmodel.cpp, CONTENTS_OPAQUE is ONLY used for block light surfaces,
	// and we want flashbang traces to pass through those, since the block light surface is only used for blocking
	// lightmap light rays during map compilation.
	FLASH_MASK &= ~CONTENTS_OPAQUE;

	ray.Init( flashPos, pos );
	enginetrace->TraceRay( ray, FLASH_MASK, &traceFilter,  &tr );

	if ((tr.fraction == 1.0f) || (tr.m_pEnt == player))
	{
		return 1.0f;
	}

	float retval = 0.0f;

	// check the point straight up.
	pos = flashPos + vecUp*50.0f;
	ray.Init( flashPos, pos );
	enginetrace->TraceRay( ray, FLASH_MASK, &traceFilter,  &tr );
	// Now shoot it to the player's eye.
	pos = player->EyePosition();
	ray.Init( tr.endpos, pos );
	enginetrace->TraceRay( ray, FLASH_MASK, &traceFilter,  &tr );

	if ((tr.fraction == 1.0f) || (tr.m_pEnt == player))
	{
		retval += FLASH_FRACTION;
	}

	// check the point up and right.
	pos = flashPos + vecRight*SIDE_OFFSET + vecUp*10.0f;
	ray.Init( flashPos, pos );
	enginetrace->TraceRay( ray, FLASH_MASK, &traceFilter,  &tr );
	// Now shoot it to the player's eye.
	pos = player->EyePosition();
	ray.Init( tr.endpos, pos );
	enginetrace->TraceRay( ray, FLASH_MASK, &traceFilter,  &tr );

	if ((tr.fraction == 1.0f) || (tr.m_pEnt == player))
	{
		retval += FLASH_FRACTION;
	}

	// Check the point up and left.
	pos = flashPos - vecRight*SIDE_OFFSET + vecUp*10.0f;
	ray.Init( flashPos, pos );
	enginetrace->TraceRay( ray, FLASH_MASK, &traceFilter,  &tr );
	// Now shoot it to the player's eye.
	pos = player->EyePosition();
	ray.Init( tr.endpos, pos );
	enginetrace->TraceRay( ray, FLASH_MASK, &traceFilter,  &tr );

	if ((tr.fraction == 1.0f) || (tr.m_pEnt == player))
	{
		retval += FLASH_FRACTION;
	}

	return retval;
}

// --------------------------------------------------------------------------------------------------- //
//
// RadiusDamage - this entity is exploding, or otherwise needs to inflict damage upon entities within a certain range.
// 
// only damage ents that can clearly be seen by the explosion!
// --------------------------------------------------------------------------------------------------- //

void RadiusFlash( 
	Vector vecSrc, 
	CBaseEntity *pevInflictor, 
	CBaseEntity *pevAttacker, 
	float flDamage, 
	int iClassIgnore, 
	int bitsDamageType )
{
	vecSrc.z += 1;// in case grenade is lying on the ground

	if ( !pevAttacker )
		pevAttacker = pevInflictor;
	
	trace_t		tr;
	float		flAdjustedDamage;
	variant_t	var;
	Vector		vecEyePos;
	float		fadeTime, fadeHold;
	Vector		vForward;
	Vector		vecLOS;
	float		flDot;
	
	CBaseEntity		*pEntity = NULL;
	static float	flRadius = 3000;
	float			falloff = flDamage / flRadius;

	//bool bInWater = (UTIL_PointContents( vecSrc, MASK_WATER ) == CONTENTS_WATER);

	// iterate on all entities in the vicinity.
	while ((pEntity = gEntList.FindEntityInSphere( pEntity, vecSrc, flRadius )) != NULL)
	{	
		bool bPlayer = pEntity->IsPlayer();
		
		if( !bPlayer )
			continue;

		vecEyePos = pEntity->EyePosition();

		//// blasts used to not travel into or out of water, users assumed it was a bug. Fix is not to run this check -wills
		//if ( bInWater && pEntity->GetWaterLevel() == WL_NotInWater)
		//	continue;
		//if (!bInWater && pEntity->GetWaterLevel() == WL_Eyes)
		//	continue;

		float percentageOfFlash = PercentageOfFlashForPlayer(pEntity, vecSrc, pevInflictor);

		if ( percentageOfFlash > 0.0 )
		{
			// decrease damage for an ent that's farther from the grenade
			flAdjustedDamage = flDamage - ( vecSrc - pEntity->EyePosition() ).Length() * falloff;

			if ( flAdjustedDamage > 0 )
			{
				// See if we were facing the flash
				AngleVectors( pEntity->EyeAngles(), &vForward );

				vecLOS = ( vecSrc - vecEyePos );

				float flDistance = vecLOS.Length();

				//DebugDrawLine( vecEyePos, vecEyePos + (100.0 * vecLOS), 0, 255, 0, true, 10.0 );
				//DebugDrawLine( vecEyePos, vecEyePos + (100.0 * vForward), 0, 0, 255, true, 10.0 );

				// Normalize both vectors so the dotproduct is in the range -1.0 <= x <= 1.0 
				vecLOS.NormalizeInPlace();


				flDot = DotProduct (vecLOS, vForward);

				float startingAlpha = 255;
	
				// if target is facing the bomb, the effect lasts longer
				if( flDot >= 0.6 )
				{
					// looking at the flashbang
					fadeTime = flAdjustedDamage * 2.5f;
					fadeHold = flAdjustedDamage * 1.25f;
				}
				else if( flDot >= 0.3 )
				{
					// looking to the side
					fadeTime = flAdjustedDamage * 1.75f;
					fadeHold = flAdjustedDamage * 0.8f;
				}
				else if( flDot >= -0.2 )
				{
					// looking to the side
					fadeTime = flAdjustedDamage * 1.00f;
					fadeHold = flAdjustedDamage * 0.5f;
				}
				else
				{
					// facing away
					fadeTime = flAdjustedDamage * 0.5f;
					fadeHold = flAdjustedDamage * 0.25f;
				//	startingAlpha = 200;
				}

				fadeTime *= percentageOfFlash;
				fadeHold *= percentageOfFlash;

				if ( bPlayer )
				{
					// blind players and bots
					CCSPlayer *player = static_cast< CCSPlayer * >( pEntity );

					// [tj] Store who was responsible for the most recent flashbang blinding.
					CCSPlayer *attacker = ToCSPlayer (pevAttacker);
					if ( attacker && player && player->IsAlive() )
					{
						player->SetLastFlashbangAttacker(attacker);
					}

					player->Blind( fadeHold, fadeTime, startingAlpha );

					// deafen players and bots
					player->Deafen( flDistance );
				}
			}	
		}
	}

	CPVSFilter filter(vecSrc);
	te->DynamicLight( filter, 0.0, &vecSrc, 255, 255, 255, 2, 400, 0.1, 768 );
}

// --------------------------------------------------------------------------------------------------- //
// CFlashbangProjectile implementation.
// --------------------------------------------------------------------------------------------------- //

CFlashbangProjectile* CFlashbangProjectile::Create( 
	const Vector &position, 
	const QAngle &angles, 
	const Vector &velocity, 
	const AngularImpulse &angVelocity, 
	CBaseCombatCharacter *pOwner )
{
	CFlashbangProjectile *pGrenade = (CFlashbangProjectile*)CBaseEntity::Create( "flashbang_projectile", position, angles, pOwner );
	
	// Set the timer for 1 second less than requested. We're going to issue a SOUND_DANGER
	// one second before detonation.
	pGrenade->SetAbsVelocity( velocity );
	pGrenade->SetupInitialTransmittedGrenadeVelocity( velocity );
	pGrenade->SetThrower( pOwner );
	pGrenade->m_flDamage = 100;
	pGrenade->ChangeTeam( pOwner->GetTeamNumber() );

	pGrenade->SetTouch( &CBaseGrenade::BounceTouch );

	pGrenade->SetThink( &CBaseCSGrenadeProjectile::DangerSoundThink );
	pGrenade->SetNextThink( gpGlobals->curtime );

	pGrenade->SetDetonateTimerLength( 1.5 );

	pGrenade->ApplyLocalAngularVelocityImpulse( angVelocity );

	pGrenade->SetGravity( BaseClass::GetGrenadeGravity() );
	pGrenade->SetFriction( BaseClass::GetGrenadeFriction() );
	pGrenade->SetElasticity( BaseClass::GetGrenadeElasticity() );

	pGrenade->m_pWeaponInfo = GetWeaponInfo( WEAPON_FLASHBANG );


	return pGrenade;
}

void CFlashbangProjectile::Spawn()
{
	SetModel( GRENADE_MODEL );
	BaseClass::Spawn();
}

void CFlashbangProjectile::Precache()
{
	PrecacheModel( GRENADE_MODEL );

	PrecacheScriptSound( "Flashbang.Explode" );
	PrecacheScriptSound( "Flashbang.Bounce" );

	BaseClass::Precache();
}

ConVar sv_flashbang_strength( "sv_flashbang_strength", "3", FCVAR_REPLICATED, "Flashbang strength", true, 2.0, true, 8.0 );

void CFlashbangProjectile::Detonate()
{
	// tell the bots a flashbang grenade has exploded
	CCSPlayer *player = ToCSPlayer(GetThrower());
	if ( player )
	{
		IGameEvent * event = gameeventmanager->CreateEvent( "flashbang_detonate" );
		if ( event )
		{
			event->SetInt( "userid", player->GetUserID() );
			event->SetFloat( "x", GetAbsOrigin().x );
			event->SetFloat( "y", GetAbsOrigin().y );
			event->SetFloat( "z", GetAbsOrigin().z );
			gameeventmanager->FireEvent( event );
		}
	}

	RadiusFlash( GetAbsOrigin(), this, GetThrower(), sv_flashbang_strength.GetFloat(), CLASS_NONE, DMG_BLAST );
	EmitSound( "Flashbang.Explode" );

	trace_t		tr;
	Vector		vecSpot = GetAbsOrigin() + Vector( 0, 0, 2 );
	UTIL_TraceLine( vecSpot, vecSpot + Vector( 0, 0, -64 ), MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr );
	UTIL_DecalTrace( &tr, "Scorch" );

	UTIL_Remove( this );
}

//TODO: Let physics handle the sound!
void CFlashbangProjectile::BounceSound( void )
{
	EmitSound( "Flashbang.Bounce" );
}
