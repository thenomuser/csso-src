//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#ifndef C_CS_PLAYER_H
#define C_CS_PLAYER_H
#ifdef _WIN32
#pragma once
#endif


#include "cs_playeranimstate.h"
#include "c_baseplayer.h"
#include "cs_shareddefs.h"
#include "weapon_csbase.h"
#include "baseparticleentity.h"
#include "beamdraw.h"
#include "cs_gamerules.h"
#include "weapon_basecsgloves.h"

#include "cs_player_shared.h"

class C_PhysicsProp;

extern ConVar cl_disablefreezecam;

class CAddonModel
{
public:
	CHandle<C_BaseAnimating> m_hEnt;	// The model for the addon.
	int m_iAddon;						// One of the ADDON_ bits telling which model this is.
	int m_iAttachmentPoint;				// Which attachment point on the player model this guy is on.
};



class C_CSPlayer : public C_BasePlayer, public ICSPlayerAnimStateHelpers
{
public:
	DECLARE_CLASS( C_CSPlayer, C_BasePlayer );
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_INTERPOLATION();

	C_CSPlayer();
	~C_CSPlayer();

	virtual void Simulate();

	void GiveCarriedHostage( EHANDLE hHostage );
	void RefreshCarriedHostage( bool bForceCreate );
	void RemoveCarriedHostage();

	bool HasDefuser() const;

	void GiveDefuser();
	void RemoveDefuser();

	bool HasNightVision() const;

	static C_CSPlayer* GetLocalCSPlayer();
	CSPlayerState State_Get() const;

	virtual float GetMinFOV() const;

	// Get how much $$$ this guy has.
	int GetAccount() const;

	// Returns one of the CS_CLASS_ enums.
	int PlayerClass() const;

	bool IsInBuyZone();
	bool CanShowTeamMenu() const;	// Returns true if we're allowed to show the team menu right now.

	// Get the amount of armor the player has.
	int ArmorValue() const;
	bool HasHelmet() const;
	int GetCurrentAssaultSuitPrice();

#if USE_TRACERS
	// create tracers
	void CreateWeaponTracer( Vector vecStart, Vector vecEnd );
#endif

	virtual const QAngle& EyeAngles();
	virtual const QAngle& GetRenderAngles();
	virtual void CalcObserverView( Vector& eyeOrigin, QAngle& eyeAngles, float& fov );

	virtual void			GetRenderBounds( Vector& theMins, Vector& theMaxs );
	virtual void			GetShadowRenderBounds( Vector &mins, Vector &maxs, ShadowType_t shadowType );
	virtual bool			GetShadowCastDirection( Vector *pDirection, ShadowType_t shadowType ) const;

	virtual void VPhysicsUpdate( IPhysicsObject *pPhysics );

	// Get the ID target entity index. The ID target is the player that is behind our crosshairs, used to
	// display the player's name.
	int GetIDTarget() const;

	virtual void NotifyShouldTransmit( ShouldTransmitState_t state );
	virtual void ClientThink();

	virtual void OnDataChanged( DataUpdateType_t type );
	virtual void PostDataUpdate( DataUpdateType_t updateType );
	virtual bool Interpolate( float currentTime );
	virtual void UpdateStepSound( surfacedata_t *psurface, const Vector &vecOrigin, const Vector &vecVelocity  );
	virtual surfacedata_t * GetFootstepSurface( const Vector &origin, const char *surfaceName );
	virtual void PlayClientJumpSound( void );
	virtual void ValidateModelIndex( void );

	virtual int	GetMaxHealth() const;

	bool		Weapon_CanSwitchTo(C_BaseCombatWeapon *pWeapon);

	virtual void UpdateClientSideAnimation();
	virtual void ProcessMuzzleFlashEvent();
	void HandleTaserAnimation();

	virtual const Vector& GetRenderOrigin( void );

	bool CreateMove( float flInputSampleTime, CUserCmd *pCmd );

	CUtlVector< C_BaseParticleEntity* > m_SmokeGrenades;

	virtual bool ShouldDraw( void );
	virtual void BuildTransformations( CStudioHdr *pStudioHdr, Vector *pos, Quaternion q[], const matrix3x4_t& cameraTransform, int boneMask, CBoneBitList &boneComputed );

