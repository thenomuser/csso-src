//========= Copyright, Valve Corporation, All rights reserved. ============
//
// Purpose:  CS-specific things to vote on
//
//=============================================================================

#include "cbase.h"
#include "cs_voteissues.h"
#include "cs_player.h"

#include "vote_controller.h"
#include "fmtstr.h"
#include "eiface.h"
#include "cs_gamerules.h"
#include "inetchannelinfo.h"
#include "cs_gamestats.h"

#ifdef CLIENT_DLL
#include "gc_clientsystem.h"
#endif // CLIENT_DLL

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar mp_maxrounds;

static bool VotableMap( const char *pszMapName )
{
	char szCanonName[64] = { 0 };
	V_strncpy( szCanonName, pszMapName, sizeof( szCanonName ) );
	IVEngineServer::eFindMapResult eResult = engine->FindMap( szCanonName, sizeof( szCanonName ) );

	switch ( eResult )
	{
		case IVEngineServer::eFindMap_Found:
		case IVEngineServer::eFindMap_NonCanonical:
		case IVEngineServer::eFindMap_PossiblyAvailable:
		case IVEngineServer::eFindMap_FuzzyMatch:
			return true;
		case IVEngineServer::eFindMap_NotFound:
			return false;
	}

	AssertMsg( false, "Unhandled engine->FindMap return value\n" );
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Base CS Issue
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Purpose: Restart Round Issue
//-----------------------------------------------------------------------------
ConVar sv_vote_issue_restart_game_allowed( "sv_vote_issue_restart_game_allowed", "1", 0, "Can people hold votes to restart the game?" );
// ConVar sv_arms_race_vote_to_restart_disallowed_after( "sv_arms_race_vote_to_restart_disallowed_after", "0", FCVAR_REPLICATED, "Arms Race gun level after which vote to restart is disallowed" ); -- gungame!

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRestartGameIssue::ExecuteCommand( void )
{
	engine->ServerCommand( CFmtStr( "mp_restartgame 1;" ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CRestartGameIssue::IsEnabled( void )
{
	if ( sv_vote_issue_restart_game_allowed.GetBool() )
	{
		/* -- gungame!
		// Vote to restart is allowed
		if ( sv_arms_race_vote_to_restart_disallowed_after.GetInt() > 0 )
		{
			// Need to test if a player has surpassed the maximum weapon progression
			if ( sv_arms_race_vote_to_restart_disallowed_after.GetInt() > CSGameRules()->GetMaxGunGameProgressiveWeaponIndex() )
			{
				// No players have surpassed the maximum weapon progression, so enable vote to restart
				return true;
			}
			else
			{
				// A player has surpassed the maximum weapon progression, so disable vote to restart
				return false;
			}
		}*/

		// No maximum weapon progression value is defined, so enable vote to restart
		return true;
	}

	// Vote to restart is not enabled
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CRestartGameIssue::CanCallVote( int iEntIndex, const char *pszDetails, vote_create_failed_t &nFailCode, int &nTime )
{
	if( !CBaseCSIssue::CanCallVote( iEntIndex, pszDetails, nFailCode, nTime ) )
		return false;

	if( !IsEnabled() )
	{
		nFailCode = VOTE_FAILED_ISSUE_DISABLED;
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CRestartGameIssue::GetDisplayString( void )
{
	return "#CStrike_vote_restart_game";
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CRestartGameIssue::GetVotePassedString( void )
{
	return "#CStrike_vote_passed_restart_game";
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRestartGameIssue::ListIssueDetails( CBasePlayer *pForWhom )
{
	if( !sv_vote_issue_restart_game_allowed.GetBool() )
		return;

	ListStandardNoArgCommand( pForWhom, GetTypeString() );
}


//-----------------------------------------------------------------------------
// Purpose: Kick Player Issue
//-----------------------------------------------------------------------------
ConVar sv_vote_issue_kick_allowed( "sv_vote_issue_kick_allowed", "1", FCVAR_REPLICATED | FCVAR_NOTIFY, "Can people hold votes to kick players from the server?" );
ConVar sv_vote_kick_ban_duration( "sv_vote_kick_ban_duration", "15", FCVAR_REPLICATED | FCVAR_NOTIFY, "How long should a kick vote ban someone from the server? (in minutes)" );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CKickIssue::ExecuteCommand( void )
{
	CCSPlayer *subject = NULL;

	ExtractDataFromDetails( m_szDetailsString, &subject );

	if( subject )
	{
		Msg( "%s (unique ID: %s) got kicked by voting.\n", subject->GetPlayerName(), subject->GetNetworkIDString() );
		// Check the cached value of player crashed state
		if ( ( sv_vote_kick_ban_duration.GetInt() > 0 ) && !m_bPlayerCrashed )
		{
			// don't roll the kick command into this, it will fail on a lan, where kickid will go through
			engine->ServerCommand( CFmtStr( "banid %d %d;", sv_vote_kick_ban_duration.GetInt(), subject->GetUserID() ) );
		}

		engine->ServerCommand( CFmtStr( "kickid_ex %d 1 You have been voted off;", subject->GetUserID() ) );
	}
	else if ( !m_bPlayerCrashed && m_uniqueIDtoBan && m_uniqueIDtoBan[0] && (sv_vote_kick_ban_duration.GetInt() > 0) )
	{
		Msg( "%s (unique ID: %s) got kicked by voting.\n", subject->GetPlayerName(), subject->GetNetworkIDString() );
		// Also enlist this user's unique ID in the banlist
		engine->ServerCommand( CFmtStr( "banid %d %s;", sv_vote_kick_ban_duration.GetInt(), m_uniqueIDtoBan ) );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CKickIssue::IsEnabled( void )
{
	return sv_vote_issue_kick_allowed.GetBool() && UTIL_HumansInGame() > 1; // PiMoN: I think its not necessary to have it enabled if you don't have anyone to kick
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CKickIssue::CanCallVote( int iEntIndex, const char *pszDetails, vote_create_failed_t &nFailCode, int &nTime )
{
	if ( !CBaseCSIssue::CanCallVote( iEntIndex, pszDetails, nFailCode, nTime ) )
		return false;

	if( !IsEnabled() )
	{
		nFailCode = VOTE_FAILED_ISSUE_DISABLED;
		return false;
	}

	CCSPlayer *pSubject = NULL;
	ExtractDataFromDetails( pszDetails, &pSubject );
	if ( !pSubject || pSubject->IsBot() )
	{
		nFailCode = VOTE_FAILED_PLAYERNOTFOUND;
		return false;
	}

	if ( pSubject->IsReplay() || pSubject->IsHLTV() )
	{
		nFailCode = VOTE_FAILED_PLAYERNOTFOUND;
		return false;
	}
	
	CBaseEntity *pVoteCaller = UTIL_EntityByIndex( iEntIndex );
	if ( pVoteCaller )
	{
		CCSPlayer *pPlayer = ToCSPlayer( pVoteCaller );
		bool bCanKickVote = false;

		if ( pSubject && pPlayer )
		{
			int voterTeam = pPlayer->GetTeamNumber();
			int nSubjectTeam = pSubject->GetTeamNumber();

			if ( CSGameRules()->IsPlayingAnyCompetitiveStrictRuleset() )
				bCanKickVote = (voterTeam == TEAM_TERRORIST || voterTeam == TEAM_CT) && (voterTeam == nSubjectTeam || nSubjectTeam == TEAM_SPECTATOR);
			else
				bCanKickVote = true;
		}
		
		if ( pSubject )
		{
			bool bDeny = false;
			if ( !engine->IsDedicatedServer() )
			{
				if ( pSubject->entindex() == 1 )
				{
					bDeny = true;
				}
			}

			// This should only be set if we've successfully authenticated via rcon
			if ( pSubject->IsAutoKickDisabled() )
			{
				bDeny = true;
			}

			if ( bDeny )
			{
				nFailCode = VOTE_FAILED_CANNOT_KICK_ADMIN;
				return false;
			}
		}

		if ( !bCanKickVote )
		{
			// Can't initiate a vote to kick a player on the other team
			return false;
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CKickIssue::OnVoteFailed( int iEntityHoldingVote )
{
	CBaseCSIssue::OnVoteFailed( iEntityHoldingVote );

	CCSPlayer *subject = NULL;
	ExtractDataFromDetails( m_szDetailsString, &subject );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CKickIssue::OnVoteStarted( void )
{
	CCSPlayer *pSubject = NULL;
	ExtractDataFromDetails(	m_szDetailsString, &pSubject );

	// Auto vote 'No' for the person being kicked unless they are idle
	// NOTE: Subtle. There's a problem with IsAwayFromKeyboard where if a player
	// has idled and is taken over by a bot, IsAwayFromKeyboard will return false
	// because the camera controller that takes over when idling will spoof 
	// input messages, making it impossible to know if the player is moving his
	// joystick or not. Being on TEAM_SPECTATOR, however, means you're idling,
	// so we don't want to autovote NO if they are on team spectator
	if ( g_voteController && pSubject && ( pSubject->GetTeamNumber( ) != TEAM_SPECTATOR ) && !pSubject->IsBot( ) )
	{
		g_voteController->TryCastVote( pSubject->entindex( ), "Option2" );
	}

	// Also when the vote starts, figure out if the player should not be banned
	// if the player is crashed/hung. Need to perform the check here instead of
	// inside Execute to prevent cheaters quitting before the vote finishes and
	// not getting banned.
	m_bPlayerCrashed = false;
	if ( pSubject )
	{
		INetChannelInfo *pNetChanInfo = engine->GetPlayerNetInfo( pSubject->entindex() );
		if ( !pNetChanInfo || pNetChanInfo->IsTimingOut() )
		{
			// don't ban the player
			DevMsg( "Will not ban kicked player: net channel was idle for %.2f sec.\n", pNetChanInfo ? pNetChanInfo->GetTimeSinceLastReceived() : 0.0f );
			m_bPlayerCrashed = true;
		}

		m_uniqueIDtoBan = pSubject->GetNetworkIDString();
	 }
 }

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CKickIssue::GetDisplayString( void )
{
	return "#CStrike_vote_kick_player_other";
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CKickIssue::GetVotePassedString( void )
{
	return "#CStrike_vote_passed_kick_player";
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CKickIssue::GetDetailsString( void )
{
	int iUserID = 0;
	iUserID = atoi( m_szDetailsString );

	CBasePlayer *pPlayer = UTIL_PlayerByUserId( iUserID );
	if ( pPlayer )
	{
		return pPlayer->GetPlayerName();
	}
	else
	{
		return "unnamed";
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CKickIssue::ExtractDataFromDetails( const char *pszDetails, CCSPlayer **pSubject )
{
	int iUserID = 0;
	iUserID = atoi( pszDetails );

	if ( iUserID >= 0 )
	{
		if( pSubject )
		{
			*pSubject = ToCSPlayer( UTIL_PlayerByUserId( iUserID ) );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CKickIssue::ListIssueDetails( CBasePlayer *pForWhom )
{
	if( !sv_vote_issue_kick_allowed.GetBool() )
		return;

	char szBuffer[MAX_COMMAND_LENGTH];
	Q_snprintf( szBuffer, MAX_COMMAND_LENGTH, "callvote %s <userID>\n", GetTypeString() );
	ClientPrint( pForWhom, HUD_PRINTCONSOLE, szBuffer );
}


//-----------------------------------------------------------------------------
// Purpose: Ban Player Issue
//-----------------------------------------------------------------------------
ConVar sv_vote_issue_ban_allowed( "sv_vote_issue_ban_allowed", "1", FCVAR_REPLICATED | FCVAR_NOTIFY, "Can people hold votes to ban players from the server?" );
ConVar sv_vote_ban_duration( "sv_vote_ban_duration", "60", FCVAR_REPLICATED | FCVAR_NOTIFY, "How long should a ban vote ban someone from the server? (in minutes)" );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBanIssue::ExecuteCommand( void )
{
	CCSPlayer *subject = NULL;

	ExtractDataFromDetails( m_szDetailsString, &subject );

	if( subject )
	{
		Msg( "%s (unique ID: %s) got banned by voting.\n", subject->GetPlayerName(), subject->GetNetworkIDString() );
		engine->ServerCommand( CFmtStr( "banid %d %d;", sv_vote_ban_duration.GetInt(), subject->GetUserID() ) );
		engine->ServerCommand( CFmtStr( "kickid_ex %d 1 You have been banned from this server;", subject->GetUserID() ) );
	}
	else if ( m_uniqueIDtoBan && m_uniqueIDtoBan[0] )
	{
		Msg( "%s (unique ID: %s) got banned by voting.\n", subject->GetPlayerName(), subject->GetNetworkIDString() );
		// Also enlist this user's unique ID in the banlist
		engine->ServerCommand( CFmtStr( "banid %d %s;", sv_vote_ban_duration.GetInt(), m_uniqueIDtoBan ) );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBanIssue::IsEnabled( void )
{
	return sv_vote_issue_ban_allowed.GetBool() && UTIL_HumansInGame() > 1; // PiMoN: I think its not necessary to have it enabled if you don't have anyone to ban
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBanIssue::CanCallVote( int iEntIndex, const char *pszDetails, vote_create_failed_t &nFailCode, int &nTime )
{
	if ( !CBaseCSIssue::CanCallVote( iEntIndex, pszDetails, nFailCode, nTime ) )
		return false;

	if( !IsEnabled() )
	{
		nFailCode = VOTE_FAILED_ISSUE_DISABLED;
		return false;
	}

	CCSPlayer *pSubject = NULL;
	ExtractDataFromDetails( pszDetails, &pSubject );
	if ( !pSubject || pSubject->IsBot() )
	{
		nFailCode = VOTE_FAILED_PLAYERNOTFOUND;
		return false;
	}

	if ( pSubject->IsReplay() || pSubject->IsHLTV() )
	{
		nFailCode = VOTE_FAILED_PLAYERNOTFOUND;
		return false;
	}
	
	CBaseEntity *pVoteCaller = UTIL_EntityByIndex( iEntIndex );
	if ( pVoteCaller )
	{		
		if ( pSubject )
		{
			bool bDeny = false;
			if ( !engine->IsDedicatedServer() )
			{
				if ( pSubject->entindex() == 1 )
				{
					bDeny = true;
				}
			}

			// This should only be set if we've successfully authenticated via rcon
			if ( pSubject->IsAutoKickDisabled() )
			{
				bDeny = true;
			}

			if ( bDeny )
			{
				nFailCode = VOTE_FAILED_CANNOT_BAN_ADMIN;
				return false;
			}
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBanIssue::OnVoteFailed( int iEntityHoldingVote )
{
	CBaseCSIssue::OnVoteFailed( iEntityHoldingVote );

	CCSPlayer *subject = NULL;
	ExtractDataFromDetails( m_szDetailsString, &subject );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBanIssue::OnVoteStarted( void )
{
	CCSPlayer *pSubject = NULL;
	ExtractDataFromDetails( m_szDetailsString, &pSubject );

	// Auto vote 'No' for the person being banned
	if ( g_voteController && pSubject && (pSubject->GetTeamNumber() != TEAM_SPECTATOR) && !pSubject->IsBot() )
	{
		g_voteController->TryCastVote( pSubject->entindex(), "Option2" );
	}

	if ( pSubject )
	{
		m_uniqueIDtoBan = pSubject->GetNetworkIDString();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CBanIssue::GetDisplayString( void )
{
	return "#CStrike_vote_ban_player";
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CBanIssue::GetVotePassedString( void )
{
	return "#CStrike_vote_passed_ban_player";
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CBanIssue::GetDetailsString( void )
{
	int iUserID = 0;
	iUserID = atoi( m_szDetailsString );

	CBasePlayer *pPlayer = UTIL_PlayerByUserId( iUserID );
	if ( pPlayer )
	{
		return pPlayer->GetPlayerName();
	}
	else
	{
		return "unnamed";
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBanIssue::ExtractDataFromDetails( const char *pszDetails, CCSPlayer **pSubject )
{
	int iUserID = 0;
	iUserID = atoi( pszDetails );

	if ( iUserID >= 0 )
	{
		if( pSubject )
		{
			*pSubject = ToCSPlayer( UTIL_PlayerByUserId( iUserID ) );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBanIssue::ListIssueDetails( CBasePlayer *pForWhom )
{
	if( !sv_vote_issue_ban_allowed.GetBool() )
		return;

	char szBuffer[MAX_COMMAND_LENGTH];
	Q_snprintf( szBuffer, MAX_COMMAND_LENGTH, "callvote %s <userID>\n", GetTypeString() );
	ClientPrint( pForWhom, HUD_PRINTCONSOLE, szBuffer );
}


//-----------------------------------------------------------------------------
// Purpose: Changelevel
//-----------------------------------------------------------------------------
ConVar sv_vote_issue_changelevel_allowed( "sv_vote_issue_changelevel_allowed", "1", 0, "Can people hold votes to change levels?" );
ConVar sv_vote_to_changelevel_before_match_point( "sv_vote_to_changelevel_before_match_point", "0", FCVAR_REPLICATED, "Restricts vote to change level to rounds prior to match point (default 0, vote is never disallowed)" );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CChangeLevelIssue::ExecuteCommand( void )
{
	engine->ServerCommand( CFmtStr( "changelevel %s;", m_szDetailsString ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CChangeLevelIssue::CanTeamCallVote( int iTeam ) const
{
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CChangeLevelIssue::IsEnabled( void )
{
	if ( sv_vote_issue_changelevel_allowed.GetBool() )
	{			 		
		return ( sv_vote_to_changelevel_before_match_point.GetInt() > 0 &&
			 CSGameRules() && CSGameRules()->IsIntermission() ) == false;
	}
	// Change Level vote disabled
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CChangeLevelIssue::CanCallVote( int iEntIndex, const char *pszDetails, vote_create_failed_t &nFailCode, int &nTime )
{
	if( !CBaseCSIssue::CanCallVote( iEntIndex, pszDetails, nFailCode, nTime ) )
		return false;

	if( !IsEnabled() )
	{
		nFailCode = VOTE_FAILED_ISSUE_DISABLED;
		return false;
	}

	if ( !Q_strcmp( pszDetails, "" ) )
	{
		nFailCode = VOTE_FAILED_MAP_NAME_REQUIRED;
		return false;
	}
	else
	{
		if ( !VotableMap( pszDetails ) )
		{
			nFailCode = VOTE_FAILED_MAP_NOT_FOUND;
			return false;
		}

		if ( MultiplayRules() && !MultiplayRules()->IsMapInMapCycle( pszDetails ) )
		{
			nFailCode = VOTE_FAILED_MAP_NOT_VALID;
			return false;
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CChangeLevelIssue::GetDisplayString( void )
{
	return "#CStrike_vote_changelevel";
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CChangeLevelIssue::GetVotePassedString( void )
{
	return "#CStrike_vote_passed_changelevel";
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CChangeLevelIssue::GetDetailsString( void )
{
	return m_szDetailsString;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CChangeLevelIssue::ListIssueDetails( CBasePlayer *pForWhom )
{
	if( !sv_vote_issue_changelevel_allowed.GetBool() )
		return;

	char szBuffer[MAX_COMMAND_LENGTH];
	Q_snprintf( szBuffer, MAX_COMMAND_LENGTH, "callvote %s <mapname>\n", GetTypeString() );
	ClientPrint( pForWhom, HUD_PRINTCONSOLE, szBuffer );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CChangeLevelIssue::IsYesNoVote( void )
{
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Nextlevel
//-----------------------------------------------------------------------------
ConVar sv_vote_issue_nextlevel_allowed( "sv_vote_issue_nextlevel_allowed", "1", 0, "Can people hold votes to set the next level?" );
ConVar sv_vote_issue_nextlevel_choicesmode( "sv_vote_issue_nextlevel_choicesmode", "1", 0, "Present players with a list of lowest playtime maps to choose from?" );
ConVar sv_vote_issue_nextlevel_allowextend( "sv_vote_issue_nextlevel_allowextend", "1", 0, "Allow players to extend the current map?" );
ConVar sv_vote_issue_nextlevel_prevent_change( "sv_vote_issue_nextlevel_prevent_change", "1", 0, "Not allowed to vote for a nextlevel if one has already been set." );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNextLevelIssue::ExecuteCommand( void )
{
	if ( Q_strcmp( m_szDetailsString, "Extend current Map" ) == 0 )
	{
		// Players want to extend the current map, so extend any existing limits
		if ( mp_timelimit.GetInt() > 0 )
		{
			engine->ServerCommand( CFmtStr( "mp_timelimit %d;", mp_timelimit.GetInt() + 20 ) );
		}
		
		if ( mp_maxrounds.GetInt() > 0 )
		{
			engine->ServerCommand( CFmtStr( "mp_maxrounds %d;", mp_maxrounds.GetInt() + 2 ) );
		}
	}
	else
	{
		nextlevel.SetValue( m_szDetailsString );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CNextLevelIssue::CanTeamCallVote( int iTeam ) const
{
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CNextLevelIssue::IsEnabled( void )
{
	return sv_vote_issue_nextlevel_allowed.GetBool();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CNextLevelIssue::CanCallVote( int iEntIndex, const char *pszDetails, vote_create_failed_t &nFailCode, int &nTime )
{
	if( !CBaseCSIssue::CanCallVote( iEntIndex, pszDetails, nFailCode, nTime ) )
		return false;

	// CSGameRules created vote
	if ( sv_vote_issue_nextlevel_choicesmode.GetBool() && iEntIndex == 99 )
	{
		// Invokes a UI down stream
		if ( Q_strcmp( pszDetails, "" ) == 0 )
		{
			return true;
		}

		return false;
	}
	
	if( !IsEnabled() )
	{
		nFailCode = VOTE_FAILED_ISSUE_DISABLED;
		return false;
	}

	if ( Q_strcmp( pszDetails, "" ) == 0 )
	{
		nFailCode = VOTE_FAILED_MAP_NAME_REQUIRED;
		return false;
	}
	else
	{
		if ( !VotableMap( pszDetails ) )
		{
			nFailCode = VOTE_FAILED_MAP_NOT_FOUND;
			return false;
		}

		if ( MultiplayRules() && !MultiplayRules()->IsMapInMapCycle( pszDetails ) )
		{
			nFailCode = VOTE_FAILED_MAP_NOT_VALID;
			return false;
		}
	}

	if ( sv_vote_issue_nextlevel_prevent_change.GetBool() )
	{
		if ( nextlevel.GetString() && *nextlevel.GetString() )
		{
			nFailCode = VOTE_FAILED_NEXTLEVEL_SET;
			return false;
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CNextLevelIssue::GetDisplayString( void )
{
	// If we don't have a map passed in already...
	if ( Q_strcmp( m_szDetailsString, "" ) == 0 )
	{
		if ( sv_vote_issue_nextlevel_choicesmode.GetBool() )
		{
			return "#CStrike_vote_nextlevel_choices";
		}
	}

	return "#CStrike_vote_nextlevel";
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CNextLevelIssue::GetVotePassedString( void )
{
	if ( Q_strcmp( m_szDetailsString, STRING( gpGlobals->mapname ) ) == 0 )
	{
		return "#CStrike_vote_passed_nextlevel_extend";
	}
	return "#CStrike_vote_passed_nextlevel";
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CNextLevelIssue::GetDetailsString( void )
{
	return m_szDetailsString;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNextLevelIssue::ListIssueDetails( CBasePlayer *pForWhom )
{
	if( !sv_vote_issue_nextlevel_allowed.GetBool() )
		return;

	if ( !sv_vote_issue_nextlevel_choicesmode.GetBool() )
	{
		char szBuffer[MAX_COMMAND_LENGTH];
		Q_snprintf( szBuffer, MAX_COMMAND_LENGTH, "callvote %s <mapname>\n", GetTypeString() );
		ClientPrint( pForWhom, HUD_PRINTCONSOLE, szBuffer );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CNextLevelIssue::IsYesNoVote( void )
{
	// If we don't have a map name already, this will trigger a list of choices
	if ( Q_strcmp( m_szDetailsString, "" ) == 0 )
	{
		if ( sv_vote_issue_nextlevel_choicesmode.GetBool() )
			return false;
	}
	
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CNextLevelIssue::GetNumberVoteOptions( void )
{
	// If we don't have a map name already, this will trigger a list of choices
	if ( Q_strcmp( m_szDetailsString, "" ) == 0 )
	{
		if ( sv_vote_issue_nextlevel_choicesmode.GetBool() )
			return MAX_VOTE_OPTIONS;
	}

	// Vote on a specific map - Yes, No
	return 2;
}

//-----------------------------------------------------------------------------
// Purpose: Scramble Teams Issue
//-----------------------------------------------------------------------------
ConVar sv_vote_issue_scramble_teams_allowed( "sv_vote_issue_scramble_teams_allowed", "1", 0, "Can people hold votes to scramble the teams?" );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CScrambleTeams::ExecuteCommand( void )
{
	engine->ServerCommand( CFmtStr( "mp_scrambleteams 2;" ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CScrambleTeams::IsEnabled( void )
{
	return sv_vote_issue_scramble_teams_allowed.GetBool();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CScrambleTeams::CanCallVote( int iEntIndex, const char *pszDetails, vote_create_failed_t &nFailCode, int &nTime )
{
	if( !CBaseCSIssue::CanCallVote( iEntIndex, pszDetails, nFailCode, nTime ) )
		return false;

	if( !IsEnabled() )
	{
		nFailCode = VOTE_FAILED_ISSUE_DISABLED;
		return false;
	}

	if ( CSGameRules() && CSGameRules()->GetScrambleTeamsOnRestart() )
	{
		nFailCode = VOTE_FAILED_SCRAMBLE_IN_PROGRESS;
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CScrambleTeams::GetDisplayString( void )
{
	return "#CStrike_vote_scramble_teams";
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CScrambleTeams::GetVotePassedString( void )
{
	return "#CStrike_vote_passed_scramble_teams";
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CScrambleTeams::ListIssueDetails( CBasePlayer *pForWhom )
{
	if( !sv_vote_issue_scramble_teams_allowed.GetBool() )
		return;

	ListStandardNoArgCommand( pForWhom, GetTypeString() );
}


//-----------------------------------------------------------------------------
// Purpose: Pause Match
//-----------------------------------------------------------------------------
ConVar sv_vote_issue_pause_match_allowed( "sv_vote_issue_pause_match_allowed", "1", 0, "Can people hold votes to pause/unpause the match?" );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPauseMatchIssue::ExecuteCommand( void )
{
	engine->ServerCommand( CFmtStr( "mp_pause_match;" ) );

	CBaseEntity *pVoteCaller = UTIL_EntityByIndex( g_voteController->GetCallingEntity( ) );
	if ( !pVoteCaller )
		return;

	CCSPlayer *pPlayer = ToCSPlayer( pVoteCaller );
	if ( !pPlayer )
		return;

	UTIL_ClientPrintAll( HUD_PRINTTALK, "#CStrike_vote_passed_pause_match_chat", pPlayer->GetPlayerName() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CPauseMatchIssue::IsEnabled( void )
{
	return CSGameRules()->IsPlayingAnyCompetitiveStrictRuleset() && sv_vote_issue_pause_match_allowed.GetBool();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CPauseMatchIssue::CanCallVote( int iEntIndex, const char *pszDetails, vote_create_failed_t &nFailCode, int &nTime )
{
	if( !CBaseCSIssue::CanCallVote( iEntIndex, pszDetails, nFailCode, nTime ) )
		return false;

	if( !IsEnabled() )
	{
		nFailCode = VOTE_FAILED_ISSUE_DISABLED;
		return false;
	}

	if ( !CSGameRules() )
		return false;

	if ( CSGameRules()->IsMatchWaitingForResume() )
	{
		nFailCode = VOTE_FAILED_MATCH_PAUSED;
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CPauseMatchIssue::GetDisplayString( void )
{
	return "#CStrike_vote_pause_match";
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CPauseMatchIssue::GetVotePassedString( void )
{
	return "#CStrike_vote_passed_pause_match";
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPauseMatchIssue::ListIssueDetails( CBasePlayer *pForWhom )
{
	if( !sv_vote_issue_pause_match_allowed.GetBool() )
		return;

	ListStandardNoArgCommand( pForWhom, GetTypeString() );
}




//-----------------------------------------------------------------------------
// Purpose: UnPause Match
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CUnpauseMatchIssue::ExecuteCommand( void )
{
	engine->ServerCommand( CFmtStr( "mp_unpause_match;" ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CUnpauseMatchIssue::IsEnabled( void )
{
	return CSGameRules()->IsPlayingAnyCompetitiveStrictRuleset() && sv_vote_issue_pause_match_allowed.GetBool();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CUnpauseMatchIssue::CanCallVote( int iEntIndex, const char *pszDetails, vote_create_failed_t &nFailCode, int &nTime )
{
	if ( !CBaseCSIssue::CanCallVote( iEntIndex, pszDetails, nFailCode, nTime ) )
		return false;

	if ( !IsEnabled() )
	{
		nFailCode = VOTE_FAILED_ISSUE_DISABLED;
		return false;
	}

	if (!CSGameRules())
		return false;

	if ( !CSGameRules()->IsMatchWaitingForResume() )
	{
		nFailCode = VOTE_FAILED_MATCH_NOT_PAUSED;
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CUnpauseMatchIssue::GetDisplayString( void )
{
	return "#CStrike_vote_unpause_match";
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CUnpauseMatchIssue::GetVotePassedString( void )
{
	return "#CStrike_vote_passed_unpause_match";
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CUnpauseMatchIssue::ListIssueDetails( CBasePlayer *pForWhom )
{
	if ( !sv_vote_issue_pause_match_allowed.GetBool() )
		return;

	ListStandardNoArgCommand( pForWhom, GetTypeString() );
}




//-----------------------------------------------------------------------------
// Purpose: TimeOut
//-----------------------------------------------------------------------------
ConVar sv_vote_issue_timeout_allowed( "sv_vote_issue_timeout_allowed", "1", 0, "Can people hold votes to time out?" );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CStartTimeOutIssue::ExecuteCommand( void )
{
	CBaseEntity *pVoteHolder = UTIL_EntityByIndex( g_voteController->GetCallingEntity( ) );
	if ( !pVoteHolder )
		return;

	if ( pVoteHolder->GetTeamNumber() == TEAM_CT )
	{
		engine->ServerCommand( CFmtStr( "timeout_ct_start;" ) );
	}
	else if ( pVoteHolder->GetTeamNumber() == TEAM_TERRORIST )
	{
		engine->ServerCommand( CFmtStr( "timeout_terrorist_start;" ) );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CStartTimeOutIssue::IsEnabled( void )
{
	return CSGameRules()->IsPlayingAnyCompetitiveStrictRuleset() && sv_vote_issue_timeout_allowed.GetBool();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CStartTimeOutIssue::CanCallVote( int iEntIndex, const char *pszDetails, vote_create_failed_t &nFailCode, int &nTime )
{
	if ( !CBaseCSIssue::CanCallVote( iEntIndex, pszDetails, nFailCode, nTime ) )
		return false;

	if ( !IsEnabled() )
	{
		nFailCode = VOTE_FAILED_ISSUE_DISABLED;
		return false;
	}

	if ( CSGameRules() && CSGameRules()->IsTimeOutActive() )
	{
		nFailCode = VOTE_FAILED_TIMEOUT_ACTIVE;
		return false;
	}

	if ( CSGameRules() && CSGameRules()->IsMatchWaitingForResume() )
	{
		nFailCode = VOTE_FAILED_MATCH_PAUSED;
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CStartTimeOutIssue::CanTeamCallVote(int iTeam ) const
{
	if ( !CSGameRules() )
		return false;

	if ( CSGameRules()->IsTimeOutActive() )
		return false;

	if ( iTeam == TEAM_CT )
	{
		return ( CSGameRules()->GetCTTimeOuts() > 0 );
	}
	else if ( iTeam == TEAM_TERRORIST )
	{
		return ( CSGameRules()->GetTerroristTimeOuts() > 0 );
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CStartTimeOutIssue::GetDisplayString( void )
{
	return "#CStrike_vote_start_timeout";
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CStartTimeOutIssue::GetVotePassedString( void )
{
	return "#CStrike_vote_passed_timeout";
}

vote_create_failed_t CStartTimeOutIssue::MakeVoteFailErrorCodeForClients( vote_create_failed_t eDefaultFailCode )
{
	switch ( eDefaultFailCode )
	{
	case VOTE_FAILED_TEAM_CANT_CALL:
	{
		if ( CSGameRules( ) && CSGameRules( )->IsTimeOutActive( ) )
			return VOTE_FAILED_TIMEOUT_ACTIVE;
		else if ( CSGameRules() && CSGameRules()->IsMatchWaitingForResume() )
			return VOTE_FAILED_MATCH_PAUSED;
		else
			return VOTE_FAILED_TIMEOUT_EXHAUSTED;
	}

	default:
		return eDefaultFailCode;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CStartTimeOutIssue::ListIssueDetails( CBasePlayer *pForWhom )
{
	if ( !sv_vote_issue_pause_match_allowed.GetBool() )
		return;

	ListStandardNoArgCommand( pForWhom, GetTypeString() );
}





//-----------------------------------------------------------------------------
// Purpose: Swap Teams Issue
//-----------------------------------------------------------------------------
ConVar sv_vote_issue_swap_teams_allowed( "sv_vote_issue_swap_teams_allowed", "1", 0, "Can people hold votes to swap the teams?" );


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSwapTeams::ExecuteCommand( void )
{
	engine->ServerCommand( CFmtStr( "mp_swapteams;" ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CSwapTeams::IsEnabled( void )
{
	return sv_vote_issue_swap_teams_allowed.GetBool();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CSwapTeams::CanCallVote( int iEntIndex, const char *pszDetails, vote_create_failed_t &nFailCode, int &nTime )
{
	if ( !CBaseCSIssue::CanCallVote( iEntIndex, pszDetails, nFailCode, nTime ) )
		return false;

	if ( !IsEnabled() )
	{
		nFailCode = VOTE_FAILED_ISSUE_DISABLED;
		return false;
	}

	if ( CSGameRules() && CSGameRules()->GetSwapTeamsOnRestart() )
	{
		nFailCode = VOTE_FAILED_SWAP_IN_PROGRESS;
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CSwapTeams::GetDisplayString( void )
{
	return "#CStrike_vote_swap_teams";
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CSwapTeams::GetVotePassedString( void )
{
	return "#CStrike_vote_passed_swap_teams";
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSwapTeams::ListIssueDetails( CBasePlayer *pForWhom )
{
	if ( !sv_vote_issue_swap_teams_allowed.GetBool() )
		return;

	ListStandardNoArgCommand( pForWhom, GetTypeString() );
}
