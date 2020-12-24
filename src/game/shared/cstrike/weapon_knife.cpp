//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_knife.h"
#include "cs_gamerules.h"

#if defined( CLIENT_DLL )
	#include "c_cs_player.h"
#else
	#include "cs_player.h"
	#include "ilagcompensationmanager.h"
	#include "cs_gamestats.h"
#endif


#define	KNIFE_BODYHIT_VOLUME 128
#define	KNIFE_WALLHIT_VOLUME 512

#define KNIFE_RANGE_SHORT 32
#define KNIFE_RANGE_LONG 48

Vector head_hull_mins( -16, -16, -18 );
Vector head_hull_maxs( 16, 16, 18 );

#ifndef CLIENT_DLL
	//-----------------------------------------------------------------------------
	// Purpose: Only send to local player if this weapon is the active weapon
	// Input  : *pStruct - 
	//			*pVarData - 
	//			*pRecipients - 
	//			objectID - 
	// Output : void*
	//-----------------------------------------------------------------------------
	void* SendProxy_SendActiveLocalKnifeDataTable( const SendProp *pProp, const void *pStruct, const void *pVarData, CSendProxyRecipients *pRecipients, int objectID )
	{
		// Get the weapon entity
		CBaseCombatWeapon *pWeapon = (CBaseCombatWeapon*)pVarData;
		if ( pWeapon )
		{
			// Only send this chunk of data to the player carrying this weapon
			CBasePlayer *pPlayer = ToBasePlayer( pWeapon->GetOwner() );
			if ( pPlayer /*&& pPlayer->GetActiveWeapon() == pWeapon*/ )
			{
				pRecipients->SetOnly( pPlayer->GetClientIndex() );
				return (void*)pVarData;
			}
		}
		
		return NULL;
	}
	REGISTER_SEND_PROXY_NON_MODIFIED_POINTER( SendProxy_SendActiveLocalKnifeDataTable );
#endif

// ----------------------------------------------------------------------------- //
// CKnife tables.
// ----------------------------------------------------------------------------- //

IMPLEMENT_NETWORKCLASS_ALIASED( Knife, DT_WeaponKnife )

BEGIN_NETWORK_TABLE_NOBASE( CKnife, DT_LocalActiveWeaponKnifeData )
	#if !defined( CLIENT_DLL )
		SendPropTime( SENDINFO( m_flSmackTime ) ),
	#else
		RecvPropTime( RECVINFO( m_flSmackTime ) ),
	#endif
END_NETWORK_TABLE()


BEGIN_NETWORK_TABLE( CKnife, DT_WeaponKnife )
	#if !defined( CLIENT_DLL )
		SendPropDataTable("LocalActiveWeaponKnifeData", 0, &REFERENCE_SEND_TABLE(DT_LocalActiveWeaponKnifeData), SendProxy_SendActiveLocalKnifeDataTable ),
	#else
		RecvPropDataTable("LocalActiveWeaponKnifeData", 0, 0, &REFERENCE_RECV_TABLE(DT_LocalActiveWeaponKnifeData)),
	#endif
END_NETWORK_TABLE()


#if defined CLIENT_DLL
BEGIN_PREDICTION_DATA( CKnife )
	DEFINE_PRED_FIELD( m_flSmackTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()
#endif


LINK_ENTITY_TO_CLASS( weapon_knife, CKnife );
PRECACHE_WEAPON_REGISTER( weapon_knife );

#ifndef CLIENT_DLL

	BEGIN_DATADESC( CKnife )
		DEFINE_THINKFUNC( Smack )
	END_DATADESC()

#endif

// ----------------------------------------------------------------------------- //
// CKnife implementation.
// ----------------------------------------------------------------------------- //

CKnife::CKnife()
{
#ifndef CLIENT_DLL

	m_swingLeft = true;

#endif
}


bool CKnife::HasPrimaryAmmo()
{
	return true;
}


bool CKnife::CanBeSelected()
{
	return true;
}

void CKnife::Precache()
{
	BaseClass::Precache();

	PrecacheScriptSound( "Weapon_Knife.Deploy" );
	PrecacheScriptSound( "Weapon_Knife.Slash" );
	PrecacheScriptSound( "Weapon_Knife.Stab" );
	PrecacheScriptSound( "Weapon_Knife.Hit" );
}

void CKnife::Spawn()
{
	Precache();

	m_iClip1 = -1;
	BaseClass::Spawn();
}


bool CKnife::Deploy()
{
	CPASAttenuationFilter filter( this );
	filter.UsePredictionRules();
	EmitSound( filter, entindex(), "Weapon_Knife.Deploy" );

	if ( !BaseClass::Deploy() )
		return false;

	if ( CBasePlayer *pOwner = ToBasePlayer( GetOwner() ) )
		pOwner->SetNextAttack( gpGlobals->curtime + 1.0f );

	return true;
}

void CKnife::Holster( int skiplocal )
{
	if ( GetPlayerOwner() )
	{
		GetPlayerOwner()->m_flNextAttack = gpGlobals->curtime + 0.5;
	}
}

void FindHullIntersection( const Vector &vecSrc, trace_t &tr, const Vector &mins, const Vector &maxs, CBaseEntity *pEntity )
{
	int			i, j, k;
	float		distance;
	Vector minmaxs[2] = {mins, maxs};
	trace_t tmpTrace;
	Vector		vecHullEnd = tr.endpos;
	Vector		vecEnd;

	distance = 1e6f;

	vecHullEnd = vecSrc + ((vecHullEnd - vecSrc)*2);
	UTIL_TraceLine( vecSrc, vecHullEnd, MASK_SOLID, pEntity, COLLISION_GROUP_NONE, &tmpTrace );
	if ( tmpTrace.fraction < 1.0 )
	{
		tr = tmpTrace;
		return;
	}

	for ( i = 0; i < 2; i++ )
	{
		for ( j = 0; j < 2; j++ )
		{
			for ( k = 0; k < 2; k++ )
			{
				vecEnd.x = vecHullEnd.x + minmaxs[i][0];
				vecEnd.y = vecHullEnd.y + minmaxs[j][1];
				vecEnd.z = vecHullEnd.z + minmaxs[k][2];

				UTIL_TraceLine( vecSrc, vecEnd, MASK_SOLID, pEntity, COLLISION_GROUP_NONE, &tmpTrace );
				if ( tmpTrace.fraction < 1.0 )
				{
					float thisDistance = (tmpTrace.endpos - vecSrc).Length();
					if ( thisDistance < distance )
					{
						tr = tmpTrace;
						distance = thisDistance;
					}
				}
			}
		}
	}
}


void CKnife::PrimaryAttack()
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( pPlayer )
	{
#if !defined (CLIENT_DLL)
		// Move other players back to history positions based on local player's lag
		lagcompensation->StartLagCompensation( pPlayer, pPlayer->GetCurrentCommand() );
#endif
		SwingOrStab( Primary_Mode );
#if !defined (CLIENT_DLL)
		lagcompensation->FinishLagCompensation( pPlayer );
#endif
	}
}

