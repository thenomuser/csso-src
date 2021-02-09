//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: CS's custom C_PlayerResource
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "c_cs_playerresource.h"
#include <shareddefs.h>
#include <cs_shareddefs.h>
#include "hud.h"
#include "gamestringpool.h"
#include "c_cs_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT(C_CS_PlayerResource, DT_CSPlayerResource, CCSPlayerResource)
	RecvPropInt( RECVINFO( m_iPlayerC4 ) ),
	RecvPropInt( RECVINFO( m_iPlayerVIP ) ),
	RecvPropVector( RECVINFO(m_vecC4) ),
	RecvPropArray3( RECVINFO_ARRAY(m_bHostageAlive), RecvPropInt( RECVINFO(m_bHostageAlive[0]))),
	RecvPropArray3( RECVINFO_ARRAY(m_isHostageFollowingSomeone), RecvPropInt( RECVINFO(m_isHostageFollowingSomeone[0]))),
	RecvPropArray3( RECVINFO_ARRAY(m_iHostageEntityIDs), RecvPropInt( RECVINFO(m_iHostageEntityIDs[0]))),
	RecvPropArray3( RECVINFO_ARRAY(m_iHostageX), RecvPropInt( RECVINFO(m_iHostageX[0]))),
	RecvPropArray3( RECVINFO_ARRAY(m_iHostageY), RecvPropInt( RECVINFO(m_iHostageY[0]))),
	RecvPropArray3( RECVINFO_ARRAY(m_iHostageZ), RecvPropInt( RECVINFO(m_iHostageZ[0]))),
	RecvPropVector( RECVINFO(m_bombsiteCenterA) ),
	RecvPropVector( RECVINFO(m_bombsiteCenterB) ),
	RecvPropArray3( RECVINFO_ARRAY(m_hostageRescueX), RecvPropInt( RECVINFO(m_hostageRescueX[0]))),
	RecvPropArray3( RECVINFO_ARRAY(m_hostageRescueY), RecvPropInt( RECVINFO(m_hostageRescueY[0]))),
	RecvPropArray3( RECVINFO_ARRAY(m_hostageRescueZ), RecvPropInt( RECVINFO(m_hostageRescueZ[0]))),
	RecvPropInt( RECVINFO( m_bBombSpotted ) ),
	RecvPropArray3( RECVINFO_ARRAY(m_bPlayerSpotted), RecvPropInt( RECVINFO(m_bPlayerSpotted[0]))),
	RecvPropArray3( RECVINFO_ARRAY(m_iMVPs), RecvPropInt( RECVINFO(m_iMVPs[0]))),
	RecvPropArray3( RECVINFO_ARRAY(m_bHasDefuser), RecvPropInt( RECVINFO(m_bHasDefuser[0]))),
#if CS_CONTROLLABLE_BOTS_ENABLED
	RecvPropArray3( RECVINFO_ARRAY(m_bControllingBot), RecvPropInt( RECVINFO(m_bControllingBot[0]))),
	RecvPropArray3( RECVINFO_ARRAY(m_iControlledPlayer), RecvPropInt( RECVINFO(m_iControlledPlayer[0]))),
	RecvPropArray3( RECVINFO_ARRAY(m_iControlledByPlayer), RecvPropInt( RECVINFO(m_iControlledByPlayer[0]))),
#endif
	RecvPropArray3( RECVINFO_ARRAY(m_szClan), RecvPropString( RECVINFO(m_szClan[0]))),
END_RECV_TABLE()
 
