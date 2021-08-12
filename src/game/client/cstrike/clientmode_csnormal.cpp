//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//===========================================================================//
#include "cbase.h"
#include "hud.h"
#include "clientmode_csnormal.h"
#include "cdll_client_int.h"
#include <engine/ivdebugoverlay.h>
#include "iinput.h"
#include "vgui/ISurface.h"
#include "vgui/IPanel.h"
#include <vgui_controls/AnimationController.h>
#include "ivmodemanager.h"
#include "buymenu.h"
#include "filesystem.h"
#include "vgui/IVGui.h"
#include "hud_basechat.h"
#include "view_shared.h"
#include "view.h"
#include "ivrenderview.h"
#include "cstrikeclassmenu.h"
#include "cstrikebuymenu.h"
#include "c_te_legacytempents.h"
#include "tempent.h"
#include "model_types.h"
#include "iefx.h"
#include "dlight.h"
#include <imapoverview.h>
#include "c_playerresource.h"
#include "c_soundscape.h"
#include <KeyValues.h>
#include "text_message.h"
#include "panelmetaclassmgr.h"
#include "vguicenterprint.h"
#include "physpropclientside.h"
#include "c_weapon__stubs.h"
#include <engine/IEngineSound.h>
#include "c_cs_hostage.h"
#include "buy_presets/buy_presets.h"
#include "bitbuf.h"
#include "usermessages.h"
#include "prediction.h"
#include "datacache/imdlcache.h"
#include "cs_shareddefs.h"
#include "cs_loadout.h"
#include "materialsystem/itexture.h"
//=============================================================================
// HPE_BEGIN:
// [tj] Needed to retrieve achievement text
// [menglish] Need access to message macros
//=============================================================================
 
#include "achievementmgr.h"
#include "hud_macros.h"
#include "c_plantedc4.h"
#include "tier1/fmtstr.h"
#include "cs_client_gamestats.h"

// [tj] We need to forward declare this, since the definition is all inside the implementation file 
class CHudHintDisplay;
 
//=============================================================================
// HPE_END
//=============================================================================


void __MsgFunc_MatchEndConditions( bf_read &msg );

class CHudChat;

ConVar default_fov( "default_fov", "90", FCVAR_CHEAT );

IClientMode *g_pClientMode = NULL;

// This is a temporary entity used to render the player's model while drawing the class selection menu.
CHandle<C_BaseAnimating> g_PlayerModel;	// player
CHandle<C_BaseAnimating> g_GlovesModel;	// gloves
CHandle<C_BaseAnimating> g_WeaponModel;	// weapon

static ITexture *g_CubemapTexture;

STUB_WEAPON_CLASS( cycler_weapon,	WeaponCycler,	C_BaseCombatWeapon );
STUB_WEAPON_CLASS( weapon_cubemap,	WeaponCubemap,	C_BaseCombatWeapon );

//-----------------------------------------------------------------------------
// HACK: the detail sway convars are archive, and default to 0.  Existing CS:S players thus have no detail
// prop sway.  We'll force them to DoD's default values for now.  What we really need in the long run is
// a system to apply changes to archived convars' defaults to existing players.
extern ConVar cl_detail_max_sway;
extern ConVar cl_detail_avoid_radius;
extern ConVar cl_detail_avoid_force;
extern ConVar cl_detail_avoid_recover_speed;
extern ConVar v_viewmodel_fov;

//-----------------------------------------------------------------------------
ConVar cl_autobuy(
	"cl_autobuy",
	"",
	FCVAR_USERINFO,
	"The order in which autobuy will attempt to purchase items" );

//-----------------------------------------------------------------------------
ConVar cl_rebuy(
	"cl_rebuy",
	"",
	FCVAR_USERINFO,
	"The order in which rebuy will attempt to repurchase items" );

//-----------------------------------------------------------------------------
void SetBuyData( const ConVar &buyVar, const char *filename )
{
	// if we already have autobuy data, don't bother re-parsing the text file
	if ( *buyVar.GetString() )
		return;

	// First, look for a mapcycle file in the cfg directory, which is preferred
	char szRecommendedName[ 256 ];
	char szResolvedName[ 256 ];
	V_sprintf_safe( szRecommendedName, "cfg/%s", filename );
	V_strcpy_safe( szResolvedName, szRecommendedName );
	if ( filesystem->FileExists( szResolvedName, "GAME" ) )
	{
		Msg( "Loading '%s'.\n", szResolvedName );
	}
	else
	{
		// Check the root
		V_strcpy_safe( szResolvedName, filename );
		if ( filesystem->FileExists( szResolvedName, "GAME" ) )
		{
			Msg( "Loading '%s'  ('%s' was not found.)\n", szResolvedName, szRecommendedName );
		}
		else
		{

			// Try cfg/xxx_default.txt
			V_strcpy_safe( szResolvedName, szRecommendedName );
			char *dotTxt = V_stristr( szResolvedName, ".txt" );
			Assert( dotTxt );
			if ( dotTxt )
			{
				V_strcpy( dotTxt, "_default.txt" );
			}
			if ( !filesystem->FileExists( szResolvedName, "GAME" ) )
			{
				Warning( "Not loading buy data.  Neither '%s' nor %s were found.\n", szResolvedName, szRecommendedName );
				return;
			}
			Msg( "Loading '%s'\n", szResolvedName );
		}
	}

	CUtlBuffer buf;
	if ( !filesystem->ReadFile( szResolvedName, "GAME", buf ) )
	{
		// WAT
		Warning( "Failed to load '%s'.\n", szResolvedName );
		return;
	}
	buf.PutChar('\0');

	char token[256];
	char buystring[256];
	V_sprintf_safe( buystring, "setinfo %s \"", buyVar.GetName() );

	const char *pfile = engine->ParseFile( (const char *)buf.Base(), token, sizeof(token) );

	bool first = true;

	while (pfile != NULL)
	{
		if (first)
		{
			first = false;
		}
		else
		{
			Q_strncat(buystring, " ", sizeof(buystring), COPY_ALL_CHARACTERS);
		}

		Q_strncat(buystring, token, sizeof(buystring), COPY_ALL_CHARACTERS);

		pfile = engine->ParseFile( pfile, token, sizeof(token) );
	}

	Q_strncat(buystring, "\"", sizeof(buystring), COPY_ALL_CHARACTERS);

	engine->ClientCmd(buystring);
}

void MsgFunc_KillCam(bf_read &msg) 
{
	C_CSPlayer *pPlayer = ToCSPlayer( C_BasePlayer::GetLocalPlayer() );

	if ( !pPlayer )
		return;

	int newMode = msg.ReadByte();

	if ( newMode != g_nKillCamMode )
	{
#if !defined( NO_ENTITY_PREDICTION )
		if ( g_nKillCamMode == OBS_MODE_NONE )
		{
			// kill cam is switch on, turn off prediction
			g_bForceCLPredictOff = true;
		}
		else if ( newMode == OBS_MODE_NONE )
		{
			// kill cam is switched off, restore old prediction setting is we switch back to normal mode
			g_bForceCLPredictOff = false;
		}
#endif
		g_nKillCamMode = newMode;
	}

	g_nKillCamTarget1	= msg.ReadByte();
	g_nKillCamTarget2	= msg.ReadByte();
}

// --------------------------------------------------------------------------------- //
// CCSModeManager.
// --------------------------------------------------------------------------------- //

class CCSModeManager : public IVModeManager
{
public:
	virtual void	Init();
	virtual void	SwitchMode( bool commander, bool force ) {}
	virtual void	LevelInit( const char *newmap );
	virtual void	LevelShutdown( void );
	virtual void	ActivateMouse( bool isactive ) {}
};