	virtual C_BaseAnimating * BecomeRagdollOnClient();
	virtual IRagdoll* GetRepresentativeRagdoll() const;

	void ImpactTrace( trace_t *pTrace, int iDamageType, const char *pCustomImpactName );

	// Have this player play the sounds from his view model's reload animation.
	void PlayReloadEffect();

	virtual void FireEvent( const Vector& origin, const QAngle& angles, int event, const char *options );

	virtual void DropPhysicsMag( const char *options ) OVERRIDE;

	bool		 HasC4( void );

	virtual void CreateLightEffects( void ) {}	//no dimlight effects

	// Sometimes the server wants to update the client's cycle to get the two to run in sync (for proper hit detection)
	virtual void SetServerIntendedCycle( float intended ) { m_serverIntendedCycle = intended; }
	virtual float GetServerIntendedCycle( void ) { return m_serverIntendedCycle; }

	virtual bool IsLookingAtWeapon( void ) const { return m_bIsLookingAtWeapon; }
	virtual bool IsHoldingLookAtWeapon( void ) const { return m_bIsHoldingLookAtWeapon; }

	virtual bool ShouldReceiveProjectedTextures( int flags )
	{
		return ( this != C_BasePlayer::GetLocalPlayer() );
	}

	void ClearSoundEvents()
	{
		m_SoundEvents.RemoveAll();
	}

	CsMusicType_t GetCurrentMusic() { return m_nCurrentMusic; } const

	void SetCurrentMusic( CsMusicType_t nMusicType )
	{
		m_nCurrentMusic = nMusicType;
 		if( nMusicType == CSMUSIC_START )
 		{
			m_flMusicRoundStartTime = gpGlobals->curtime;
		}
		m_flCurrentMusicStartTime = gpGlobals->curtime;
	}

	float GetCurrentMusicElapsed()
	{
		return  gpGlobals->curtime - m_flCurrentMusicStartTime;
	}

	float GetMusicStartRoundElapsed()
	{
		return  gpGlobals->curtime - m_flMusicRoundStartTime;
	}

	// [menglish] Returns whether this player is dominating or is being dominated by the specified player
	bool IsPlayerDominated( int iPlayerIndex );
	bool IsPlayerDominatingMe( int iPlayerIndex );

	virtual void CalcFreezeCamView( Vector& eyeOrigin, QAngle& eyeAngles, float& fov );

	virtual float GetDeathCamInterpolationTime();

	AcquireResult::Type CanAcquire( CSWeaponID weaponId, AcquireMethod::Type acquireMethod );
	int					GetCarryLimit( CSWeaponID weaponId );

	bool IsOtherEnemy( CCSPlayer *pPlayer );
	bool IsOtherEnemy( int nEntIndex );


// Called by shared code.
public:

	// IPlayerAnimState overrides.
	virtual CWeaponCSBase* CSAnim_GetActiveWeapon();
	virtual bool CSAnim_CanMove();

	// View model prediction setup
	virtual void		CalcView( Vector& eyeOrigin, QAngle& eyeAngles, float& zNear, float& zFar, float& fov );


	void DoAnimationEvent( PlayerAnimEvent_t event, int nData = 0 );


// Implemented in shared code.
public:
	virtual float GetPlayerMaxSpeed();
	bool IsPrimaryOrSecondaryWeapon( CSWeaponType nType );

	bool GetUseConfigurationForHighPriorityUseEntity( CBaseEntity *pEntity, CConfigurationForHighPriorityUseEntity_t &cfg );
	bool GetUseConfigurationForHighPriorityUseEntity( CBaseEntity *pEntity );
	CBaseEntity *GetUsableHighPriorityEntity( void );

	void GetBulletTypeParameters(
		int iBulletType,
		float &fPenetrationPower,
		float &flPenetrationDistance );

	void FireBullet(
		Vector vecSrc,
		const QAngle &shootAngles,
		float flDistance,
		float flPenetration,
		int nPenetrationCount,
		int iBulletType,
		int iDamage,
		float flRangeModifier,
		CBaseEntity *pevAttacker,
		bool bDoEffects,
		float xSpread, float ySpread );