//=============================================================================
// HPE_END
//=============================================================================

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_CS_PlayerResource::C_CS_PlayerResource()
{
	m_Colors[TEAM_TERRORIST] = COLOR_RED;
	m_Colors[TEAM_CT] = COLOR_BLUE;
	memset( m_iMVPs, 0, sizeof( m_iMVPs ) );
	memset( m_bHasDefuser, 0, sizeof( m_bHasDefuser ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_CS_PlayerResource::~C_CS_PlayerResource()
{
}

bool C_CS_PlayerResource::IsVIP(int iIndex )
{
	return m_iPlayerVIP == iIndex;
}

bool C_CS_PlayerResource::HasC4(int iIndex )
{
#if CS_CONTROLLABLE_BOTS_ENABLED
	if ( GetControlledByPlayer( iIndex ) > 0 )
	{
		return m_iPlayerC4 == GetControlledByPlayer( iIndex );
	}
	
	if ( GetControlledPlayer( iIndex ) > 0 )
	{
		return false;
	}
#endif
	return m_iPlayerC4 == iIndex;
}

bool C_CS_PlayerResource::IsHostageAlive(int iIndex)
{
	if ( iIndex < 0 || iIndex >= MAX_HOSTAGES )
		return false;

	return m_bHostageAlive[iIndex];
}

bool C_CS_PlayerResource::IsHostageFollowingSomeone(int iIndex)
{
	if ( iIndex < 0 || iIndex >= MAX_HOSTAGES )
		return false;

	return m_isHostageFollowingSomeone[iIndex];
}

int C_CS_PlayerResource::GetHostageEntityID(int iIndex)
{
	if ( iIndex < 0 || iIndex >= MAX_HOSTAGES )
		return -1;

	return m_iHostageEntityIDs[iIndex];
}

const Vector C_CS_PlayerResource::GetHostagePosition( int iIndex )
{
	if ( iIndex < 0 || iIndex >= MAX_HOSTAGES )
		return vec3_origin;

	Vector ret;

	ret.x = m_iHostageX[iIndex];
	ret.y = m_iHostageY[iIndex];
	ret.z = m_iHostageZ[iIndex];

	return ret;
}

const Vector C_CS_PlayerResource::GetC4Postion()
{
	if ( m_iPlayerC4 > 0 )
	{
		// C4 is carried by player
		C_BasePlayer *pPlayer = UTIL_PlayerByIndex( m_iPlayerC4 );

		if ( pPlayer )
		{
			return pPlayer->GetAbsOrigin();
		}
	}

	// C4 is lying on ground
	return m_vecC4;
}

const Vector C_CS_PlayerResource::GetBombsiteAPosition()
{
	return m_bombsiteCenterA;
}

const Vector C_CS_PlayerResource::GetBombsiteBPosition()
{
	return m_bombsiteCenterB;
}

const Vector C_CS_PlayerResource::GetHostageRescuePosition( int iIndex )
{
	if ( iIndex < 0 || iIndex >= MAX_HOSTAGE_RESCUES )
		return vec3_origin;

	Vector ret;

	ret.x = m_hostageRescueX[iIndex];
	ret.y = m_hostageRescueY[iIndex];
	ret.z = m_hostageRescueZ[iIndex];

	return ret;
}

int C_CS_PlayerResource::GetPlayerClass( int iIndex )
{
	if ( !IsConnected( iIndex ) )
	{
		return CS_CLASS_NONE;
	}

	return m_iPlayerClasses[ iIndex ];
}

//--------------------------------------------------------------------------------------------------------
bool C_CS_PlayerResource::IsBombSpotted( void ) const
{
	return m_bBombSpotted;
}


//--------------------------------------------------------------------------------------------------------
bool C_CS_PlayerResource::IsPlayerSpotted( int iIndex )
{
	if ( !IsConnected( iIndex ) )
		return false;

	return m_bPlayerSpotted[iIndex];
}

//-----------------------------------------------------------------------------
const char *C_CS_PlayerResource::GetClanTag( int iIndex )
{
	if ( iIndex < 1 || iIndex > MAX_PLAYERS )
	{
		Assert( false );
		return "";
	}

	if ( !IsConnected( iIndex ) )
		return "";

	return m_szClan[iIndex];
}

#if CS_CONTROLLABLE_BOTS_ENABLED
bool C_CS_PlayerResource::IsControllingBot( int index )
{
	return m_bControllingBot[index];
}

int C_CS_PlayerResource::GetControlledPlayer( int index )
{
	return m_iControlledPlayer[index];
}

int C_CS_PlayerResource::GetControlledByPlayer( int index )
{
	return m_iControlledByPlayer[index];
}

ConVar cl_add_bot_prefix( "cl_add_bot_prefix", "1", FCVAR_ARCHIVE, "Whether to add a BOT prefix to bot names or not.", true, 0, true, 1 );
void C_CS_PlayerResource::UpdatePlayerName( int slot )
{
	if ( slot < 1 || slot > MAX_PLAYERS )
	{
		Error( "UpdatePlayerName with bogus slot %d\n", slot );
		return;
	}

	if ( !m_szUnconnectedName )
	{
		m_szUnconnectedName = AllocPooledString( PLAYER_UNCONNECTED_NAME );
	}
	
	player_info_t sPlayerInfo;
	if ( IsConnected( slot ) && engine->GetPlayerInfo( slot, &sPlayerInfo ) )
	{
		m_szName[slot] = AllocPooledString( sPlayerInfo.name );

		if ( sPlayerInfo.fakeplayer && cl_add_bot_prefix.GetBool() )
		{
#if CS_CONTROLLABLE_BOTS_ENABLED
			int controlledByPlayer = GetControlledByPlayer( slot );
			if ( controlledByPlayer > 0 )
			{
				engine->GetPlayerInfo( controlledByPlayer, &sPlayerInfo );
				char buffer[64];
				Q_snprintf( buffer, sizeof( buffer ), "BOT (%s)", sPlayerInfo.name );
				m_szName[slot] = AllocPooledString( buffer );
			}
			else
#endif
			{
				char buffer[64];
				Q_snprintf( buffer, sizeof( buffer ), "BOT %s", sPlayerInfo.name );
				m_szName[slot] = AllocPooledString( buffer );
			}
		}
	}
	else 
	{
		m_szName[slot] = m_szUnconnectedName;
	}
}
#endif

C_CS_PlayerResource * GetCSResources( void )
{
	return (C_CS_PlayerResource*)g_PR;
}

//-----------------------------------------------------------------------------
int C_CS_PlayerResource::GetNumMVPs( int iIndex )
{
	if ( !IsConnected( iIndex ) )
		return false;

	return m_iMVPs[iIndex];
} 

//-----------------------------------------------------------------------------
bool C_CS_PlayerResource::HasDefuser( int iIndex )
{
	if ( !IsConnected( iIndex ) )
		return false;

#if CS_CONTROLLABLE_BOTS_ENABLED
	if ( GetControlledByPlayer( iIndex ) > 0 )
	{
		return m_bHasDefuser[GetControlledByPlayer( iIndex )];
	}

	if ( GetControlledPlayer( iIndex ) > 0 )
	{
		return false;
	}
#endif

	return m_bHasDefuser[iIndex];
}