void CKnife::SecondaryAttack()
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( pPlayer && !pPlayer->m_bIsDefusing && !CSGameRules()->IsFreezePeriod() )
	{
#if !defined (CLIENT_DLL)
		// Move other players back to history positions based on local player's lag
		lagcompensation->StartLagCompensation( pPlayer, pPlayer->GetCurrentCommand() );
#endif
		SwingOrStab( Secondary_Mode );
#if !defined (CLIENT_DLL)
		lagcompensation->FinishLagCompensation( pPlayer );
#endif
	}
}

#include "effect_dispatch_data.h"

void CKnife::Smack( void )
{
	if ( !GetPlayerOwner() )
		return;

	m_trHit.m_pEnt = m_pTraceHitEnt;

	if ( !m_trHit.m_pEnt || (m_trHit.surface.flags & SURF_SKY) )
		return;

	if ( m_trHit.fraction == 1.0 )
		return;

	if ( m_trHit.m_pEnt )
	{
		CPASAttenuationFilter filter( this );
		filter.UsePredictionRules();

		if( m_trHit.m_pEnt->IsPlayer()  )
		{
			EmitSound( filter, entindex(), m_bStab?"Weapon_Knife.Stab":"Weapon_Knife.Hit" );
		}
		else
		{
			EmitSound( filter, entindex(), "Weapon_Knife.HitWall" );
		}
	}

	CEffectData data;
	data.m_vOrigin = m_trHit.endpos;
	data.m_vStart = m_trHit.startpos;
	data.m_nSurfaceProp = m_trHit.surface.surfaceProps;
	data.m_nDamageType = DMG_SLASH;
	data.m_nHitBox = m_trHit.hitbox;
#ifdef CLIENT_DLL
	data.m_hEntity = m_trHit.m_pEnt->GetRefEHandle();
#else
	data.m_nEntIndex = m_trHit.m_pEnt->entindex();
#endif

	CPASFilter filter( data.m_vOrigin );
	
#ifndef CLIENT_DLL
	filter.RemoveRecipient( GetPlayerOwner() );
#endif

	data.m_vAngles = GetPlayerOwner()->GetAbsAngles();
	data.m_fFlags = 0x1;	//IMPACT_NODECAL;
	te->DispatchEffect( filter, 0.0, data.m_vOrigin, "KnifeSlash", data );
}

void CKnife::WeaponIdle()
{
	if (m_flTimeWeaponIdle > gpGlobals->curtime)
		return;

	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	if ( pPlayer->IsShieldDrawn() )
		 return;

	SetWeaponIdleTime( gpGlobals->curtime + 20 );

	// only idle if the slid isn't back
	SendWeaponAnim( ACT_VM_IDLE );
}

//=============================================================================
// HPE_BEGIN:
// [tj] Hacky cheat code to control knife damage
//=============================================================================
#ifndef CLIENT_DLL
	ConVar KnifeDamageScale("knife_damage_scale", "100", FCVAR_DEVELOPMENTONLY);
#endif

//=============================================================================
// HPE_END
//=============================================================================


bool CKnife::SwingOrStab( CSWeaponMode weaponMode )
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return false;

	// bStab: false=primary, true=secondary
	float fRange = (weaponMode == Primary_Mode) ? KNIFE_RANGE_LONG : KNIFE_RANGE_SHORT; // knife range

	Vector vForward; AngleVectors( pPlayer->EyeAngles(), &vForward );
	Vector vecSrc = pPlayer->Weapon_ShootPosition();
	Vector vecEnd = vecSrc + vForward * fRange;

	trace_t tr;
	UTIL_TraceLine( vecSrc, vecEnd, MASK_SOLID, pPlayer, COLLISION_GROUP_NONE, &tr );

	//check for hitting glass - TODO - fix this hackiness, doesn't always line up with what FindHullIntersection returns
#ifndef CLIENT_DLL
	CTakeDamageInfo glassDamage( pPlayer, pPlayer, 42.0f, DMG_BULLET | DMG_NEVERGIB );
	TraceAttackToTriggers( glassDamage, tr.startpos, tr.endpos, vForward );
#endif

	if ( tr.fraction >= 1.0 )
	{
		UTIL_TraceHull( vecSrc, vecEnd, head_hull_mins, head_hull_maxs, MASK_SOLID, pPlayer, COLLISION_GROUP_NONE, &tr );
		if ( tr.fraction < 1.0 )
		{
			// Calculate the point of intersection of the line (or hull) and the object we hit
			// This is and approximation of the "best" intersection
			CBaseEntity *pHit = tr.m_pEnt;
			if ( !pHit || pHit->IsBSPModel() )
				FindHullIntersection( vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, pPlayer );
			vecEnd = tr.endpos;	// This is the point on the actual surface (the hull could have hit space)
		}
	}

	bool bDidHit = tr.fraction < 1.0f;

#ifndef CLIENT_DLL

	bool bFirstSwing = (m_flNextPrimaryAttack + 0.4) < gpGlobals->curtime;
	if ( bFirstSwing )
	{
		m_swingLeft = true;
	}

