//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Client side C_CSTeam class
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "engine/IEngineSound.h"
#include "hud.h"
#include "recvproxy.h"
#include "c_cs_team.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


IMPLEMENT_CLIENTCLASS_DT(C_CSTeam, DT_CSTeam, CCSTeam)
	RecvPropInt( RECVINFO(m_iScoreThisPhase)),
END_RECV_TABLE()

BEGIN_PREDICTION_DATA( C_CSTeam )
	DEFINE_PRED_FIELD( m_iScoreThisPhase, FIELD_INTEGER, FTYPEDESC_PRIVATE ),
END_PREDICTION_DATA();

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_CSTeam::C_CSTeam()
{
	m_iScoreThisPhase = 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_CSTeam::~C_CSTeam()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int C_CSTeam::Get_ScoreThisPhase( void )
{
	return m_iScoreThisPhase;
}


//-----------------------------------------------------------------------------
// Purpose: Get the C_Team for the specified team number
//-----------------------------------------------------------------------------
C_CSTeam *GetGlobalCSTeam( int iTeamNumber )
{
	return (C_CSTeam*) GetGlobalTeam( iTeamNumber );
}