	bool HandleBulletPenetration(
		float &flPenetration,
		int &iEnterMaterial,
		bool &hitGrate,
		trace_t &tr,
		Vector &vecDir,
		surfacedata_t *pSurfaceData,
		float flPenetrationModifier,
		float flDamageModifier,
		bool bDoEffects,
		int iDamageType,
		float flPenetrationPower,
		int &nPenetrationCount,
		Vector &vecSrc,
		float flDistance,
		float flCurrentDistance,
		float &fCurrentDamage );

	virtual QAngle	GetAimPunchAngle( void );
	QAngle	GetRawAimPunchAngle( void ) const;

	void KickBack(
		float fAngle,
		float fMagnitude );

	// Returns true if the player is allowed to move.
	bool CanMove() const;

	void OnJump( float fImpulse );
	void OnLand( float fVelocity );

	bool HasC4() const;	// Is this player carrying a C4 bomb?
	bool IsVIP() const;	// Is this player the VIP?

	virtual void SetAnimation( PLAYER_ANIM playerAnim );
	
	virtual bool ShouldInterpolate( void );


public:

	void UpdateIDTarget( void );
	void RemoveAddonModels( void );
	void UpdateMinModels( void );

	void SetActivity( Activity eActivity );
	Activity GetActivity( void ) const;

	// Global/static methods
	virtual void ThirdPersonSwitch( bool bThirdperson );

	IPlayerAnimState *GetPlayerAnimState() { return m_PlayerAnimState; }

public:

	IPlayerAnimState *m_PlayerAnimState;

	// Used to control animation state.
	Activity m_Activity;

	CNetworkVar( bool, m_bIsWalking );
	// Predicted variables.
	CNetworkVar( bool, m_bResumeZoom );
	CNetworkVar( int , m_iLastZoom ); // after firing a shot, set the FOV to 90, and after showing the animation, bring the FOV back to last zoom level.
	CNetworkVar( CSPlayerState, m_iPlayerState );	// SupraFiend: this gives the current state in the joining process, the states are listed above
	CNetworkVar( bool, m_bIsDefusing );			// tracks whether this player is currently defusing a bomb
	CNetworkVar( bool, m_bIsGrabbingHostage );	// tracks whether this player is currently grabbing a hostage
	CNetworkVar( bool, m_bHasMovedSinceSpawn ); // Whether player has moved from spawn position
	CNetworkVar( float, m_fImmuneToDamageTime );	// When gun game spawn damage immunity will expire
	CNetworkVar( bool, m_bImmunity );	// tracks whether this player is currently immune in gun game
	CNetworkVar( bool, m_bInBombZone );
	CNetworkVar( bool, m_bInBuyZone );
	CNetworkVar( int, m_iThrowGrenadeCounter );	// used to trigger grenade throw animations.

	CNetworkVar( bool, m_bKilledByTaser );
	CNetworkVar( int, m_iMoveState );		// Is the player trying to run or walk or idle?  Tells us what the player is "trying" to do.

	const PlayerViewmodelArmConfig *m_pViewmodelArmConfig;

	bool IsInHostageRescueZone( void );

	// This is a combination of the ADDON_ flags in cs_shareddefs.h.
	CNetworkVar( int, m_iAddonBits );

	// Clients don't know about holstered weapons, so we need to be told about them here
	CNetworkVar( int, m_iPrimaryAddon );
	CNetworkVar( int, m_iSecondaryAddon );
	CNetworkVar( int, m_iKnifeAddon );

	// How long the progress bar takes to get to the end. If this is 0, then the progress bar
	// should not be drawn.
	CNetworkVar( int, m_iProgressBarDuration );

	// When the progress bar should start.
	CNetworkVar( float, m_flProgressBarStartTime );

	CNetworkVar( bool, m_bDuckOverride ); // force the player to duck regardless of if they're holding crouch