#endif

	float fPrimDelay, fSecDelay;

	if ( weaponMode == Secondary_Mode )
	{
		fPrimDelay = fSecDelay = bDidHit ? 1.1f : 1.0f;
	}
	else // swing
	{
		fPrimDelay = bDidHit ? 0.5f : 0.4f;
		fSecDelay = bDidHit ? 0.5f : 0.5f;
	}

	if ( pPlayer->HasShield() )
	{
		fPrimDelay += 0.7f; // 0.7 seconds slower if we carry a shield
		fSecDelay += 0.7f;
	}

	m_flNextPrimaryAttack = gpGlobals->curtime + fPrimDelay;
	m_flNextSecondaryAttack = gpGlobals->curtime + fSecDelay;
	SetWeaponIdleTime( gpGlobals->curtime + 2 );

	bool bBackStab = false;

	if ( bDidHit )
	{
		// server side damage calculations
		CBaseEntity *pEntity = tr.m_pEnt;

#ifndef CLIENT_DLL
		// player "shoot" animation
		pPlayer->SetAnimation( PLAYER_ATTACK1 );

		ClearMultiDamage();

		float flDamage = 0.f;	// set below
#endif
		if ( pEntity && pEntity->IsPlayer() )
		{
			Vector vTragetForward;

			AngleVectors( pEntity->GetAbsAngles(), &vTragetForward );

			Vector2D vecLOS = (pEntity->GetAbsOrigin() - pPlayer->GetAbsOrigin()).AsVector2D();
			Vector2DNormalize( vecLOS );

			float flDot = vecLOS.Dot( vTragetForward.AsVector2D() );

			//Triple the damage if we are stabbing them in the back.
			if ( flDot > 0.475f )
				bBackStab = true;
		}

#ifndef CLIENT_DLL
		if ( weaponMode == Secondary_Mode )
		{
			if ( bBackStab )
			{
				flDamage = 180.0f;
			}
			else
			{
				flDamage = 65.0f;
			}
		}
		else
		{
			if ( bBackStab )
			{
				flDamage = 90;
			}
			else if ( bFirstSwing )
			{
				// first swing does full damage
				flDamage = 40;
			}
			else
			{
				// subsequent swings do less	
				flDamage = 25;
			}
		}

		// [tj] Hacky cheat to lower knife damage for testing
		flDamage *= (KnifeDamageScale.GetInt() / 100.0f);
#endif

		if ( weaponMode == Secondary_Mode )
		{
			SendWeaponAnim( bBackStab ? ACT_VM_SWINGHARD : ACT_VM_HITCENTER2 );
		}
		else // swing
		{
			SendWeaponAnim( bBackStab ? ACT_VM_SWINGHIT : ACT_VM_HITCENTER );
		}

#ifndef CLIENT_DLL
		CCSPlayer::StartNewBulletGroup();
		CTakeDamageInfo info( pPlayer, pPlayer, this, flDamage, DMG_SLASH | DMG_NEVERGIB );

		CalculateMeleeDamageForce( &info, vForward, tr.endpos, 1.0f / flDamage );
		pEntity->DispatchTraceAttack( info, vForward, &tr );
		ApplyMultiDamage();
#endif

		if ( tr.m_pEnt )
		{
			CPASAttenuationFilter filter( this );
			filter.UsePredictionRules();

			if ( tr.m_pEnt->IsPlayer() )
			{
				EmitSound( filter, entindex(), (weaponMode == Secondary_Mode) ? "Weapon_Knife.Stab" : "Weapon_Knife.Hit" );
			}
			else
			{
				EmitSound( filter, entindex(), "Weapon_Knife.HitWall" );
			}
		}

		CEffectData data;
		data.m_vOrigin = tr.endpos;
		data.m_vStart = tr.startpos;
		data.m_nSurfaceProp = tr.surface.surfaceProps;
		data.m_nDamageType = DMG_SLASH;
		data.m_nHitBox = tr.hitbox;
#ifdef CLIENT_DLL
		data.m_hEntity = tr.m_pEnt->GetRefEHandle();
#else
		data.m_nEntIndex = tr.m_pEnt->entindex();
#endif

		CPASFilter filter( data.m_vOrigin );

#ifndef CLIENT_DLL
		filter.RemoveRecipient( pPlayer );
#endif

		data.m_vAngles = pPlayer->GetAbsAngles();
		data.m_fFlags = 0x1;	//IMPACT_NODECAL;
		te->DispatchEffect( filter, 0.0, data.m_vOrigin, "KnifeSlash", data );
	}
	else
	{
		// play wiff or swish sound
		CPASAttenuationFilter filter( this );
		filter.UsePredictionRules();
		EmitSound( filter, entindex(), "Weapon_Knife.Slash" );

		if ( weaponMode == Secondary_Mode )
		{
			SendWeaponAnim( ACT_VM_MISSCENTER2 );
		}
		else // swing
		{
			SendWeaponAnim( ACT_VM_MISSCENTER );
		}
	}

#ifndef CLIENT_DLL

	if ( weaponMode == Secondary_Mode )
	{
		pPlayer->DoAnimationEvent( bBackStab ? PLAYERANIMEVENT_FIRE_GUN_SECONDARY_SPECIAL1 : PLAYERANIMEVENT_FIRE_GUN_SECONDARY );
		m_swingLeft = true;
	}
	else // swing
	{
		// See if we are back stabbing and if we're swinging left (opt) or right.
		pPlayer->DoAnimationEvent( bBackStab ? (m_swingLeft ? PLAYERANIMEVENT_FIRE_GUN_PRIMARY_OPT_SPECIAL1 : PLAYERANIMEVENT_FIRE_GUN_PRIMARY_SPECIAL1) : (m_swingLeft ? PLAYERANIMEVENT_FIRE_GUN_PRIMARY_OPT : PLAYERANIMEVENT_FIRE_GUN_PRIMARY) );
		m_swingLeft = !m_swingLeft;
	}
#endif

	return bDidHit;
}

void CKnife::ItemPostFrame( void )
{
	if( m_flSmackTime > 0 && gpGlobals->curtime > m_flSmackTime )
	{
		Smack();
		m_flSmackTime = -1;
	}

	BaseClass::ItemPostFrame();
}

bool CKnife::CanDrop()
{
	return false;
}


// ----------------------------------------------------------------------------- //
// CKnifeT implementation.
// ----------------------------------------------------------------------------- //
IMPLEMENT_NETWORKCLASS_ALIASED( KnifeT, DT_WeaponKnifeT )

BEGIN_NETWORK_TABLE_NOBASE( CKnifeT, DT_LocalActiveWeaponKnifeTData )
	#if !defined( CLIENT_DLL )
		SendPropTime( SENDINFO( m_flSmackTime ) ),
	#else
		RecvPropTime( RECVINFO( m_flSmackTime ) ),
	#endif
END_NETWORK_TABLE()


BEGIN_NETWORK_TABLE( CKnifeT, DT_WeaponKnifeT )
	#if !defined( CLIENT_DLL )
		SendPropDataTable("LocalActiveWeaponKnifeData", 0, &REFERENCE_SEND_TABLE(DT_LocalActiveWeaponKnifeTData), SendProxy_SendActiveLocalKnifeDataTable ),
	#else
		RecvPropDataTable("LocalActiveWeaponKnifeData", 0, 0, &REFERENCE_RECV_TABLE(DT_LocalActiveWeaponKnifeTData)),
	#endif
END_NETWORK_TABLE()


