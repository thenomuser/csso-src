//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "c_cs_player.h"
#include "c_user_message_register.h"
#include "view.h"
#include "iclientvehicle.h"
#include "ivieweffects.h"
#include "input.h"
#include "IEffects.h"
#include "fx.h"
#include "c_basetempentity.h"
#include "hud_macros.h"	//HOOK_COMMAND
#include "engine/ivdebugoverlay.h"
#include "smoke_fog_overlay.h"
#include "bone_setup.h"
#include "in_buttons.h"
#include "r_efx.h"
#include "dlight.h"
#include "shake.h"
#include "cl_animevent.h"
#include "c_physicsprop.h"
#include "props_shared.h"
#include "obstacle_pushaway.h"
#include "death_pose.h"
#include "interpolatortypes.h"

#include "effect_dispatch_data.h"	//for water ripple / splash effect
#include "c_te_effect_dispatch.h"	//ditto
#include "c_te_legacytempents.h"
#include "fx_cs_blood.h"
#include "c_cs_playerresource.h"
#include "c_team.h"
#include "c_cs_hostage.h"
#include "prediction.h"

#include <vgui_controls/Panel.h>
#include "ragdoll_shared.h"
#include "collisionutils.h"

#include "npcevent.h"

// NVNT - haptics system for spectating
#include "haptics/haptic_utils.h"

#include "steam/steam_api.h"

#include "cs_blackmarket.h"				// for vest/helmet prices

#include "cs_loadout.h"

#include <vgui/ILocalize.h>

#if defined( CCSPlayer )
	#undef CCSPlayer
#endif

#include "materialsystem/imesh.h"		//for materials->FindMaterial
#include "iviewrender.h"				//for view->

#include "iviewrender_beams.h"			// flashlight beam

#include "physpropclientside.h"			// for dropping physics mags

// [menglish] Adding and externing variables needed for the freezecam
static Vector WALL_MIN(-WALL_OFFSET,-WALL_OFFSET,-WALL_OFFSET);
static Vector WALL_MAX(WALL_OFFSET,WALL_OFFSET,WALL_OFFSET);

extern ConVar	spec_freeze_time;
extern ConVar	spec_freeze_traveltime;
extern ConVar	spec_freeze_traveltime_long;
extern ConVar	spec_freeze_distance_min;
extern ConVar	spec_freeze_distance_max;
extern ConVar	spec_freeze_target_fov;
extern ConVar	spec_freeze_target_fov_long;

ConVar cl_crosshair_sniper_width( "cl_crosshair_sniper_width", "1", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "If >1 sniper scope cross lines gain extra width (1 for single-pixel hairline)" );

ConVar cl_left_hand_ik( "cl_left_hand_ik", "1", 0, "Attach player's left hand to rifle with IK." );

ConVar cl_ragdoll_physics_enable( "cl_ragdoll_physics_enable", "1", 0, "Enable/disable ragdoll physics." );

ConVar fov_cs_debug( "fov_cs_debug", "0", FCVAR_CHEAT, "Sets the view fov if cheats are on." );

#define FREEZECAM_LONGCAM_DIST	320  // over this amount, the camera will zoom close on target

#define sv_magazine_drop_physics 1
#define sv_magazine_drop_time 15

//ConVar sv_magazine_drop_physics( "sv_magazine_drop_physics", "1", FCVAR_REPLICATED | FCVAR_RELEASE, "Players drop physical weapon magazines when reloading." );
//ConVar sv_magazine_drop_time( "sv_magazine_drop_time", "15", FCVAR_REPLICATED | FCVAR_RELEASE, "Duration physical magazines stay in the world.", true, 2.0f, true, 20.0f );

/*
ConVar cl_minmodels( "cl_minmodels", "0", 0, "Uses one player model for each team." );
ConVar cl_min_ct( "cl_min_ct", "1", 0, "Controls which CT model is used when cl_minmodels is set.", true, 1, true, 7 );
ConVar cl_min_t( "cl_min_t", "1", 0, "Controls which Terrorist model is used when cl_minmodels is set.", true, 1, true, 7 );
*/

// [jason] Adjusts the safe extents of the camera placement for the freeze cam to prevent it from penetrating the killer's geometry
ConVar cl_freeze_cam_penetration_tolerance( "cl_freeze_cam_penetration_tolerance", "0", 0, "If the freeze cam gets closer to target than this distance, we snap to death cam instead (0 = use character bounds instead, -1 = disable this safety check" );

ConVar cl_ragdoll_crumple( "cl_ragdoll_crumple", "1" );

