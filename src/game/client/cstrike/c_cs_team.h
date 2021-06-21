//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Client side CTFTeam class
//
// $NoKeywords: $
//=============================================================================//

#ifndef C_CS_TEAM_H
#define C_CS_TEAM_H
#ifdef _WIN32
#pragma once
#endif

#include "c_team.h"
#include "shareddefs.h"

class C_BaseEntity;
class C_BaseObject;
class CBaseTechnology;

//-----------------------------------------------------------------------------
// Purpose: CS's Team manager
//-----------------------------------------------------------------------------
class C_CSTeam : public C_Team
{
	DECLARE_CLASS( C_CSTeam, C_Team );
public:
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

					C_CSTeam();
	virtual			~C_CSTeam();

	// Data Handling
	int		Get_ScoreThisPhase( void );

private:
	// Data received from the server
	int		m_iScoreThisPhase;
};

// Global team handling functions
C_CSTeam *GetGlobalCSTeam( int iTeamNumber );


#endif // C_CS_TEAM_H