static CCSModeManager g_ModeManager;
IVModeManager *modemanager = ( IVModeManager * )&g_ModeManager;

// --------------------------------------------------------------------------------- //
// CCSModeManager implementation.
// --------------------------------------------------------------------------------- //

#define SCREEN_FILE		"scripts/vgui_screens.txt"

void CCSModeManager::Init()
{
	g_pClientMode = GetClientModeNormal();
	
	PanelMetaClassMgr()->LoadMetaClassDefinitionFile( SCREEN_FILE );
}

void CCSModeManager::LevelInit( const char *newmap )
{
	g_pClientMode->LevelInit( newmap );

	SetBuyData( cl_autobuy, "autobuy.txt" );
	SetBuyData( cl_rebuy, "rebuy.txt" );

#if !defined( NO_ENTITY_PREDICTION )
	if ( g_nKillCamMode > OBS_MODE_NONE )
	{
		g_bForceCLPredictOff = false;
	}
#endif

	g_nKillCamMode		= OBS_MODE_NONE;
	g_nKillCamTarget1	= 0;
	g_nKillCamTarget2	= 0;

	// HACK: the detail sway convars are archive, and default to 0.  Existing CS:S players thus have no detail
	// prop sway.  We'll force them to DoD's default values for now.
	if ( !cl_detail_max_sway.GetFloat() &&
		!cl_detail_avoid_radius.GetFloat() &&
		!cl_detail_avoid_force.GetFloat() &&
		!cl_detail_avoid_recover_speed.GetFloat() )
	{
		cl_detail_max_sway.SetValue( "5" );
		cl_detail_avoid_radius.SetValue( "64" );
		cl_detail_avoid_force.SetValue( "0.4" );
		cl_detail_avoid_recover_speed.SetValue( "0.25" );
	}

	g_CubemapTexture = NULL;
	g_CubemapTexture = g_pMaterialSystem->FindTexture( "editor/cube_vertigo", NULL, true );
	g_CubemapTexture->IncrementReferenceCount();
}