	CNetworkVar( float, m_flStamina );
	CNetworkVar( int, m_iDirection );	// The current lateral kicking direction; 1 = right,  0 = left
	CNetworkVar( int, m_iShotsFired );	// number of shots fired recently
	CNetworkVar( bool, m_bNightVisionOn );
	CNetworkVar( bool, m_bHasNightVision );

    // [dwenger] Added for fun-fact support
    //CNetworkVar( bool, m_bPickedUpDefuser );
    //CNetworkVar( bool, m_bDefusedWithPickedUpKit );

	CNetworkVar( float, m_flVelocityModifier );
	CNetworkVar( float, m_flGroundAccelLinearFracLastTime );

	CNetworkVar( bool, m_bNeedToChangeGloves );

	bool		m_bDetected;

	EHANDLE	m_hRagdoll;

	EHANDLE	m_hCarriedHostage;
	EHANDLE	m_hCarriedHostageProp;
	bool	m_bPlayingHostageCarrySound;

	CWeaponCSBase* GetActiveCSWeapon() const;
	CWeaponCSBase* GetCSWeapon( CSWeaponID id ) const;

	virtual ShadowType_t		ShadowCastType();

#ifdef CS_SHIELD_ENABLED
	bool HasShield( void ) { return m_bHasShield; }
	bool IsShieldDrawn( void ) { return m_bShieldDrawn;	}
	void SetShieldDrawnState( bool bState ) { m_bShieldDrawn = bState; }
#else
	bool HasShield( void ) { return false; }
	bool IsShieldDrawn( void ) { return false; }
	void SetShieldDrawnState( bool bState ) {}
#endif

	float m_flNightVisionAlpha;

	float m_flFlashBangTime;		// end time
	float m_flFlashScreenshotAlpha;
	float m_flFlashOverlayAlpha;
	bool m_bFlashBuildUp;
	bool m_bFlashDspHasBeenCleared;
	bool m_bFlashScreenshotHasBeenGrabbed;

	bool IsFlashBangActive( void ) { return ( m_flFlashDuration > 0.0f ) && ( gpGlobals->curtime < m_flFlashBangTime ); }
	float GetFlashStartTime( void ) { return (m_flFlashBangTime - m_flFlashDuration); }
	float GetFlashTimeElapsed( void ) { return MAX( gpGlobals->curtime - GetFlashStartTime(), 0.0f ); }

	bool IsBlinded( void ) { return (m_flFlashBangTime - 1.0f) > gpGlobals->curtime; }
	CNetworkVar( float, m_flFlashMaxAlpha );
	CNetworkVar( float, m_flFlashDuration );

	// Having the RecvProxy in the player allows us to keep the var private
	static void RecvProxy_CycleLatch( const CRecvProxyData *pData, void *pStruct, void *pOut );

	// Bots and hostages auto-duck during jumps
	bool m_duckUntilOnGround;

	Vector m_lastStandingPos; // used by the gamemovement code for finding ladders

	void SurpressLadderChecks( const Vector& pos, const Vector& normal );
	bool CanGrabLadder( const Vector& pos, const Vector& normal );

// [tj] checks if this player has another given player on their Steam friends list.
	bool HasPlayerAsFriend( C_CSPlayer* player );

	bool IsAbleToInstantRespawn( void );

private:
	void UpdateFlashBangEffect( void );

	CountdownTimer m_ladderSurpressionTimer;
	Vector m_lastLadderNormal;
	Vector m_lastLadderPos;

	void UpdateRadar();
	void UpdateSoundEvents();

	void CreateAddonModel( int i );
	void UpdateAddonModels();
	void UpdateHostageCarryModels();

	void UpdateGlovesModel();
	void RemoveGlovesModel();
	CBaseCSGloves* m_pCSGloves;

	void PushawayThink();

	void FireGameEvent( IGameEvent *event );

	int		m_iAccount;
	bool	m_bHasHelmet;

public:
	int		m_iClass;

private:
	int		m_ArmorValue;
	QAngle	m_angEyeAngles;
	bool	m_bHasDefuser;
	bool	m_bInHostageRescueZone;
	float	m_fNextThinkPushAway;

    bool    m_bPlayingFreezeCamSound;

#ifdef CS_SHIELD_ENABLED
	bool	m_bHasShield;
	bool	m_bShieldDrawn;
#endif