#if defined CLIENT_DLL
BEGIN_PREDICTION_DATA( CKnifeT )
	DEFINE_PRED_FIELD( m_flSmackTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()
#endif


LINK_ENTITY_TO_CLASS( weapon_knife_t, CKnifeT );
PRECACHE_WEAPON_REGISTER( weapon_knife_t );

#ifndef CLIENT_DLL

	BEGIN_DATADESC( CKnifeT )
		DEFINE_THINKFUNC( Smack )
	END_DATADESC()

#endif


// ----------------------------------------------------------------------------- //
// CKnifeCSS implementation.
// ----------------------------------------------------------------------------- //
IMPLEMENT_NETWORKCLASS_ALIASED( KnifeCSS, DT_WeaponKnifeCSS )

BEGIN_NETWORK_TABLE_NOBASE( CKnifeCSS, DT_LocalActiveWeaponKnifeCSSData )
	#if !defined( CLIENT_DLL )
		SendPropTime( SENDINFO( m_flSmackTime ) ),
	#else
		RecvPropTime( RECVINFO( m_flSmackTime ) ),
	#endif
END_NETWORK_TABLE()


BEGIN_NETWORK_TABLE( CKnifeCSS, DT_WeaponKnifeCSS )
	#if !defined( CLIENT_DLL )
		SendPropDataTable("LocalActiveWeaponKnifeData", 0, &REFERENCE_SEND_TABLE(DT_LocalActiveWeaponKnifeCSSData), SendProxy_SendActiveLocalKnifeDataTable ),
	#else
		RecvPropDataTable("LocalActiveWeaponKnifeData", 0, 0, &REFERENCE_RECV_TABLE(DT_LocalActiveWeaponKnifeCSSData)),
	#endif
END_NETWORK_TABLE()


#if defined CLIENT_DLL
BEGIN_PREDICTION_DATA( CKnifeCSS )
	DEFINE_PRED_FIELD( m_flSmackTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()
#endif


LINK_ENTITY_TO_CLASS( weapon_knife_css, CKnifeCSS );
PRECACHE_WEAPON_REGISTER( weapon_knife_css );

#ifndef CLIENT_DLL

	BEGIN_DATADESC( CKnifeCSS )
		DEFINE_THINKFUNC( Smack )
	END_DATADESC()

#endif


// ----------------------------------------------------------------------------- //
// CKnifeKarambit implementation.
// ----------------------------------------------------------------------------- //
IMPLEMENT_NETWORKCLASS_ALIASED( KnifeKarambit, DT_WeaponKnifeKarambit )

BEGIN_NETWORK_TABLE_NOBASE( CKnifeKarambit, DT_LocalActiveWeaponKnifeKarambitData )
	#if !defined( CLIENT_DLL )
		SendPropTime( SENDINFO( m_flSmackTime ) ),
	#else
		RecvPropTime( RECVINFO( m_flSmackTime ) ),
	#endif
END_NETWORK_TABLE()


BEGIN_NETWORK_TABLE( CKnifeKarambit, DT_WeaponKnifeKarambit )
	#if !defined( CLIENT_DLL )
		SendPropDataTable("LocalActiveWeaponKnifeData", 0, &REFERENCE_SEND_TABLE(DT_LocalActiveWeaponKnifeKarambitData), SendProxy_SendActiveLocalKnifeDataTable ),
	#else
		RecvPropDataTable("LocalActiveWeaponKnifeData", 0, 0, &REFERENCE_RECV_TABLE(DT_LocalActiveWeaponKnifeKarambitData)),
	#endif
END_NETWORK_TABLE()


#if defined CLIENT_DLL
BEGIN_PREDICTION_DATA( CKnifeKarambit )
	DEFINE_PRED_FIELD( m_flSmackTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()
#endif


LINK_ENTITY_TO_CLASS( weapon_knife_karambit, CKnifeKarambit );
PRECACHE_WEAPON_REGISTER( weapon_knife_karambit );

#ifndef CLIENT_DLL

	BEGIN_DATADESC( CKnifeKarambit )
		DEFINE_THINKFUNC( Smack )
	END_DATADESC()

#endif


// ----------------------------------------------------------------------------- //
// CKnifeFlip implementation.
// ----------------------------------------------------------------------------- //
IMPLEMENT_NETWORKCLASS_ALIASED( KnifeFlip, DT_WeaponKnifeFlip )

BEGIN_NETWORK_TABLE_NOBASE( CKnifeFlip, DT_LocalActiveWeaponKnifeFlipData )
	#if !defined( CLIENT_DLL )
		SendPropTime( SENDINFO( m_flSmackTime ) ),
	#else
		RecvPropTime( RECVINFO( m_flSmackTime ) ),
	#endif
END_NETWORK_TABLE()


BEGIN_NETWORK_TABLE( CKnifeFlip, DT_WeaponKnifeFlip )
	#if !defined( CLIENT_DLL )
		SendPropDataTable("LocalActiveWeaponKnifeData", 0, &REFERENCE_SEND_TABLE(DT_LocalActiveWeaponKnifeFlipData), SendProxy_SendActiveLocalKnifeDataTable ),
	#else
		RecvPropDataTable("LocalActiveWeaponKnifeData", 0, 0, &REFERENCE_RECV_TABLE(DT_LocalActiveWeaponKnifeFlipData)),
	#endif
END_NETWORK_TABLE()


#if defined CLIENT_DLL
BEGIN_PREDICTION_DATA( CKnifeFlip )
	DEFINE_PRED_FIELD( m_flSmackTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()
#endif


LINK_ENTITY_TO_CLASS( weapon_knife_flip, CKnifeFlip );
PRECACHE_WEAPON_REGISTER( weapon_knife_flip );

#ifndef CLIENT_DLL

	BEGIN_DATADESC( CKnifeFlip )
		DEFINE_THINKFUNC( Smack )
	END_DATADESC()

#endif


// ----------------------------------------------------------------------------- //
// CKnifeBayonet implementation.
// ----------------------------------------------------------------------------- //
IMPLEMENT_NETWORKCLASS_ALIASED( KnifeBayonet, DT_WeaponKnifeBayonet )

BEGIN_NETWORK_TABLE_NOBASE( CKnifeBayonet, DT_LocalActiveWeaponKnifeBayonetData )
	#if !defined( CLIENT_DLL )
		SendPropTime( SENDINFO( m_flSmackTime ) ),
	#else
		RecvPropTime( RECVINFO( m_flSmackTime ) ),
	#endif
END_NETWORK_TABLE()


BEGIN_NETWORK_TABLE( CKnifeBayonet, DT_WeaponKnifeBayonet )
	#if !defined( CLIENT_DLL )
		SendPropDataTable("LocalActiveWeaponKnifeData", 0, &REFERENCE_SEND_TABLE(DT_LocalActiveWeaponKnifeBayonetData), SendProxy_SendActiveLocalKnifeDataTable ),
	#else
		RecvPropDataTable("LocalActiveWeaponKnifeData", 0, 0, &REFERENCE_RECV_TABLE(DT_LocalActiveWeaponKnifeBayonetData)),
	#endif
END_NETWORK_TABLE()


#if defined CLIENT_DLL
BEGIN_PREDICTION_DATA( CKnifeBayonet )
	DEFINE_PRED_FIELD( m_flSmackTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()
#endif


LINK_ENTITY_TO_CLASS( weapon_knife_bayonet, CKnifeBayonet );
PRECACHE_WEAPON_REGISTER( weapon_knife_bayonet );

#ifndef CLIENT_DLL

	BEGIN_DATADESC( CKnifeBayonet )
		DEFINE_THINKFUNC( Smack )
	END_DATADESC()

#endif


// ----------------------------------------------------------------------------- //
// CKnifeM9Bayonet implementation.
// ----------------------------------------------------------------------------- //
IMPLEMENT_NETWORKCLASS_ALIASED( KnifeM9Bayonet, DT_WeaponKnifeM9Bayonet )

BEGIN_NETWORK_TABLE_NOBASE( CKnifeM9Bayonet, DT_LocalActiveWeaponKnifeM9BayonetData )
	#if !defined( CLIENT_DLL )
		SendPropTime( SENDINFO( m_flSmackTime ) ),
	#else
		RecvPropTime( RECVINFO( m_flSmackTime ) ),
	#endif
END_NETWORK_TABLE()


BEGIN_NETWORK_TABLE( CKnifeM9Bayonet, DT_WeaponKnifeM9Bayonet )
	#if !defined( CLIENT_DLL )
		SendPropDataTable("LocalActiveWeaponKnifeData", 0, &REFERENCE_SEND_TABLE(DT_LocalActiveWeaponKnifeM9BayonetData), SendProxy_SendActiveLocalKnifeDataTable ),
	#else
		RecvPropDataTable("LocalActiveWeaponKnifeData", 0, 0, &REFERENCE_RECV_TABLE(DT_LocalActiveWeaponKnifeM9BayonetData)),
	#endif
END_NETWORK_TABLE()


#if defined CLIENT_DLL
BEGIN_PREDICTION_DATA( CKnifeM9Bayonet )
	DEFINE_PRED_FIELD( m_flSmackTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()
#endif


LINK_ENTITY_TO_CLASS( weapon_knife_m9_bayonet, CKnifeM9Bayonet );
PRECACHE_WEAPON_REGISTER( weapon_knife_m9_bayonet );

#ifndef CLIENT_DLL

	BEGIN_DATADESC( CKnifeM9Bayonet )
		DEFINE_THINKFUNC( Smack )
	END_DATADESC()

#endif


// ----------------------------------------------------------------------------- //
// CKnifeButterfly implementation.
// ----------------------------------------------------------------------------- //
IMPLEMENT_NETWORKCLASS_ALIASED( KnifeButterfly, DT_WeaponKnifeButterfly )

BEGIN_NETWORK_TABLE_NOBASE( CKnifeButterfly, DT_LocalActiveWeaponKnifeButterflyData )
	#if !defined( CLIENT_DLL )
		SendPropTime( SENDINFO( m_flSmackTime ) ),
	#else
		RecvPropTime( RECVINFO( m_flSmackTime ) ),
	#endif
END_NETWORK_TABLE()


BEGIN_NETWORK_TABLE( CKnifeButterfly, DT_WeaponKnifeButterfly )
	#if !defined( CLIENT_DLL )
		SendPropDataTable("LocalActiveWeaponKnifeData", 0, &REFERENCE_SEND_TABLE(DT_LocalActiveWeaponKnifeButterflyData), SendProxy_SendActiveLocalKnifeDataTable ),
	#else
		RecvPropDataTable("LocalActiveWeaponKnifeData", 0, 0, &REFERENCE_RECV_TABLE(DT_LocalActiveWeaponKnifeButterflyData)),
	#endif
END_NETWORK_TABLE()


#if defined CLIENT_DLL
BEGIN_PREDICTION_DATA( CKnifeButterfly )
	DEFINE_PRED_FIELD( m_flSmackTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()
#endif


LINK_ENTITY_TO_CLASS( weapon_knife_butterfly, CKnifeButterfly );
PRECACHE_WEAPON_REGISTER( weapon_knife_butterfly );

#ifndef CLIENT_DLL

	BEGIN_DATADESC( CKnifeButterfly )
		DEFINE_THINKFUNC( Smack )
	END_DATADESC()

#endif


// ----------------------------------------------------------------------------- //
// CKnifeGut implementation.
// ----------------------------------------------------------------------------- //
IMPLEMENT_NETWORKCLASS_ALIASED( KnifeGut, DT_WeaponKnifeGut )

BEGIN_NETWORK_TABLE_NOBASE( CKnifeGut, DT_LocalActiveWeaponKnifeGutData )
	#if !defined( CLIENT_DLL )
		SendPropTime( SENDINFO( m_flSmackTime ) ),
	#else
		RecvPropTime( RECVINFO( m_flSmackTime ) ),
	#endif
END_NETWORK_TABLE()


BEGIN_NETWORK_TABLE( CKnifeGut, DT_WeaponKnifeGut )
	#if !defined( CLIENT_DLL )
		SendPropDataTable("LocalActiveWeaponKnifeData", 0, &REFERENCE_SEND_TABLE(DT_LocalActiveWeaponKnifeGutData), SendProxy_SendActiveLocalKnifeDataTable ),
	#else
		RecvPropDataTable("LocalActiveWeaponKnifeData", 0, 0, &REFERENCE_RECV_TABLE(DT_LocalActiveWeaponKnifeGutData)),
	#endif
END_NETWORK_TABLE()


#if defined CLIENT_DLL
BEGIN_PREDICTION_DATA( CKnifeGut )
	DEFINE_PRED_FIELD( m_flSmackTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()
#endif


LINK_ENTITY_TO_CLASS( weapon_knife_gut, CKnifeGut );
PRECACHE_WEAPON_REGISTER( weapon_knife_gut );

#ifndef CLIENT_DLL

	BEGIN_DATADESC( CKnifeGut )
		DEFINE_THINKFUNC( Smack )
	END_DATADESC()

#endif


// ----------------------------------------------------------------------------- //
// CKnifeTactical implementation.
// ----------------------------------------------------------------------------- //
IMPLEMENT_NETWORKCLASS_ALIASED( KnifeTactical, DT_WeaponKnifeTactical )

BEGIN_NETWORK_TABLE_NOBASE( CKnifeTactical, DT_LocalActiveWeaponKnifeTacticalData )
	#if !defined( CLIENT_DLL )
		SendPropTime( SENDINFO( m_flSmackTime ) ),
	#else
		RecvPropTime( RECVINFO( m_flSmackTime ) ),
	#endif
END_NETWORK_TABLE()


BEGIN_NETWORK_TABLE( CKnifeTactical, DT_WeaponKnifeTactical )
	#if !defined( CLIENT_DLL )
		SendPropDataTable("LocalActiveWeaponKnifeData", 0, &REFERENCE_SEND_TABLE(DT_LocalActiveWeaponKnifeTacticalData), SendProxy_SendActiveLocalKnifeDataTable ),
	#else
		RecvPropDataTable("LocalActiveWeaponKnifeData", 0, 0, &REFERENCE_RECV_TABLE(DT_LocalActiveWeaponKnifeTacticalData)),
	#endif
END_NETWORK_TABLE()


#if defined CLIENT_DLL
BEGIN_PREDICTION_DATA( CKnifeTactical )
	DEFINE_PRED_FIELD( m_flSmackTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()
#endif


LINK_ENTITY_TO_CLASS( weapon_knife_tactical, CKnifeTactical );
PRECACHE_WEAPON_REGISTER( weapon_knife_tactical );

#ifndef CLIENT_DLL
	BEGIN_DATADESC( CKnifeTactical )
		DEFINE_THINKFUNC( Smack )
	END_DATADESC()
#endif


// ----------------------------------------------------------------------------- //
// CKnifeFalchion implementation.
// ----------------------------------------------------------------------------- //
IMPLEMENT_NETWORKCLASS_ALIASED( KnifeFalchion, DT_WeaponKnifeFalchion )

BEGIN_NETWORK_TABLE_NOBASE( CKnifeFalchion, DT_LocalActiveWeaponKnifeFalchionData )
	#if !defined( CLIENT_DLL )
		SendPropTime( SENDINFO( m_flSmackTime ) ),
	#else
		RecvPropTime( RECVINFO( m_flSmackTime ) ),
	#endif
END_NETWORK_TABLE()


BEGIN_NETWORK_TABLE( CKnifeFalchion, DT_WeaponKnifeFalchion )
	#if !defined( CLIENT_DLL )
		SendPropDataTable("LocalActiveWeaponKnifeData", 0, &REFERENCE_SEND_TABLE(DT_LocalActiveWeaponKnifeFalchionData), SendProxy_SendActiveLocalKnifeDataTable ),
	#else
		RecvPropDataTable("LocalActiveWeaponKnifeData", 0, 0, &REFERENCE_RECV_TABLE(DT_LocalActiveWeaponKnifeFalchionData)),
	#endif
END_NETWORK_TABLE()


#if defined CLIENT_DLL
BEGIN_PREDICTION_DATA( CKnifeFalchion )
	DEFINE_PRED_FIELD( m_flSmackTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()
#endif


LINK_ENTITY_TO_CLASS( weapon_knife_falchion, CKnifeFalchion );
PRECACHE_WEAPON_REGISTER( weapon_knife_falchion );

#ifndef CLIENT_DLL
	BEGIN_DATADESC( CKnifeFalchion )
		DEFINE_THINKFUNC( Smack )
	END_DATADESC()
#endif


// ----------------------------------------------------------------------------- //
// CKnifeSurvivalBowie implementation.
// ----------------------------------------------------------------------------- //
IMPLEMENT_NETWORKCLASS_ALIASED( KnifeSurvivalBowie, DT_WeaponKnifeSurvivalBowie )

BEGIN_NETWORK_TABLE_NOBASE( CKnifeSurvivalBowie, DT_LocalActiveWeaponKnifeSurvivalBowieData )
	#if !defined( CLIENT_DLL )
		SendPropTime( SENDINFO( m_flSmackTime ) ),
	#else
		RecvPropTime( RECVINFO( m_flSmackTime ) ),
	#endif
END_NETWORK_TABLE()


BEGIN_NETWORK_TABLE( CKnifeSurvivalBowie, DT_WeaponKnifeSurvivalBowie )
	#if !defined( CLIENT_DLL )
		SendPropDataTable("LocalActiveWeaponKnifeData", 0, &REFERENCE_SEND_TABLE(DT_LocalActiveWeaponKnifeSurvivalBowieData), SendProxy_SendActiveLocalKnifeDataTable ),
	#else
		RecvPropDataTable("LocalActiveWeaponKnifeData", 0, 0, &REFERENCE_RECV_TABLE(DT_LocalActiveWeaponKnifeSurvivalBowieData)),
	#endif
END_NETWORK_TABLE()


#if defined CLIENT_DLL
BEGIN_PREDICTION_DATA( CKnifeSurvivalBowie )
	DEFINE_PRED_FIELD( m_flSmackTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()
#endif


LINK_ENTITY_TO_CLASS( weapon_knife_survival_bowie, CKnifeSurvivalBowie );
PRECACHE_WEAPON_REGISTER( weapon_knife_survival_bowie );

#ifndef CLIENT_DLL
	BEGIN_DATADESC( CKnifeSurvivalBowie )
		DEFINE_THINKFUNC( Smack )
	END_DATADESC()
#endif


// ----------------------------------------------------------------------------- //
// CKnifeCanis implementation.
// ----------------------------------------------------------------------------- //
IMPLEMENT_NETWORKCLASS_ALIASED( KnifeCanis, DT_WeaponKnifeCanis )

BEGIN_NETWORK_TABLE_NOBASE( CKnifeCanis, DT_LocalActiveWeaponKnifeCanisData )
	#if !defined( CLIENT_DLL )
		SendPropTime( SENDINFO( m_flSmackTime ) ),
	#else
		RecvPropTime( RECVINFO( m_flSmackTime ) ),
	#endif
END_NETWORK_TABLE()


BEGIN_NETWORK_TABLE( CKnifeCanis, DT_WeaponKnifeCanis )
	#if !defined( CLIENT_DLL )
		SendPropDataTable("LocalActiveWeaponKnifeData", 0, &REFERENCE_SEND_TABLE(DT_LocalActiveWeaponKnifeCanisData), SendProxy_SendActiveLocalKnifeDataTable ),
	#else
		RecvPropDataTable("LocalActiveWeaponKnifeData", 0, 0, &REFERENCE_RECV_TABLE(DT_LocalActiveWeaponKnifeCanisData)),
	#endif
END_NETWORK_TABLE()


#if defined CLIENT_DLL
BEGIN_PREDICTION_DATA( CKnifeCanis )
	DEFINE_PRED_FIELD( m_flSmackTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()
#endif


LINK_ENTITY_TO_CLASS( weapon_knife_canis, CKnifeCanis );
PRECACHE_WEAPON_REGISTER( weapon_knife_canis );

#ifndef CLIENT_DLL
BEGIN_DATADESC( CKnifeCanis )
		DEFINE_THINKFUNC( Smack )
	END_DATADESC()
#endif


// ----------------------------------------------------------------------------- //
// CKnifeCord implementation.
// ----------------------------------------------------------------------------- //
IMPLEMENT_NETWORKCLASS_ALIASED( KnifeCord, DT_WeaponKnifeCord )

BEGIN_NETWORK_TABLE_NOBASE( CKnifeCord, DT_LocalActiveWeaponKnifeCordData )
	#if !defined( CLIENT_DLL )
		SendPropTime( SENDINFO( m_flSmackTime ) ),
	#else
		RecvPropTime( RECVINFO( m_flSmackTime ) ),
	#endif
END_NETWORK_TABLE()


BEGIN_NETWORK_TABLE( CKnifeCord, DT_WeaponKnifeCord )
	#if !defined( CLIENT_DLL )
		SendPropDataTable("LocalActiveWeaponKnifeData", 0, &REFERENCE_SEND_TABLE(DT_LocalActiveWeaponKnifeCordData), SendProxy_SendActiveLocalKnifeDataTable ),
	#else
		RecvPropDataTable("LocalActiveWeaponKnifeData", 0, 0, &REFERENCE_RECV_TABLE(DT_LocalActiveWeaponKnifeCordData)),
	#endif
END_NETWORK_TABLE()


#if defined CLIENT_DLL
BEGIN_PREDICTION_DATA( CKnifeCord )
	DEFINE_PRED_FIELD( m_flSmackTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()
#endif


LINK_ENTITY_TO_CLASS( weapon_knife_cord, CKnifeCord );
PRECACHE_WEAPON_REGISTER( weapon_knife_cord );

#ifndef CLIENT_DLL
BEGIN_DATADESC( CKnifeCord )
		DEFINE_THINKFUNC( Smack )
	END_DATADESC()
#endif


// ----------------------------------------------------------------------------- //
// CKnifeGypsy implementation.
// ----------------------------------------------------------------------------- //
IMPLEMENT_NETWORKCLASS_ALIASED( KnifeGypsy, DT_WeaponKnifeGypsy )

BEGIN_NETWORK_TABLE_NOBASE( CKnifeGypsy, DT_LocalActiveWeaponKnifeGypsyData )
	#if !defined( CLIENT_DLL )
		SendPropTime( SENDINFO( m_flSmackTime ) ),
	#else
		RecvPropTime( RECVINFO( m_flSmackTime ) ),
	#endif
END_NETWORK_TABLE()


BEGIN_NETWORK_TABLE( CKnifeGypsy, DT_WeaponKnifeGypsy )
	#if !defined( CLIENT_DLL )
		SendPropDataTable("LocalActiveWeaponKnifeData", 0, &REFERENCE_SEND_TABLE(DT_LocalActiveWeaponKnifeGypsyData), SendProxy_SendActiveLocalKnifeDataTable ),
	#else
		RecvPropDataTable("LocalActiveWeaponKnifeData", 0, 0, &REFERENCE_RECV_TABLE(DT_LocalActiveWeaponKnifeGypsyData)),
	#endif
END_NETWORK_TABLE()


#if defined CLIENT_DLL
BEGIN_PREDICTION_DATA( CKnifeGypsy )
	DEFINE_PRED_FIELD( m_flSmackTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()
#endif


LINK_ENTITY_TO_CLASS( weapon_knife_gypsy_jackknife, CKnifeGypsy );
PRECACHE_WEAPON_REGISTER( weapon_knife_gypsy_jackknife );

#ifndef CLIENT_DLL
BEGIN_DATADESC( CKnifeGypsy )
		DEFINE_THINKFUNC( Smack )
	END_DATADESC()
#endif


// ----------------------------------------------------------------------------- //
// CKnifeOutdoor implementation.
// ----------------------------------------------------------------------------- //
IMPLEMENT_NETWORKCLASS_ALIASED( KnifeOutdoor, DT_WeaponKnifeOutdoor )

BEGIN_NETWORK_TABLE_NOBASE( CKnifeOutdoor, DT_LocalActiveWeaponKnifeOutdoorData )
	#if !defined( CLIENT_DLL )
		SendPropTime( SENDINFO( m_flSmackTime ) ),
	#else
		RecvPropTime( RECVINFO( m_flSmackTime ) ),
	#endif
END_NETWORK_TABLE()


BEGIN_NETWORK_TABLE( CKnifeOutdoor, DT_WeaponKnifeOutdoor )
	#if !defined( CLIENT_DLL )
		SendPropDataTable("LocalActiveWeaponKnifeData", 0, &REFERENCE_SEND_TABLE(DT_LocalActiveWeaponKnifeOutdoorData), SendProxy_SendActiveLocalKnifeDataTable ),
	#else
		RecvPropDataTable("LocalActiveWeaponKnifeData", 0, 0, &REFERENCE_RECV_TABLE(DT_LocalActiveWeaponKnifeOutdoorData)),
	#endif
END_NETWORK_TABLE()


#if defined CLIENT_DLL
BEGIN_PREDICTION_DATA( CKnifeOutdoor )
	DEFINE_PRED_FIELD( m_flSmackTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()
#endif


LINK_ENTITY_TO_CLASS( weapon_knife_outdoor, CKnifeOutdoor );
PRECACHE_WEAPON_REGISTER( weapon_knife_outdoor );

#ifndef CLIENT_DLL
BEGIN_DATADESC( CKnifeOutdoor )
		DEFINE_THINKFUNC( Smack )
	END_DATADESC()
#endif


// ----------------------------------------------------------------------------- //
// CKnifeSkeleton implementation.
// ----------------------------------------------------------------------------- //
IMPLEMENT_NETWORKCLASS_ALIASED( KnifeSkeleton, DT_WeaponKnifeSkeleton )

BEGIN_NETWORK_TABLE_NOBASE( CKnifeSkeleton, DT_LocalActiveWeaponKnifeSkeletonData )
	#if !defined( CLIENT_DLL )
		SendPropTime( SENDINFO( m_flSmackTime ) ),
	#else
		RecvPropTime( RECVINFO( m_flSmackTime ) ),
	#endif
END_NETWORK_TABLE()


BEGIN_NETWORK_TABLE( CKnifeSkeleton, DT_WeaponKnifeSkeleton )
	#if !defined( CLIENT_DLL )
		SendPropDataTable("LocalActiveWeaponKnifeData", 0, &REFERENCE_SEND_TABLE(DT_LocalActiveWeaponKnifeSkeletonData), SendProxy_SendActiveLocalKnifeDataTable ),
	#else
		RecvPropDataTable("LocalActiveWeaponKnifeData", 0, 0, &REFERENCE_RECV_TABLE(DT_LocalActiveWeaponKnifeSkeletonData)),
	#endif
END_NETWORK_TABLE()


#if defined CLIENT_DLL
BEGIN_PREDICTION_DATA( CKnifeSkeleton )
	DEFINE_PRED_FIELD( m_flSmackTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()
#endif


LINK_ENTITY_TO_CLASS( weapon_knife_skeleton, CKnifeSkeleton );
PRECACHE_WEAPON_REGISTER( weapon_knife_skeleton );

#ifndef CLIENT_DLL
BEGIN_DATADESC( CKnifeSkeleton )
		DEFINE_THINKFUNC( Smack )
	END_DATADESC()
#endif


// ----------------------------------------------------------------------------- //
// CKnifeStiletto implementation.
// ----------------------------------------------------------------------------- //
IMPLEMENT_NETWORKCLASS_ALIASED( KnifeStiletto, DT_WeaponKnifeStiletto )

BEGIN_NETWORK_TABLE_NOBASE( CKnifeStiletto, DT_LocalActiveWeaponKnifeStilettoData )
	#if !defined( CLIENT_DLL )
		SendPropTime( SENDINFO( m_flSmackTime ) ),
	#else
		RecvPropTime( RECVINFO( m_flSmackTime ) ),
	#endif
END_NETWORK_TABLE()


BEGIN_NETWORK_TABLE( CKnifeStiletto, DT_WeaponKnifeStiletto )
	#if !defined( CLIENT_DLL )
		SendPropDataTable("LocalActiveWeaponKnifeData", 0, &REFERENCE_SEND_TABLE(DT_LocalActiveWeaponKnifeStilettoData), SendProxy_SendActiveLocalKnifeDataTable ),
	#else
		RecvPropDataTable("LocalActiveWeaponKnifeData", 0, 0, &REFERENCE_RECV_TABLE(DT_LocalActiveWeaponKnifeStilettoData)),
	#endif
END_NETWORK_TABLE()


#if defined CLIENT_DLL
BEGIN_PREDICTION_DATA( CKnifeStiletto )
	DEFINE_PRED_FIELD( m_flSmackTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()
#endif


LINK_ENTITY_TO_CLASS( weapon_knife_stiletto, CKnifeStiletto );
PRECACHE_WEAPON_REGISTER( weapon_knife_stiletto );

#ifndef CLIENT_DLL
BEGIN_DATADESC( CKnifeStiletto )
		DEFINE_THINKFUNC( Smack )
	END_DATADESC()
#endif


// ----------------------------------------------------------------------------- //
// CKnifeUrsus implementation.
// ----------------------------------------------------------------------------- //
IMPLEMENT_NETWORKCLASS_ALIASED( KnifeUrsus, DT_WeaponKnifeUrsus )

BEGIN_NETWORK_TABLE_NOBASE( CKnifeUrsus, DT_LocalActiveWeaponKnifeUrsusData )
	#if !defined( CLIENT_DLL )
		SendPropTime( SENDINFO( m_flSmackTime ) ),
	#else
		RecvPropTime( RECVINFO( m_flSmackTime ) ),
	#endif
END_NETWORK_TABLE()


BEGIN_NETWORK_TABLE( CKnifeUrsus, DT_WeaponKnifeUrsus )
	#if !defined( CLIENT_DLL )
		SendPropDataTable("LocalActiveWeaponKnifeData", 0, &REFERENCE_SEND_TABLE(DT_LocalActiveWeaponKnifeUrsusData), SendProxy_SendActiveLocalKnifeDataTable ),
	#else
		RecvPropDataTable("LocalActiveWeaponKnifeData", 0, 0, &REFERENCE_RECV_TABLE(DT_LocalActiveWeaponKnifeUrsusData)),
	#endif
END_NETWORK_TABLE()


#if defined CLIENT_DLL
BEGIN_PREDICTION_DATA( CKnifeUrsus )
	DEFINE_PRED_FIELD( m_flSmackTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()
#endif


LINK_ENTITY_TO_CLASS( weapon_knife_ursus, CKnifeUrsus );
PRECACHE_WEAPON_REGISTER( weapon_knife_ursus );

#ifndef CLIENT_DLL
BEGIN_DATADESC( CKnifeUrsus )
		DEFINE_THINKFUNC( Smack )
	END_DATADESC()
#endif


// ----------------------------------------------------------------------------- //
// CKnifeWidowmaker implementation.
// ----------------------------------------------------------------------------- //
IMPLEMENT_NETWORKCLASS_ALIASED( KnifeWidowmaker, DT_WeaponKnifeWidowmaker )

BEGIN_NETWORK_TABLE_NOBASE( CKnifeWidowmaker, DT_LocalActiveWeaponKnifeWidowmakerData )
	#if !defined( CLIENT_DLL )
		SendPropTime( SENDINFO( m_flSmackTime ) ),
	#else
		RecvPropTime( RECVINFO( m_flSmackTime ) ),
	#endif
END_NETWORK_TABLE()


BEGIN_NETWORK_TABLE( CKnifeWidowmaker, DT_WeaponKnifeWidowmaker )
	#if !defined( CLIENT_DLL )
		SendPropDataTable("LocalActiveWeaponKnifeData", 0, &REFERENCE_SEND_TABLE(DT_LocalActiveWeaponKnifeWidowmakerData), SendProxy_SendActiveLocalKnifeDataTable ),
	#else
		RecvPropDataTable("LocalActiveWeaponKnifeData", 0, 0, &REFERENCE_RECV_TABLE(DT_LocalActiveWeaponKnifeWidowmakerData)),
	#endif
END_NETWORK_TABLE()


#if defined CLIENT_DLL
BEGIN_PREDICTION_DATA( CKnifeWidowmaker )
	DEFINE_PRED_FIELD( m_flSmackTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()
#endif


LINK_ENTITY_TO_CLASS( weapon_knife_widowmaker, CKnifeWidowmaker );
PRECACHE_WEAPON_REGISTER( weapon_knife_widowmaker );

#ifndef CLIENT_DLL
BEGIN_DATADESC( CKnifeWidowmaker )
		DEFINE_THINKFUNC( Smack )
	END_DATADESC()
#endif


// ----------------------------------------------------------------------------- //
// CKnifePush implementation.
// ----------------------------------------------------------------------------- //
IMPLEMENT_NETWORKCLASS_ALIASED( KnifePush, DT_WeaponKnifePush )

BEGIN_NETWORK_TABLE_NOBASE( CKnifePush, DT_LocalActiveWeaponKnifePushData )
	#if !defined( CLIENT_DLL )
		SendPropTime( SENDINFO( m_flSmackTime ) ),
	#else
		RecvPropTime( RECVINFO( m_flSmackTime ) ),
	#endif
END_NETWORK_TABLE()


BEGIN_NETWORK_TABLE( CKnifePush, DT_WeaponKnifePush )
	#if !defined( CLIENT_DLL )
		SendPropDataTable("LocalActiveWeaponKnifeData", 0, &REFERENCE_SEND_TABLE(DT_LocalActiveWeaponKnifePushData), SendProxy_SendActiveLocalKnifeDataTable ),
	#else
		RecvPropDataTable("LocalActiveWeaponKnifeData", 0, 0, &REFERENCE_RECV_TABLE(DT_LocalActiveWeaponKnifePushData)),
	#endif
END_NETWORK_TABLE()


#if defined CLIENT_DLL
BEGIN_PREDICTION_DATA( CKnifePush )
	DEFINE_PRED_FIELD( m_flSmackTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()
#endif


LINK_ENTITY_TO_CLASS( weapon_knife_Push, CKnifePush );
PRECACHE_WEAPON_REGISTER( weapon_knife_Push );

#ifndef CLIENT_DLL
BEGIN_DATADESC( CKnifePush )
		DEFINE_THINKFUNC( Smack )
	END_DATADESC()
#endif