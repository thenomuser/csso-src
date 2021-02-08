//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#include "cbase.h"
#include "c_plantedc4.h"
#include "c_te_legacytempents.h"
#include "tempent.h"
#include "engine/IEngineSound.h"
#include "dlight.h"
#include "iefx.h"
#include "SoundEmitterSystem/isoundemittersystembase.h"
#include <bitbuf.h>
#include "cs_gamerules.h"
#include "util_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define PLANTEDC4_MSG_JUSTBLEW 1
#define TIME_TO_DETONATE_WARNING 10

ConVar cl_c4dynamiclight( "cl_c4dynamiclight", "0", 0, "Draw dynamic light when planted c4 flashes" );

IMPLEMENT_CLIENTCLASS_DT(C_PlantedC4, DT_PlantedC4, CPlantedC4)
	RecvPropBool( RECVINFO(m_bBombTicking) ),
	RecvPropFloat( RECVINFO(m_flC4Blow) ),
	RecvPropFloat( RECVINFO(m_flTimerLength) ),
	RecvPropFloat( RECVINFO(m_flDefuseLength) ),
	RecvPropFloat( RECVINFO(m_flDefuseCountDown) ),
	RecvPropBool( RECVINFO( m_bBombDefused ) ),
END_RECV_TABLE()

CUtlVector< C_PlantedC4* > g_PlantedC4s;

C_PlantedC4::C_PlantedC4()
{
	g_PlantedC4s.AddToTail( this );

	m_flNextRadarFlashTime = gpGlobals->curtime;
	m_bRadarFlash = true;

	// Don't beep right away, leave time for the planting sound
	m_flNextGlow = gpGlobals->curtime + 1.0;
	m_flNextBeep = gpGlobals->curtime + 1.0;

	SetBodygroup( FindBodygroupByName( "gift" ), UTIL_IsNewYear() );
}


C_PlantedC4::~C_PlantedC4()
{
	g_PlantedC4s.FindAndRemove( this );
}

void C_PlantedC4::SetDormant( bool bDormant )
{
	BaseClass::SetDormant( bDormant );
	
	// Remove us from the list of planted C4s.
	if ( bDormant )
	{
		g_PlantedC4s.FindAndRemove( this );
		m_bTenSecWarning = false;
	}
	else
	{
		if ( g_PlantedC4s.Find( this ) == -1 )
			g_PlantedC4s.AddToTail( this );
	}
}

void C_PlantedC4::Spawn( void )
{
	BaseClass::Spawn();

	AddFlag( FL_OBJECT );

	SetBodygroup( FindBodygroupByName( "gift" ), UTIL_IsNewYear() );

	SetNextClientThink( CLIENT_THINK_ALWAYS );

	m_bTenSecWarning = false;
	m_bExplodeWarning = false;
	m_bTriggerWarning = false;
}

void C_PlantedC4::ClientThink( void )
{
	BaseClass::ClientThink();

	// If it's dormant, don't beep or anything..
	if ( IsDormant() )
		return;

	if ( !m_bBombTicking )
	{
		// disable C4 thinking if not armed
		SetNextClientThink( CLIENT_THINK_NEVER );
		return;
	}

	if( gpGlobals->curtime > m_flNextBeep )
	{
		// as it gets closer to going off, increase the radius

		CLocalPlayerFilter filter;
		float attenuation;
		float freq;

		//the percent complete of the bomb timer
		float fComplete = ( ( m_flC4Blow - gpGlobals->curtime ) / m_flTimerLength );
		
		fComplete = clamp( fComplete, 0.0f, 1.0f );

		attenuation = MIN( 0.3 + 0.6 * fComplete, 1.0 );
		
		CSoundParameters params;

		if ( ( m_flC4Blow - gpGlobals->curtime ) > 1.0f && GetParametersForSound( "C4.PlantSound", params, NULL ) )
		{
			EmitSound_t ep( params );
			ep.m_SoundLevel = ATTN_TO_SNDLVL( attenuation );
			ep.m_pOrigin = &GetAbsOrigin();

			EmitSound( filter, SOUND_FROM_WORLD, ep );
		}

		freq = MAX( 0.1 + 0.9 * fComplete, 0.15 );

		m_flNextBeep = gpGlobals->curtime + freq;
	}
	
	if( ( m_flC4Blow - gpGlobals->curtime ) <= TIME_TO_DETONATE_WARNING &&
		!m_bTenSecWarning )
	{
		m_bTenSecWarning = true;
		
		CLocalPlayerFilter filter;
		PlayMusicSelection( filter, CSMUSIC_BOMBTEN );
	}

	if ( (m_flC4Blow - gpGlobals->curtime) < 1.0f &&
		 !m_bTriggerWarning )
	{
		EmitSound( "C4.ExplodeTriggerTrip" );

		int ledAttachmentIndex = LookupAttachment( "led" );
		DispatchParticleEffect( "c4_timer_light_trigger", PATTACH_POINT_FOLLOW, this, ledAttachmentIndex, false );

		m_bTriggerWarning = true;
	}

	if ( (m_flC4Blow - gpGlobals->curtime) < 0.0f &&
		 !m_bExplodeWarning )
	{
		EmitSound( "C4.ExplodeWarning" );

		m_bExplodeWarning = true;
	}

	if( gpGlobals->curtime > m_flNextGlow )
	{
		if ( gpGlobals->curtime > m_flNextGlow && (m_flC4Blow - gpGlobals->curtime) > 1.0f )
		{
			int ledAttachmentIndex = LookupAttachment( "led" );
			DispatchParticleEffect( "c4_timer_light", PATTACH_POINT_FOLLOW, this, ledAttachmentIndex, false );
		}

		float freq = 0.1 + 0.9 * ((m_flC4Blow - gpGlobals->curtime) / m_flTimerLength);

		if ( freq < 0.15 ) freq = 0.15;

		m_flNextGlow = gpGlobals->curtime + freq;
	}
}


//=============================================================================
// HPE_BEGIN:
// [menglish] Create the client side explosion particle effect for when the bomb explodes and hide the bomb
//=============================================================================
void C_PlantedC4::Explode( void )
{
	AddEffects( EF_NODRAW );
	SetDormant( true );
}
//=============================================================================
// HPE_END
//=============================================================================