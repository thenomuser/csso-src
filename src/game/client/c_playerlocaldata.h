//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Defines the player specific data that is sent only to the player
//			to whom it belongs.
//
// $NoKeywords: $
//=============================================================================//

#ifndef C_PLAYERLOCALDATA_H
#define C_PLAYERLOCALDATA_H
#ifdef _WIN32
#pragma once
#endif

#include "basetypes.h"
#include "mathlib/vector.h"
#include "playernet_vars.h"

//-----------------------------------------------------------------------------
// Purpose: Player specific data ( sent only to local player, too )
//-----------------------------------------------------------------------------
class CPlayerLocalData
{
public:
	DECLARE_PREDICTABLE();
	DECLARE_CLASS_NOBASE( CPlayerLocalData );
	DECLARE_EMBEDDED_NETWORKVAR();

	CPlayerLocalData() :
		m_iv_viewPunchAngle( "CPlayerLocalData::m_iv_viewPunchAngle" ),
		m_iv_aimPunchAngle( "CPlayerLocalData::m_iv_aimPunchAngle" ),
		m_iv_aimPunchAngleVel( "CPlayerLocalData::m_iv_aimPunchAngleVel" )
	{
		m_iv_viewPunchAngle.Setup( &m_viewPunchAngle, LATCH_SIMULATION_VAR );
		m_iv_aimPunchAngle.Setup( &m_aimPunchAngle, LATCH_SIMULATION_VAR );
		m_iv_aimPunchAngleVel.Setup( &m_aimPunchAngleVel, LATCH_SIMULATION_VAR );
		m_flFOVRate = 0;

		m_flOldFallVelocity = 0.0;

		m_bInLanding = false;
		m_flLandingTime = -1.0f;

		m_flLastDuckTime = -1.0f;
	}

	unsigned char			m_chAreaBits[MAX_AREA_STATE_BYTES];				// Area visibility flags.
	unsigned char			m_chAreaPortalBits[MAX_AREA_PORTAL_STATE_BYTES];// Area portal visibility flags.

	int						m_iHideHUD;			// bitfields containing sections of the HUD to hide
	
	float					m_flFOVRate;		// rate at which the FOV changes
	

	bool					m_bDucked;
	bool					m_bDucking;
	float					m_flLastDuckTime;	// last time the player pressed duck
	bool					m_bInDuckJump;
	float					m_flDucktime;
	float					m_flDuckJumpTime;
	float					m_flJumpTime;
	int						m_nStepside;
	float					m_flFallVelocity;
	float					m_flOldFallVelocity;
	int						m_nOldButtons;
	// Base velocity that was passed in to server physics so 
	//  client can predict conveyors correctly.  Server zeroes it, so we need to store here, too.
	Vector					m_vecClientBaseVelocity;
	CNetworkQAngle( m_viewPunchAngle );			// auto-decaying view angle adjustment
	CInterpolatedVar< QAngle >	m_iv_viewPunchAngle;
	CNetworkQAngle( m_aimPunchAngle );			// auto-decaying aim angle adjustment
	CInterpolatedVar< QAngle >	m_iv_aimPunchAngle;
	CNetworkQAngle( m_aimPunchAngleVel );		// velocity of auto-decaying aim angle adjustment
	CInterpolatedVar< QAngle >	m_iv_aimPunchAngleVel;
	bool					m_bDrawViewmodel;
	bool					m_bWearingSuit;
	bool					m_bPoisoned;
	float					m_flStepSize;
	bool					m_bAllowAutoMovement;

	bool					m_bInLanding;
	float					m_flLandingTime;

	// 3d skybox
	sky3dparams_t			m_skybox3d;
	// fog params
	fogplayerparams_t		m_PlayerFog;
	// audio environment
	audioparams_t			m_audio;

	bool					m_bSlowMovement;

};

#endif // C_PLAYERLOCALDATA_H