ConVar cl_dm_buyrandomweapons( "cl_dm_buyrandomweapons", "1", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Player will automatically receive a random weapon on spawn in deathmatch if this is set to 1 (otherwise, they will receive the last weapon)" );

const float CycleLatchTolerance = 0.15;	// amount we can diverge from the server's cycle before we're corrected

extern ConVar mp_playerid_delay;
extern ConVar mp_playerid_hold;
extern ConVar sv_allowminmodels;

extern ConVar mp_buy_anywhere;

class CAddonInfo
{
public:
	const char *m_pAttachmentName;
	const char *m_pWeaponClassName;	// The addon uses the w_ model from this weapon.
	const char *m_pModelName;		//If this is present, will use this model instead of looking up the weapon
	const char *m_pHolsterName;
};


// These must follow the ADDON_ ordering.
CAddonInfo g_AddonInfo[] =
{
	{ "grenade0",	"weapon_flashbang",		0, 0 },
	{ "grenade1",	"weapon_flashbang",		0, 0 },
	{ "grenade2",	"weapon_hegrenade",		0, 0 },
	{ "grenade3",	"weapon_smokegrenade",	0, 0 },
	{ "c4",			"weapon_c4",			0, 0 },
	{ "defusekit",	0,						"models/weapons/w_defuser.mdl", 0 },
	{ "primary",	0,						0, 0 },	// Primary addon model is looked up based on m_iPrimaryAddon
	{ "pistol",		0,						0, 0 },	// Pistol addon model is looked up based on m_iSecondaryAddon
	{ "eholster",	0,						"models/weapons/w_eq_eholster_elite.mdl", "models/weapons/w_eq_eholster.mdl" },
	{ "knife",		0,						0, 0 },	// Knife addon model is looked up based on m_iKnifeAddon
	{ "grenade4",	"weapon_decoy",			0, 0 },
};

bool LineGoesThroughSmoke( Vector from, Vector to, bool grenadeBloat )
{
	float totalSmokedLength = 0.0f;	// distance along line of sight covered by smoke

	// compute unit vector and length of line of sight segment
	//Vector sightDir = to - from;
	//float sightLength = sightDir.NormalizeInPlace();

	C_CSPlayer *pPlayer = C_CSPlayer::GetLocalCSPlayer();

	if ( !pPlayer )
		return false;

	const float smokeRadiusSq = SmokeGrenadeRadius * SmokeGrenadeRadius * grenadeBloat * grenadeBloat;
	C_BaseParticleEntity *ent;
	for ( int i=0; i < pPlayer->m_SmokeGrenades.Count(); i++ )
	{
		float flLengthAdd = 0;
		if ( CSGameRules() )
		{
			ent = pPlayer->m_SmokeGrenades[i];
			if ( !ent || ent->IsEFlagSet(EFL_DORMANT) ) // PiMoN: is EFL_DORMANT check actually needed?
				continue;

			flLengthAdd = CSGameRules()->CheckTotalSmokedLength( smokeRadiusSq, ent->GetAbsOrigin(), from, to );
			// get the totalSmokedLength and check to see if the line starts or stops in smoke.  If it does this will return -1 and we should just bail early
			if ( flLengthAdd == -1 )
				return true;

			totalSmokedLength += flLengthAdd;
		}
	}

	// define how much smoke a bot can see thru
	const float maxSmokedLength = 0.7f * SmokeGrenadeRadius;

	// return true if the total length of smoke-covered line-of-sight is too much
	return (totalSmokedLength > maxSmokedLength);
}

// -------------------------------------------------------------------------------- //
// Player animation event. Sent to the client when a player fires, jumps, reloads, etc..
// -------------------------------------------------------------------------------- //

class C_TEPlayerAnimEvent : public C_BaseTempEntity
{
public:
	DECLARE_CLASS( C_TEPlayerAnimEvent, C_BaseTempEntity );
	DECLARE_CLIENTCLASS();

	virtual void PostDataUpdate( DataUpdateType_t updateType )
	{
		// Create the effect.
		C_CSPlayer *pPlayer = dynamic_cast< C_CSPlayer* >( m_hPlayer.Get() );
		if ( pPlayer && !pPlayer->IsDormant() )
		{
			pPlayer->DoAnimationEvent( (PlayerAnimEvent_t)m_iEvent.Get(), m_nData );
		}
	}

public:
	CNetworkHandle( CBasePlayer, m_hPlayer );
	CNetworkVar( int, m_iEvent );
	CNetworkVar( int, m_nData );
};

IMPLEMENT_CLIENTCLASS_EVENT( C_TEPlayerAnimEvent, DT_TEPlayerAnimEvent, CTEPlayerAnimEvent );

BEGIN_RECV_TABLE_NOBASE( C_TEPlayerAnimEvent, DT_TEPlayerAnimEvent )
	RecvPropEHandle( RECVINFO( m_hPlayer ) ),
	RecvPropInt( RECVINFO( m_iEvent ) ),
	RecvPropInt( RECVINFO( m_nData ) )
END_RECV_TABLE()

BEGIN_PREDICTION_DATA( C_CSPlayer )
#ifdef CS_SHIELD_ENABLED
	DEFINE_PRED_FIELD( m_bShieldDrawn, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
#endif
	DEFINE_PRED_FIELD_TOL( m_flStamina, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, 0.1f ),
	DEFINE_PRED_FIELD( m_flCycle, FIELD_FLOAT, FTYPEDESC_OVERRIDE | FTYPEDESC_PRIVATE | FTYPEDESC_NOERRORCHECK ),
	DEFINE_PRED_FIELD( m_iShotsFired, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_iDirection, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bIsWalking, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bResumeZoom, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_iLastZoom, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bDuckOverride, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bInBombZone, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),

END_PREDICTION_DATA()

vgui::IImage* GetDefaultAvatarImage( C_BasePlayer *pPlayer )
{
	vgui::IImage* result = NULL;

	switch ( pPlayer ? pPlayer->GetTeamNumber() : TEAM_MAXCOUNT )
	{
		case TEAM_TERRORIST: 
			result = vgui::scheme()->GetImage( CSTRIKE_DEFAULT_T_AVATAR, true );
			break;

		case TEAM_CT:		 
			result = vgui::scheme()->GetImage( CSTRIKE_DEFAULT_CT_AVATAR, true );
			break;

		default:
			result = vgui::scheme()->GetImage( CSTRIKE_DEFAULT_AVATAR, true );
			break;
	}

	return result;
}

// ----------------------------------------------------------------------------- //
// Client ragdoll entity.
// ----------------------------------------------------------------------------- //

float g_flDieTranslucentTime = 0.6;

class C_CSRagdoll : public C_BaseAnimatingOverlay
{
public:
	DECLARE_CLASS( C_CSRagdoll, C_BaseAnimatingOverlay );
	DECLARE_CLIENTCLASS();

	C_CSRagdoll();
	~C_CSRagdoll();

	virtual void OnDataChanged( DataUpdateType_t type );

	int GetPlayerEntIndex() const;
	IRagdoll* GetIRagdoll() const;
	void GetRagdollInitBoneArrays( matrix3x4_t *pDeltaBones0, matrix3x4_t *pDeltaBones1, matrix3x4_t *pCurrentBones, float boneDt ) OVERRIDE;

	void ApplyRandomTaserForce( void );
	void ImpactTrace( trace_t *pTrace, int iDamageType, const char *pCustomImpactName );

	virtual void ComputeFxBlend();
	virtual bool IsTransparent();
	bool IsInitialized() { return m_bInitialized; }
	// fading ragdolls don't cast shadows
	virtual ShadowType_t ShadowCastType() 
	{ 
		if ( m_flRagdollSinkStart == -1 )
			return BaseClass::ShadowCastType();
		return SHADOWS_NONE;
	}

private:

	C_CSRagdoll( const C_CSRagdoll & ) {}

	void Interp_Copy( C_BaseAnimatingOverlay *pSourceEntity );

	void CreateLowViolenceRagdoll( void );
	void CreateCSRagdoll( void );

private:

	EHANDLE	m_hPlayer;
	CNetworkVector( m_vecRagdollVelocity );
	CNetworkVector( m_vecRagdollOrigin );
	CNetworkVar(int, m_iDeathPose );
	CNetworkVar(int, m_iDeathFrame );
	float m_flRagdollSinkStart;
	bool m_bInitialized;
	bool m_bCreatedWhilePlaybackSkipping;
	C_BaseAnimating* m_pGlovesModel;
};


IMPLEMENT_CLIENTCLASS_DT_NOBASE( C_CSRagdoll, DT_CSRagdoll, CCSRagdoll )
	RecvPropVector( RECVINFO_NAME( m_vecNetworkOrigin, m_vecOrigin ) ),
	RecvPropVector( RECVINFO(m_vecRagdollOrigin) ),
	RecvPropEHandle( RECVINFO( m_hPlayer ) ),
	RecvPropInt( RECVINFO( m_nModelIndex ) ),
	RecvPropInt( RECVINFO(m_nForceBone) ),
	RecvPropVector( RECVINFO(m_vecForce) ),
	RecvPropVector( RECVINFO( m_vecRagdollVelocity ) ),
	RecvPropInt( RECVINFO(m_iDeathPose) ),
	RecvPropInt( RECVINFO(m_iDeathFrame) ),
	RecvPropInt(RECVINFO(m_iTeamNum)),
	RecvPropInt( RECVINFO(m_bClientSideAnimation)),
END_RECV_TABLE()


C_CSRagdoll::C_CSRagdoll()
{
	m_flRagdollSinkStart = -1;
	m_bInitialized = false;
	m_bCreatedWhilePlaybackSkipping = engine->IsSkippingPlayback();
	m_pGlovesModel = NULL;
}

C_CSRagdoll::~C_CSRagdoll()
{
	PhysCleanupFrictionSounds( this );

	if ( m_pGlovesModel )
	{
		m_pGlovesModel->Remove();
		m_pGlovesModel = NULL;
	}
}

void C_CSRagdoll::GetRagdollInitBoneArrays( matrix3x4_t *pDeltaBones0, matrix3x4_t *pDeltaBones1, matrix3x4_t *pCurrentBones, float boneDt )
{
	// turn off interp so we can setup bones in multiple positions
	SetEffects( EF_NOINTERP );

	// populate bone arrays for current positions and starting velocity positions
	InvalidateBoneCache();
	SetupBones( pCurrentBones, MAXSTUDIOBONES, BONE_USED_BY_ANYTHING, gpGlobals->curtime );
	Plat_FastMemcpy( pDeltaBones0, pCurrentBones, sizeof( matrix3x4_t ) * MAXSTUDIOBONES );

	// set death anim
	CBaseAnimatingOverlay *pRagdollOverlay = GetBaseAnimatingOverlay();
	int n = pRagdollOverlay->GetNumAnimOverlays();
	if ( n > 0 )
	{
		CAnimationLayer *pLastRagdollLayer = pRagdollOverlay->GetAnimOverlay(n-1);
		if ( pLastRagdollLayer )
		{
			pLastRagdollLayer->SetSequence( m_iDeathPose );
			pLastRagdollLayer->SetWeight( 1 );
		}
	}

	SetAbsOrigin( GetAbsOrigin() );

	// set up bones in velocity adding positions
	InvalidateBoneCache();
	SetupBones( pDeltaBones1, MAXSTUDIOBONES, BONE_USED_BY_ANYTHING, gpGlobals->curtime );

	//fallback
	Vector vecRagdollVelocityPush = m_vecRagdollVelocity;
	
	C_CSPlayer *pPlayer = dynamic_cast< C_CSPlayer* >( m_hPlayer.Get() );
	if ( pPlayer )
	{
		vecRagdollVelocityPush = pPlayer->m_vecLastAliveLocalVelocity * boneDt;
	}

	if ( vecRagdollVelocityPush.Length() > CS_PLAYER_SPEED_RUN * 3 )
		vecRagdollVelocityPush = vecRagdollVelocityPush.Normalized() * CS_PLAYER_SPEED_RUN * 3;
	
	// apply global extra velocity manually instead of relying on prediction to do it. This means all bones get the same vel...
	for ( int i=0; i<MAXSTUDIOBONES; i++ )
	{
		pDeltaBones1[i].SetOrigin( pDeltaBones0[i].GetOrigin() + vecRagdollVelocityPush );

		//debugoverlay->AddBoxOverlay( pCurrentBones[i].GetOrigin(), -Vector(0.1, 0.1, 0.1), Vector(0.1, 0.1, 0.1), QAngle(0,0,0), 255,0,0,255, 5 );
		////debugoverlay->AddBoxOverlay( pDeltaBones1[i].GetOrigin(), -Vector(0.1, 0.1, 0.1), Vector(0.1, 0.1, 0.1), QAngle(0,0,0), 0,255,0,255, 5 );
		////debugoverlay->AddBoxOverlay( pDeltaBones0[i].GetOrigin(), -Vector(0.1, 0.1, 0.1), Vector(0.1, 0.1, 0.1), QAngle(0,0,0), 0,0,255,255, 5 );
		//debugoverlay->AddLineOverlay( pDeltaBones0[i].GetOrigin(), pDeltaBones1[i].GetOrigin(), 255,0,0, true, 5 );
	}
}

void C_CSRagdoll::Interp_Copy( C_BaseAnimatingOverlay *pSourceEntity )
{
	if ( !pSourceEntity )
		return;

	VarMapping_t *pSrc = pSourceEntity->GetVarMapping();
	VarMapping_t *pDest = GetVarMapping();

	// Find all the VarMapEntry_t's that represent the same variable.
	for ( int i = 0; i < pDest->m_Entries.Count(); i++ )
	{
		VarMapEntry_t *pDestEntry = &pDest->m_Entries[i];
		for ( int j=0; j < pSrc->m_Entries.Count(); j++ )
		{
			VarMapEntry_t *pSrcEntry = &pSrc->m_Entries[j];
			if ( !Q_strcmp( pSrcEntry->watcher->GetDebugName(),
				pDestEntry->watcher->GetDebugName() ) )
			{
				pDestEntry->watcher->Copy( pSrcEntry->watcher );
				break;
			}
		}
	}
}



ConVar cl_random_taser_bone_y( "cl_random_taser_bone_y", "-1.0", 0, "The Y position used for the random taser force." );
ConVar cl_random_taser_force_y( "cl_random_taser_force_y", "-1.0", 0, "The Y position used for the random taser force." );
ConVar cl_random_taser_power( "cl_random_taser_power", "4000.0", 0, "Power used when applying the taser effect." );

void C_CSRagdoll::ApplyRandomTaserForce( void )
{
	IPhysicsObject *pPhysicsObject = VPhysicsGetObject();

	if( !pPhysicsObject )
		return;

	int boneID = LookupBone( RandomInt( 0, 1 ) ? "ValveBiped.Bip01_L_Hand" : "ValveBiped.Bip01_R_Hand" );
	if( boneID < 0 )
	{
		// error, couldn't find a bone matching this name, early out
		AssertMsg( false, "couldn't find a bone matching this name, early out" );
		return;
	}
	
	Vector bonePos;
	QAngle boneAngle;
	GetBonePosition( boneID, bonePos, boneAngle );

	bonePos.y += cl_random_taser_bone_y.GetFloat();
	Vector dir( random->RandomFloat( -1.0f, 1.0f ), random->RandomFloat( -1.0f, 1.0f ), cl_random_taser_force_y.GetFloat() );
	VectorNormalize( dir );

	dir *= cl_random_taser_power.GetFloat();  // adjust  strength

	// apply force where we hit it
	pPhysicsObject->ApplyForceOffset( dir, bonePos );	

	// make sure the ragdoll is "awake" to process our updates, at least for a bit
	m_pRagdoll->ResetRagdollSleepAfterTime();
}

void C_CSRagdoll::ImpactTrace( trace_t *pTrace, int iDamageType, const char *pCustomImpactName )
{
	static const float RAGDOLL_IMPACT_MAGNITUDE = 8000.0f;

	IPhysicsObject *pPhysicsObject = VPhysicsGetObject();

	if( !pPhysicsObject )
		return;

	Vector dir = pTrace->endpos - pTrace->startpos;
	
	if ( iDamageType == DMG_BLAST )
	{
		VectorNormalize( dir );
		dir *= RAGDOLL_IMPACT_MAGNITUDE;  // adjust impact strenght
				
		// apply force at object mass center
		pPhysicsObject->ApplyForceCenter( dir );
	}
	else
	{
		Vector hitpos;  
	
		VectorMA( pTrace->startpos, pTrace->fraction, dir, hitpos );
		VectorNormalize( dir );

		// apply force where we hit it (shock/taser is handled with a special death type )
		if ( (iDamageType & DMG_SHOCK ) == 0 )
		{
			// Blood spray!
			float flDamage = 10.0f;
			// This does smaller splotches on the guy and splats blood on the world.
			TraceBleed( flDamage, dir, pTrace, iDamageType );
			FX_CS_BloodSpray( hitpos, dir, flDamage );

			dir *= RAGDOLL_IMPACT_MAGNITUDE;  // adjust impact strenght
			pPhysicsObject->ApplyForceOffset( dir, hitpos );
		}
	}

	m_pRagdoll->ResetRagdollSleepAfterTime();
}


void C_CSRagdoll::CreateLowViolenceRagdoll( void )
{
	// Just play a death animation.
	// Find a death anim to play.
	int iMinDeathAnim = 9999, iMaxDeathAnim = -9999;
	for ( int iAnim=1; iAnim < 100; iAnim++ )
	{
		char str[512];
		Q_snprintf( str, sizeof( str ), "death%d", iAnim );
		if ( LookupSequence( str ) == -1 )
			break;

		iMinDeathAnim = MIN( iMinDeathAnim, iAnim );
		iMaxDeathAnim = MAX( iMaxDeathAnim, iAnim );
	}

	if ( iMinDeathAnim == 9999 )
	{
		CreateCSRagdoll();
		return;
	}

	SetNetworkOrigin( m_vecRagdollOrigin );
	SetAbsOrigin( m_vecRagdollOrigin );
	SetAbsVelocity( m_vecRagdollVelocity );

	C_CSPlayer *pPlayer = dynamic_cast< C_CSPlayer* >( m_hPlayer.Get() );
	if ( pPlayer )
	{
		if ( !pPlayer->IsDormant() )
		{
			// move my current model instance to the ragdoll's so decals are preserved.
			pPlayer->SnatchModelInstance( this );
		}

		SetAbsAngles( pPlayer->GetRenderAngles() );
		SetNetworkAngles( pPlayer->GetRenderAngles() );
	}

	int iDeathAnim = RandomInt( iMinDeathAnim, iMaxDeathAnim );
	char str[512];
	Q_snprintf( str, sizeof( str ), "death%d", iDeathAnim );
	SetSequence( LookupSequence( str ) );
	ForceClientSideAnimationOn();

	Interp_Reset( GetVarMapping() );
}


void C_CSRagdoll::CreateCSRagdoll()
{
	// First, initialize all our data. If we have the player's entity on our client,
	// then we can make ourselves start out exactly where the player is.
	C_CSPlayer *pPlayer = dynamic_cast< C_CSPlayer* >( m_hPlayer.Get() );

	// mark this to prevent model changes from overwriting the death sequence with the server sequence
	SetReceivedSequence();

	if ( pPlayer && !pPlayer->IsDormant() )
	{
		// move my current model instance to the ragdoll's so decals are preserved.
		pPlayer->SnatchModelInstance( this );

		VarMapping_t *varMap = GetVarMapping();

		// Copy all the interpolated vars from the player entity.
		// The entity uses the interpolated history to get bone velocity.
		bool bRemotePlayer = (pPlayer != C_BasePlayer::GetLocalPlayer());
		if ( bRemotePlayer )
		{
			Interp_Copy( pPlayer );

			SetAbsAngles( pPlayer->GetRenderAngles() );
			GetRotationInterpolator().Reset();

			m_flAnimTime = pPlayer->m_flAnimTime;
			SetSequence( pPlayer->GetSequence() );
			m_flPlaybackRate = pPlayer->GetPlaybackRate();
		}
		else
		{
			// This is the local player, so set them in a default
			// pose and slam their velocity, angles and origin
			SetAbsOrigin( m_vecRagdollOrigin );

			SetAbsAngles( pPlayer->GetRenderAngles() );

			SetAbsVelocity( m_vecRagdollVelocity );
		}

		// in addition to base cycle, duplicate overlay layers and pose params onto the ragdoll, 
		// so the starting pose is as accurate as possible.

		SetCycle( pPlayer->GetCycle() );

		for ( int i=0; i<MAXSTUDIOPOSEPARAM; i++ )
		{
			//Msg( "Setting pose param %i to %.2f\n", i, pPlayer->GetPoseParameter( i ) );
			SetPoseParameter( i, pPlayer->GetPoseParameter( i ) );
		}

		CBaseAnimatingOverlay *pPlayerOverlay = pPlayer->GetBaseAnimatingOverlay();
		CBaseAnimatingOverlay *pRagdollOverlay = GetBaseAnimatingOverlay();
		if ( pPlayerOverlay )
		{
			int layerCount = pPlayerOverlay->GetNumAnimOverlays();
			pRagdollOverlay->SetNumAnimOverlays(layerCount);
			for( int layerIndex = 0; layerIndex < layerCount; ++layerIndex )
			{
				CAnimationLayer *playerLayer = pPlayerOverlay->GetAnimOverlay(layerIndex);
				CAnimationLayer *ragdollLayer = pRagdollOverlay->GetAnimOverlay(layerIndex);
				if( playerLayer && ragdollLayer )
				{
					ragdollLayer->SetCycle( playerLayer->GetCycle() );
					ragdollLayer->SetOrder( playerLayer->GetOrder() );
					ragdollLayer->SetSequence( playerLayer->GetSequence() );
					ragdollLayer->SetWeight( playerLayer->GetWeight() );
				}
			}
		}

		m_flPlaybackRate = pPlayer->GetPlaybackRate();


		if ( !bRemotePlayer )
		{
			Interp_Reset( varMap );
		}

		CopySequenceTransitions( pPlayer );

		// add a separate gloves model if needed
		if ( !m_pGlovesModel && DoesModelSupportGloves() && CSLoadout()->HasGlovesSet( pPlayer, pPlayer->GetTeamNumber() ) )
		{

			m_pGlovesModel = new C_BaseAnimating;
			if ( m_pGlovesModel->InitializeAsClientEntity( GetGlovesInfo( CSLoadout()->GetGlovesForPlayer( pPlayer, pPlayer->GetTeamNumber() ) )->szWorldModel, RENDER_GROUP_OPAQUE_ENTITY ) )
			{
				// hide the gloves first
				SetBodygroup( FindBodygroupByName( "gloves" ), 1 );

				m_pGlovesModel->FollowEntity( this ); // attach to player model
				m_pGlovesModel->AddEffects( EF_BONEMERGE_FASTCULL ); // EF_BONEMERGE is already applied on FollowEntity()

				int skin = 0;
				if ( pPlayer->m_pViewmodelArmConfig )
					skin = pPlayer->m_pViewmodelArmConfig->iSkintoneIndex;
				else
				{
					CStudioHdr *pHdr = pPlayer->GetModelPtr();
					if ( pHdr )
						skin = GetPlayerViewmodelArmConfigForPlayerModel( pHdr->pszName() )->iSkintoneIndex;
				}

				m_pGlovesModel->m_nSkin = skin; // set the corrent skin tone
			}
			else
			{
				m_pGlovesModel->Release();
				SetBodygroup( FindBodygroupByName( "gloves" ), 0 );
			}
		}

		pPlayer->MoveBoneAttachments( this );
	}
	else
	{
		// overwrite network origin so later interpolation will
		// use this position
		SetNetworkOrigin( m_vecRagdollOrigin );

		SetAbsOrigin( m_vecRagdollOrigin );
		SetAbsVelocity( m_vecRagdollVelocity );

		Interp_Reset( GetVarMapping() );
	}

	// Turn it into a ragdoll.
	if ( cl_ragdoll_physics_enable.GetInt() )
	{
		// Make us a ragdoll..
		m_nRenderFX = kRenderFxRagdoll;

		matrix3x4_t boneDelta0[MAXSTUDIOBONES];
		matrix3x4_t boneDelta1[MAXSTUDIOBONES];
		matrix3x4_t currentBones[MAXSTUDIOBONES];
		const float boneDt = 0.05f;

		// We used to get these values from the local player object when he ragdolled, but he had some bad values when using prediction.
		// It ends up that just getting the bone array values for this ragdoll works best for both the local and remote players.
		ConVarRef cl_ragdoll_crumple( "cl_ragdoll_crumple" );
		if ( cl_ragdoll_crumple.GetBool() )
		{
			BaseClass::GetRagdollInitBoneArrays( boneDelta0, boneDelta1, currentBones, boneDt );
		}
		else
		{
			GetRagdollInitBoneArrays( boneDelta0, boneDelta1, currentBones, boneDt );
		}

		InitAsClientRagdoll( boneDelta0, boneDelta1, currentBones, boneDt );
		m_flRagdollSinkStart = -1;
	}
	else
	{
		m_flRagdollSinkStart = gpGlobals->curtime;
		DestroyShadow();
		ClientLeafSystem()->SetRenderGroup( GetRenderHandle(), RENDER_GROUP_TRANSLUCENT_ENTITY );
	}
	m_bInitialized = true;
}

void C_CSRagdoll::ComputeFxBlend( void )
{
	if ( m_flRagdollSinkStart == -1 )
	{
		BaseClass::ComputeFxBlend();
	}
	else
	{
		float elapsed = gpGlobals->curtime - m_flRagdollSinkStart;
		float flVal = RemapVal( elapsed, 0, g_flDieTranslucentTime, 255, 0 );
		flVal = clamp( flVal, 0, 255 );
		m_nRenderFXBlend = (int)flVal;

#ifdef _DEBUG
		m_nFXComputeFrame = gpGlobals->framecount;
#endif
	}
}


bool C_CSRagdoll::IsTransparent( void )
{
	if ( m_flRagdollSinkStart == -1 )
	{
		return BaseClass::IsTransparent();
	}
	else
	{
		return true;
	}
}


void C_CSRagdoll::OnDataChanged( DataUpdateType_t type )
{
	BaseClass::OnDataChanged( type );

	if ( type == DATA_UPDATE_CREATED )
	{
		// Prevent replays from creating ragdolls on the first frame of playback after skipping through playback.
		// If a player died (leaving a ragdoll) previous to the first frame of replay playback,
		// their ragdoll wasn't yet initialized because OnDataChanged events are queued but not processed
		// until the first render. 
		if ( engine->IsPlayingDemo() && m_bCreatedWhilePlaybackSkipping )
		{
			Release();
			return;
		}

		if ( g_RagdollLVManager.IsLowViolence() )
		{
			CreateLowViolenceRagdoll();
		}
		else
		{
			CreateCSRagdoll();
		}
	}
	else
	{
		if ( !cl_ragdoll_physics_enable.GetInt() )
		{
			// Don't let it set us back to a ragdoll with data from the server.
			m_nRenderFX = kRenderFxNone;
		}
	}
}

IRagdoll* C_CSRagdoll::GetIRagdoll() const
{
	return m_pRagdoll;
}

//-----------------------------------------------------------------------------
// Purpose: Called when the player toggles nightvision
// Input  : *pData - the int value of the nightvision state
//			*pStruct - the player
//			*pOut -
//-----------------------------------------------------------------------------
void RecvProxy_NightVision( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	C_CSPlayer *pPlayerData = (C_CSPlayer *) pStruct;

	bool bNightVisionOn = ( pData->m_Value.m_Int > 0 );

	if ( pPlayerData->m_bNightVisionOn != bNightVisionOn )
	{
		if ( bNightVisionOn )
			 pPlayerData->m_flNightVisionAlpha = 1;
	}

	pPlayerData->m_bNightVisionOn = bNightVisionOn;
}

void RecvProxy_FlashTime( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	C_CSPlayer *pPlayerData = (C_CSPlayer * ) pStruct;
	pPlayerData->m_bFlashBuildUp = false;

	float flNewFlashDuration = pData->m_Value.m_Float;
	if ( flNewFlashDuration == 0.0f )
	{
		// Disable flashbang effect
		pPlayerData->m_flFlashScreenshotAlpha = 0.0f;
		pPlayerData->m_flFlashOverlayAlpha = 0.0f;
		pPlayerData->m_bFlashBuildUp = false;
		pPlayerData->m_bFlashScreenshotHasBeenGrabbed = false;
		pPlayerData->m_flFlashDuration = 0.0f;
		pPlayerData->m_flFlashBangTime = 0.0f;
		pPlayerData->m_bFlashDspHasBeenCleared = false;
	
		C_CSPlayer *pLocalCSPlayer = C_CSPlayer::GetLocalCSPlayer();
		if (pLocalCSPlayer)
		{
			pLocalCSPlayer->m_bFlashDspHasBeenCleared = false;
		}
		return;
	}

	// If local player is spectating in mode other than first-person, reduce effect duration by half
	C_CSPlayer *pLocalPlayer = C_CSPlayer::GetLocalCSPlayer();
	if ( pLocalPlayer && pLocalPlayer->GetObserverMode() != OBS_MODE_NONE && pLocalPlayer->GetObserverMode() != OBS_MODE_IN_EYE )
	{
		flNewFlashDuration *= 0.5f;
	}

	if ( pLocalPlayer && pLocalPlayer->GetObserverMode() != OBS_MODE_NONE && 
		 flNewFlashDuration > 0.0f && pPlayerData->m_flFlashDuration == flNewFlashDuration )
	{
		// Ignore this update. This is a resend from the server triggered by the spectator changing target.
		return;
	}

	if ( !pPlayerData->IsFlashBangActive() && flNewFlashDuration > 0.0f )
	{
		// reset flash alpha to start of effect build-up
		pPlayerData->m_flFlashScreenshotAlpha = 1.0f;
		pPlayerData->m_flFlashOverlayAlpha = 1.0f;
		pPlayerData->m_bFlashBuildUp = true;
		pPlayerData->m_bFlashScreenshotHasBeenGrabbed = false;
	}

	pPlayerData->m_flFlashDuration = flNewFlashDuration;
	pPlayerData->m_flFlashBangTime = gpGlobals->curtime + pPlayerData->m_flFlashDuration;
	pPlayerData->m_bFlashDspHasBeenCleared = false;
	
	C_CSPlayer *pLocalCSPlayer = C_CSPlayer::GetLocalCSPlayer();
	if (pLocalCSPlayer)
	{
		pLocalCSPlayer->m_bFlashDspHasBeenCleared = false;
	}
}

void C_CSPlayer::UpdateFlashBangEffect( void )
{
	if ( ( m_flFlashBangTime < gpGlobals->curtime ) || ( m_flFlashMaxAlpha <= 0.0f ) )
	{
		// FlashBang is inactive
		m_flFlashScreenshotAlpha = 0.0f;
		m_flFlashOverlayAlpha = 0.0f;
		return;
	}

	static const float FLASH_BUILD_UP_PER_FRAME = 45.0f;
	static const float FLASH_BUILD_UP_DURATION = ( 255.0f / FLASH_BUILD_UP_PER_FRAME ) * ( 1.0f / 60.0f );

	float flFlashTimeElapsed = GetFlashTimeElapsed();

	if ( m_bFlashBuildUp )
	{
		// build up
		m_flFlashScreenshotAlpha = Clamp( ( flFlashTimeElapsed / FLASH_BUILD_UP_DURATION ) * m_flFlashMaxAlpha.Get(), 0.0f, m_flFlashMaxAlpha.Get() );
		m_flFlashOverlayAlpha = m_flFlashScreenshotAlpha;

		if ( flFlashTimeElapsed >= FLASH_BUILD_UP_DURATION )
		{
			m_bFlashBuildUp = false;
		}
	}
	else
	{
		// cool down
		float flFlashTimeLeft = m_flFlashBangTime - gpGlobals->curtime;
		m_flFlashScreenshotAlpha = ( m_flFlashMaxAlpha * flFlashTimeLeft ) / m_flFlashDuration;
		m_flFlashScreenshotAlpha = Clamp( m_flFlashScreenshotAlpha, 0.0f, m_flFlashMaxAlpha.Get() );

		float flAlphaPercentage = 1.0f;
		const float certainBlindnessTimeThresh = 3.0f; // yes this is a magic number, necessary to match CS/CZ flashbang effectiveness cause the rendering system is completely different.

		if (flFlashTimeLeft > certainBlindnessTimeThresh)
		{
			// if we still have enough time of blindness left, make sure the player can't see anything yet.
			flAlphaPercentage = 1.0f;
		}
		else
		{
			// blindness effects shorter than 'certainBlindness`TimeThresh' will start off at less than 255 alpha.
			flAlphaPercentage = flFlashTimeLeft / certainBlindnessTimeThresh;

			// reduce alpha level quicker with dx 8 support and higher to compensate
			// for having the burn-in effect.
			flAlphaPercentage *= flAlphaPercentage;
		}

		m_flFlashOverlayAlpha = flAlphaPercentage *= m_flFlashMaxAlpha; // scale a [0..1) value to a [0..MaxAlpha] value for the alpha.

		// make sure the alpha is in the range of [0..MaxAlpha]
		m_flFlashOverlayAlpha = Max( m_flFlashOverlayAlpha, 0.0f );
		m_flFlashOverlayAlpha = Min( m_flFlashOverlayAlpha, m_flFlashMaxAlpha.Get());
	}
}

void RecvProxy_HasDefuser( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	C_CSPlayer *pPlayerData = (C_CSPlayer *)pStruct;

	if (pPlayerData == NULL)
	{
		return;
	}

	bool drawIcon = false;

	if (pData->m_Value.m_Int == 0)
	{
		pPlayerData->RemoveDefuser();
	}
	else
	{
		if (pPlayerData->HasDefuser() == false)
		{
			drawIcon = true;
		}
		pPlayerData->GiveDefuser();
	}
}

void C_CSPlayer::RecvProxy_CycleLatch( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	// This receive proxy looks to see if the server's value is close enough to what we think it should
	// be.  We've been running the same code; this is an error correction for changes we didn't simulate
	// while they were out of PVS.
	C_CSPlayer *pPlayer = (C_CSPlayer *)pStruct;
	if( pPlayer->IsLocalPlayer() )
		return; // Don't need to fixup ourselves.

	float incomingCycle = (float)(pData->m_Value.m_Int) / 16; // Came in as 4 bit fixed point
	float currentCycle = pPlayer->GetCycle();
	bool closeEnough = fabs(currentCycle - incomingCycle) < CycleLatchTolerance;
	if( fabs(currentCycle - incomingCycle) > (1 - CycleLatchTolerance) )
	{
		closeEnough = true;// Handle wrapping around 1->0
	}

	if( !closeEnough )
	{
		// Server disagrees too greatly.  Correct our value.
		if ( pPlayer && pPlayer->GetTeam() )
		{
			DevMsg( 2, "%s %s(%d): Cycle latch wants to correct %.2f in to %.2f.\n",
				pPlayer->GetTeam()->Get_Name(), pPlayer->GetPlayerName(), pPlayer->entindex(), currentCycle, incomingCycle );
		}
		pPlayer->SetServerIntendedCycle( incomingCycle );
	}
}

void __MsgFunc_ReloadEffect( bf_read &msg )
{
	int iPlayer = msg.ReadShort();
	C_CSPlayer *pPlayer = dynamic_cast< C_CSPlayer* >( C_BaseEntity::Instance( iPlayer ) );
	if ( pPlayer )
		pPlayer->PlayReloadEffect();

}
USER_MESSAGE_REGISTER( ReloadEffect );

BEGIN_RECV_TABLE_NOBASE( C_CSPlayer, DT_CSLocalPlayerExclusive )
	RecvPropFloat( RECVINFO(m_flStamina) ),
	RecvPropInt( RECVINFO( m_iDirection ) ),
	RecvPropInt( RECVINFO( m_iShotsFired ) ),
	RecvPropFloat( RECVINFO( m_flVelocityModifier ) ),
	RecvPropBool( RECVINFO( m_bDuckOverride ) ),
	RecvPropBool( RECVINFO( m_bIsHoldingLookAtWeapon ) ),
	RecvPropBool( RECVINFO( m_bIsLookingAtWeapon ) ),

	RecvPropVector( RECVINFO_NAME( m_vecNetworkOrigin, m_vecOrigin ) ),

    //=============================================================================
    // HPE_BEGIN:
    // [tj]Set up the receive table for per-client domination data
    //=============================================================================

    RecvPropArray3( RECVINFO_ARRAY( m_bPlayerDominated ), RecvPropBool( RECVINFO( m_bPlayerDominated[0] ) ) ),
    RecvPropArray3( RECVINFO_ARRAY( m_bPlayerDominatingMe ), RecvPropBool( RECVINFO( m_bPlayerDominatingMe[0] ) ) )

    //=============================================================================
    // HPE_END
    //=============================================================================

END_RECV_TABLE()


BEGIN_RECV_TABLE_NOBASE( C_CSPlayer, DT_CSNonLocalPlayerExclusive )
	RecvPropVector( RECVINFO_NAME( m_vecNetworkOrigin, m_vecOrigin ) ),
END_RECV_TABLE()


IMPLEMENT_CLIENTCLASS_DT( C_CSPlayer, DT_CSPlayer, CCSPlayer )
	RecvPropDataTable( "cslocaldata", 0, 0, &REFERENCE_RECV_TABLE(DT_CSLocalPlayerExclusive) ),
	RecvPropDataTable( "csnonlocaldata", 0, 0, &REFERENCE_RECV_TABLE(DT_CSNonLocalPlayerExclusive) ),
	RecvPropInt( RECVINFO( m_iAddonBits ) ),
	RecvPropInt( RECVINFO( m_iPrimaryAddon ) ),
	RecvPropInt( RECVINFO( m_iSecondaryAddon ) ),
	RecvPropInt( RECVINFO( m_iKnifeAddon ) ),
	RecvPropInt( RECVINFO( m_iThrowGrenadeCounter ) ),
	RecvPropInt( RECVINFO( m_iPlayerState ) ),
	RecvPropInt( RECVINFO( m_iAccount ) ),
	RecvPropBool( RECVINFO( m_bInBombZone ) ),
	RecvPropInt( RECVINFO( m_bInBuyZone ) ),
	RecvPropInt( RECVINFO( m_bInNoDefuseArea ) ),
	RecvPropBool( RECVINFO( m_bKilledByTaser ) ),
	RecvPropInt( RECVINFO( m_iMoveState ) ),
	RecvPropInt( RECVINFO( m_iClass ) ),
	RecvPropInt( RECVINFO( m_ArmorValue ) ),
	RecvPropFloat( RECVINFO( m_angEyeAngles[0] ) ),
	RecvPropFloat( RECVINFO( m_angEyeAngles[1] ) ),
	RecvPropFloat( RECVINFO( m_flStamina ) ),
	RecvPropInt( RECVINFO( m_bHasDefuser ), 0, RecvProxy_HasDefuser ),
	RecvPropInt( RECVINFO( m_bNightVisionOn), 0, RecvProxy_NightVision ),
	RecvPropBool( RECVINFO( m_bHasNightVision ) ),
	RecvPropBool( RECVINFO( m_bIsGrabbingHostage ) ),
	RecvPropEHandle( RECVINFO( m_hCarriedHostage ) ),
	RecvPropEHandle( RECVINFO( m_hCarriedHostageProp ) ),
	RecvPropBool( RECVINFO( m_bIsWalking ) ),
	RecvPropFloat( RECVINFO( m_flGroundAccelLinearFracLastTime ) ),


    //=============================================================================
    // HPE_BEGIN:
    // [dwenger] Added for fun-fact support
    //=============================================================================

    //RecvPropBool( RECVINFO( m_bPickedUpDefuser ) ),
    //RecvPropBool( RECVINFO( m_bDefusedWithPickedUpKit ) ),

    //=============================================================================
    // HPE_END
    //=============================================================================

    RecvPropBool( RECVINFO( m_bInHostageRescueZone ) ),
	RecvPropInt( RECVINFO( m_ArmorValue ) ),
	RecvPropBool( RECVINFO( m_bIsDefusing ) ),
	RecvPropBool( RECVINFO( m_bResumeZoom ) ),
	RecvPropFloat( RECVINFO( m_fImmuneToDamageTime ) ),
	RecvPropBool( RECVINFO( m_bImmunity ) ),
	RecvPropBool( RECVINFO( m_bHasMovedSinceSpawn ) ),
	RecvPropInt( RECVINFO( m_iLastZoom ) ),

#ifdef CS_SHIELD_ENABLED
	RecvPropBool( RECVINFO( m_bHasShield ) ),
	RecvPropBool( RECVINFO( m_bShieldDrawn ) ),
#endif
	RecvPropInt( RECVINFO( m_bHasHelmet ) ),
	RecvPropVector( RECVINFO( m_vecRagdollVelocity ) ),
	RecvPropFloat( RECVINFO( m_flFlashDuration ), 0, RecvProxy_FlashTime ),
	RecvPropFloat( RECVINFO( m_flFlashMaxAlpha)),
	RecvPropInt( RECVINFO( m_iProgressBarDuration ) ),
	RecvPropFloat( RECVINFO( m_flProgressBarStartTime ) ),
	RecvPropEHandle( RECVINFO( m_hRagdoll ) ),
	RecvPropInt( RECVINFO( m_cycleLatch ), 0, &C_CSPlayer::RecvProxy_CycleLatch ),

#if CS_CONTROLLABLE_BOTS_ENABLED
	RecvPropBool( RECVINFO( m_bIsControllingBot ) ),
	RecvPropBool( RECVINFO( m_bHasControlledBotThisRound ) ),
	RecvPropBool( RECVINFO( m_bCanControlObservedBot ) ),
	RecvPropInt( RECVINFO( m_iControlledBotEntIndex ) ),
#endif

	RecvPropBool( RECVINFO( m_bNeedToChangeGloves ) ),
	RecvPropInt( RECVINFO( m_iLoadoutSlotGlovesCT ) ),
	RecvPropInt( RECVINFO( m_iLoadoutSlotGlovesT ) ),
	RecvPropInt( RECVINFO( m_iLoadoutSlotKnifeWeaponCT ) ),
	RecvPropInt( RECVINFO( m_iLoadoutSlotKnifeWeaponT ) ),

END_RECV_TABLE()



C_CSPlayer::C_CSPlayer() :
	m_iv_angEyeAngles( "C_CSPlayer::m_iv_angEyeAngles" )
{
	m_PlayerAnimState = CreatePlayerAnimState( this, this, LEGANIM_9WAY, true );

	m_flThirdpersonRecoil = 0;

	m_angEyeAngles.Init();

	AddVar( &m_angEyeAngles, &m_iv_angEyeAngles, LATCH_SIMULATION_VAR );

	m_iLastAddonBits = m_iAddonBits = 0;
	m_iLastPrimaryAddon = m_iLastSecondaryAddon = WEAPON_NONE;
	m_iProgressBarDuration = 0;
	m_flProgressBarStartTime = 0.0f;
	m_ArmorValue = 0;
	m_bHasHelmet = false;
	m_iIDEntIndex = 0;
	m_delayTargetIDTimer.Reset();
	m_iOldIDEntIndex = 0;
	m_holdTargetIDTimer.Reset();
	m_iDirection = 0;

	m_Activity = ACT_IDLE;

	m_pFlashlightBeam = NULL;
	m_fNextThinkPushAway = 0.0f;

	m_serverIntendedCycle = -1.0f;

	view->SetScreenOverlayMaterial( NULL );

	m_iTargetedWeaponEntIndex = 0;

	m_vecFreezeFrameEnd = Vector( 0, 0, 0 );
	m_flFreezeFrameTilt = 0;
	m_bFreezeFrameCloseOnKiller = false;
	m_nFreezeFrameShiftSideDist = 0;

    m_bPlayingFreezeCamSound = false;

	m_nextTaserShakeTime = 0.0f;
	m_firstTaserShakeTime = 0.0f;
	m_bKilledByTaser = false;

	m_bShouldAutobuyDMWeapons = false;

	ListenForGameEvent( "round_start" );
	ListenForGameEvent( "item_pickup" );
	ListenForGameEvent( "cs_pre_restart" );
	ListenForGameEvent( "player_death" );
	ListenForGameEvent( "player_spawn" );
	ListenForGameEvent( "player_update_viewmodel" );

	m_bPlayingHostageCarrySound = false;

	m_iMoveState = MOVESTATE_IDLE;

	m_flFlashScreenshotAlpha = 0.0f;
	m_flFlashOverlayAlpha = 0.0f;
	m_bFlashBuildUp = false;
	m_bFlashScreenshotHasBeenGrabbed = false;
	m_bFlashDspHasBeenCleared = true;

	SetCurrentMusic( CSMUSIC_NONE );
	m_flMusicRoundStartTime = 0.0;
	m_vecObserverInterpolateOffset = vec3_origin;
	m_bObserverInterpolationNeedsDeferredSetup = false;
	m_flObsInterp_PathLength = 0.0f;
	m_obsInterpState = OBSERVER_INTERP_NONE;
	m_qObsInterp_OrientationStart = m_qObsInterp_OrientationTravelDir = Quaternion( 0, 0, 0, 0 );

	m_flNextMagDropTime = 0;
	m_nLastMagDropAttachmentIndex = -1;

	m_vecLastAliveLocalVelocity.Init();

	m_pViewmodelArmConfig = NULL;
}


C_CSPlayer::~C_CSPlayer()
{
	RemoveAddonModels();

	ReleaseFlashlight();

	RemoveGlovesModel();

	m_PlayerAnimState->Release();
}


class CTraceFilterOmitPlayers : public CTraceFilterSimple
{
public:
	CTraceFilterOmitPlayers( const IHandleEntity *passentity = NULL, int collisionGroup = MASK_SHOT )
		: CTraceFilterSimple( passentity, collisionGroup )
	{
	}

	virtual bool ShouldHitEntity( IHandleEntity *pHandleEntity, int contentsMask )
	{
		CBaseEntity *pEntity = EntityFromEntityHandle( pHandleEntity );
		if ( !pEntity )
			return NULL;

		if ( pEntity->IsPlayer() )
			return false;

		// Honor BlockLOS - this lets us see through partially-broken doors, etc
		if ( !pEntity->BlocksLOS() )
			return false;

		return CTraceFilterSimple::ShouldHitEntity( pHandleEntity, contentsMask );
	}
};

bool C_CSPlayer::HasDefuser() const
{
	return m_bHasDefuser;
}

void C_CSPlayer::GiveDefuser()
{
	m_bHasDefuser = true;
}

void C_CSPlayer::RemoveDefuser()
{
	m_bHasDefuser = false;
}

bool C_CSPlayer::HasNightVision() const
{
	return m_bHasNightVision;
}

bool C_CSPlayer::IsVIP() const
{
	C_CS_PlayerResource *pCSPR = (C_CS_PlayerResource*)GameResources();

	if ( !pCSPR )
		return false;

	return pCSPR->IsVIP( entindex() );
}

C_CSPlayer* C_CSPlayer::GetLocalCSPlayer()
{
	return (C_CSPlayer*)C_BasePlayer::GetLocalPlayer();
}


CSPlayerState C_CSPlayer::State_Get() const
{
	return m_iPlayerState;
}


float C_CSPlayer::GetMinFOV() const
{
	// Min FOV for AWP.
	return 10;
}


int C_CSPlayer::GetAccount() const
{
	return m_iAccount;
}


int C_CSPlayer::PlayerClass() const
{
	return m_iClass;
}

bool C_CSPlayer::CanShowTeamMenu() const
{
	return true;
}


int C_CSPlayer::ArmorValue() const
{
	return m_ArmorValue;
}

bool C_CSPlayer::HasHelmet() const
{
	return m_bHasHelmet;
}

int C_CSPlayer::GetCurrentAssaultSuitPrice()
{
	// WARNING: This price logic also exists in CCSPlayer::AttemptToBuyAssaultSuit
	// and must be kept in sync if changes are made.

	int fullArmor = ArmorValue() >= 100 ? 1 : 0;
	if ( fullArmor && !HasHelmet() )
	{
		return HELMET_PRICE;
	}
	else if ( !fullArmor && HasHelmet() )
	{
		return KEVLAR_PRICE;
	}
	else
	{
		// NOTE: This applies to the case where you already have both
		// as well as the case where you have neither.  In the case
		// where you have both, the item should still have a price
		// and become disabled when you have little or no money left.
		return ASSAULTSUIT_PRICE;
	}
}

const QAngle& C_CSPlayer::GetRenderAngles()
{
	if ( IsRagdoll() )
	{
		return vec3_angle;
	}
	else
	{
		return m_PlayerAnimState->GetRenderAngles();
	}
}


float g_flFattenAmt = 4;
void C_CSPlayer::GetShadowRenderBounds( Vector &mins, Vector &maxs, ShadowType_t shadowType )
{
	if ( shadowType == SHADOWS_SIMPLE )
	{
		// Don't let the render bounds change when we're using blobby shadows, or else the shadow
		// will pop and stretch.
		mins = CollisionProp()->OBBMins();
		maxs = CollisionProp()->OBBMaxs();
	}
	else
	{
		GetRenderBounds( mins, maxs );

		// We do this because the normal bbox calculations don't take pose params into account, and
		// the rotation of the guy's upper torso can place his gun a ways out of his bbox, and
		// the shadow will get cut off as he rotates.
		//
		// Thus, we give it some padding here.
		mins -= Vector( g_flFattenAmt, g_flFattenAmt, 0 );
		maxs += Vector( g_flFattenAmt, g_flFattenAmt, 0 );
	}
}


void C_CSPlayer::GetRenderBounds( Vector& theMins, Vector& theMaxs )
{
	// TODO POSTSHIP - this hack/fix goes hand-in-hand with a fix in CalcSequenceBoundingBoxes in utils/studiomdl/simplify.cpp.
	// When we enable the fix in CalcSequenceBoundingBoxes, we can get rid of this.
	//
	// What we're doing right here is making sure it only uses the bbox for our lower-body sequences since,
	// with the current animations and the bug in CalcSequenceBoundingBoxes, are WAY bigger than they need to be.
	C_BaseAnimating::GetRenderBounds( theMins, theMaxs );

	// If we're ducking, we should reduce the render height by the difference in standing and ducking heights.
	// This prevents shadows from drawing above ducking players etc.
	if ( GetFlags() & FL_DUCKING )
	{
		theMaxs.z -= 18.5f;
	}
}


bool C_CSPlayer::GetShadowCastDirection( Vector *pDirection, ShadowType_t shadowType ) const
{
	if ( shadowType == SHADOWS_SIMPLE )
	{
		// Blobby shadows should sit directly underneath us.
		pDirection->Init( 0, 0, -1 );
		return true;
	}
	else
	{
		return BaseClass::GetShadowCastDirection( pDirection, shadowType );
	}
}


void C_CSPlayer::VPhysicsUpdate( IPhysicsObject *pPhysics )
{
	BaseClass::VPhysicsUpdate( pPhysics );
}


int C_CSPlayer::GetIDTarget() const
{
	if ( !m_delayTargetIDTimer.IsElapsed() )
		return 0;

	if ( m_iIDEntIndex )
	{
		return m_iIDEntIndex;
	}

	if ( m_iOldIDEntIndex && !m_holdTargetIDTimer.IsElapsed() )
	{
		return m_iOldIDEntIndex;
	}

	return 0;
}

int C_CSPlayer::GetTargetedWeapon( void ) const
{
	return m_iTargetedWeaponEntIndex;
}


void InitializeAddonModelFromWeapon( CWeaponCSBase *weapon, C_BreakableProp *addon )
{
	if ( !weapon )
	{
		return;
	}

	const CCSWeaponInfo& weaponInfo = weapon->GetCSWpnData();
	if ( weaponInfo.m_szAddonModel[0] == 0 )
	{
		addon->InitializeAsClientEntity( weaponInfo.szWorldModel, RENDER_GROUP_OPAQUE_ENTITY );
	}
	else
	{
		addon->InitializeAsClientEntity( weaponInfo.m_szAddonModel, RENDER_GROUP_OPAQUE_ENTITY );
	}
}

class C_PlayerAddonModel : public C_BreakableProp
{
public:
	virtual const Vector& GetAbsOrigin( void ) const
	{
		// if the player carrying this addon is in lod state (meaning outside the camera frustum)
		// we don't need to set up all the player's attachment bones just to find out where exactly
		// the addon model wants to render. Just return the player's origin.

		CBaseEntity *pMoveParent = GetMoveParent();

		if ( pMoveParent && pMoveParent->IsPlayer() )
		{
			C_CSPlayer *pCSPlayer = static_cast<C_CSPlayer *>( pMoveParent );

			if ( pCSPlayer && ( pCSPlayer->IsDormant() || !pCSPlayer->IsVisible() ) )
				return pCSPlayer->GetAbsOrigin();

		}

		return BaseClass::GetAbsOrigin();
	}

	virtual bool ShouldDraw()
	{
		CBaseEntity *pMoveParent = GetMoveParent();
		if ( pMoveParent && pMoveParent->IsPlayer() )
		{
			C_CSPlayer *pCSPlayer = static_cast<C_CSPlayer *>( pMoveParent );
			if ( pCSPlayer && ( pCSPlayer->IsDormant() || !pCSPlayer->IsVisible() ) )
				return false;
		}

		return BaseClass::ShouldDraw();
	}

	virtual bool IsFollowingEntity()
	{
		// addon models are ALWAYS following players
		return true;
	}

};

void C_CSPlayer::CreateAddonModel( int i )
{
	COMPILE_TIME_ASSERT( (sizeof( g_AddonInfo ) / sizeof( g_AddonInfo[0] )) == NUM_ADDON_BITS );

	// Create the model entity.
	CAddonInfo *pAddonInfo = &g_AddonInfo[i];

	int iAttachment = LookupAttachment( pAddonInfo->m_pAttachmentName );
	float iScale = 1;

	C_PlayerAddonModel *pEnt = new C_PlayerAddonModel;

	int addonType = (1 << i);
	if ( addonType == ADDON_PISTOL || addonType == ADDON_PRIMARY || addonType == ADDON_KNIFE )
	{
		CCSWeaponInfo *weaponInfo;
		if ( addonType == ADDON_PRIMARY )
			weaponInfo = GetWeaponInfo( (CSWeaponID) m_iPrimaryAddon.Get() );
		else if ( addonType == ADDON_PISTOL )
			weaponInfo = GetWeaponInfo( (CSWeaponID) m_iSecondaryAddon.Get() );
		else
			weaponInfo = GetWeaponInfo( (CSWeaponID) m_iKnifeAddon.Get() );

		if ( !weaponInfo )
		{
			Warning( "C_CSPlayer::CreateAddonModel: Unable to get weapon info.\n" );
			pEnt->Release();
			return;
		}
		if ( weaponInfo->m_szAddonModel[0] == 0 )
		{
			pEnt->InitializeAsClientEntity( weaponInfo->szWorldModel, RENDER_GROUP_OPAQUE_ENTITY );
		}
		else
		{
			pEnt->InitializeAsClientEntity( weaponInfo->m_szAddonModel, RENDER_GROUP_OPAQUE_ENTITY );
		}

		// check if there's a special attachment specified in weapon config
		if ( weaponInfo->m_szAddonLocation[0] != 0 )
		{
			int iNewAttachment = LookupAttachment( weaponInfo->m_szAddonLocation );

			// does this special attachment exist?
			if ( iNewAttachment > 0 )
				iAttachment = iNewAttachment;
		}

		iScale = weaponInfo->m_flAddonScale;
	}
	else if( pAddonInfo->m_pModelName )
	{
		if ( addonType == ADDON_PISTOL2 && !(m_iAddonBits & ADDON_PISTOL ) )
		{
			pEnt->InitializeAsClientEntity( pAddonInfo->m_pHolsterName, RENDER_GROUP_OPAQUE_ENTITY );
		}
		else
		{
			pEnt->InitializeAsClientEntity( pAddonInfo->m_pModelName, RENDER_GROUP_OPAQUE_ENTITY );
		}
	}
	else
	{
		WEAPON_FILE_INFO_HANDLE	hWpnInfo = LookupWeaponInfoSlot( pAddonInfo->m_pWeaponClassName );
		if ( hWpnInfo == GetInvalidWeaponInfoHandle() )
		{
			Assert( false );
			return;
		}

		CCSWeaponInfo *pWeaponInfo = dynamic_cast< CCSWeaponInfo* >( GetFileWeaponInfoFromHandle( hWpnInfo ) );
		if ( pWeaponInfo )
		{
			if ( pWeaponInfo->m_szAddonModel[0] == 0 )
				pEnt->InitializeAsClientEntity( pWeaponInfo->szWorldModel, RENDER_GROUP_OPAQUE_ENTITY );
			else
				pEnt->InitializeAsClientEntity( pWeaponInfo->m_szAddonModel, RENDER_GROUP_OPAQUE_ENTITY );
		}
		else
		{
			pEnt->Release();
			Warning( "C_CSPlayer::CreateAddonModel: Unable to get weapon info for %s.\n", pAddonInfo->m_pWeaponClassName );
			return;
		}
	}

	if ( iAttachment <= 0 )
		return;

	if ( Q_strcmp( pAddonInfo->m_pAttachmentName, "c4" ) )
	{
		// fade out all attached models except C4
		pEnt->SetFadeMinMax( 400, 500 );

		pEnt->SetBodygroup( pEnt->FindBodygroupByName( "gift" ), UTIL_IsNewYear() );
	}

	// Create the addon.
	CAddonModel *pAddon = &m_AddonModels[m_AddonModels.AddToTail()];

	pAddon->m_hEnt = pEnt;
	pAddon->m_iAddon = i;
	pAddon->m_iAttachmentPoint = iAttachment;
	pEnt->SetParent( this, pAddon->m_iAttachmentPoint );
	pEnt->SetLocalOrigin( Vector( 0, 0, 0 ) );
	pEnt->SetLocalAngles( QAngle( 0, 0, 0 ) );
	pEnt->SetModelScale( iScale );
	if ( IsLocalPlayer() )
	{
		pEnt->SetSolid( SOLID_NONE );
		pEnt->RemoveEFlags( EFL_USE_PARTITION_WHEN_NOT_SOLID );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : bThirdperson - 
//-----------------------------------------------------------------------------
void C_CSPlayer::ThirdPersonSwitch( bool bThirdperson )
{
	BaseClass::ThirdPersonSwitch( bThirdperson );

	if ( m_hCarriedHostageProp != NULL )
	{
		C_HostageCarriableProp *pHostageProp = static_cast< C_HostageCarriableProp* >( m_hCarriedHostageProp.Get() );
		if ( pHostageProp )
		{
			UpdateHostageCarryModels();
		}
	}
}

void C_CSPlayer::CalcView( Vector &eyeOrigin, QAngle &eyeAngles, float &zNear, float &zFar, float &fov )
{
	BaseClass::CalcView( eyeOrigin, eyeAngles, zNear, zFar, fov );

#if IRONSIGHT
	CWeaponCSBase *pWeapon = GetActiveCSWeapon();
	if (pWeapon)
	{
		CIronSightController* pIronSightController = pWeapon->GetIronSightController();
		if (pIronSightController)
		{
			//bias the local client FOV change so ironsight transitions are nicer
			fov = pIronSightController->GetIronSightFOV(GetDefaultFOV(), true);
		}
	}
#endif //IRONSIGHT
}

void C_CSPlayer::UpdateHostageCarryModels()
{
	if ( m_hCarriedHostage )
	{
		if ( m_hCarriedHostageProp != NULL )
		{
			C_HostageCarriableProp *pHostageProp = static_cast< C_HostageCarriableProp* >(m_hCarriedHostageProp.Get());
			if ( pHostageProp )
			{
				pHostageProp->UpdateVisibility();
			}
		}

		C_BaseViewModel *pViewModel = assert_cast<C_BaseViewModel *>(GetViewModel( HOSTAGE_VIEWMODEL ));
		if ( pViewModel )
		{
			pViewModel->UpdateVisibility();
		}
	}

}

void C_CSPlayer::UpdateAddonModels()
{
	int iCurAddonBits = m_iAddonBits;

	// Don't put addon models on the local player unless in third person.
	if ( IsLocalPlayer() && !C_BasePlayer::ShouldDrawLocalPlayer() )
		iCurAddonBits = 0;

	// If the local player is observing this entity in first-person mode, get rid of its addons.
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if ( pPlayer && pPlayer->GetObserverMode() == OBS_MODE_IN_EYE && pPlayer->GetObserverTarget() == this )
		iCurAddonBits = 0;

	// Any changes to the attachments we should have?
	if ( m_iLastAddonBits == iCurAddonBits &&
		m_iLastPrimaryAddon == m_iPrimaryAddon &&
		m_iLastSecondaryAddon == m_iSecondaryAddon &&
		m_iLastKnifeAddon == m_iKnifeAddon )
	{
		return;
	}

	bool rebuildPistol2Addon = false;
	if ( m_iSecondaryAddon == WEAPON_ELITE && ((m_iLastAddonBits ^ iCurAddonBits) & ADDON_PISTOL) != 0 )
	{
		rebuildPistol2Addon = true;
	}
	m_iLastAddonBits = iCurAddonBits;
	m_iLastPrimaryAddon = m_iPrimaryAddon;
	m_iLastSecondaryAddon = m_iSecondaryAddon;
	m_iLastKnifeAddon = m_iKnifeAddon;

	// Get rid of any old models.
	int i,iNext;
	for ( i=m_AddonModels.Head(); i != m_AddonModels.InvalidIndex(); i = iNext )
	{
		iNext = m_AddonModels.Next( i );
		CAddonModel *pModel = &m_AddonModels[i];

		int addonBit = 1<<pModel->m_iAddon;
		if ( !( iCurAddonBits & addonBit ) || (rebuildPistol2Addon && addonBit == ADDON_PISTOL2) )
		{
			if ( pModel->m_hEnt.Get() )
				pModel->m_hEnt->Release();

			m_AddonModels.Remove( i );
		}
	}

	// Figure out which models we have now.
	int curModelBits = 0;
	FOR_EACH_LL( m_AddonModels, j )
	{
		curModelBits |= (1<<m_AddonModels[j].m_iAddon);
	}

	// Add any new models.
	for ( i=0; i < NUM_ADDON_BITS; i++ )
	{
		if ( (iCurAddonBits & (1<<i)) && !( curModelBits & (1<<i) ) )
		{
			// Ok, we're supposed to have this one.
			CreateAddonModel( i );
		}
	}
}


void C_CSPlayer::UpdateGlovesModel()
{
	if ( !DoesModelSupportGloves() || !CSLoadout()->HasGlovesSet( this, GetTeamNumber() ) || !IsAlive() )
	{
		RemoveGlovesModel();
		return;
	}

	const char *pszGlovesModel = GetGlovesInfo( CSLoadout()->GetGlovesForPlayer( this, GetTeamNumber() ) )->szWorldModel;
	if ( !m_pCSGloves )
	{
		m_pCSGloves = new CBaseCSGloves( pszGlovesModel );
		m_pCSGloves->Equip( this );
	}
	
	const char *pszModelName = m_pCSGloves->GetModelName();
	if ( pszModelName && pszModelName[0] )
	{
		if ( V_stricmp( STRING( pszModelName ), pszGlovesModel ) != 0 )
		{
			m_pCSGloves->UpdateGlovesModel();
		}
	}
}


void C_CSPlayer::RemoveAddonModels()
{
	m_iAddonBits = 0;
	UpdateAddonModels();
}


void C_CSPlayer::RemoveGlovesModel()
{
	if ( m_pCSGloves )
	{
		m_pCSGloves->UnEquip();
		m_pCSGloves = NULL;
	}
}


void C_CSPlayer::FireGameEvent( IGameEvent *event )
{
	const char *name = event->GetName();
	C_BasePlayer *pLocalPlayer = C_BasePlayer::GetLocalPlayer();

	int EventUserID = event->GetInt( "userid", -1 );
	//int LocalPlayerID = ( pLocalPlayer != NULL ) ? pLocalPlayer->GetUserID() : -2;
	//int PlayerUserID = GetUserID();

	if ( Q_strcmp( "item_pickup", name ) == 0 )
	{
		if ( /*Q_strcmp( event->GetString( "item" ), "c4" ) == 0 &&*/
			 (pLocalPlayer && pLocalPlayer->GetUserID() == EventUserID) )
		{
			// if we aren't playing the sound on the server, play a "silent" version on the client
			if ( event->GetBool( "silent" ) )
				EmitSound( "Player.PickupWeaponSilent" );
		}
	}
	else if ( Q_strcmp( "round_start", name ) == 0 )
	{
		if ( IsLocalPlayer() && CSGameRules() && CSGameRules()->GetGamemode() == GameModes::DEATHMATCH )
			m_bShouldAutobuyDMWeapons = true;
	}
	else if ( Q_strcmp( name, "cs_pre_restart" ) == 0 )
	{
		if ( ( this->GetTeamNumber() == TEAM_SPECTATOR ) || ( this->IsLocalPlayer() ) )
		{
			CLocalPlayerFilter filter;
			PlayMusicSelection( filter, CSMUSIC_START );
		}

		SetCurrentMusic( CSMUSIC_START );
	}
	else if ( (Q_strcmp( "bot_takeover", name ) == 0 || Q_strcmp( "spec_target_updated", name ) == 0) && GetUserID() == EventUserID )
	{
		C_CSPlayer *pLocalCSPlayer = C_CSPlayer::GetLocalCSPlayer();
		if (pLocalCSPlayer)
		{
			// FlashBang effect needs to update it's screenshot
			pLocalCSPlayer->m_bFlashScreenshotHasBeenGrabbed = false;
			
			if ( pLocalPlayer->GetObserverMode() != OBS_MODE_NONE && pLocalPlayer->GetUserID() == EventUserID )
			{
				C_CSPlayer *pFlashBangPlayer = GetHudPlayer();
				if ( pFlashBangPlayer )
				{
					pFlashBangPlayer->m_bFlashScreenshotHasBeenGrabbed = false;
				}
			}
		}
	}
	else if ( Q_strcmp( "player_death", name ) == 0 )
	{
		C_CSPlayer* csPlayer = ToCSPlayer( UTIL_PlayerByUserId( EventUserID ) );
		if (csPlayer)
		{
			if ( csPlayer->IsLocalPlayer() )
			{
				//reset target ID 
				m_iIDEntIndex = 0;
				m_delayTargetIDTimer.Reset();
				m_iOldIDEntIndex = 0;
				m_holdTargetIDTimer.Reset();
				m_iTargetedWeaponEntIndex = 0;

				C_RecipientFilter filter;
				filter.AddRecipient( this );
				PlayMusicSelection( filter, CSMUSIC_DEATHCAM );
			}

			csPlayer->RemoveGlovesModel();
		}
	}
	else if ( Q_strcmp( "player_spawn", name ) == 0 )
	{
		if ( pLocalPlayer && pLocalPlayer->GetUserID() == EventUserID )
		{
			// we've just spawned, so reset our entity id stuff
			m_iIDEntIndex = 0;
			m_delayTargetIDTimer.Reset();
			m_iOldIDEntIndex = 0;
			m_holdTargetIDTimer.Reset();
			m_iTargetedWeaponEntIndex = 0;

			RemoveAddonModels();

			if ( IsLocalPlayer() && CSGameRules() && CSGameRules()->GetGamemode() == GameModes::DEATHMATCH )
				m_bShouldAutobuyDMWeapons = true;
		}

		C_CSPlayer* csPlayer = ToCSPlayer( UTIL_PlayerByUserId( EventUserID ) );
		if ( csPlayer )
		{
			csPlayer->RemoveGlovesModel();
			csPlayer->m_pViewmodelArmConfig = NULL;
		}

	}
	else if ( Q_strcmp( "player_update_viewmodel", name ) == 0 )
	{
		if ( pLocalPlayer && pLocalPlayer->GetUserID() == EventUserID )
		{
			m_pViewmodelArmConfig = NULL;
		}
	}
}

CON_COMMAND_F( dm_togglerandomweapons, "Turns random weapons in deathmatch on/off", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_SERVER_CAN_EXECUTE )
{
	C_BasePlayer *pLocalPlayer = C_BasePlayer::GetLocalPlayer();
	C_CSPlayer* pPlayer = ToCSPlayer(pLocalPlayer);
	if ( pPlayer )
		pPlayer->ToggleRandomWeapons();
}

void C_CSPlayer::ToggleRandomWeapons( void )
{
	ConVarRef cl_dm_buyrandomweapons( "cl_dm_buyrandomweapons" );
	float flTimeLeft = m_fImmuneToDamageTime - gpGlobals->curtime;
	if ( cl_dm_buyrandomweapons.GetBool() )
	{
		cl_dm_buyrandomweapons.SetValue(false);
		if ( flTimeLeft <= 0 )
		{
			HintMessage( "#Cstrike_TitlesTXT_DM_RandomOFF" ); // PiMoN: better to use ClientPrint but who cares you cant even display any hints from client
		}
	}
	else
	{
		cl_dm_buyrandomweapons.SetValue(true);
		if ( flTimeLeft <= 0 )
		{
			HintMessage( "#Cstrike_TitlesTXT_DM_RandomON" ); // PiMoN: better to use ClientPrint but who cares you cant even display any hints from client
		}
		engine->ClientCmd_Unrestricted( "buyrandom" );
	}

	CLocalPlayerFilter filter;
	EmitSound( filter, GetSoundSourceIndex(), "BuyPreset.Updated" );
}


void C_CSPlayer::NotifyShouldTransmit( ShouldTransmitState_t state )
{
	// Remove all addon models if we go out of the PVS.
	if ( state == SHOULDTRANSMIT_END )
	{
		RemoveAddonModels();

		RemoveGlovesModel();

		if( m_pFlashlightBeam != NULL )
		{
			ReleaseFlashlight();
		}
	}

	BaseClass::NotifyShouldTransmit( state );
}


void C_CSPlayer::UpdateSoundEvents()
{
	int iNext;
	for ( int i=m_SoundEvents.Head(); i != m_SoundEvents.InvalidIndex(); i = iNext )
	{
		iNext = m_SoundEvents.Next( i );

		CCSSoundEvent *pEvent = &m_SoundEvents[i];
		if ( gpGlobals->curtime >= pEvent->m_flEventTime )
		{
			CLocalPlayerFilter filter;
			EmitSound( filter, GetSoundSourceIndex(), STRING( pEvent->m_SoundName ) );

			m_SoundEvents.Remove( i );
		}
	}
}

//-----------------------------------------------------------------------------
void C_CSPlayer::UpdateMinModels( void )
{
	SetModelByIndex( m_nModelIndex );
}

// NVNT gate for spectating.
static bool inSpectating_Haptics = false;
//-----------------------------------------------------------------------------
void C_CSPlayer::ClientThink()
{
	if ( IsAlive() )
	{
		m_vecLastAliveLocalVelocity = (m_vecLastAliveLocalVelocity * 0.8) + (GetLocalVelocity() * 0.2);
	}

	BaseClass::ClientThink();

	// velocity music handling
	if( GetCurrentMusic() == CSMUSIC_START && GetMusicStartRoundElapsed() > 0.5 )
	{
		Vector vAbsVelocity = GetAbsVelocity();
		float flAbsVelocity = vAbsVelocity.Length2D();
		if( flAbsVelocity > 10 )
		{
			if( this == GetHudPlayer() )
			{
				CLocalPlayerFilter filter;
				PlayMusicSelection( filter, CSMUSIC_ACTION );
			}
			SetCurrentMusic( CSMUSIC_ACTION );
		}
	}

	UpdateSoundEvents();

	UpdateAddonModels();

	UpdateGlovesModel();

	UpdateFlashBangEffect();

	UpdateHostageCarryModels();

	// don't show IDs in chase spec mode
	bool inSpecMode = ( GetObserverMode() == OBS_MODE_CHASE || GetObserverMode() == OBS_MODE_DEATHCAM );

	if ( IsLocalPlayer() && !inSpecMode && IsAlive() && ( mp_forcecamera.GetInt() != OBS_ALLOW_NONE ) )
	{
		UpdateIDTarget();
		UpdateTargetedWeapon();
	}

	if ( gpGlobals->curtime >= m_fNextThinkPushAway )
	{
		PerformObstaclePushaway( this );
		m_fNextThinkPushAway =  gpGlobals->curtime + PUSHAWAY_THINK_INTERVAL;
	}

	ConVarRef mp_respawn_immunitytime( "mp_respawn_immunitytime" );
	float flImmuneTime = mp_respawn_immunitytime.GetFloat();
	if ( flImmuneTime > 0 || CSGameRules()->IsWarmupPeriod() )
	{
		if ( m_bImmunity )
		{
			SetRenderMode( kRenderTransAlpha );
			SetRenderColorA( 128 );
		}
		else
		{
			SetRenderMode( kRenderNormal, true );
			SetRenderColorA( 255 );
		}
	}
	else
	{
		if ( GetRenderColor().a < 255 )
		{
			SetRenderMode( kRenderNormal, true );
			SetRenderColorA( 255 );
		}
	}

	if ( CSGameRules()->GetGamemode() == GameModes::DEATHMATCH && this == GetLocalPlayer() && IsAlive() && GetObserverMode() == OBS_MODE_NONE )
	{
		float flTimeLeft = m_fImmuneToDamageTime - gpGlobals->curtime;
		if ( flTimeLeft >= 0 )
		{
			//wchar_t szNotice[64] = L"";
			wchar_t wzTime[8] = L"";
			int nMinLeft = (int)flTimeLeft / 60;
			int nSecLeft = (int)flTimeLeft - ( nMinLeft * 60 ); 
			int nMSecLeft = (flTimeLeft - ((float)(nMinLeft*60) + (float)nSecLeft)) * 10; 
			V_swprintf_safe( wzTime, L"%d.%d", nSecLeft, nMSecLeft );

			wchar_t wzBuyBind[32] = L"";
			UTIL_ReplaceKeyBindings( L"%buymenu%", 0, wzBuyBind, sizeof( wzBuyBind ) );

			wchar_t wzAutoBuyBind[32] = L"";
			UTIL_ReplaceKeyBindings( L"%autobuy%", 0, wzAutoBuyBind, sizeof( wzAutoBuyBind ) );

			wchar_t wszLocalized[256];
			if ( cl_dm_buyrandomweapons.GetBool() )
			{
				if ( flTimeLeft < 1.0f && m_bHasMovedSinceSpawn )
					g_pVGuiLocalize->ConstructString( wszLocalized, sizeof( wszLocalized ), g_pVGuiLocalize->Find( "#Cstrike_TitlesTXT_DM_InvulnExpire_RandomON" ), 1, wzAutoBuyBind );
				else if ( flTimeLeft < 0.1 )
					g_pVGuiLocalize->ConstructString( wszLocalized, sizeof( wszLocalized ), g_pVGuiLocalize->Find( "#Cstrike_TitlesTXT_DM_BuyMenuExpire_RandomON" ), 1, wzAutoBuyBind );
				else
					g_pVGuiLocalize->ConstructString( wszLocalized, sizeof( wszLocalized ), g_pVGuiLocalize->Find( "#Cstrike_TitlesTXT_DM_BuyMenu_RandomON" ), 3, wzBuyBind, wzTime, wzAutoBuyBind );
			}	
			else
			{
				if ( flTimeLeft < 1.0f && m_bHasMovedSinceSpawn )
					g_pVGuiLocalize->ConstructString( wszLocalized, sizeof( wszLocalized ), g_pVGuiLocalize->Find( "#Cstrike_TitlesTXT_DM_InvulnExpire_RandomOFF" ), 1, wzAutoBuyBind );
				else if ( flTimeLeft < 0.1 )
					g_pVGuiLocalize->ConstructString( wszLocalized, sizeof( wszLocalized ), g_pVGuiLocalize->Find( "#Cstrike_TitlesTXT_DM_BuyMenuExpire_RandomOFF" ), 1, wzAutoBuyBind );
				else	
					g_pVGuiLocalize->ConstructString( wszLocalized, sizeof( wszLocalized ), g_pVGuiLocalize->Find( "#Cstrike_TitlesTXT_DM_BuyMenu_RandomOFF" ), 3, wzBuyBind, wzTime, wzAutoBuyBind );
			}

			char szLocalized[256];
			g_pVGuiLocalize->ConvertUnicodeToANSI( wszLocalized, szLocalized, sizeof( szLocalized ) );
			HintMessage( szLocalized ); // PiMoN: better to use ClientPrint but who cares you cant even display any hints from client
		}
	}
	
	// Otherwise buy random or get previous round's gear, depending on cl_dm_buyrandomweapons.
	if ( m_bShouldAutobuyDMWeapons )
	{
		if ( this == GetLocalPlayer() && IsAlive() && (GetTeamNumber() == TEAM_CT || GetTeamNumber() == TEAM_TERRORIST) )
		{
			if ( cl_dm_buyrandomweapons.GetBool() )
			{
				engine->ClientCmd_Unrestricted( "buyrandom" );
			}
			else
			{
				engine->ClientCmd_Unrestricted( "rebuy" );
			}

			m_bShouldAutobuyDMWeapons = false;
		}
	}

	// NVNT - check for spectating forces
	if ( IsLocalPlayer() )
	{
		if ( GetTeamNumber() == TEAM_SPECTATOR || !this->IsAlive() || GetLocalOrInEyeCSPlayer() != this )
		{
			if (!inSpectating_Haptics)
			{
				if ( haptics )
					haptics->SetNavigationClass("spectate");

				inSpectating_Haptics = true;
			}
		}
		else
		{
			if (inSpectating_Haptics)
			{
				if ( haptics )
					haptics->SetNavigationClass("on_foot");

				inSpectating_Haptics = false;
			}
		}

		if ( m_iObserverMode == OBS_MODE_FREEZECAM )
		{
			//=============================================================================
			// HPE_BEGIN:
			// [Forrest] Added sv_disablefreezecam check
			//=============================================================================
			static ConVarRef sv_disablefreezecam( "sv_disablefreezecam" );
			if ( !m_bPlayingFreezeCamSound && !cl_disablefreezecam.GetBool() && !sv_disablefreezecam.GetBool() )
				//=============================================================================
				// HPE_END
				//=============================================================================
			{
				// Play sound
				m_bPlayingFreezeCamSound = true;

				CLocalPlayerFilter filter;
				EmitSound_t ep;
				ep.m_nChannel = CHAN_VOICE;
				ep.m_pSoundName =  "UI/freeze_cam.wav";
				ep.m_flVolume = VOL_NORM;
				ep.m_SoundLevel = SNDLVL_NORM;
				ep.m_bEmitCloseCaption = false;

				EmitSound( filter, GetSoundSourceIndex(), ep );
			}
		}
		else
		{
			m_bPlayingFreezeCamSound = false;
		}
	}
}


void C_CSPlayer::OnDataChanged( DataUpdateType_t type )
{
	BaseClass::OnDataChanged( type );

	if ( type == DATA_UPDATE_CREATED )
	{
		SetNextClientThink( CLIENT_THINK_ALWAYS );

		if ( IsLocalPlayer() )
		{
			if ( CSGameRules() && CSGameRules()->IsBlackMarket() )
			{
				CSGameRules()->m_pPrices = NULL;
				CSGameRules()->m_StringTableBlackMarket = NULL;
				CSGameRules()->GetBlackMarketPriceList();

				CSGameRules()->SetBlackMarketPrices( false );
			}
		}
	}

	if ( m_bPlayingHostageCarrySound == false && m_hCarriedHostage )
	{
		m_bPlayingHostageCarrySound = true;
		EmitSound( "Hostage.Breath" );
	}
	else if ( m_bPlayingHostageCarrySound == true && !m_hCarriedHostage )
	{
		m_bPlayingHostageCarrySound = false;
		StopSound( "Hostage.Breath" );
	}

	UpdateVisibility();
}


void C_CSPlayer::ValidateModelIndex( void )
{
	UpdateMinModels();
}


void C_CSPlayer::PostDataUpdate( DataUpdateType_t updateType )
{
	// C_BaseEntity assumes we're networking the entity's angles, so pretend that it
	// networked the same value we already have.
	SetNetworkAngles( GetLocalAngles() );

	BaseClass::PostDataUpdate( updateType );
}

//-----------------------------------------------------------------------------
// Purpose:
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool C_CSPlayer::Interpolate( float currentTime )
{
	if ( !BaseClass::Interpolate( currentTime ) )
		return false;

	if ( CSGameRules()->IsFreezePeriod() )
	{
		// don't interpolate players position during freeze period
		SetAbsOrigin( GetNetworkOrigin() );
	}

	return true;
}

void C_CSPlayer::PlayClientJumpSound( void )
{
	// during prediction play footstep sounds only once
	if ( prediction->InPrediction() && !prediction->IsFirstTimePredicted() )
		return;

	CLocalPlayerFilter filter;
	EmitSound( filter, entindex(), "Default.WalkJump" );
}

int	C_CSPlayer::GetMaxHealth() const
{
	return 100;
}

bool C_CSPlayer::ShouldInterpolate()
{
	// [msmith] Do we need to check this for split screen as well?
	// If this is the player, (or being observed by the player ) then we want to interpolate it.
	if ( this == GetLocalOrInEyeCSPlayer() )
	{
		return true;
	}

	return BaseClass::ShouldInterpolate();
}

//-----------------------------------------------------------------------------
// Purpose: Return the local player, or the player being spectated in-eye
//-----------------------------------------------------------------------------
C_CSPlayer* GetLocalOrInEyeCSPlayer( void )
{
	C_CSPlayer *player = C_CSPlayer::GetLocalCSPlayer();

	if( player && player->GetObserverMode() == OBS_MODE_IN_EYE )
	{
		C_BaseEntity *target = player->GetObserverTarget();

		if( target && target->IsPlayer() )
		{
			return ToCSPlayer( target );
		}
	}
	return player;
}

//-----------------------------------------------------------------------------
// Purpose: Return the local player, or the player being spectated
//-----------------------------------------------------------------------------
C_CSPlayer* GetHudPlayer( void )
{
	C_CSPlayer *player = C_CSPlayer::GetLocalCSPlayer();

	if ( player && ( player->GetObserverMode() == OBS_MODE_IN_EYE || player->GetObserverMode() == OBS_MODE_CHASE ) )
	{
		C_BaseEntity *target = player->GetObserverTarget();

		if( target && target->IsPlayer() )
		{
			return ToCSPlayer( target );
		}
	}
	return player;
}

#define MAX_FLASHBANG_OPACITY 75.0f

//-----------------------------------------------------------------------------
// Purpose: Update this client's targetid entity
//-----------------------------------------------------------------------------
void C_CSPlayer::UpdateIDTarget()
{
	// Clear old target and find a new one
	m_iIDEntIndex = 0;

	// don't show IDs if mp_playerid == 2
	if ( mp_playerid.GetInt() == 2 )
		return;

	//Check how much of a screen fade we have.
	//if it's more than 75 then we can't see what's going on so we don't display the id.
	byte color[4];
	bool blend;
	vieweffects->GetFadeParams( &color[0], &color[1], &color[2], &color[3], &blend );

	if ( color[3] > MAX_FLASHBANG_OPACITY && ( IsAlive() || GetObserverMode() == OBS_MODE_IN_EYE ) )
		 return;

	trace_t tr;
	Vector vecStart, vecEnd;
	VectorMA( MainViewOrigin(), 2500, MainViewForward(), vecEnd );
	VectorMA( MainViewOrigin(), 10,   MainViewForward(), vecStart );
	UTIL_TraceLine( vecStart, vecEnd, MASK_VISIBLE_AND_NPCS, GetLocalOrInEyeCSPlayer(), COLLISION_GROUP_NONE, &tr );
	if ( !tr.startsolid && !tr.DidHitNonWorldEntity() )
	{
		CTraceFilterSimple filter( GetLocalOrInEyeCSPlayer(), COLLISION_GROUP_NONE );

		// Check for player hitboxes extending outside their collision bounds
		const float rayExtension = 40.0f;
		UTIL_ClipTraceToPlayers(vecStart, vecEnd + MainViewForward() * rayExtension, MASK_SOLID|CONTENTS_HITBOX, &filter, &tr );
	}

	if ( !tr.startsolid && tr.DidHitNonWorldEntity() )
	{
		C_BaseEntity *pEntity = tr.m_pEnt;

		if ( pEntity && (pEntity != this) )
		{
			if ( mp_playerid.GetInt() == 1 ) // only show team names
			{
				if ( pEntity->GetTeamNumber() != GetTeamNumber() )
					return;
			}

			if ( LineGoesThroughSmoke( vecStart, pEntity->WorldSpaceCenter(), 1.0f ) )
				return;

			if ( pEntity->IsBaseCombatWeapon() )
				return;

			if ( !GetIDTarget() && ( !m_iOldIDEntIndex || m_holdTargetIDTimer.IsElapsed() ) )
			{
				// track when we first mouse over the target
				m_delayTargetIDTimer.Start( mp_playerid_delay.GetFloat() );
			}
			m_iIDEntIndex = pEntity->entindex();

			m_iOldIDEntIndex = m_iIDEntIndex;
			m_holdTargetIDTimer.Start( mp_playerid_hold.GetFloat() );
		}
	}
}

void C_CSPlayer::UpdateTargetedWeapon( void )
{
	m_iTargetedWeaponEntIndex = 0;

	Vector aimDir;
	AngleVectors( GetFinalAimAngle(), &aimDir );

	// FIXME: if you drop a weapon at a teammates' feet, you won't get the HUD prompt text because the teammate id
	// trace (which uses the bounding box of the teammate) is prioritized in the hud over the prompt to pick up the weapon.
	// Pressing USE while looking at a weapon a teammate is standing on will still swap to it, since this trace is 
	// succeeding - but you don't get the on-screen prompt. This kinda sucks during buytime, ideally the hud should
	// support drawing the teammate name AND the weapon pickup promt at the same time.

	trace_t result;
	CTraceFilterOmitPlayers traceFilter; // don't hit players with this trace
	UTIL_TraceLine( EyePosition(), EyePosition() + MAX_WEAPON_NAME_POPUP_RANGE * aimDir, MASK_SHOT, &traceFilter, &result );

	if ( result.DidHitNonWorldEntity() && result.m_pEnt->IsBaseCombatWeapon() )
	{
		if ( LineGoesThroughSmoke( EyePosition(), result.m_pEnt->WorldSpaceCenter(), 1.0f ) )
			return;

		//now that we have a weapon, we check to see if we are also looking at a bomb
		// setting the weaponEntIndex to the bomb prevents the hint coming up to 
		// pick up a weapon if it occupies the same space as a bomb
		if ( GetUsableHighPriorityEntity() )
			return;

		// Set if to point at the weapon
		m_iTargetedWeaponEntIndex = result.m_pEnt->entindex();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Input handling
//-----------------------------------------------------------------------------
bool C_CSPlayer::CreateMove( float flInputSampleTime, CUserCmd *pCmd )
{
	// Bleh... we will wind up needing to access bones for attachments in here.
	C_BaseAnimating::AutoAllowBoneAccess boneaccess( true, true );

	return BaseClass::CreateMove( flInputSampleTime, pCmd );
}

//-----------------------------------------------------------------------------
// Purpose: Flash this entity on the radar
//-----------------------------------------------------------------------------
bool C_CSPlayer::IsInHostageRescueZone()
{
	return 	m_bInHostageRescueZone;
}

CWeaponCSBase* C_CSPlayer::GetActiveCSWeapon() const
{
	return dynamic_cast< CWeaponCSBase* >( GetActiveWeapon() );
}

CWeaponCSBase* C_CSPlayer::GetCSWeapon( CSWeaponID id ) const
{
	for (int i=0;i<MAX_WEAPONS;i++)
	{
		CBaseCombatWeapon *weapon = GetWeapon( i );
		if ( weapon )
		{
			CWeaponCSBase *csWeapon = dynamic_cast< CWeaponCSBase * >( weapon );
			if ( csWeapon )
			{
				if ( id == csWeapon->GetWeaponID() )
				{
					return csWeapon;
				}
			}
		}
	}

	return NULL;
}

//REMOVEME
/*
void C_CSPlayer::SetFireAnimation( PLAYER_ANIM playerAnim )
{
	Activity idealActivity = ACT_WALK;

	// Figure out stuff about the current state.
	float speed = GetAbsVelocity().Length2D();
	bool isMoving = ( speed != 0.0f ) ? true : false;
	bool isDucked = ( GetFlags() & FL_DUCKING ) ? true : false;
	bool isStillJumping = false; //!( GetFlags() & FL_ONGROUND );
	bool isRunning = false;

	if ( speed > ARBITRARY_RUN_SPEED )
	{
		isRunning = true;
	}

	// Now figure out what to do based on the current state and the new state.
	switch ( playerAnim )
	{
	default:
	case PLAYER_RELOAD:
	case PLAYER_ATTACK1:
	case PLAYER_IDLE:
	case PLAYER_WALK:
		// Are we still jumping?
		// If so, keep playing the jump animation.
		if ( !isStillJumping )
		{
			idealActivity = ACT_WALK;

			if ( isDucked )
			{
				idealActivity = !isMoving ? ACT_CROUCHIDLE : ACT_RUN_CROUCH;
			}
			else
			{
				if ( isRunning )
				{
					idealActivity = ACT_RUN;
				}
				else
				{
					idealActivity = isMoving ? ACT_WALK : ACT_IDLE;
				}
			}

			// Allow body yaw to override for standing and turning in place
			idealActivity = m_PlayerAnimState.BodyYawTranslateActivity( idealActivity );
		}
		break;

	case PLAYER_JUMP:
		idealActivity = ACT_HOP;
		break;

	case PLAYER_DIE:
		// Uses Ragdoll now???
		idealActivity = ACT_DIESIMPLE;
		break;

	// FIXME:  Use overlays for reload, start/leave aiming, attacking
	case PLAYER_START_AIMING:
	case PLAYER_LEAVE_AIMING:
		idealActivity = ACT_WALK;
		break;
	}

	CWeaponCSBase *pWeapon = GetActiveCSWeapon();

	if ( pWeapon )
	{
		Activity aWeaponActivity = idealActivity;

		if ( playerAnim == PLAYER_ATTACK1 )
		{
			switch ( idealActivity )
			{
				case ACT_WALK:
				default:
					aWeaponActivity = ACT_PLAYER_WALK_FIRE;
					break;
				case ACT_RUN:
					aWeaponActivity = ACT_PLAYER_RUN_FIRE;
					break;
				case ACT_IDLE:
					aWeaponActivity = ACT_PLAYER_IDLE_FIRE;
					break;
				case ACT_CROUCHIDLE:
					aWeaponActivity = ACT_PLAYER_CROUCH_FIRE;
					break;
				case ACT_RUN_CROUCH:
					aWeaponActivity = ACT_PLAYER_CROUCH_WALK_FIRE;
					break;
			}
		}

		m_PlayerAnimState.SetWeaponLayerSequence( pWeapon->GetCSWpnData().m_szAnimExtension, aWeaponActivity );
	}
}
*/

ShadowType_t C_CSPlayer::ShadowCastType( void )
{
	if ( !IsVisible() )
		 return SHADOWS_NONE;

	return SHADOWS_RENDER_TO_TEXTURE_DYNAMIC;
}

//-----------------------------------------------------------------------------
// Purpose: Returns whether or not we can switch to the given weapon.
// Input  : pWeapon -
//-----------------------------------------------------------------------------
bool C_CSPlayer::Weapon_CanSwitchTo( CBaseCombatWeapon *pWeapon )
{
	if ( !pWeapon->CanDeploy() )
		return false;

	if ( GetActiveWeapon() )
	{
		if ( !GetActiveWeapon()->CanHolster() )
			return false;
	}

	return true;
}

ConVar clTaserShakeFreqMin( "clTaserShakeFreqMin", "0.2", 0, "how often the shake is applied (min time)" );
ConVar clTaserShakeFreqMax( "clTaserShakeFreqMax", "0.7", 0, "how often the shake is applied (max time)" );

ConVar clTaserShakeTimeTotal( "clTaserShakeTimeTotal", "7.0", 0, "time the taser shake is applied." );


void C_CSPlayer::HandleTaserAnimation()
{
	if ( m_bKilledByTaser )
	{
		if ( m_nextTaserShakeTime < gpGlobals->curtime )
		{
			// we're ready to apply a taser force
			C_CSRagdoll *pRagdoll = (C_CSRagdoll* )m_hRagdoll.Get();
			if ( pRagdoll )
			{
				pRagdoll->ApplyRandomTaserForce();
			}

			if ( m_firstTaserShakeTime == 0.0f )
			{
				m_firstTaserShakeTime = gpGlobals->curtime;
				EmitSound("Player.DeathTaser" ); // play death audio here
			}

			if ( m_firstTaserShakeTime + clTaserShakeTimeTotal.GetFloat() < gpGlobals->curtime )
			{
				// we've waited more than clTaserShakeTimeTotal since our first shake so we're done with the taze effect...  AKA: "DON'T TAZE ME BRO"
				m_bKilledByTaser = false;
				m_firstTaserShakeTime = 0.0f;
			}
			else
			{
				// set the timer for our next shake
				m_nextTaserShakeTime = gpGlobals->curtime + RandomFloat( clTaserShakeFreqMin.GetFloat(), clTaserShakeFreqMax.GetFloat() );
			}
		}
	}
}


void C_CSPlayer::UpdateClientSideAnimation()
{
	// We do this in a different order than the base class.
	// We need our cycle to be valid for when we call the playeranimstate update code,
	// or else it'll synchronize the upper body anims with the wrong cycle.
	if ( GetSequence() != -1 )
	{
		// move frame forward
		FrameAdvance( 0.0f ); // 0 means to use the time we last advanced instead of a constant
	}

	// Update the animation data. It does the local check here so this works when using
	// a third-person camera (and we don't have valid player angles).
	if ( this == C_CSPlayer::GetLocalCSPlayer() )
		m_PlayerAnimState->Update( EyeAngles()[YAW], EyeAngles()[PITCH] );
	else
		m_PlayerAnimState->Update( m_angEyeAngles[YAW], m_angEyeAngles[PITCH] );

	if ( GetSequence() != -1 )
	{
		// latch old values
		OnLatchInterpolatedVariables( LATCH_ANIMATION_VAR );
	}

	if ( m_bKilledByTaser )
	{
		HandleTaserAnimation();
	}

	// We only update the view model for the local player.
	if ( IsLocalPlayer() )
	{
		CWeaponCSBase *pWeapon = GetActiveCSWeapon();
		if ( pWeapon )
		{
			C_BaseViewModel *pViewModel = assert_cast<C_BaseViewModel *>( GetViewModel( pWeapon->m_nViewModelIndex ) );
			if ( pViewModel )
			{
				pViewModel->UpdateAllViewmodelAddons();
			}
		}
		else
		{
			//We have a null weapon so remove the add ons for all the view models for this player.
			for ( int i=0; i<MAX_VIEWMODELS; ++i )
			{
				C_BaseViewModel *pViewModel = assert_cast<C_BaseViewModel *>( GetViewModel( i ) );
				if ( pViewModel )
				{
					pViewModel->RemoveViewmodelArmModels();
					pViewModel->RemoveViewmodelStatTrak();
				}
			}
		}
	}
}

void C_CSPlayer::ProcessMuzzleFlashEvent()
{
	CWeaponCSBase *pWeapon = GetActiveCSWeapon();

	if ( !pWeapon )
		return;

	// Muzzle Flash Effect.
	int iAttachmentIndex = pWeapon->GetMuzzleAttachmentIndex( pWeapon );
	const char* pszEffect = pWeapon->GetMuzzleFlashEffectName( true );

	if ( pszEffect && Q_strlen(pszEffect ) > 0 && iAttachmentIndex >= 0 && pWeapon->ShouldDraw() && pWeapon->IsVisible() && !pWeapon->GetOwner()->IsDormant() )
	{
		DispatchParticleEffect( pszEffect, PATTACH_POINT_FOLLOW, pWeapon, iAttachmentIndex, false );
	}

#if 0
	// Brass Eject Effect.
	iAttachmentIndex = pWeapon->GetEjectBrassAttachmentIndex( pWeapon );
	pszEffect = pWeapon->GetCSWpnData().m_szEjectBrassEffect;
	if ( pszEffect && Q_strlen(pszEffect ) > 0 && iAttachmentIndex >= 0 && pWeapon->ShouldDraw() && pWeapon->IsVisible() && !pWeapon->GetOwner()->IsDormant() )
	{
		DispatchParticleEffect( pszEffect, PATTACH_POINT_FOLLOW, pWeapon, iAttachmentIndex, false );
	}
#endif
}

const QAngle& C_CSPlayer::EyeAngles()
{
	if ( IsLocalPlayer() && !g_nKillCamMode )
	{
		return BaseClass::EyeAngles();
	}
	else
	{
		return m_angEyeAngles;
	}
}

bool C_CSPlayer::ShouldDraw( void )
{
	// If we're dead, our ragdoll will be drawn for us instead.
	if ( !IsAlive() )
		return false;

	if( GetTeamNumber() == TEAM_SPECTATOR )
		return false;

	if( IsLocalPlayer() )
	{
		if ( IsRagdoll() )
			return true;
	}

	C_BasePlayer *pLocalPlayer = C_BasePlayer::GetLocalPlayer();

	// keep drawing players we're observing with the interpolating spectator camera
	if ( pLocalPlayer && pLocalPlayer->GetObserverInterpState() == OBSERVER_INTERP_TRAVELING )
	{
		return true;
	}

	// don't draw players we're observing in first-person
	if ( pLocalPlayer && pLocalPlayer->GetObserverTarget() == ToBasePlayer(this) && pLocalPlayer->GetObserverMode() == OBS_MODE_IN_EYE )
	{
		return false;
	}

	return BaseClass::ShouldDraw();
}

#define APPROX_CENTER_PLAYER Vector(0,0,50)

bool C_CSPlayer::GetAttachment( int number, Vector &origin )
{
	if ( IsDormant() )
	{
		origin = GetAbsOrigin() + APPROX_CENTER_PLAYER;
		return true;
	}
	return BaseClass::GetAttachment( number, origin );
}

bool C_CSPlayer::GetAttachment( int number, Vector &origin, QAngle &angles )
{
	if ( IsDormant() )
	{
		origin = GetAbsOrigin() + APPROX_CENTER_PLAYER;
		angles = GetAbsAngles();
		return true;
	}
	return BaseClass::GetAttachment( number, origin, angles );
}

bool FindWeaponAttachmentBone( C_BaseCombatWeapon *pWeapon, int &iWeaponBone )
{
	if ( !pWeapon )
		return false;

	CStudioHdr *pHdr = pWeapon->GetModelPtr();
	if ( !pHdr )
		return false;

	for ( iWeaponBone=0; iWeaponBone < pHdr->numbones(); iWeaponBone++ )
	{
		if ( stricmp( pHdr->pBone( iWeaponBone )->pszName(), "ValveBiped.weapon_bone_LHand" ) == 0 )
			break;
	}

	return iWeaponBone != pHdr->numbones();
}


bool FindMyAttachmentBone( C_BaseAnimating *pModel, int &iBone, CStudioHdr *pHdr )
{
	if ( !pHdr )
		return false;

	for ( iBone=0; iBone < pHdr->numbones(); iBone++ )
	{
		if ( stricmp( pHdr->pBone( iBone )->pszName(), "ValveBiped.Bip01_L_Hand" ) == 0 )
			break;
	}

	return iBone != pHdr->numbones();
}


inline bool IsBoneChildOf( CStudioHdr *pHdr, int iBone, int iParent )
{
	if ( iBone == iParent )
		return false;

	while ( iBone != -1 )
	{
		if ( iBone == iParent )
			return true;

		iBone = pHdr->pBone( iBone )->parent;
	}
	return false;
}

void ApplyDifferenceTransformToChildren(
	C_BaseAnimating *pModel,
	const matrix3x4_t &mSource,
	const matrix3x4_t &mDest,
	int iParentBone )
{
	CStudioHdr *pHdr = pModel->GetModelPtr();
	if ( !pHdr )
		return;

	// Build a matrix to go from mOriginalHand to mHand.
	// ( mDest * Inverse( mSource ) ) * mSource = mDest
	matrix3x4_t mSourceInverse, mToDest;
	MatrixInvert( mSource, mSourceInverse );
	ConcatTransforms( mDest, mSourceInverse, mToDest );

	// Now multiply iMyBone and all its children by mToWeaponBone.
	for ( int i=0; i < pHdr->numbones(); i++ )
	{
		if ( IsBoneChildOf( pHdr, i, iParentBone ) )
		{
			matrix3x4_t &mCur = pModel->GetBoneForWrite( i );
			matrix3x4_t mNew;
			ConcatTransforms( mToDest, mCur, mNew );
			mCur = mNew;
		}
	}
}


void GetCorrectionMatrices(
	const matrix3x4_t &mShoulder,
	const matrix3x4_t &mElbow,
	const matrix3x4_t &mHand,
	matrix3x4_t &mShoulderCorrection,
	matrix3x4_t &mElbowCorrection
	)
{
	extern void Studio_AlignIKMatrix( matrix3x4_t &mMat, const Vector &vAlignTo );

	// Get the positions of each node so we can get the direction vectors.
	Vector vShoulder, vElbow, vHand;
	MatrixPosition( mShoulder, vShoulder );
	MatrixPosition( mElbow, vElbow );
	MatrixPosition( mHand, vHand );

	// Get rid of the translation.
	matrix3x4_t mOriginalShoulder = mShoulder;
	matrix3x4_t mOriginalElbow = mElbow;
	MatrixSetColumn( Vector( 0, 0, 0 ), 3, mOriginalShoulder );
	MatrixSetColumn( Vector( 0, 0, 0 ), 3, mOriginalElbow );

	// Let the IK code align them like it would if we did IK on the joint.
	matrix3x4_t mAlignedShoulder = mOriginalShoulder;
	matrix3x4_t mAlignedElbow = mOriginalElbow;
	Studio_AlignIKMatrix( mAlignedShoulder, vElbow-vShoulder );
	Studio_AlignIKMatrix( mAlignedElbow, vHand-vElbow );

	// Figure out the transformation from the aligned bones to the original ones.
	matrix3x4_t mInvAlignedShoulder, mInvAlignedElbow;
	MatrixInvert( mAlignedShoulder, mInvAlignedShoulder );
	MatrixInvert( mAlignedElbow, mInvAlignedElbow );

	ConcatTransforms( mInvAlignedShoulder, mOriginalShoulder, mShoulderCorrection );
	ConcatTransforms( mInvAlignedElbow, mOriginalElbow, mElbowCorrection );
}


void C_CSPlayer::BuildTransformations( CStudioHdr *pHdr, Vector *pos, Quaternion q[], const matrix3x4_t& cameraTransform, int boneMask, CBoneBitList &boneComputed )
{
	// First, setup our model's transformations like normal.
	BaseClass::BuildTransformations( pHdr, pos, q, cameraTransform, boneMask, boneComputed );

	if ( !IsVisible() || IsDormant() || (IsLocalPlayer() && !C_BasePlayer::ShouldDrawLocalPlayer()) || !ShouldDraw() )
		return;

	if ( !cl_left_hand_ik.GetInt() )
		return;

	// If our current weapon has a bone named L_Hand_Attach, then we attach the player's
	// left hand (Valvebiped.Bip01_L_Hand) to it.
	C_BaseCombatWeapon *pWeapon = GetActiveWeapon();

	if ( !pWeapon )
		return;

	// Have the weapon setup its bones.
	pWeapon->SetupBones( NULL, 0, BONE_USED_BY_ANYTHING, gpGlobals->curtime );

	int iWeaponBone = 0;
	if ( FindWeaponAttachmentBone( pWeapon, iWeaponBone ) )
	{
		int iMyBone = 0;
		if ( FindMyAttachmentBone( this, iMyBone, pHdr ) )
		{
			int iHand = iMyBone;
			int iElbow = pHdr->pBone( iHand )->parent;
			int iShoulder = pHdr->pBone( iElbow )->parent;
			matrix3x4_t *pBones = &GetBoneForWrite( 0 );

			// Store off the original hand position.
			matrix3x4_t mSource = pBones[iHand];


			// Figure out the rotation offset from the current shoulder and elbow bone rotations
			// and what the IK code's alignment code is going to produce, because we'll have to
			// re-apply that offset after the IK runs.
			matrix3x4_t mShoulderCorrection, mElbowCorrection;
			GetCorrectionMatrices( pBones[iShoulder], pBones[iElbow], pBones[iHand], mShoulderCorrection, mElbowCorrection );


			// Do the IK solution.
			Vector vHandTarget;
			MatrixPosition( pWeapon->GetBone( iWeaponBone ), vHandTarget );
			Studio_SolveIK( iShoulder, iElbow, iHand, vHandTarget, pBones );


			// Now reapply the rotation correction.
			matrix3x4_t mTempShoulder = pBones[iShoulder];
			matrix3x4_t mTempElbow = pBones[iElbow];
			ConcatTransforms( mTempShoulder, mShoulderCorrection, pBones[iShoulder] );
			ConcatTransforms( mTempElbow, mElbowCorrection, pBones[iElbow] );


			// Now apply the transformation on the hand to the fingers.
			matrix3x4_t &mDest = GetBoneForWrite( iHand );
			ApplyDifferenceTransformToChildren( this, mSource, mDest, iHand );
		}
	}
}


C_BaseAnimating * C_CSPlayer::BecomeRagdollOnClient()
{
	return NULL;
}


IRagdoll* C_CSPlayer::GetRepresentativeRagdoll() const
{
	if ( m_hRagdoll.Get() )
	{
		C_CSRagdoll *pRagdoll = (C_CSRagdoll*)m_hRagdoll.Get();

		return pRagdoll->GetIRagdoll();
	}
	else
	{
		return NULL;
	}
}


void C_CSPlayer::PlayReloadEffect()
{
	// Only play the effect for other players.
	if ( this == C_CSPlayer::GetLocalCSPlayer() )
	{
		Assert( false ); // We shouldn't have been sent this message.
		return;
	}

	// Get the view model for our current gun.
	CWeaponCSBase *pWeapon = GetActiveCSWeapon();
	if ( !pWeapon )
		return;

	// The weapon needs two models, world and view, but can only cache one. Synthesize the other.
	const CCSWeaponInfo &info = pWeapon->GetCSWpnData();
	const model_t *pModel = modelinfo->GetModel( modelinfo->GetModelIndex( info.szViewModel ) );
	if ( !pModel )
		return;
	CStudioHdr studioHdr( modelinfo->GetStudiomodel( pModel ), mdlcache );
	if ( !studioHdr.IsValid() )
		return;

	// Find the reload animation.
	for ( int iSeq=0; iSeq < studioHdr.GetNumSeq(); iSeq++ )
	{
		mstudioseqdesc_t *pSeq = &studioHdr.pSeqdesc( iSeq );

		if ( pSeq->activity == ACT_VM_RELOAD )
		{
			float poseParameters[MAXSTUDIOPOSEPARAM];
			memset( poseParameters, 0, sizeof( poseParameters ) );
			float cyclesPerSecond = Studio_CPS( &studioHdr, *pSeq, iSeq, poseParameters );

			// Now read out all the sound events with their timing
			for ( int iEvent=0; iEvent < pSeq->numevents; iEvent++ )
			{
				mstudioevent_t *pEvent = pSeq->pEvent( iEvent );

				if ( pEvent->event == CL_EVENT_SOUND )
				{
					CCSSoundEvent event;
					event.m_SoundName = pEvent->options;
					event.m_flEventTime = gpGlobals->curtime + pEvent->cycle / cyclesPerSecond;
					m_SoundEvents.AddToTail( event );
				}
			}

			break;
		}
	}
}

void C_CSPlayer::DoAnimationEvent( PlayerAnimEvent_t event, int nData )
{
	if ( event == PLAYERANIMEVENT_THROW_GRENADE )
	{
		// Let the server handle this event. It will update m_iThrowGrenadeCounter and the client will
		// pick up the event in CCSPlayerAnimState.
	}
	else
	{
		m_PlayerAnimState->DoAnimationEvent( event, nData );
	}
}

void C_CSPlayer::DropPhysicsMag( const char *options )
{
	// create a client-side physical magazine model to drop in the world and clatter to the floor. Realism!

	if ( !sv_magazine_drop_physics )
		return;

	CWeaponCSBase *pWeapon = GetActiveCSWeapon();
	if ( !pWeapon || pWeapon->GetCSWpnData().m_szMagModel[0] == 0 )
		return;
	
	Vector attachOrigin = GetAbsOrigin() + Vector(0,0,50);
	QAngle attachAngles = QAngle(0,0,0);

	// find the best attachment position to drop the mag from

	int iMagAttachIndex = -1;

	if ( options && options[0] != 0 )
	{
		// if a custom attachment is specified, look for it on the weapon, then the player.
		iMagAttachIndex = pWeapon->LookupAttachment( options );
		if ( iMagAttachIndex <= 0 )
			iMagAttachIndex = LookupAttachment( options );
	}

	if ( iMagAttachIndex <= 0 )
	{
		// we either didn't specify a custom attachment, or the one we did wasn't found. Find the default, 'mag_eject' on the weapon, then the player.
		iMagAttachIndex = pWeapon->LookupAttachment( "mag_eject" );
		if ( iMagAttachIndex <= 0 )
			iMagAttachIndex = LookupAttachment( "mag_eject" );
	}

	if ( iMagAttachIndex <= 0 )
	{
		// no luck looking for the custom attachment, or "mag_eject". How about "shell_eject"? Wrong, but better than nothing...
		iMagAttachIndex = pWeapon->LookupAttachment( "shell_eject" );
	}
	

	// limit mag drops to one per second, in case animations accidentally overlap or events erroneously get fired too rapidly
	// let new attachment indices through though, for elites
	if ( m_flNextMagDropTime > gpGlobals->curtime && iMagAttachIndex == m_nLastMagDropAttachmentIndex )
		return;
	m_flNextMagDropTime = gpGlobals->curtime + 1;
	m_nLastMagDropAttachmentIndex = iMagAttachIndex;

	if ( iMagAttachIndex <= 0 )
	{
		return;
	}

	if ( !IsDormant() )
	{
		if ( !pWeapon->GetAttachment( iMagAttachIndex, attachOrigin, attachAngles ) )
		{
			GetAttachment( iMagAttachIndex, attachOrigin, attachAngles );
		}
	}

	// hide the animation-driven w_model magazine
	pWeapon->SetBodygroup( FindBodygroupByName( "magazine" ), 1 );

	// The local first-person player can't drop mags in the correct world-space location, otherwise the mag would appear in mid-air.
	// Instead, first try to drop the mag slightly above the origin of the player.
	// However if the player is looking nearly straight down, they'll still see the mag appear. If this is the case,
	// drop the mags from 10 units behind their eyes. This means the mag ALWAYS drops in from "off-screen"
	if ( ( IsLocalPlayer() && !ShouldDraw() ) || GetSpectatorMode() == OBS_MODE_IN_EYE )
	{
		if ( EyeAngles().x < 42.0f ) //not looking extremely vertically downward
		{
			attachOrigin = GetAbsOrigin() + Vector(0,0,20);
		}
		else
		{
			attachOrigin = EyePosition() - ( Forward() * 10 );
		}
	}

	C_PhysPropClientside *pEntity = C_PhysPropClientside::CreateNew();
	if ( !pEntity )
		return;

	pEntity->SetModelName( pWeapon->GetCSWpnData().m_szMagModel );
	pEntity->SetPhysicsMode( PHYSICS_MULTIPLAYER_CLIENTSIDE );
	pEntity->SetCollisionGroup( COLLISION_GROUP_DEBRIS );
	pEntity->SetFadeMinMax( 500.0f, 550.0f );
	pEntity->SetLocalOrigin( attachOrigin );
	pEntity->SetLocalAngles( attachAngles );
	pEntity->m_iHealth = 0;
	pEntity->m_takedamage = DAMAGE_NO;

	if ( !pEntity->Initialize() )
	{
		pEntity->Release();
		return;
	}

	// fade out after set time
	pEntity->StartFadeOut( sv_magazine_drop_time );

	// apply starting velocity
	IPhysicsObject *pPhysicsObject = pEntity->VPhysicsGetObject();
	if( pPhysicsObject )
	{
		if ( !IsDormant() ) // don't apply velocity to mags dropped by dormant players
		{
			Vector vecMagVelocity; vecMagVelocity.Init();
			Quaternion quatMagAngular; quatMagAngular.Init();
			if ( !pWeapon->GetAttachmentVelocity( iMagAttachIndex, vecMagVelocity, quatMagAngular ) )
			{
				GetAttachmentVelocity( iMagAttachIndex, vecMagVelocity, quatMagAngular );
			}

			// if the local attachment is returning no motion, pull velocity from the player just to be sure
			if ( vecMagVelocity == vec3_origin )
				vecMagVelocity = GetLocalVelocity();

			QAngle angMagAngular; angMagAngular.Init();
			QuaternionAngles( quatMagAngular, angMagAngular );

			AngularImpulse angImpMagAngular; angImpMagAngular.Init();
			QAngleToAngularImpulse( angMagAngular, angImpMagAngular );

			// clamp to 300 max
			if ( vecMagVelocity.Length() > 300.0f )
				vecMagVelocity = vecMagVelocity.Normalized() * 300.0f;

			pPhysicsObject->SetVelocity( &vecMagVelocity, &angImpMagAngular );
		}
	}
	else
	{
		pEntity->Release();
		return;
	}
}

void C_CSPlayer::FireEvent( const Vector& origin, const QAngle& angles, int event, const char *options )
{
	if( event == 7001 )
	{
		bool bInWater = ( enginetrace->GetPointContents(origin) & CONTENTS_WATER );

		//Msg( "run event ( %d )\n", bInWater ? 1 : 0 );

		if( bInWater )
		{
			//run splash
			CEffectData data;

			//trace up from foot position to the water surface
			trace_t tr;
			Vector vecTrace(0,0,1024);
			UTIL_TraceLine( origin, origin + vecTrace, MASK_WATER, NULL, COLLISION_GROUP_NONE, &tr );
			if ( tr.fractionleftsolid )
			{
				data.m_vOrigin = origin + (vecTrace * tr.fractionleftsolid);
			}
			else
			{
				data.m_vOrigin = origin;
			}

			data.m_vNormal = Vector( 0,0,1 );
			data.m_flScale = random->RandomFloat( 4.0f, 5.0f );
			DispatchEffect( "watersplash", data );
		}
	}
	else if( event == 7002 )
	{
		bool bInWater = ( enginetrace->GetPointContents(origin) & CONTENTS_WATER );

		//Msg( "walk event ( %d )\n", bInWater ? 1 : 0 );

		if( bInWater )
		{
			//walk ripple
			CEffectData data;

			//trace up from foot position to the water surface
			trace_t tr;
			Vector vecTrace(0,0,1024);
			UTIL_TraceLine( origin, origin + vecTrace, MASK_WATER, NULL, COLLISION_GROUP_NONE, &tr );
			if ( tr.fractionleftsolid )
			{
				data.m_vOrigin = origin + (vecTrace * tr.fractionleftsolid);
			}
			else
			{
				data.m_vOrigin = origin;
			}

			data.m_vNormal = Vector( 0,0,1 );
			data.m_flScale = random->RandomFloat( 4.0f, 7.0f );
			DispatchEffect( "waterripple", data );
		}
	}
	else if ( event == AE_CL_EJECT_MAG_UNHIDE )
	{
		// mag is unhidden by the server
		return;
	}
	else if ( event == AE_CL_EJECT_MAG )
	{
		DropPhysicsMag( options );

		// hack: in first-person, the player isn't playing their third-person gun animations, so the events on the third-person gun model don't fire.
		// This means the event is fired from the player, and the player only fires ONE mag drop event. So while the elite model drops two data-driven mags
		// in third person, we need to help it out a little bit in first-person. So here's some one-off code to drop another physics mag only for the elite,
		// and only in first-person.

		CWeaponCSBase *pWeapon = GetActiveCSWeapon();
		if ( pWeapon && pWeapon->IsA(WEAPON_ELITE) )
		{
			DropPhysicsMag( "mag_eject2" );
		}
		return;
	}
	else
		BaseClass::FireEvent( origin, angles, event, options );
}


void C_CSPlayer::SetActivity( Activity eActivity )
{
	m_Activity = eActivity;
}


Activity C_CSPlayer::GetActivity() const
{
	return m_Activity;
}


const Vector& C_CSPlayer::GetRenderOrigin( void )
{
	if ( m_hRagdoll.Get() )
	{
		C_CSRagdoll *pRagdoll = (C_CSRagdoll*)m_hRagdoll.Get();
		if ( pRagdoll->IsInitialized() )
			return pRagdoll->GetRenderOrigin();
	}

	return BaseClass::GetRenderOrigin();
}


void C_CSPlayer::Simulate( void )
{
	if( this != C_BasePlayer::GetLocalPlayer() )
	{
		if ( IsEffectActive( EF_DIMLIGHT ) )
		{
			QAngle eyeAngles = EyeAngles();
			Vector vForward;
			AngleVectors( eyeAngles, &vForward );

			int iAttachment = LookupAttachment( "muzzle_flash" );

			if ( iAttachment < 0 )
				return;

			Vector vecOrigin;
			QAngle dummy;
			GetAttachment( iAttachment, vecOrigin, dummy );

			trace_t tr;
			UTIL_TraceLine( vecOrigin, vecOrigin + (vForward * 200), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );

			if( !m_pFlashlightBeam )
			{
				BeamInfo_t beamInfo;
				beamInfo.m_nType = TE_BEAMPOINTS;
				beamInfo.m_vecStart = tr.startpos;
				beamInfo.m_vecEnd = tr.endpos;
				beamInfo.m_pszModelName = "sprites/glow01.vmt";
				beamInfo.m_pszHaloName = "sprites/glow01.vmt";
				beamInfo.m_flHaloScale = 3.0;
				beamInfo.m_flWidth = 8.0f;
				beamInfo.m_flEndWidth = 35.0f;
				beamInfo.m_flFadeLength = 300.0f;
				beamInfo.m_flAmplitude = 0;
				beamInfo.m_flBrightness = 60.0;
				beamInfo.m_flSpeed = 0.0f;
				beamInfo.m_nStartFrame = 0.0;
				beamInfo.m_flFrameRate = 0.0;
				beamInfo.m_flRed = 255.0;
				beamInfo.m_flGreen = 255.0;
				beamInfo.m_flBlue = 255.0;
				beamInfo.m_nSegments = 8;
				beamInfo.m_bRenderable = true;
				beamInfo.m_flLife = 0.5;
				beamInfo.m_nFlags = FBEAM_FOREVER | FBEAM_ONLYNOISEONCE | FBEAM_NOTILE | FBEAM_HALOBEAM;

				m_pFlashlightBeam = beams->CreateBeamPoints( beamInfo );
			}

			if( m_pFlashlightBeam )
			{
				BeamInfo_t beamInfo;
				beamInfo.m_vecStart = tr.startpos;
				beamInfo.m_vecEnd = tr.endpos;
				beamInfo.m_flRed = 255.0;
				beamInfo.m_flGreen = 255.0;
				beamInfo.m_flBlue = 255.0;

				beams->UpdateBeamInfo( m_pFlashlightBeam, beamInfo );

				dlight_t *el = effects->CL_AllocDlight( 0 );
				el->origin = tr.endpos;
				el->radius = 50;
				el->color.r = 200;
				el->color.g = 200;
				el->color.b = 200;
				el->die = gpGlobals->curtime + 0.1;
			}
		}
		else if ( m_pFlashlightBeam )
		{
			ReleaseFlashlight();
		}
	}

	BaseClass::Simulate();
}

void C_CSPlayer::ReleaseFlashlight( void )
{
	if( m_pFlashlightBeam )
	{
		m_pFlashlightBeam->flags = 0;
		m_pFlashlightBeam->die = gpGlobals->curtime - 1;

		m_pFlashlightBeam = NULL;
	}
}

bool C_CSPlayer::HasC4( void )
{
	if( this == C_CSPlayer::GetLocalPlayer() )
	{
		return Weapon_OwnsThisType( "weapon_c4" );
	}
	else
	{
		C_CS_PlayerResource *pCSPR = (C_CS_PlayerResource*)GameResources();

		return pCSPR->HasC4( entindex() );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : float
//-----------------------------------------------------------------------------
float C_CSPlayer::GetFOV( void )
{
	float flCurFOV = BaseClass::GetFOV();

	if ( flCurFOV == GetDefaultFOV() )
	{
		if ( !sv_cheats )
		{
			sv_cheats = cvar->FindVar( "sv_cheats" );
		}

		if ( sv_cheats->GetBool() && fov_cs_debug.GetInt() > 0 )
		{
			return fov_cs_debug.GetInt();
		}
	}

#if IRONSIGHT
	CWeaponCSBase *pWeapon = GetActiveCSWeapon();
	if ( pWeapon )
	{
		CIronSightController* pIronSightController = pWeapon->GetIronSightController();
		if ( pIronSightController )
		{
			//bias the local client FOV change so ironsight transitions are nicer
			flCurFOV = pIronSightController->GetIronSightFOV( GetDefaultFOV(), true );
		}
	}
#endif //IRONSIGHT

	if ( GetObserverInterpState() == OBSERVER_INTERP_TRAVELING )
		flCurFOV = GetDefaultFOV();

	return flCurFOV;
}


//-----------------------------------------------------------------------------
void C_CSPlayer::CalcObserverView( Vector& eyeOrigin, QAngle& eyeAngles, float& fov )
{
	/**
	 * TODO: Fix this!
	// CS:S standing eyeheight is above the collision volume, so we need to pull it
	// down when we go into close quarters.
	float maxEyeHeightAboveBounds = VEC_VIEW_SCALED( this ).z - VEC_HULL_MAX_SCALED( this ).z;
	if ( GetObserverMode() == OBS_MODE_IN_EYE &&
		maxEyeHeightAboveBounds > 0.0f &&
		GetObserverTarget() &&
		GetObserverTarget()->IsPlayer() )
	{
		const float eyeClearance = 12.0f; // eye pos must be this far below the ceiling

		C_CSPlayer *target = ToCSPlayer( GetObserverTarget() );

		Vector offset = eyeOrigin - GetAbsOrigin();

		Vector vHullMin = VEC_HULL_MIN_SCALED( this );
		vHullMin.z = 0.0f;
		Vector vHullMax = VEC_HULL_MAX_SCALED( this );

		Vector start = GetAbsOrigin();
		start.z += vHullMax.z;
		Vector end = start;
		end.z += eyeClearance + VEC_VIEW_SCALED( this ).z - vHullMax_SCALED( this ).z;

		vHullMax.z = 0.0f;

		Vector fudge( 1, 1, 0 );
		vHullMin += fudge;
		vHullMax -= fudge;

		trace_t trace;
		Ray_t ray;
		ray.Init( start, end, vHullMin, vHullMax );
		UTIL_TraceRay( ray, MASK_PLAYERSOLID, target, COLLISION_GROUP_PLAYER_MOVEMENT, &trace );

		if ( trace.fraction < 1.0f )
		{
			float est = start.z + trace.fraction * (end.z - start.z) - GetAbsOrigin().z - eyeClearance;
			if ( ( target->GetFlags() & FL_DUCKING ) == 0 && !target->GetFallVelocity() && !target->IsDucked() )
			{
				offset.z = est;
			}
			else
			{
				offset.z = MIN( est, offset.z );
			}
			eyeOrigin.z = GetAbsOrigin().z + offset.z;
		}
	}
	*/

	BaseClass::CalcObserverView( eyeOrigin, eyeAngles, fov );
}

ConVar cl_obs_interp_enable( "cl_obs_interp_enable", "1", FCVAR_ARCHIVE, "Enables interpolation between observer targets" );
ConVar cl_obs_interp_pos_rate( "cl_obs_interp_pos_rate", "0.27" );
ConVar cl_obs_interp_pos_halflife( "cl_obs_interp_pos_halflife", "0.26" );
ConVar cl_obs_interp_snap_dist( "cl_obs_interp_snap_dist", "1" );
ConVar cl_obs_interp_settle_dist( "cl_obs_interp_settle_dist", "16" );
ConVar cl_obs_interp_dist_to_turn_to_face( "cl_obs_interp_dist_to_turn_to_face", "500", 0, "Changing to a target further than this will cause the camera to face the direction of travel" );
ConVar cl_obs_interp_angle_progress_headstart( "cl_obs_interp_angle_progress_headstart", "0.025" );
ConVar cl_obs_interp_turn_to_face_start_frac( "cl_obs_interp_turn_to_face_start_frac", "0.1" );
ConVar cl_obs_interp_turn_to_face_end_frac( "cl_obs_interp_turn_to_face_end_frac", "0.65" );
ConVar cl_obs_interp_obstruction_behavior( "cl_obs_interp_obstruction_behavior", "2" );
extern ConVar sv_disable_observer_interpolation;

void C_CSPlayer::InterpolateObserverView( Vector& vOutOrigin, QAngle& vOutAngles )
{
	Assert( vOutAngles.IsValid() && vOutOrigin.IsValid() );
	// Interpolate the view between observer target changes
	if ( m_obsInterpState != OBSERVER_INTERP_NONE && ShouldInterpolateObserverChanges() )
	{
		// We flag observer interpolation on as soon as observer targets change in the recvproxy change callback,
		// but we can't get entity position that early in the frame, so some setup is deferred until this point. We
		// have to set observer interpolation earlier, before the view is set up for this frame, otherwise we'll get a flicker
		// as the first frame after an observer target change will draw at the final position.
		if ( m_bObserverInterpolationNeedsDeferredSetup )
		{
			
			// Initial setup
			m_vecObserverInterpolateOffset = vOutOrigin - m_vecObserverInterpStartPos;
			m_flObsInterp_PathLength = m_vecObserverInterpolateOffset.Length();
			Vector vRight = m_vecObserverInterpolateOffset.Cross( Vector( 0, 0, 1 ) );
			Vector vUp = vRight.Cross( m_vecObserverInterpolateOffset );
			BasisToQuaternion( m_vecObserverInterpolateOffset.Normalized(), vRight.Normalized(), vUp.Normalized(), m_qObsInterp_OrientationTravelDir );
			m_bObserverInterpolationNeedsDeferredSetup = false;
		}

		float flPosProgress = ExponentialDecay( cl_obs_interp_pos_halflife.GetFloat(), cl_obs_interp_pos_rate.GetFloat(), gpGlobals->frametime );

		// Decay the offset vector until we reach the new observer position
		m_vecObserverInterpolateOffset *= flPosProgress;
		vOutOrigin -= m_vecObserverInterpolateOffset;

		// Angle interpolation is a function of position progress so they stay in sync (adding a slight head start for aesthetic reasons)
		float flPathProgress, flObserverInterpolateOffset = m_vecObserverInterpolateOffset.Length();
		if ( m_flObsInterp_PathLength <= 0.0001f * flObserverInterpolateOffset )
		{
			flPathProgress = 1.0f;
		}
		else
		{
			flPathProgress = ( flObserverInterpolateOffset / m_flObsInterp_PathLength ) - cl_obs_interp_angle_progress_headstart.GetFloat();
			flPathProgress = 1.0f - Clamp( flPathProgress, 0.0f, 1.0f );
		}

		// Messy and in flux... still tuning the interpolation code below
		Quaternion q1, q2;
		Quaternion qFinal;
		AngleQuaternion( vOutAngles, qFinal );
		float t = 0;
		if ( m_flObsInterp_PathLength > cl_obs_interp_dist_to_turn_to_face.GetFloat() && GetObserverMode() == OBS_MODE_IN_EYE )
		{
			// at a far enough distance, turn to face direction of motion before ending at final angles
			//QuaternionSlerp( m_qObsInterp_OrientationStart, m_qObsInterp_OrientationTravelDir, flPathProgress, q1 );
			//QuaternionSlerp( m_qObsInterp_OrientationTravelDir, qFinal, flPathProgress, q2 );

			if ( flPathProgress < cl_obs_interp_turn_to_face_start_frac.GetFloat() )
			{
				//QuaternionSlerp( m_qObsInterp_OrientationStart, m_qObsInterp_OrientationTravelDir, flPathProgress, q1 );
				q1 = m_qObsInterp_OrientationStart;
				q2 = m_qObsInterp_OrientationTravelDir;
				t = RemapVal( flPathProgress, 0.0, cl_obs_interp_turn_to_face_start_frac.GetFloat(), 0.0, 1.0 );
			}
			else if ( flPathProgress < cl_obs_interp_turn_to_face_end_frac.GetFloat() )
			{
				q1 = q2 = m_qObsInterp_OrientationTravelDir;
			}
			else
			{
				q1 = m_qObsInterp_OrientationTravelDir;
				q2 = qFinal;
				t = SimpleSplineRemapValClamped( flPathProgress, cl_obs_interp_turn_to_face_end_frac.GetFloat(), 1.0, 0.0, 1.0 );
			}
		}
		else
		{
			// Otherwise, interpolate from start to end orientation
			q1 = m_qObsInterp_OrientationStart;
			q2 = qFinal;
			t = flPathProgress;
		}

		Quaternion qOut;
		//QuaternionSlerp( q1, q2, flPathProgress, qOut );
		QuaternionSlerp( q1, q2, t, qOut );
		QuaternionAngles( qOut, vOutAngles );

		Assert( cl_obs_interp_snap_dist.GetFloat() < cl_obs_interp_settle_dist.GetFloat() );
		// At a close enough dist snap to final and stop interpolating
		if ( m_vecObserverInterpolateOffset.LengthSqr() < cl_obs_interp_snap_dist.GetFloat()*cl_obs_interp_snap_dist.GetFloat() )
		{
			m_obsInterpState = OBSERVER_INTERP_NONE;
			UpdateObserverTargetVisibility();
		}
		else if ( m_vecObserverInterpolateOffset.LengthSqr() < cl_obs_interp_settle_dist.GetFloat() * cl_obs_interp_settle_dist.GetFloat() )
		{
			m_obsInterpState = OBSERVER_INTERP_SETTLING;
			UpdateObserverTargetVisibility();
		}
	}
	Assert( vOutAngles.IsValid() && vOutOrigin.IsValid() );
}


void C_CSPlayer::UpdateObserverTargetVisibility( void ) const
{
	C_CSPlayer *pObservedPlayer = dynamic_cast < C_CSPlayer* >( GetObserverTarget() );
	if ( pObservedPlayer )
	{
		extern void UpdateViewmodelVisibility( C_BasePlayer *player );
		UpdateViewmodelVisibility( pObservedPlayer );
		pObservedPlayer->UpdateVisibility();
	}
}


bool C_CSPlayer::ShouldInterpolateObserverChanges() const
{
	if ( !cl_obs_interp_enable.GetBool() )
		return false;

	// server doesn't want this
	if ( sv_disable_observer_interpolation.GetBool() )
		return false;

	// Disallow when playing on a team in a competitive match
	bool bIsPlayingOnCompetitiveTeam = CSGameRules() && CSGameRules()->IsPlayingAnyCompetitiveStrictRuleset() && ( GetTeamNumber() == TEAM_CT || GetTeamNumber() == TEAM_TERRORIST );
	if ( bIsPlayingOnCompetitiveTeam )
		return false;

	// supported modes
	if ( GetObserverMode() != OBS_MODE_IN_EYE && GetObserverMode() != OBS_MODE_CHASE && GetObserverMode() != OBS_MODE_ROAMING )
		return false;

	// If we are in hltv and have a camera man, only run for that player. Otherwise, only run for the local player.
	CBasePlayer *pInterpPlayer = C_CSPlayer::GetLocalCSPlayer();

	if ( pInterpPlayer != this )
		return false;

	return true;
}

// Set up initial state for interpolating between observer positions
void C_CSPlayer::StartObserverInterpolation( const QAngle& startAngles )
{
	// Find obstructions in the path, skip past them if needed
	m_vecObserverInterpStartPos = MainViewOrigin();
	C_CSPlayer* pObserverTarget = dynamic_cast< C_CSPlayer* > ( GetObserverTarget() );
	if ( cl_obs_interp_obstruction_behavior.GetInt() > 0 && pObserverTarget )
	{
		trace_t tr;
		Ray_t ray;
		// HACK: This is wrong for chase and roam modes, but this is too early in the frame to call
		// CalcObserverView functions without trigger the absqueries valid assert... Revisit if needed,
		// but this will probably be a good enough test to see if we would lerp through solid
		ray.Init( pObserverTarget->GetNetworkOrigin() + VEC_VIEW, m_vecObserverInterpStartPos );
		CTraceFilterWorldAndPropsOnly filter;
		enginetrace->TraceRay( ray, MASK_VISIBLE, &filter, &tr );

		if ( tr.DidHit() )
		{
			if ( cl_obs_interp_obstruction_behavior.GetInt() == 1 )
			{
				m_vecObserverInterpStartPos = tr.endpos;
			}
			else if ( cl_obs_interp_obstruction_behavior.GetInt() == 2 )
			{
				m_obsInterpState = OBSERVER_INTERP_NONE;
				UpdateObserverTargetVisibility();
				return;
			}
		}
	}

	AngleQuaternion( startAngles, m_qObsInterp_OrientationStart );
	m_obsInterpState = OBSERVER_INTERP_TRAVELING;
	m_bObserverInterpolationNeedsDeferredSetup = true;
	UpdateObserverTargetVisibility();
}


C_CSPlayer::eObserverInterpState C_CSPlayer::GetObserverInterpState( void ) const
{
	if ( !ShouldInterpolateObserverChanges() )
		return OBSERVER_INTERP_NONE;

	return m_obsInterpState;
}

void C_CSPlayer::SetObserverTarget( EHANDLE hTarget )
{
	EHANDLE prevTarget = m_hObserverTarget;
	BaseClass::SetObserverTarget( hTarget );
	if ( hTarget && prevTarget != hTarget && ShouldInterpolateObserverChanges() )
	{
		StartObserverInterpolation( EyeAngles() );
	}
}

// [tj] checks if this player has another given player on their Steam friends list.
bool C_CSPlayer::HasPlayerAsFriend(C_CSPlayer* player)
{
    if (!steamapicontext || !steamapicontext->SteamFriends() || !steamapicontext->SteamUtils() || !player)
    {
        return false;
    }

    player_info_t pi;
    if ( !engine->GetPlayerInfo( player->entindex(), &pi ) )
    {
        return false;
    }

    if ( !pi.friendsID )
    {
        return false;
    }

    // check and see if they're on the local player's friends list
    CSteamID steamID( pi.friendsID, 1, steamapicontext->SteamUtils()->GetConnectedUniverse(), k_EAccountTypeIndividual );
    return steamapicontext->SteamFriends()->HasFriend( steamID, k_EFriendFlagImmediate);
}

// [menglish] Returns whether this player is dominating or is being dominated by the specified player
bool C_CSPlayer::IsPlayerDominated( int iPlayerIndex )
{
	return m_bPlayerDominated.Get( iPlayerIndex );
}

bool C_CSPlayer::IsPlayerDominatingMe( int iPlayerIndex )
{
	return m_bPlayerDominatingMe.Get( iPlayerIndex );
}


// helper interpolation functions
namespace Interpolators
{
	inline float Linear( float t ) { return t; }

	inline float SmoothStep( float t )
	{
		t = 3 * t * t - 2.0f * t * t * t;
		return t;
	}

	inline float SmoothStep2( float t )
	{
		return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
	}

	inline float SmoothStepStart( float t )
	{
		t = 0.5f * t;
		t = 3 * t * t - 2.0f * t * t * t;
		t = t* 2.0f;
		return t;
	}

	inline float SmoothStepEnd( float t )
	{
		t = 0.5f * t + 0.5f;
		t = 3 * t * t - 2.0f * t * t * t;
		t = (t - 0.5f) * 2.0f;
		return t;
	}
}

float C_CSPlayer::GetFreezeFrameInterpolant( void )
{

	float fCurTime = gpGlobals->curtime - m_flFreezeFrameStartTime;
	float fTravelTime = !m_bFreezeFrameCloseOnKiller ? spec_freeze_traveltime.GetFloat() : spec_freeze_traveltime_long.GetFloat();
	float fInterpolant = clamp( fCurTime / fTravelTime, 0.0f, 1.0f );

	return Interpolators::SmoothStepEnd( fInterpolant );
}

//-----------------------------------------------------------------------------
// Purpose: Calculate the view for the player while he's in freeze frame observer mode
//-----------------------------------------------------------------------------
void C_CSPlayer::CalcFreezeCamView( Vector& eyeOrigin, QAngle& eyeAngles, float& fov )
{
	C_BaseEntity *pTarget = GetObserverTarget();

	Vector prevEyeOrigin = eyeOrigin;
	QAngle prevEyeAngles = eyeAngles;

	float fInterpolant = GetFreezeFrameInterpolant();

	static ConVarRef sv_disablefreezecam( "sv_disablefreezecam" );
	if ( m_bAbortedFreezeFrame || !pTarget || pTarget == this || cl_disablefreezecam.GetBool() || sv_disablefreezecam.GetBool() )
	{
		if ( !pTarget )
		{
			if ( !m_bStartedFreezeFrame )
			{
				// randomly pick left or right
				int nLeftOrRight = RandomInt( 0, 1 );
				// sup dutch
				m_flFreezeFrameTilt = (nLeftOrRight > 0) ? RandomFloat( 4, 10 ) : RandomFloat( -4, -10 );
				m_bStartedFreezeFrame = true;
			}
			eyeAngles.z = Lerp( fInterpolant, 0.0f, m_flFreezeFrameTilt );
		}

		return CalcDeathCamView( eyeOrigin, eyeAngles, fov );
	}

	Vector targetOrig = pTarget->GetRenderOrigin();
	float flDistToTarg = (GetAbsOrigin() - targetOrig).Length2D();

	Vector vLookAt = pTarget->GetObserverCamOrigin();	// Returns ragdoll origin if they're ragdolled
	vLookAt += GetChaseCamViewOffset( pTarget );
	Vector vecCamTarget = vLookAt;
	if ( pTarget->IsAlive() )
	{
		// Look at their chest, not their head
		Vector maxs = GameRules()->GetViewVectors()->m_vHullMax;
		vecCamTarget.z -= (maxs.z * 0.25);
	}
	else
	{
		vecCamTarget.z += VEC_DEAD_VIEWHEIGHT.z;	// look over ragdoll, not through
	}

	float flScaler = pTarget->IsAlive() ? 0.1 : 0.075;
	float flDistFromCurToTarg2D = vLookAt.AsVector2D().DistTo( eyeOrigin.AsVector2D() );
	vecCamTarget.z -= clamp( (flDistFromCurToTarg2D) *flScaler, 0, 34 );

	// Figure out a view position in front of the target
	Vector vecEyeOnPlane = eyeOrigin;
	vecEyeOnPlane.z = vecCamTarget.z;
	Vector vTargetPos = vecCamTarget;
	Vector vToTarget = vTargetPos - vecEyeOnPlane;
	VectorNormalize( vToTarget );

	// Stop a few units away from the target, and shift up to be at the same height
	vTargetPos = vecCamTarget - (vToTarget * m_flFreezeFrameDistance);

	float flEyePosZ = pTarget->EyePosition().z;
	vTargetPos.z = flEyePosZ + m_flFreezeZOffset;

	if ( vToTarget == Vector( 0, 0, 0 ) )
	{
		// Abort!
		m_bAbortedFreezeFrame = true;
		return;
	}

	// Now trace out from the target, so that we're put in front of any walls
	trace_t trace;
	Vector vecHMinWall( -16, -16, -16 );
	Vector vecHMaxWall( 16, 16, 16 );
	C_BaseEntity::PushEnableAbsRecomputations( false ); // HACK don't recompute positions while doing RayTrace
	UTIL_TraceHull( vecCamTarget, vTargetPos, vecHMinWall, vecHMaxWall, MASK_SHOT, pTarget, COLLISION_GROUP_NONE, &trace );
	C_BaseEntity::PopEnableAbsRecomputations();
	if ( trace.fraction < 1.0 )
	{
		// The camera's going to be really close to the target. So we don't end up
		// looking at someone's chest, aim close freezecams at the target's eyes.
		vTargetPos = trace.endpos;
		vecCamTarget = vLookAt;

		// To stop all close in views looking up at character's chins, move the view up.
		vTargetPos.z += fabs( vecCamTarget.z - vTargetPos.z ) * 0.85;
		C_BaseEntity::PushEnableAbsRecomputations( false ); // HACK don't recompute positions while doing RayTrace
		UTIL_TraceHull( vecCamTarget, vTargetPos, WALL_MIN, WALL_MAX, MASK_SHOT, pTarget, COLLISION_GROUP_NONE, &trace );
		C_BaseEntity::PopEnableAbsRecomputations();
		vTargetPos = trace.endpos;
	}

	// move the eye toward our killer
	if ( !m_bStartedFreezeFrame )
	{
		m_vecFreezeFrameStart = eyeOrigin;

		m_bFreezeFrameCloseOnKiller = false;

		// randomly pick left or right
		int nLeftOrRight = RandomInt( 0, 1 );
		m_nFreezeFrameShiftSideDist = nLeftOrRight == 1 ? RandomInt( 12, 22 ) : RandomInt( -12, -22 );

		// sup dutch
		m_flFreezeFrameTilt = ( m_nFreezeFrameShiftSideDist > 0 ) ? RandomFloat( 6, 14 ) : RandomFloat( -6, -14 );

		if ( flDistToTarg >= FREEZECAM_LONGCAM_DIST )
		{
			m_bFreezeFrameCloseOnKiller = true;
		}
		else
		{
			// do a one time hull trace to the target and see if it's wide enough
			C_BaseEntity::PushEnableAbsRecomputations( false ); // HACK don't recompute positions while doing RayTrace
			Vector vecHMin(-16,-16,-16 );
			Vector vecHMax(16,16,16 );
			trace_t trace3;
			UTIL_TraceHull( m_vecFreezeFrameStart, vTargetPos, vecHMin, vecHMax, MASK_SHOT, this, COLLISION_GROUP_DEBRIS, &trace3 );
			C_BaseEntity::PopEnableAbsRecomputations();
			if ( trace3.fraction < 1.0 )
			{
				//Abort!
				m_bFreezeFrameCloseOnKiller = true;
			}
		}

		m_bStartedFreezeFrame = true;
	}

	//Vector vCamEnd = EyePosition() - ( vToTarget * 90 );
	if ( !m_bFreezeFrameCloseOnKiller && flDistToTarg < FREEZECAM_LONGCAM_DIST )
	{
		Vector vCamEnd = m_vecFreezeFrameStart - (vToTarget * 8);
		vCamEnd.z += MAX( (m_vecFreezeFrameStart.z - vecCamTarget.z)*0.75, 8 );

		Vector forward, right, up;
		QAngle angTemp;
		VectorAngles( vToTarget, angTemp );
		AngleVectors( angTemp, &forward, &right, &up );

		Vector vLRStart = m_vecFreezeFrameStart;
		Vector vLREnd = vCamEnd - (right * m_nFreezeFrameShiftSideDist);
		trace_t traceLR;
		UTIL_TraceHull( m_vecFreezeFrameStart, vLREnd, WALL_MIN, WALL_MAX, MASK_SOLID, this, COLLISION_GROUP_NONE, &traceLR );

		IRagdoll *pRagdoll = GetRepresentativeRagdoll();
		if ( pRagdoll )
		{
			vLRStart = pRagdoll->GetRagdollOrigin();
			vLRStart.z = trace.endpos.z;
		}
		float distToOffset = vLRStart.AsVector2D().DistTo( trace.endpos.AsVector2D() );
		if ( traceLR.fraction < 0.2 || distToOffset < 16 )
		{
			// Abort!
			m_bFreezeFrameCloseOnKiller = true;
		}

		m_vecFreezeFrameEnd = traceLR.endpos;
	}

	float flNewFOV = spec_freeze_target_fov.GetFloat();

	// dont frame both player, just zoom on killer
	if ( m_bFreezeFrameCloseOnKiller )
	{
		m_vecFreezeFrameEnd = vTargetPos;
		flNewFOV = spec_freeze_target_fov_long.GetFloat();
	}

	// Look directly at the target
	vToTarget = vecCamTarget - prevEyeOrigin;
	VectorNormalize( vToTarget );
	VectorAngles( vToTarget, eyeAngles );
	// apply the tilt
	eyeAngles.z = Lerp( fInterpolant, 0.0f, m_flFreezeFrameTilt );

	// set the fov to the convar, but have it hit the target slightly before the camera stops
	fov = clamp( Lerp( fInterpolant + 0.05f, (float) GetDefaultFOV(), flNewFOV ), flNewFOV, GetDefaultFOV() );
	Interpolator_CurveInterpolate( INTERPOLATE_SIMPLE_CUBIC, m_vecFreezeFrameStart - Vector( 0, 0, -12 ), m_vecFreezeFrameStart, m_vecFreezeFrameEnd, vTargetPos, fInterpolant, eyeOrigin );

	float fCurTime = gpGlobals->curtime - m_flFreezeFrameStartTime;
	float fTravelTime = !m_bFreezeFrameCloseOnKiller ? spec_freeze_traveltime.GetFloat() : spec_freeze_traveltime_long.GetFloat();

	// [jason] check that our target position does not fall within the render extents of the target we're looking at;
	//	this can happen if our killer is in a tight spot and the camera is trying to avoid clipping geometry
	const int kFreezeCamTolerance = cl_freeze_cam_penetration_tolerance.GetInt();
	if ( !m_bSentFreezeFrame && kFreezeCamTolerance >= 0 )
	{
		// at really long distances the camera moves long distances each frame
		// this means that thecamera could bypass the target spot by a bunch, so
		// let' check to see if the target will surpass the target pos next frame and just go ahead and stop
		bool bStopCamera = false;
		float distFromPrev = eyeOrigin.AsVector2D().DistTo( prevEyeOrigin.AsVector2D() );
		float distToTargetPos = m_vecFreezeFrameEnd.AsVector2D().DistTo( eyeOrigin.AsVector2D() );

		// either use the render extents of the target, or the value we specified in the convar
		float targetRadius = (float) kFreezeCamTolerance;
		if ( m_bFreezeFrameCloseOnKiller && targetRadius <= 0.0f )
		{
			Vector vecMins, vecMaxs;
			pTarget->GetRenderBounds( vecMins, vecMaxs );
			targetRadius = vecMins.AsVector2D().DistTo( vecMaxs.AsVector2D() ) * 0.3f;
		}

		// figure out ho much we moved last frame and keep from clipping too far into the killer's face
		if ( distToTargetPos - (distFromPrev - distToTargetPos) < (targetRadius*0.5f) )
		{
			bStopCamera = true;
		}

		// disregard height, treat the target as an infinite cylinder so we don't end up with extreme up/down view angles
		float distFromTarget = vLookAt.AsVector2D().DistTo( eyeOrigin.AsVector2D() );

		if ( (m_bFreezeFrameCloseOnKiller && distFromTarget < targetRadius) )
		{
			DevMsg( "CS_PLAYER: Detected overlap: Extents of target = %3.2f, Dist from them = %3.2f \n", targetRadius, distFromTarget );

			bStopCamera = true;
		}

		if ( bStopCamera )
		{
			eyeOrigin = prevEyeOrigin;
			eyeAngles = prevEyeAngles;
			fTravelTime = fCurTime;

			m_flFreezeFrameTilt = eyeAngles.z;
		}
	}

	if ( fCurTime >= fTravelTime && !m_bSentFreezeFrame )
	{
		IGameEvent *pEvent = gameeventmanager->CreateEvent( "freezecam_started" );
		if ( pEvent )
		{
			gameeventmanager->FireEventClientSide( pEvent );
		}

		m_bSentFreezeFrame = true;
		view->FreezeFrame( spec_freeze_time.GetFloat() );
	}
}

float C_CSPlayer::GetDeathCamInterpolationTime()
{
	static ConVarRef sv_disablefreezecam( "sv_disablefreezecam" );
	if ( cl_disablefreezecam.GetBool() || sv_disablefreezecam.GetBool() || !GetObserverTarget() )
		return spec_freeze_time.GetFloat();
	else
		return CS_DEATH_ANIMATION_TIME;

}

void C_CSPlayer::CalcDeathCamView( Vector& eyeOrigin, QAngle& eyeAngles, float& fov )
{
	CBaseEntity* pKiller = NULL;

	if ( mp_forcecamera.GetInt() == OBS_ALLOW_ALL )
	{
		// if mp_forcecamera is off let user see killer or look around
		pKiller = GetObserverTarget();
		eyeAngles = EyeAngles();
	}

	// NOTE: CS_DEATH_ANIMATION_TIME differs from base class implementation
	float interpolation = ( gpGlobals->curtime - m_flDeathTime ) / CS_DEATH_ANIMATION_TIME;

	interpolation = clamp( interpolation, 0.0f, 1.0f );

	m_flObserverChaseDistance += gpGlobals->frametime*48.0f;
	m_flObserverChaseDistance = clamp( m_flObserverChaseDistance, 16, CHASE_CAM_DISTANCE );

	QAngle aForward = eyeAngles;
	Vector origin = EyePosition();

	IRagdoll *pRagdoll = GetRepresentativeRagdoll();
	if ( pRagdoll )
	{
		origin = pRagdoll->GetRagdollOrigin();
		origin.z += VEC_DEAD_VIEWHEIGHT.z; // look over ragdoll, not through
	}

	if ( pKiller && pKiller->IsPlayer() && pKiller != this )
	{
		//Get the vector from the dead body EyePos to the killers EyePos
		Vector vKiller = pKiller->EyePosition() - origin;

		// Get the angles for that vector
		QAngle aKiller; VectorAngles( vKiller, aKiller );
		// Interpolate from the original eye angles to point at the killers EyePos
		InterpolateAngles( aForward, aKiller, eyeAngles, interpolation );
	}

	// Get the vector for our new view.  It should be looking at the killer
	Vector vForward; AngleVectors( eyeAngles, &vForward );

	VectorNormalize( vForward );
	// Add the two vectors with the negative chase distance as a scale
	VectorMA( origin, -m_flObserverChaseDistance, vForward, eyeOrigin );

	trace_t trace; // clip against world
	C_BaseEntity::PushEnableAbsRecomputations( false ); // HACK don't recompute positions while doing RayTrace
	UTIL_TraceHull( origin, eyeOrigin, WALL_MIN, WALL_MAX, MASK_SOLID, this, COLLISION_GROUP_NONE, &trace );
	C_BaseEntity::PopEnableAbsRecomputations();

	if ( trace.fraction < 1.0 )
	{
		eyeOrigin = trace.endpos;
		m_flObserverChaseDistance = VectorLength( origin - eyeOrigin );
	}

	fov = GetFOV();
}


//=============================================================================
// HPE_END
//=============================================================================