void CCSModeManager::LevelShutdown( void )
{
	g_pClientMode->LevelShutdown();

	g_CubemapTexture->DecrementReferenceCount();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
ClientModeCSNormal::ClientModeCSNormal()
{
	m_CCDeathHandle = INVALID_CLIENT_CCHANDLE;
	m_CCDeathPercent = 0.0f;
	m_CCFreezePeriodHandle_CT = INVALID_CLIENT_CCHANDLE;
	m_CCFreezePeriodPercent_CT = 0.0f;
	m_CCFreezePeriodHandle_T = INVALID_CLIENT_CCHANDLE;
	m_CCFreezePeriodPercent_T = 0.0f;

	HOOK_MESSAGE( MatchEndConditions );
}

void ClientModeCSNormal::Init()
{
	BaseClass::Init();

	ListenForGameEvent( "round_end" );
	ListenForGameEvent( "round_start" );
	ListenForGameEvent( "round_time_warning" );
	ListenForGameEvent( "cs_round_start_beep" );
	ListenForGameEvent( "cs_round_final_beep" );
	ListenForGameEvent( "player_team" );
	ListenForGameEvent( "player_death" );
	ListenForGameEvent( "bomb_planted" );
	ListenForGameEvent( "bomb_exploded" );
	ListenForGameEvent( "bomb_defused" );
	ListenForGameEvent( "hostage_follows" );
	ListenForGameEvent( "hostage_killed" );
	ListenForGameEvent( "hostage_hurt" );
	ListenForGameEvent( "round_freeze_end" );
	ListenForGameEvent( "round_mvp" );
	ListenForGameEvent( "bot_takeover" );

	usermessages->HookMessage( "KillCam", MsgFunc_KillCam );

	// [tj] Add the shared HUD elements to the render groups responsible for hiding 
	//		conflicting UI
	CHudElement* hintBox = (CHudElement*)GET_HUDELEMENT( CHudHintDisplay );
	if (hintBox)
	{
		hintBox->RegisterForRenderGroup("hide_for_scoreboard");
		hintBox->RegisterForRenderGroup("hide_for_round_panel");
	}


	if ( m_CCDeathHandle == INVALID_CLIENT_CCHANDLE )
	{
		const char *szRawFile = "materials/correction/cc_death.raw";
		m_CCDeathPercent = 0.0f;
		m_CCDeathHandle = g_pColorCorrectionMgr->AddColorCorrection( szRawFile );
	}

	if ( m_CCFreezePeriodHandle_CT == INVALID_CLIENT_CCHANDLE )
	{
		const char *szRawFile = "materials/correction/cc_freeze_ct.raw";
		m_CCFreezePeriodPercent_CT = 0.0f;
		m_CCFreezePeriodHandle_CT = g_pColorCorrectionMgr->AddColorCorrection( szRawFile );
	}

	if ( m_CCFreezePeriodHandle_T == INVALID_CLIENT_CCHANDLE )
	{
		const char *szRawFile = "materials/correction/cc_freeze_t.raw";
		m_CCFreezePeriodPercent_T = 0.0f;
		m_CCFreezePeriodHandle_T = g_pColorCorrectionMgr->AddColorCorrection( szRawFile );
	}

	m_fDelayedCTWinTime = -1.0f;
}

void ClientModeCSNormal::InitViewport()
{
	BaseClass::InitViewport();

	m_pViewport = new CounterStrikeViewport();
	m_pViewport->Start( gameuifuncs, gameeventmanager );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ClientModeCSNormal::LevelShutdown( void )
{
	BaseClass::LevelShutdown();

	// Remove any lingering debug overlays, since it's possible they won't get cleaned up automatically later.
	// This is in response to anecdotal reports that players can 'mark' the world with showimpacts or grenade trajectories,
	// then use them to their advantage on subsequent games played immediately on the same map.
	debugoverlay->ClearAllOverlays();
}


void ClientModeCSNormal::Update()
{
	BaseClass::Update();

	// Override the hud's visibility if this is a logo (like E3 demo) map.
	if ( CSGameRules() && CSGameRules()->IsLogoMap() )
		m_pViewport->SetVisible( false );

	if ( (m_fDelayedCTWinTime > 0.0f) && (gpGlobals->curtime >= m_fDelayedCTWinTime) )
	{
		CLocalPlayerFilter filter;
		C_BaseEntity::EmitSound( filter, SOUND_FROM_LOCAL_PLAYER, "Event.CTWin");

		m_fDelayedCTWinTime = -1.0f;
	}

	// halftime music needs a delay thusly
	static bool bStartedHalfTimeMusic = false;
	static float flHalfTimeStart = 0.0;
	
	if( CSGameRules() && CSGameRules()->GetPhase() == GAMEPHASE_HALFTIME  )
	{
		if( !bStartedHalfTimeMusic && gpGlobals->curtime - flHalfTimeStart > 6.5 )
		{
			bStartedHalfTimeMusic = true;
			CSingleUserRecipientFilter filter(C_BasePlayer::GetLocalPlayer());
			PlayMusicSelection(filter, CSMUSIC_HALFTIME);
		}
	}
	else
	{
		flHalfTimeStart = gpGlobals->curtime;
		bStartedHalfTimeMusic = false;
	}
}

//--------------------------------------------------------------------------------------------------------
void ClientModeCSNormal::UpdateColorCorrectionWeights( void )
{
	C_CSPlayer* pPlayer = C_CSPlayer::GetLocalCSPlayer();

	if ( !pPlayer )
	{
		m_CCDeathPercent = 0.0f;
		m_CCFreezePeriodPercent_CT = 0.0f;
		m_CCFreezePeriodPercent_T = 0.0f;
		return;
	}

	bool isDying = false;
	if ( !pPlayer->IsAlive() && (pPlayer->GetObserverMode() == OBS_MODE_DEATHCAM) )
	{
		isDying = true;
	}

	m_CCDeathPercent = clamp( m_CCDeathPercent + ((isDying) ? 0.1f : -0.1f), 0.0f, 1.0f );

	// apply CT and T CCs from spectated players as well
	// no need for that with death CC
	pPlayer = GetHudPlayer();
	if ( !pPlayer )
	{
		m_CCDeathPercent = 0.0f;
		m_CCFreezePeriodPercent_CT = 0.0f;
		m_CCFreezePeriodPercent_T = 0.0f;
		return;
	}
	
	float flTimer = 0;

	bool bFreezePeriod = CSGameRules()->IsFreezePeriod();
	bool bImmune = pPlayer->m_bImmunity;
	if ( bFreezePeriod || bImmune )
	{
		float flFadeBegin = 2.0f;

		// countdown to the start of the round while we're in freeze period
		if ( bImmune )
		{
			// if freeze time is also active and freeze time is longer than immune time, use that time instead
			if ( bFreezePeriod && (CSGameRules()->GetRoundStartTime() - gpGlobals->curtime) > (pPlayer->m_fImmuneToDamageTime - gpGlobals->curtime))
				flTimer = CSGameRules()->GetRoundStartTime() - gpGlobals->curtime;
			else
			{
				flTimer = pPlayer->m_fImmuneToDamageTime - gpGlobals->curtime;
				flFadeBegin = 0.5;
			}
		}
		else if ( bFreezePeriod )
		{
			flTimer = CSGameRules()->GetRoundStartTime() - gpGlobals->curtime;
		}

		if ( flTimer > flFadeBegin )
		{
			if ( pPlayer->GetTeamNumber() == TEAM_CT )
			{
				m_CCFreezePeriodPercent_CT = 1.0f;
				m_CCFreezePeriodPercent_T = 0.0f;
			}
			else if ( pPlayer->GetTeamNumber() == TEAM_TERRORIST )
			{
				m_CCFreezePeriodPercent_CT = 0.0f;
				m_CCFreezePeriodPercent_T = 1.0f;
			}
			else
			{
				m_CCFreezePeriodPercent_CT = 0.0f;
				m_CCFreezePeriodPercent_T = 0.0f;
			}
		}
		else
		{
			if ( pPlayer->GetTeamNumber() == TEAM_CT )
			{
				m_CCFreezePeriodPercent_CT = clamp( flTimer / flFadeBegin, 0.05f, 1.0f );
				m_CCFreezePeriodPercent_T = 0;
			}
			else if ( pPlayer->GetTeamNumber() == TEAM_TERRORIST )
			{
				m_CCFreezePeriodPercent_T = clamp( flTimer / flFadeBegin, 0.05f, 1.0f );
				m_CCFreezePeriodPercent_CT = 0;
			}
			else
			{
				m_CCFreezePeriodPercent_CT = 0.0f;
				m_CCFreezePeriodPercent_T = 0.0f;
			}
		}
	}
	else
	{
		m_CCFreezePeriodPercent_T = 0;
		m_CCFreezePeriodPercent_CT = 0;
	}

}

void ClientModeCSNormal::OnColorCorrectionWeightsReset( void )
{
	UpdateColorCorrectionWeights();
	g_pColorCorrectionMgr->SetColorCorrectionWeight( m_CCDeathHandle, m_CCDeathPercent );
	g_pColorCorrectionMgr->SetColorCorrectionWeight( m_CCFreezePeriodHandle_CT, m_CCFreezePeriodPercent_CT );
	g_pColorCorrectionMgr->SetColorCorrectionWeight( m_CCFreezePeriodHandle_T, m_CCFreezePeriodPercent_T );
}

/*
void ClientModeCSNormal::UpdateSpectatorMode( void )
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if ( !pPlayer )
		return;

	IMapOverview * overviewmap = m_pViewport->GetMapOverviewInterface();

	if ( !overviewmap )
		return;

	overviewmap->SetTime( gpGlobals->curtime );

	int obs_mode = pPlayer->GetObserverMode();

	if ( obs_mode < OBS_MODE_IN_EYE )
		return;

	Vector worldpos = pPlayer->GetLocalOrigin();
	QAngle angles; engine->GetViewAngles( angles );

	C_BaseEntity *target = pPlayer->GetObserverTarget();

	if ( target && (obs_mode == OBS_MODE_IN_EYE || obs_mode == OBS_MODE_CHASE) )
	{
		worldpos = target->GetAbsOrigin();

		if ( obs_mode == OBS_MODE_IN_EYE )
		{
			angles = target->GetAbsAngles();
		}
	}

	Vector2D mappos = overviewmap->WorldToMap( worldpos );

	overviewmap->SetCenter( mappos );
	overviewmap->SetAngle( angles.y );	
	
	for ( int i = 1; i<= MAX_PLAYERS; i++)
	{
		C_BaseEntity *ent = ClientEntityList().GetEnt( i );

		if ( !ent || !ent->IsPlayer() )
			continue;

		C_BasePlayer *p = ToBasePlayer( ent );

		// update position of active players in our PVS
		Vector position = p->GetAbsOrigin();
		QAngle angle = p->GetAbsAngles();

		if ( p->IsDormant() )
		{
			// if player is not in PVS, use PlayerResources data
			position = g_PR->GetPosition( i );
			angles[1] = g_PR->GetViewAngle( i );
		}
		
		overviewmap->SetPlayerPositions( i-1, position, angles );
	}
} */

//-----------------------------------------------------------------------------
// Purpose: We've received a keypress from the engine. Return 1 if the engine is allowed to handle it.
//-----------------------------------------------------------------------------
int	ClientModeCSNormal::KeyInput( int down, ButtonCode_t keynum, const char *pszCurrentBinding )
{
	// don't process input in LogoMaps
	if( CSGameRules() && CSGameRules()->IsLogoMap() )
		return 1;
	
	return BaseClass::KeyInput( down, keynum, pszCurrentBinding );
}



IClientMode *GetClientModeNormal()
{
	static ClientModeCSNormal g_ClientModeNormal;
	return &g_ClientModeNormal;
}


ClientModeCSNormal* GetClientModeCSNormal()
{
	Assert( dynamic_cast< ClientModeCSNormal* >( GetClientModeNormal() ) );

	return static_cast< ClientModeCSNormal* >( GetClientModeNormal() );
}

#if IRONSIGHT
#ifdef DEBUG
	ConVar ironsight_scoped_viewmodel_fov( "ironsight_scoped_viewmodel_fov", "54", FCVAR_CHEAT, "The fov of the viewmodel when ironsighted" );
#else
	#define IRONSIGHT_SCOPED_FOV 54.0f
#endif
#endif

float ClientModeCSNormal::GetViewModelFOV( void )
{
#if IRONSIGHT
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if ( pPlayer )
	{
		CWeaponCSBase *pIronSightWeapon = (CWeaponCSBase*)pPlayer->GetActiveWeapon();
		if ( pIronSightWeapon )
		{
			CIronSightController* pIronSightController = pIronSightWeapon->GetIronSightController();
			if ( pIronSightController && pIronSightController->IsInIronSight() )
			{
				return FLerp( v_viewmodel_fov.GetFloat(),	
					#ifdef DEBUG
						ironsight_scoped_viewmodel_fov.GetFloat(),
					#else
						IRONSIGHT_SCOPED_FOV,
					#endif
				pIronSightController->GetIronSightAmount() );
			}
		}
	}
#endif
	return v_viewmodel_fov.GetFloat();
}

int ClientModeCSNormal::GetDeathMessageStartHeight( void )
{
	return m_pViewport->GetDeathMessageStartHeight();
}

void ClientModeCSNormal::FireGameEvent( IGameEvent *event )
{
	CBaseHudChat *pHudChat = (CBaseHudChat *)GET_HUDELEMENT( CHudChat );
	C_BasePlayer *pLocalPlayer = C_BasePlayer::GetLocalPlayer();
	CLocalPlayerFilter filter;
	
	if ( !pLocalPlayer || !pHudChat )
		return;

	const char *eventname = event->GetName();

	if ( !eventname || !eventname[0] )
		return;

	if ( Q_strcmp( "round_start", eventname ) == 0 )
	{
		// recreate all client side physics props
		C_PhysPropClientside::RecreateAll();

		// remove hostage ragdolls
		for ( int i=0; i<g_HostageRagdolls.Count(); ++i )
		{
			// double-check that the EHANDLE is still valid
			if ( g_HostageRagdolls[i] )
			{
				g_HostageRagdolls[i]->Release();
			}
		}
		g_HostageRagdolls.RemoveAll();

		// Just tell engine to clear decals
		engine->ClientCmd( "r_cleardecals\n" );

		//stop any looping sounds
		enginesound->StopAllSounds( true );

		Soundscape_OnStopAllSounds();	// Tell the soundscape system.

		// Remove any left over particle effects from the last round.
		ParticleMgr()->SetRemoveAllParticleEffects();
	}
	else if ( Q_strcmp( "round_end", eventname ) == 0 )
	{
		int winningTeam = event->GetInt("winner");
		int reason = event->GetInt("reason");

		if ( reason != Game_Commencing )
		{
			// if spectating play music for team being spectated at that moment
			C_BasePlayer* pTeamPlayer = pLocalPlayer;
			if ( pLocalPlayer->GetTeamNumber() == TEAM_SPECTATOR || pLocalPlayer->IsHLTV() )
			{
				pTeamPlayer = GetHudPlayer();
			}
			if ( winningTeam == pTeamPlayer->GetTeamNumber() )
			{
				PlayMusicSelection( filter, CSMUSIC_WONROUND );
			}
			else
			{
				PlayMusicSelection( filter, CSMUSIC_LOSTROUND );
			}
		}

		// play endround announcer sound
		if ( winningTeam == TEAM_CT )
		{
			if ( reason == Bomb_Defused )
			{
				C_BaseEntity::EmitSound( filter, SOUND_FROM_LOCAL_PLAYER, "Event.BombDefused" );
				m_fDelayedCTWinTime = gpGlobals->curtime + C_BaseEntity::GetSoundDuration( "Event.BombDefused", NULL ) + 0.3;
			}
			else
			{
				C_BaseEntity::EmitSound( filter, SOUND_FROM_LOCAL_PLAYER, "Event.CTWin");
			}
		}
		else if ( winningTeam == TEAM_TERRORIST )
		{
			C_BaseEntity::EmitSound( filter, SOUND_FROM_LOCAL_PLAYER, "Event.TERWin");
		}
		else
		{
			C_BaseEntity::EmitSound( filter, SOUND_FROM_LOCAL_PLAYER, "Event.RoundDraw");
		}
		
		// [pfreese] Only show centerprint message for game commencing; the rest of 
		// these messages are handled by the end-of-round panel.
		// [Forrest] Show all centerprint messages if the end-of-round panel is disabled.
		static ConVarRef sv_nowinpanel( "sv_nowinpanel" );
		static ConVarRef cl_nowinpanel( "cl_nowinpanel" );
		bool isFinishedGunGameRound = CSGameRules()->GetGamemode() == GameModes::ARMS_RACE && (reason == CTs_Win || reason == Terrorists_Win);
		if ( isFinishedGunGameRound || reason == Game_Commencing || sv_nowinpanel.GetBool() || cl_nowinpanel.GetBool() )
		{
			internalCenterPrint->Print( hudtextmessage->LookupString( event->GetString("message") ) );

			// we are starting a new round; clear the current match stats
			g_CSClientGameStats.ResetMatchStats();
		}
	}
	else if ( Q_strcmp( "player_team", eventname ) == 0 )
	{
		CBaseHudChat *pHudChat = (CBaseHudChat *)GET_HUDELEMENT( CHudChat );
		C_BasePlayer *pPlayer = USERID2PLAYER( event->GetInt("userid") );
		
		if ( !pPlayer )
			return;

		bool bDisconnected = event->GetBool("disconnect");

		if ( bDisconnected )
			return;

		int iTeam = event->GetInt("team");

		if ( pPlayer->IsLocalPlayer() )
		{
			// that's me
			pPlayer->TeamChange( iTeam );
		}
		
		bool bSilent = event->GetBool( "silent" );
		if ( !bSilent )
		{
			wchar_t wszLocalized[100];
			wchar_t wszPlayerName[MAX_PLAYER_NAME_LENGTH];
			char szLocalized[100];
			bool bIsBot = event->GetBool("isbot"); // squelch 'bot has joined the game' messages

			if ( iTeam == TEAM_SPECTATOR && !bIsBot )
			{
				g_pVGuiLocalize->ConvertANSIToUnicode( pPlayer->GetPlayerName(), wszPlayerName, sizeof(wszPlayerName) );
				g_pVGuiLocalize->ConstructString( wszLocalized, sizeof( wszLocalized ), g_pVGuiLocalize->Find( "#Cstrike_game_join_spectators" ), 1, wszPlayerName );

				g_pVGuiLocalize->ConvertUnicodeToANSI( wszLocalized, szLocalized, sizeof(szLocalized) );
				pHudChat->Printf( CHAT_FILTER_NONE, "%s", szLocalized );
			}
			else if ( iTeam == TEAM_TERRORIST && !bIsBot )
			{
				g_pVGuiLocalize->ConvertANSIToUnicode( pPlayer->GetPlayerName(), wszPlayerName, sizeof(wszPlayerName) );
				g_pVGuiLocalize->ConstructString( wszLocalized, sizeof( wszLocalized ), g_pVGuiLocalize->Find( "#Cstrike_game_join_terrorist" ), 1, wszPlayerName );

				g_pVGuiLocalize->ConvertUnicodeToANSI( wszLocalized, szLocalized, sizeof(szLocalized) );
				pHudChat->Printf( CHAT_FILTER_NONE, "%s", szLocalized );
			}
			else if ( iTeam == TEAM_CT && !bIsBot )
			{
				g_pVGuiLocalize->ConvertANSIToUnicode( pPlayer->GetPlayerName(), wszPlayerName, sizeof(wszPlayerName) );
				g_pVGuiLocalize->ConstructString( wszLocalized, sizeof( wszLocalized ), g_pVGuiLocalize->Find( "#Cstrike_game_join_ct" ), 1, wszPlayerName );

				g_pVGuiLocalize->ConvertUnicodeToANSI( wszLocalized, szLocalized, sizeof(szLocalized) );
				pHudChat->Printf( CHAT_FILTER_NONE, "%s", szLocalized );
			}
		}
	}
	else if ( Q_strcmp( "bomb_planted", eventname ) == 0 )
	{
		//C_BasePlayer *pPlayer = USERID2PLAYER( event->GetInt("userid") );

		wchar_t wszLocalized[100];
		wchar_t seconds[4];

		V_swprintf_safe( seconds, L"%d", mp_c4timer.GetInt() );

		g_pVGuiLocalize->ConstructString( wszLocalized, sizeof( wszLocalized ), g_pVGuiLocalize->Find( "#Cstrike_TitlesTXT_Bomb_Planted" ), 1, seconds );

		// show centerprint message
		internalCenterPrint->Print( wszLocalized );

		PlayMusicSelection( filter, CSMUSIC_BOMB );

		// play sound
		 C_BaseEntity::EmitSound( filter, SOUND_FROM_LOCAL_PLAYER, "Event.BombPlanted")  ;
	}
	else if ( Q_strcmp( "bomb_defused", eventname ) == 0 )
	{
		// C_BasePlayer *pPlayer = USERID2PLAYER( event->GetInt("userid") );
	}

	// [menglish] Tell the client side bomb that the bomb has exploding here creating the explosion particle effect	 
	else if ( Q_strcmp( "bomb_exploded", eventname ) == 0 )
	{
		if ( g_PlantedC4s.Count() > 0 )
		{
			// bomb is planted
			C_PlantedC4 *pC4 = g_PlantedC4s[0];
			pC4->Explode();
		}
	}
	else if ( Q_strcmp( "hostage_follows", eventname ) == 0 )
	{
		internalCenterPrint->Print( "#Cstrike_TitlesTXT_Hostage_Being_Taken" );

		bool roundWasAlreadyWon = ( CSGameRules()->m_iRoundWinStatus != WINNER_NONE );
		if ( !roundWasAlreadyWon )
		{
			PlayMusicSelection( filter, CSMUSIC_HOSTAGE );
		}
	}
	else if ( Q_strcmp( "hostage_killed", eventname ) == 0 )
	{
		// play sound for spectators and CTs
		if ( pLocalPlayer->IsObserver() || (pLocalPlayer->GetTeamNumber() == TEAM_CT) )
		{
			C_BaseEntity::EmitSound( filter, SOUND_FROM_LOCAL_PLAYER, "Event.HostageKilled")  ;
		}

		// Show warning to killer
		if ( pLocalPlayer->GetUserID() == event->GetInt("userid") )
		{
			internalCenterPrint->Print( "#Cstrike_TitlesTXT_Killed_Hostage" );
		}
	}
	else if ( Q_strcmp( "hostage_hurt", eventname ) == 0 )
	{
		// Let the loacl player know he harmed a hostage
		if ( pLocalPlayer->GetUserID() == event->GetInt("userid") )
		{
			internalCenterPrint->Print( "#Cstrike_TitlesTXT_Injured_Hostage" );
		}
	}
	else if ( Q_strcmp( "player_death", eventname ) == 0 )
	{
		C_BasePlayer *pPlayer = USERID2PLAYER( event->GetInt("userid") );

		C_CSPlayer* csPlayer = ToCSPlayer(pPlayer);
		if (csPlayer)
		{
			csPlayer->ClearSoundEvents();
		}

		if ( pPlayer == C_BasePlayer::GetLocalPlayer() )
		{
			// we just died, hide any buy panels
			gViewPortInterface->ShowPanel( PANEL_BUY, false );
			gViewPortInterface->ShowPanel( PANEL_BUY_CT, false );
			gViewPortInterface->ShowPanel( PANEL_BUY_TER, false );
			gViewPortInterface->ShowPanel( PANEL_BUY_EQUIP_CT, false );
			gViewPortInterface->ShowPanel( PANEL_BUY_EQUIP_TER, false );
		}
	}
	else if ( Q_strcmp( "player_changename", eventname ) == 0 )
	{
		return; // server sends a colorized text string for this
	}

    // [tj] We handle this here instead of in the base class 
    //      The reason is that we don't use string tables to localize.
    //      Instead, we use the steam localization mechanism.

    // [dwenger] Remove dependency on stats system for name of achievement.
    else if ( Q_strcmp( "achievement_earned", eventname ) == 0 )
    {
        CBaseHudChat *hudChat = (CBaseHudChat *)GET_HUDELEMENT( CHudChat );
        int iPlayerIndex = event->GetInt( "player" );
        C_BasePlayer *pPlayer = UTIL_PlayerByIndex( iPlayerIndex );
        int iAchievement = event->GetInt( "achievement" );

        if ( !hudChat || !pPlayer )
            return;


        CAchievementMgr *pAchievementMgr = dynamic_cast<CAchievementMgr *>( engine->GetAchievementMgr() );
        if ( !pAchievementMgr )
            return;

        IAchievement *pAchievement = pAchievementMgr->GetAchievementByID( iAchievement );
        if ( pAchievement )
        {
            if ( !pPlayer->IsDormant() && pPlayer->ShouldAnnounceAchievement() )
            {
                pPlayer->SetNextAchievementAnnounceTime( gpGlobals->curtime + ACHIEVEMENT_ANNOUNCEMENT_MIN_TIME );

                //Do something for the player - Actually we should probably do this client-side when the achievement is first earned.
                if (pPlayer->IsLocalPlayer()) 
                {
                }
                pPlayer->OnAchievementAchieved( iAchievement );
            }

            if ( g_PR )
            {
                wchar_t wszPlayerName[MAX_PLAYER_NAME_LENGTH];
                g_pVGuiLocalize->ConvertANSIToUnicode( g_PR->GetPlayerName( iPlayerIndex ), wszPlayerName, sizeof( wszPlayerName ) );

                wchar_t achievementName[1024];
                const wchar_t* constAchievementName = &achievementName[0];

                constAchievementName = ACHIEVEMENT_LOCALIZED_NAME( pAchievement );

                if (constAchievementName)
                {
                    wchar_t wszLocalizedString[128];
                    g_pVGuiLocalize->ConstructString( wszLocalizedString, sizeof( wszLocalizedString ), g_pVGuiLocalize->Find( "#Achievement_Earned" ), 2, wszPlayerName, constAchievementName/*wszAchievementString*/ );

                    char szLocalized[128];
                    g_pVGuiLocalize->ConvertUnicodeToANSI( wszLocalizedString, szLocalized, sizeof( szLocalized ) );

                    hudChat->ChatPrintf( iPlayerIndex, CHAT_FILTER_ACHIEVEMENT, "%s", szLocalized );

					/*
                    if (pPlayer->IsLocalPlayer()) 
                    {
                        char achievementDescription[1024];
                        const char* constAchievementDescription = &achievementDescription[0];

                        constAchievementDescription = pUserStats->GetAchievementDisplayAttribute( pAchievement->GetName(), "desc" );  
                        hudChat->ChatPrintf( iPlayerIndex, CHAT_FILTER_ACHIEVEMENT, "(%s)", constAchievementDescription );
                    }
					*/
                }
            }
        }
    }
	else if ( V_strcmp( "round_freeze_end", eventname ) == 0 )
	{
		int nObsMode = pLocalPlayer->GetObserverMode();
		if ( nObsMode == OBS_MODE_FIXED || nObsMode == OBS_MODE_ROAMING )
		{
			C_CSPlayer* pCSLocalPlayer = ToCSPlayer( pLocalPlayer );
			if ( pCSLocalPlayer->GetCurrentMusic() == CSMUSIC_START )
			{
				CLocalPlayerFilter filter;
				PlayMusicSelection( filter, CSMUSIC_ACTION );
				pCSLocalPlayer->SetCurrentMusic( CSMUSIC_ACTION );
			}
		}
	}
	if ( V_strcmp( "round_time_warning", eventname ) == 0 )
	{
		if(	!CSGameRules()->m_bBombPlanted )
		{
			PlayMusicSelection( filter, CSMUSIC_ROUNDTEN );
			for ( int i = 1; i <= gpGlobals->maxClients; i++ )
			{
				CCSPlayer* pPlayer = ToCSPlayer(UTIL_PlayerByIndex(i));
				if( !pPlayer )
					continue;
			}	
		}
	}
	else if ( V_strcmp( "cs_round_start_beep", eventname ) == 0 )
	{
		bool bTeamPanelActive = ( gViewPortInterface->GetActivePanel() &&  ( V_strcmp( gViewPortInterface->GetActivePanel()->GetName(), PANEL_TEAM ) == 0 ) );
	
		if( !bTeamPanelActive )
		{
			CLocalPlayerFilter filter;
			CBaseEntity::EmitSound( filter, 0, "UI.CounterBeep" );
		}
	}
	else if ( V_strcmp( "cs_round_final_beep", eventname ) == 0 )
	{
		bool bTeamPanelActive = ( gViewPortInterface->GetActivePanel() && ( V_strcmp( gViewPortInterface->GetActivePanel()->GetName(), PANEL_TEAM ) == 0 ) );

		if( !bTeamPanelActive )
		{
			CBaseEntity::EmitSound( filter, 0, "UI.CounterDoneBeep" );
		}

		int nObsMode = pLocalPlayer->GetObserverMode();
		if( nObsMode == OBS_MODE_FIXED || nObsMode == OBS_MODE_ROAMING )
		{
			C_CSPlayer *pCSLocalPlayer = ToCSPlayer(pLocalPlayer);
			if(pCSLocalPlayer->GetCurrentMusic() == CSMUSIC_START )
			{
				CLocalPlayerFilter filter;
				PlayMusicSelection(filter, CSMUSIC_ACTION);
				pCSLocalPlayer->SetCurrentMusic(CSMUSIC_ACTION);
			}
		}
	}
	else if ( V_strcmp( "round_mvp", eventname ) == 0 )
	{
		PlayMusicSelection( filter, CSMUSIC_MVP );
	}
	else if ( V_strcmp( "bot_takeover", eventname ) == 0 )
	{
		C_BasePlayer* pBot = UTIL_PlayerByUserId( event->GetInt( "botid" ) );
		if ( pBot && pLocalPlayer && pLocalPlayer->GetUserID() == event->GetInt( "userid" ) )
		{
			wchar_t wszLocalized[100];
			wchar_t wszPlayerName[MAX_PLAYER_NAME_LENGTH];
			g_pVGuiLocalize->ConvertANSIToUnicode( pBot->GetPlayerName(), wszPlayerName, sizeof( wszPlayerName ) );
			g_pVGuiLocalize->ConstructString( wszLocalized, sizeof( wszLocalized ), g_pVGuiLocalize->Find( "#Cstrike_TitlesTXT_Hint_Bot_Takeover" ), 1, wszPlayerName );

			internalCenterPrint->Print( wszLocalized );
		}
	}

	else
	{
		BaseClass::FireGameEvent( event );
	}
}


bool ShouldRecreateImageEntity( C_BaseAnimating *pEnt, const char *pNewModelName )
{
	if ( !pNewModelName || !pNewModelName[0] )
		return false;

	if ( !pEnt )
		return true;

	const model_t *pModel = pEnt->GetModel();

	if ( !pModel )
		return true;

	const char *pName = modelinfo->GetModelName( pModel );
	if ( !pName )
		return true;

	// reload only if names are different
	return( Q_stricmp( pName, pNewModelName ) != 0 );
}

ConVar cl_simple_player_lighting( "cl_simple_player_lighting", "0", FCVAR_ARCHIVE );
void UpdateImageEntity(
	const char *szWeaponClassname,
	const char *szPlayerModel,
	int x, int y, int width, int height,
	float viewX, float viewY, float viewZ, float viewFOV,
	bool bIsClassSelection )
{
	C_CSPlayer *pLocalPlayer = C_CSPlayer::GetLocalCSPlayer();
	
	if ( !pLocalPlayer )
		return;

	MDLCACHE_CRITICAL_SECTION();

	const char* szWeaponModel = NULL;
	const char* szWeaponSequence = NULL;
	int iTeamNumber = pLocalPlayer->GetTeamNumber();

	if ( !szPlayerModel || !szPlayerModel[0] )
		szPlayerModel = modelinfo->GetModelName( pLocalPlayer->GetModel() );

	bool bActiveWeapon = false;
	if ( !szWeaponClassname || !szWeaponClassname[0] )
	{
		C_BaseCombatWeapon *pPrimaryWeapon = pLocalPlayer->Weapon_GetSlot( WEAPON_SLOT_RIFLE );
		C_BaseCombatWeapon *pSecondaryWeapon = pLocalPlayer->Weapon_GetSlot( WEAPON_SLOT_PISTOL );
		C_BaseCombatWeapon *pKnifeWeapon = pLocalPlayer->Weapon_GetSlot( WEAPON_SLOT_KNIFE );
		C_BaseCombatWeapon *pActiveWeapon = pLocalPlayer->GetActiveWeapon();

		if ( pPrimaryWeapon )
		{
			szWeaponClassname = pPrimaryWeapon->GetClassname();
			bActiveWeapon = true;
		}
		else if ( pSecondaryWeapon )
		{
			szWeaponClassname = pSecondaryWeapon->GetClassname();
			bActiveWeapon = true;
		}
		else if ( pKnifeWeapon )
		{
			szWeaponClassname = pKnifeWeapon->GetClassname();
			bActiveWeapon = true;
		}
		else if ( pActiveWeapon )
		{
			szWeaponClassname = pActiveWeapon->GetClassname();
			bActiveWeapon = true;
		}
		else if ( bIsClassSelection )
		{
			szWeaponClassname = "weapon_ak47";
			if ( Q_strncmp( V_UnqualifiedFileName( szPlayerModel ), "ctm_", 4 ) == 0 )
			{
				// give CTs an m4
				szWeaponClassname = "weapon_m4a4";
			}
		}
	}

	if ( iTeamNumber < TEAM_TERRORIST ) // invalid team number
	{
		if ( Q_strncmp( V_UnqualifiedFileName( szPlayerModel ), "ctm_", 4 ) == 0 )
			iTeamNumber = TEAM_CT;
		else
			iTeamNumber = TEAM_TERRORIST;
	}

	bool bSilenced = true;

	if ( !szWeaponClassname || !szWeaponClassname[0] )
	{
		szWeaponSequence = "t_buymenu_nowep";
	}
	else
	{
		// don't swap active weapon for a loadout one
		if ( !bActiveWeapon )
		{
			const char* szLoadoutWeapon = CSLoadout()->GetWeaponFromSlot( pLocalPlayer, CSLoadout()->GetSlotFromWeapon( iTeamNumber, szWeaponClassname + 7 ) ); // +7 to get rid of weapon_ prefix
			if ( szLoadoutWeapon && szLoadoutWeapon[0] )
				szWeaponClassname = UTIL_VarArgs( "weapon_%s", szLoadoutWeapon );
		}

		WEAPON_FILE_INFO_HANDLE	hWpnInfo = LookupWeaponInfoSlot( szWeaponClassname );
		if ( hWpnInfo == GetInvalidWeaponInfoHandle() )
		{
			if ( Q_strcmp( szWeaponClassname, "item_defuser" ) == 0 )
			{
				szWeaponModel = "models/weapons/w_defuser.mdl";
				szWeaponSequence = "t_buymenu_defuser";
			}
			else if ( Q_strcmp( szWeaponClassname, "item_kevlar" ) == 0 )
			{
				szWeaponModel = "models/weapons/w_eq_armor.mdl";
				szWeaponSequence = "t_buymenu_armor_helmet";
			}
			else if ( Q_strcmp( szWeaponClassname, "item_assaultsuit" ) == 0 )
			{
				szWeaponModel = "models/weapons/w_eq_armor_helmet.mdl";
				szWeaponSequence = "t_buymenu_armor_helmet";
			}
			else
			{
				Warning( "UpdateBuyMenuImageEntity: Unable to get weapon info for %s.\n", szWeaponClassname );
				return;
			}
		}
		else
		{
			CCSWeaponInfo *pWeaponInfo = dynamic_cast<CCSWeaponInfo*>(GetFileWeaponInfoFromHandle( hWpnInfo ));
			if ( pWeaponInfo )
			{
				szWeaponModel = pWeaponInfo->szWorldModel;
				if ( bIsClassSelection )
				{
					if ( iTeamNumber == TEAM_TERRORIST )
						szWeaponSequence = pWeaponInfo->m_szClassMenuAnimT;
					else
						szWeaponSequence = pWeaponInfo->m_szClassMenuAnim;
				}
				else
				{
					if ( iTeamNumber == TEAM_TERRORIST )
						szWeaponSequence = pWeaponInfo->m_szBuyMenuAnimT;
					else
						szWeaponSequence = pWeaponInfo->m_szBuyMenuAnim;
				}
			}
			else
			{
				Warning( "UpdateBuyMenuImageEntity: Unable to get weapon info for %s.\n", szWeaponClassname );
				return;
			}
		}
	}

	C_BaseAnimating *pPlayerModel = g_PlayerModel.Get();

	// Does the entity even exist yet?
	bool recreatePlayer = ShouldRecreateImageEntity( pPlayerModel, szPlayerModel );
	if ( recreatePlayer )
	{
		if ( pPlayerModel )
			pPlayerModel->Remove();

		pPlayerModel = new C_BaseAnimating;
		pPlayerModel->InitializeAsClientEntity( szPlayerModel, RENDER_GROUP_OPAQUE_ENTITY );
		pPlayerModel->AddEffects( EF_NODRAW ); // don't let the renderer draw the model normally
		pPlayerModel->m_flAnimTime = gpGlobals->curtime;
		pPlayerModel->SetSequence( pPlayerModel->LookupSequence( szWeaponSequence ) );

		g_PlayerModel = pPlayerModel;
	}

	bool bCreateGloves = false;
	const char *szGlovesViewModel = NULL;
	if ( CSLoadout()->HasGlovesSet( pLocalPlayer, pLocalPlayer->GetTeamNumber() ) )
	{
		szGlovesViewModel = GetGlovesInfo( CSLoadout()->GetGlovesForPlayer( pLocalPlayer, pLocalPlayer->GetTeamNumber() ) )->szViewModel;
	}
	if ( pPlayerModel && szGlovesViewModel && pLocalPlayer->m_szPlayerDefaultGloves && pPlayerModel->DoesModelSupportGloves( szGlovesViewModel, pLocalPlayer->m_szPlayerDefaultGloves ) )
	{
		bCreateGloves = true;
	}

	C_BaseAnimating *pWeaponModel = g_WeaponModel.Get();

	// Does the entity even exist yet?
	if ( recreatePlayer || ShouldRecreateImageEntity( pWeaponModel, szWeaponModel ) )
	{
		if ( pWeaponModel )
			pWeaponModel->Remove();

		pWeaponModel = new C_BaseAnimating;
		pWeaponModel->InitializeAsClientEntity( szWeaponModel, RENDER_GROUP_OPAQUE_ENTITY );
		pWeaponModel->AddEffects( EF_NODRAW ); // don't let the renderer draw the model normally
		pWeaponModel->FollowEntity( pPlayerModel ); // attach to player model
		pWeaponModel->m_flAnimTime = gpGlobals->curtime;

		int silencerBodygroup = pWeaponModel->FindBodygroupByName( "silencer" );
		if ( silencerBodygroup > -1 )
			pWeaponModel->SetBodygroup( silencerBodygroup, bSilenced ? 0 : 1 );
		g_WeaponModel = pWeaponModel;
	}
	else if ( !szWeaponModel || !szWeaponModel[0] )
	{
		// so the weapon model is gone when playing a nowep sequence
		if ( pWeaponModel )
			pWeaponModel->Remove();
	}

	C_BaseAnimating *pGlovesModel = g_GlovesModel.Get();

	if ( bCreateGloves )
	{
		const char* pGlovesName = GetGlovesInfo( CSLoadout()->GetGlovesForPlayer( pLocalPlayer, pLocalPlayer->GetTeamNumber() ) )->szWorldModel;

		// Does the entity even exist yet?
		if ( recreatePlayer || ShouldRecreateImageEntity( pGlovesModel, pGlovesName ) )
		{
			if ( pGlovesModel )
				pGlovesModel->Remove();

			pGlovesModel = new C_BaseAnimating;
			pGlovesModel->InitializeAsClientEntity( pGlovesName, RENDER_GROUP_OPAQUE_ENTITY );
			pGlovesModel->AddEffects( EF_NODRAW ); // don't let the renderer draw the model normally
			pGlovesModel->FollowEntity( pPlayerModel ); // attach to player model
			pGlovesModel->m_nSkin = GetPlayerViewmodelArmConfigForPlayerModel( modelinfo->GetModelName( pLocalPlayer->GetModel() ) )->iSkintoneIndex; // set the corrent skin tone
			pGlovesModel->m_flAnimTime = gpGlobals->curtime;

			g_GlovesModel = pGlovesModel;
		}

		pPlayerModel->SetBodygroup( pPlayerModel->FindBodygroupByName( "gloves" ), 1 );
	}
	else
	{
		pPlayerModel->SetBodygroup( pPlayerModel->FindBodygroupByName( "gloves" ), 0 );
		if ( pGlovesModel )
		{
			pGlovesModel->Remove();
		}
	}

	Vector playerPos = vec3_origin;
	pPlayerModel->SetAbsOrigin( playerPos );
	pPlayerModel->SetAbsAngles( vec3_angle );

	// now set the sequence for this player model if needed
	if ( !bIsClassSelection )
	{
		int sequence = pPlayerModel->LookupSequence( szWeaponSequence );
		if ( pPlayerModel->GetSequence() != sequence )
			pPlayerModel->SetSequence( sequence );
	}
	pPlayerModel->FrameAdvance( gpGlobals->frametime );

	// Now draw it.
	CViewSetup view;
	view.x = x;
	view.y = y;
	view.width = width;
	view.height = height;

	view.m_bOrtho = false;
	view.fov = viewFOV;

	Vector viewOrigin = playerPos + Vector( viewX, viewY, viewZ );
	view.origin = viewOrigin;

	view.angles.Init( 0.0f, 180.0f, 0.0f );
	view.zNear = VIEW_NEARZ;
	view.zFar = 1000;

	CMatRenderContextPtr pRenderContext( materials );

	// PiMoN: bind a cubemap for swag
	pRenderContext->BindLocalCubemap( g_CubemapTexture );

	pRenderContext->SetLightingOrigin( vec3_origin );
	pRenderContext->SetAmbientLight( 0.4, 0.4, 0.4 );

	// PiMoN: let this model have a proper lighting for once!
	if ( cl_simple_player_lighting.GetBool() )
	{
		static LightDesc_t spotLight( Vector( 128, 0, 128 ), Vector( 1, 1, 1 ), Vector( 0, 0, 64 ), 0.5f, 1.0f );
		g_pStudioRender->SetLocalLights( 1, &spotLight );
	}
	else
	{
		static LightDesc_t lights[3];
		lights[0].InitDirectional( Vector( -0.50f, 0.80f, 0.00f ), Vector( 0.21f, 0.21f, 0.22f ) );
		lights[1].InitDirectional( Vector( 0.70f, -0.80f, 0.00f ), Vector( 0.07f, 0.10f, 0.13f ) );
		lights[2].InitSpot( Vector( 66.32f, -17.06f, 124.60f ), Vector( 1.10f, 1.25f, 1.35f ), Vector( 0.0f, 0.0f, 56.0f ), 0.25f, 1.0f );
		g_pStudioRender->SetLocalLights( 3, lights );
	}

	Frustum dummyFrustum;
	render->Push3DView( view, 0, NULL, dummyFrustum );

	modelrender->SuppressEngineLighting( true );
	float color[3] = { 1.0f, 1.0f, 1.0f };
	render->SetColorModulation( color );
	render->SetBlend( 1.0f );
	pPlayerModel->DrawModel( STUDIO_RENDER );
	if ( pWeaponModel )
	{
		pWeaponModel->DrawModel( STUDIO_RENDER );
	}
	if ( bCreateGloves && pGlovesModel )
	{
		pGlovesModel->DrawModel( STUDIO_RENDER );
	}
	modelrender->SuppressEngineLighting( false );

	render->PopView( dummyFrustum );

	pRenderContext->BindLocalCubemap( NULL );
	pRenderContext.SafeRelease();
}

bool WillPanelBeVisible( vgui::VPANEL hPanel )
{
	while ( hPanel )
	{
		if ( !vgui::ipanel()->IsVisible( hPanel ) )
			return false;

		hPanel = vgui::ipanel()->GetParent( hPanel );
	}
	return true;
}

void ClientModeCSNormal::PostRenderVGui()
{
	// If the team menu is up, then we will render the model of the character that is currently selected.
	for ( int i=0; i < g_ClassImagePanels.Count(); i++ )
	{
		CCSClassImagePanel *pPanel = g_ClassImagePanels[i];
		if ( WillPanelBeVisible( pPanel->GetVPanel() ) )
		{
			// Ok, we have a visible class image panel.
			int x, y, w, h;
			pPanel->GetBounds( x, y, w, h );
			pPanel->LocalToScreen( x, y );

			// Allow for the border.
			x += 1;
			y += 1;
			w -= 2;
			h -= 2;

			UpdateImageEntity( NULL, pPanel->m_ModelName, x, y, w, h, pPanel->m_ViewXPos, pPanel->m_ViewYPos, pPanel->m_ViewZPos, pPanel->m_ViewFOV, true );
			return;
		}
	}

	// If the team menu is up, then we will render the model of the character that is currently selected.
	for ( int i=0; i < g_BuyMenuPlayerImagePanels.Count(); i++ )
	{
		CCSBuyMenuPlayerImagePanel *pPanel = g_BuyMenuPlayerImagePanels[i];
		if ( WillPanelBeVisible( pPanel->GetVPanel() ) )
		{
			// Ok, we have a visible class image panel.
			int x, y, w, h;
			pPanel->GetBounds( x, y, w, h );

			// Allow for the border.
			x += 1;
			y += 1;
			w -= 2;
			h -= 2;

			UpdateImageEntity( NULL, NULL, x, y, w, h, pPanel->m_ViewXPos, pPanel->m_ViewYPos, pPanel->m_ViewZPos, pPanel->m_ViewFOV, false );
			return;
		}
	}

	// If the buy menu is up, then we will render the model of the weapon that is currently selected.
	for ( int i=0; i < g_BuyMenuImagePanels.Count(); i++ )
	{
		CCSBuyMenuImagePanel *pPanel = g_BuyMenuImagePanels[i];
		if ( WillPanelBeVisible( pPanel->GetVPanel() ) )
		{
			// Ok, we have a visible class image panel.
			int x, y, w, h;
			pPanel->GetBounds( x, y, w, h );
			pPanel->LocalToScreen( x, y );

			// Allow for the border.
			x += 1;
			y += 1;
			w -= 2;
			h -= 2;

			UpdateImageEntity( pPanel->m_WeaponName, NULL, x, y, w, h, pPanel->m_ViewXPos, pPanel->m_ViewYPos, pPanel->m_ViewZPos, pPanel->m_ViewFOV, false );
			return;
		}
	}
}

bool ClientModeCSNormal::ShouldDrawViewModel( void )
{
	C_CSPlayer *pPlayer = GetHudPlayer();
	
	if( pPlayer && pPlayer->GetFOV() != CSGameRules()->DefaultFOV() && pPlayer->m_bIsScoped )
	{
		CWeaponCSBase *pWpn = pPlayer->GetActiveCSWeapon();

		if( pWpn && pWpn->HideViewModelWhenZoomed() )
		{
			return false;
		}
	}

	return BaseClass::ShouldDrawViewModel();
}


bool ClientModeCSNormal::CanRecordDemo( char *errorMsg, int length ) const
{
	C_CSPlayer *player = C_CSPlayer::GetLocalCSPlayer();
	if ( !player )
	{
		return true;
	}

	if ( !player->IsAlive() )
	{
		return true;
	}

	// don't start recording while flashed, as it would remove the flash
	if ( player->m_flFlashBangTime > gpGlobals->curtime )
	{
		Q_strncpy( errorMsg, "Cannot record demos while blind.", length );
		return false;
	}

	// don't start recording while smoke grenades are spewing smoke, as the existing smoke would be destroyed
	C_BaseEntityIterator it;
	C_BaseEntity *ent;
	while ( (ent = it.Next()) != NULL )
	{
		if ( Q_strcmp( ent->GetClassname(), "class C_ParticleSmokeGrenade" ) == 0 )
		{
			Q_strncpy( errorMsg, "Cannot record demos while a smoke grenade is active.", length );
			return false;
		}
	}

	return true;
}

//=============================================================================
// HPE_BEGIN:
// [menglish] Save server information shown to the client in a persistent place
//=============================================================================
 
void ClientModeCSNormal::SetServerName(wchar_t* name)
{
	V_wcsncpy(m_pServerName, name, sizeof( m_pServerName ) );
}

void ClientModeCSNormal::SetMapName(wchar_t* name)
{
	V_wcsncpy(m_pMapName, name, sizeof( m_pMapName ) );
}

//=============================================================================
// HPE_END
//=============================================================================

// Receive the PlayerIgnited user message and send out a clientside event for achievements to hook.
void __MsgFunc_MatchEndConditions( bf_read &msg )
{
	int iFragLimit = (int) msg.ReadLong();
	int iMaxRounds = (int) msg.ReadLong();
	int iTimeLimit = (int) msg.ReadLong();

	IGameEvent *event = gameeventmanager->CreateEvent( "match_end_conditions" );
	if ( event )
	{
		event->SetInt( "frags", iFragLimit );
		event->SetInt( "max_rounds", iMaxRounds );
		event->SetInt( "time", iTimeLimit );
		gameeventmanager->FireEventClientSide( event );
	}
}