	Vector m_vecRagdollVelocity;

	CInterpolatedVar< QAngle >	m_iv_angEyeAngles;

	// ID Target
	int					m_iIDEntIndex;
	CountdownTimer		m_delayTargetIDTimer;

	// Show the ID target after the cursor leaves the entity
	int					m_iOldIDEntIndex;
	CountdownTimer		m_holdTargetIDTimer;

	void ReleaseFlashlight( void );
	Beam_t	*m_pFlashlightBeam;

	class CCSSoundEvent
	{
	public:
		string_t m_SoundName;
		float m_flEventTime;	// Play the event when gpGlobals->curtime goes past this.
	};
	CUtlLinkedList<CCSSoundEvent,int> m_SoundEvents;

	// manage per play music
	CsMusicType_t m_nCurrentMusic;
	float m_flCurrentMusicStartTime;
	float m_flMusicRoundStartTime;

	// This is the list of addons hanging off the guy (grenades, C4, nightvision, etc).
	CUtlLinkedList<CAddonModel, int> m_AddonModels;
	int m_iLastAddonBits;
	int m_iLastPrimaryAddon;
	int m_iLastSecondaryAddon;
	int m_iLastKnifeAddon;

	int m_cycleLatch;				// server periodically updates this to fix up our anims, here it is a 4 bit fixed point
	float m_serverIntendedCycle;	// server periodically updates this to fix up our anims, here it is the float we want, or -1 for no override


    // [tj] Network variables that track who are dominating and being dominated by
    CNetworkArray( bool, m_bPlayerDominated, MAX_PLAYERS+1 );		// array of state per other player whether player is dominating other players
    CNetworkArray( bool, m_bPlayerDominatingMe, MAX_PLAYERS+1 );	// array of state per other player whether other players are dominating this player

	CNetworkVar( bool, m_bIsLookingAtWeapon );
	CNetworkVar( bool, m_bIsHoldingLookAtWeapon );

public:
	CNetworkVar( int, m_iLoadoutSlotGlovesCT );
	CNetworkVar( int, m_iLoadoutSlotGlovesT );

	float m_flThirdpersonRecoil;

	// taser items	
	float m_nextTaserShakeTime;
	float m_firstTaserShakeTime;

	C_CSPlayer( const C_CSPlayer & );

public:

	virtual bool	GetAttachment( int number, Vector &origin );
	virtual	bool	GetAttachment( int number, Vector &origin, QAngle &angles );

	bool	IsBotOrControllingBot();

#if CS_CONTROLLABLE_BOTS_ENABLED
	bool IsControllingBot()							const { return m_bIsControllingBot; }
	bool CanControlObservedBot()					const { return m_bCanControlObservedBot; }

	int GetControlledBotIndex()						const { return m_iControlledBotEntIndex; }

private:
	bool		m_bIsControllingBot;	// Are we controlling a bot? 
	// Note that this can be TRUE even if GetControlledPlayer() returns NULL, IFF we started controlling a bot and then the bot was deleted for some reason. 
	bool		m_bCanControlObservedBot;	// True if the player can control the bot s/he is observing
	int			m_iControlledBotEntIndex;

	CNetworkVar( bool, m_bHasControlledBotThisRound );
#endif

private:
	float		m_flNextMagDropTime;
	int			m_nLastMagDropAttachmentIndex;

public:
	Vector m_vecLastAliveLocalVelocity;
};

C_CSPlayer* GetLocalOrInEyeCSPlayer( void );
C_CSPlayer* GetHudPlayer( void );	// get the player we should show the HUD for (local or observed)

inline C_CSPlayer *ToCSPlayer( CBaseEntity *pEntity )
{
	if ( !pEntity || !pEntity->IsPlayer() )
		return NULL;

	return dynamic_cast<C_CSPlayer*>( pEntity );
}

namespace vgui
{
	class IImage;
}

vgui::IImage* GetDefaultAvatarImage( C_BasePlayer *pPlayer );

bool LineGoesThroughSmoke( Vector from, Vector to, bool grenadeBloat );




#endif // C_CS_PLAYER_H
