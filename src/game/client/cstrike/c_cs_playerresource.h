//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: CS's custom C_PlayerResource
//
// $NoKeywords: $
//=============================================================================//

#ifndef C_CS_PLAYERRESOURCE_H
#define C_CS_PLAYERRESOURCE_H
#ifdef _WIN32
#pragma once
#endif

#include "cs_shareddefs.h"
#include "c_playerresource.h"

#define MAX_DECORATED_PLAYER_NAME_LENGTH ( ( MAX_NETWORKID_LENGTH * 10 ) + 20 )

class C_CS_PlayerResource : public C_PlayerResource
{
	DECLARE_CLASS( C_CS_PlayerResource, C_PlayerResource );
public:
	DECLARE_CLIENTCLASS();

					C_CS_PlayerResource();
	virtual			~C_CS_PlayerResource();

	bool			IsVIP(int iIndex );
	bool			HasC4(int iIndex );
	bool			IsHostageAlive(int iIndex);
	bool			IsHostageFollowingSomeone(int iIndex);
	const Vector	GetHostagePosition( int index );
	int				GetHostageEntityID(int iIndex);
	const Vector	GetC4Postion();
	const Vector	GetBombsiteAPosition();
	const Vector	GetBombsiteBPosition();
	const Vector	GetHostageRescuePosition( int index );
	int				GetPlayerClass( int iIndex );

	bool			IsBombSpotted( void ) const;
	bool			IsPlayerSpotted( int iIndex );

	const char		*GetClanTag( int index );

	int				GetNumMVPs( int iIndex );
	bool			HasDefuser( int iIndex );

#if CS_CONTROLLABLE_BOTS_ENABLED
	bool			IsControllingBot( int index );
	int				GetControlledPlayer( int index );
	int				GetControlledByPlayer( int index );
	virtual void	UpdatePlayerName( int slot );
#endif

protected:

	int		m_iPlayerC4;	// entity index of C4 carrier or 0
	int		m_iPlayerVIP;	// entity index of VIP player or 0
	Vector	m_vecC4;		// position of C4
	Vector	m_bombsiteCenterA;	
	Vector	m_bombsiteCenterB;	

	bool	m_bHostageAlive[MAX_HOSTAGES];
	bool	m_isHostageFollowingSomeone[MAX_HOSTAGES];
	int		m_iHostageEntityIDs[MAX_HOSTAGES];
	int		m_iHostageX[MAX_HOSTAGES];
	int		m_iHostageY[MAX_HOSTAGES];
	int		m_iHostageZ[MAX_HOSTAGES];

	int		m_hostageRescueX[MAX_HOSTAGE_RESCUES];
	int		m_hostageRescueY[MAX_HOSTAGE_RESCUES];
	int		m_hostageRescueZ[MAX_HOSTAGE_RESCUES];

	bool	m_bBombSpotted;
	bool	m_bPlayerSpotted[ MAX_PLAYERS + 1 ];
	int		m_iPlayerClasses[ MAX_PLAYERS + 1 ];

	char	m_szClan[MAX_PLAYERS+1][MAX_CLAN_TAG_LENGTH];

	int		m_iMVPs[ MAX_PLAYERS + 1 ];	 
	bool	m_bHasDefuser[ MAX_PLAYERS + 1 ];

#if CS_CONTROLLABLE_BOTS_ENABLED
	bool	m_bControllingBot[MAX_PLAYERS + 1];
	int		m_iControlledPlayer[MAX_PLAYERS + 1];
	int		m_iControlledByPlayer[MAX_PLAYERS + 1];
#endif
};

C_CS_PlayerResource *GetCSResources( void );

#endif // C_CS_PLAYERRESOURCE_H
