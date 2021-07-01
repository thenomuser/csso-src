//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: a small piece of HUD that shows alive counter and win counter for each team
//
// $NoKeywords: $
//
//=============================================================================//

#include "cbase.h"
#include "hudelement.h"
#include "hud_numericdisplay.h"
#include "c_cs_player.h"
#include "c_cs_team.h"
#include "c_cs_playerresource.h"

using namespace vgui;

ConVar hud_playercount_pos( "hud_playercount_pos", "0", FCVAR_ARCHIVE, "0 = default (top), 1 = bottom" );

//-----------------------------------------------------------------------------
// Purpose: CT version
//-----------------------------------------------------------------------------
class CHudWinCounterCT: public CHudElement, public CHudNumericDisplay
{
	DECLARE_CLASS_SIMPLE( CHudWinCounterCT, CHudNumericDisplay );

public:
	CHudWinCounterCT( const char *pElementName );
	virtual void Init( void );
	virtual void Paint( void );
	virtual void Think();
	virtual bool ShouldDraw();
	virtual void ApplySchemeSettings( IScheme *scheme );

private:
	int		m_iCounter;

	CHudTexture *m_pTeamIcon;

	CPanelAnimationVarAliasType( float, icon_xpos, "icon_xpos", "0", "proportional_float" );
	CPanelAnimationVarAliasType( float, icon_ypos, "icon_ypos", "0", "proportional_float" );

	float icon_tall;
	float icon_wide;

	int m_iOriginalXPos;
	int m_iOriginalYPos;

	bool m_bIsAtTheTop;
};

DECLARE_HUDELEMENT( CHudWinCounterCT );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudWinCounterCT::CHudWinCounterCT( const char *pElementName ) : CHudElement( pElementName ), CHudNumericDisplay(NULL, "HudWinCounterCT")
{
	SetHiddenBits( HIDEHUD_PLAYERDEAD );

	m_iOriginalXPos = -1;
	m_iOriginalYPos = -1;
	m_bIsAtTheTop = false;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudWinCounterCT::Init()
{
	m_iCounter = 0;
	SetIndent( false );
	SetDisplayValue( m_iCounter );
}

void CHudWinCounterCT::ApplySchemeSettings( IScheme *scheme )
{
	BaseClass::ApplySchemeSettings( scheme );

	if ( !m_pTeamIcon )
	{
		m_pTeamIcon = gHUD.GetIcon( "ct_icon" );
	}

	if ( m_pTeamIcon )
	{
		icon_tall = GetTall() - YRES(2);
		float scale = icon_tall / (float) m_pTeamIcon->Height();
		icon_wide = (scale) * (float) m_pTeamIcon->Width();
	}

	GetPos( m_iOriginalXPos, m_iOriginalYPos );
}

void CHudWinCounterCT::Paint( void )
{
	if ( m_pTeamIcon )
	{
		m_pTeamIcon->DrawSelf( icon_xpos, icon_ypos, icon_wide, icon_tall, GetFgColor() );
	}

	BaseClass::Paint();
}

void CHudWinCounterCT::Think()
{
	if ( m_bIsAtTheTop != hud_playercount_pos.GetBool() )
	{
		m_bIsAtTheTop = hud_playercount_pos.GetBool();

		if ( m_bIsAtTheTop )
		{
			int ypos = ScreenHeight() - m_iOriginalYPos - GetTall(); // inverse its Y pos
			SetPos( m_iOriginalXPos, ypos );
		}
		else
		{
			SetPos( m_iOriginalXPos, m_iOriginalYPos );
		}
	}

	C_CSTeam *team = GetGlobalCSTeam( TEAM_CT );
	if ( team )
		SetDisplayValue( team->Get_Score() );
}

bool CHudWinCounterCT::ShouldDraw()
{
	//necessary?
	C_CSPlayer *pPlayer = C_CSPlayer::GetLocalCSPlayer();
	if ( pPlayer && pPlayer->IsObserver() )
		return false;

	return true;
}

class CHudAliveCounterCT: public CHudElement, public CHudNumericDisplay
{
	DECLARE_CLASS_SIMPLE( CHudAliveCounterCT, CHudNumericDisplay );

public:
	CHudAliveCounterCT( const char *pElementName );
	virtual void Init( void );
	virtual void Think();
	virtual bool ShouldDraw();
	virtual void ApplySchemeSettings( IScheme *scheme );

private:
	int		m_iCounter;

	CPanelAnimationVarAliasType( float, icon_xpos, "icon_xpos", "0", "proportional_float" );
	CPanelAnimationVarAliasType( float, icon_ypos, "icon_ypos", "0", "proportional_float" );

	float icon_tall;
	float icon_wide;

	int m_iOriginalXPos;
	int m_iOriginalYPos;

	bool m_bIsAtTheBottom;
};

DECLARE_HUDELEMENT( CHudAliveCounterCT );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudAliveCounterCT::CHudAliveCounterCT( const char *pElementName ): CHudElement( pElementName ), CHudNumericDisplay( NULL, "HudAliveCounterCT" )
{
	SetHiddenBits( HIDEHUD_PLAYERDEAD );

	m_iOriginalXPos = -1;
	m_iOriginalYPos = -1;
	m_bIsAtTheBottom = false;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudAliveCounterCT::Init()
{
	m_iCounter = 0;
	SetIndent( false );
	SetCenter( true );
	SetDisplayValue( m_iCounter );
}

void CHudAliveCounterCT::ApplySchemeSettings( IScheme *scheme )
{
	BaseClass::ApplySchemeSettings( scheme );

	GetPos( m_iOriginalXPos, m_iOriginalYPos );
}

void CHudAliveCounterCT::Think()
{
	if ( m_bIsAtTheBottom != hud_playercount_pos.GetBool() )
	{
		m_bIsAtTheBottom = hud_playercount_pos.GetBool();

		if ( m_bIsAtTheBottom )
		{
			int ypos = ScreenHeight() - m_iOriginalYPos - GetTall(); // inverse its Y pos
			SetPos( m_iOriginalXPos, ypos );
		}
		else
		{
			SetPos( m_iOriginalXPos, m_iOriginalYPos );
		}
	}

	if ( g_PR )
	{
		// Count the players on the team.
		m_iCounter = 0;
		for ( int playerIndex = 1; playerIndex <= MAX_PLAYERS; playerIndex++ )
		{
			if ( g_PR->IsConnected( playerIndex ) && g_PR->GetTeam( playerIndex ) == TEAM_CT && g_PR->IsAlive( playerIndex ) )
				++m_iCounter;
		}
		SetDisplayValue( m_iCounter );
	}
}

bool CHudAliveCounterCT::ShouldDraw()
{
	//necessary?
	C_CSPlayer *pPlayer = C_CSPlayer::GetLocalCSPlayer();
	if ( pPlayer && pPlayer->IsObserver() )
		return false;

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: T version
//-----------------------------------------------------------------------------
class CHudWinCounterT: public CHudElement, public CHudNumericDisplay
{
	DECLARE_CLASS_SIMPLE( CHudWinCounterT, CHudNumericDisplay );

public:
	CHudWinCounterT( const char *pElementName );
	virtual void Init( void );
	virtual void Paint( void );
	virtual void Think();
	virtual bool ShouldDraw();
	virtual void ApplySchemeSettings( IScheme *scheme );

private:
	int		m_iCounter;

	CHudTexture *m_pTeamIcon;

	CPanelAnimationVarAliasType( float, icon_xpos, "icon_xpos", "0", "proportional_float" );
	CPanelAnimationVarAliasType( float, icon_ypos, "icon_ypos", "0", "proportional_float" );

	float icon_tall;
	float icon_wide;

	int m_iOriginalXPos;
	int m_iOriginalYPos;

	bool m_bIsAtTheTop;
};

DECLARE_HUDELEMENT( CHudWinCounterT );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudWinCounterT::CHudWinCounterT( const char *pElementName ): CHudElement( pElementName ), CHudNumericDisplay( NULL, "HudWinCounterT" )
{
	SetHiddenBits( HIDEHUD_PLAYERDEAD );

	m_iOriginalXPos = -1;
	m_iOriginalYPos = -1;

	m_bIsAtTheTop = false;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudWinCounterT::Init()
{
	m_iCounter = 0;
	SetIndent( true );
	SetDisplayValue( m_iCounter );
}

void CHudWinCounterT::ApplySchemeSettings( IScheme *scheme )
{
	BaseClass::ApplySchemeSettings( scheme );

	if ( !m_pTeamIcon )
	{
		m_pTeamIcon = gHUD.GetIcon( "t_icon" );
	}

	if ( m_pTeamIcon )
	{
		icon_tall = GetTall() - YRES(2);
		float scale = icon_tall / (float) m_pTeamIcon->Height();
		icon_wide = (scale) * (float) m_pTeamIcon->Width();
	}

	GetPos( m_iOriginalXPos, m_iOriginalYPos );
}

void CHudWinCounterT::Paint( void )
{
	if ( m_pTeamIcon )
	{
		m_pTeamIcon->DrawSelf( icon_xpos, icon_ypos, icon_wide, icon_tall, GetFgColor() );
	}

	BaseClass::Paint();
}

void CHudWinCounterT::Think()
{
	if ( m_bIsAtTheTop != hud_playercount_pos.GetBool() )
	{
		m_bIsAtTheTop = hud_playercount_pos.GetBool();

		if ( m_bIsAtTheTop )
		{
			int ypos = ScreenHeight() - m_iOriginalYPos - GetTall(); // inverse its Y pos
			SetPos( m_iOriginalXPos, ypos );
		}
		else
		{
			SetPos( m_iOriginalXPos, m_iOriginalYPos );
		}
	}

	C_CSTeam *team = GetGlobalCSTeam( TEAM_TERRORIST );
	if ( team )
		SetDisplayValue( team->Get_Score() );
}

bool CHudWinCounterT::ShouldDraw()
{
	//necessary?
	C_CSPlayer *pPlayer = C_CSPlayer::GetLocalCSPlayer();
	if ( pPlayer && pPlayer->IsObserver() )
		return false;

	return true;
}

class CHudAliveCounterT: public CHudElement, public CHudNumericDisplay
{
	DECLARE_CLASS_SIMPLE( CHudAliveCounterT, CHudNumericDisplay );

public:
	CHudAliveCounterT( const char *pElementName );
	virtual void Init( void );
	virtual void Think();
	virtual bool ShouldDraw();
	virtual void ApplySchemeSettings( IScheme *scheme );

private:
	int		m_iCounter;

	CPanelAnimationVarAliasType( float, icon_xpos, "icon_xpos", "0", "proportional_float" );
	CPanelAnimationVarAliasType( float, icon_ypos, "icon_ypos", "0", "proportional_float" );

	float icon_tall;
	float icon_wide;

	int m_iOriginalXPos;
	int m_iOriginalYPos;

	bool m_bIsAtTheBottom;
};

DECLARE_HUDELEMENT( CHudAliveCounterT );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudAliveCounterT::CHudAliveCounterT( const char *pElementName ): CHudElement( pElementName ), CHudNumericDisplay( NULL, "HudAliveCounterT" )
{
	SetHiddenBits( HIDEHUD_PLAYERDEAD );

	m_iOriginalXPos = -1;
	m_iOriginalYPos = -1;
	m_bIsAtTheBottom = false;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudAliveCounterT::Init()
{
	m_iCounter = 0;
	SetIndent( false );
	SetCenter( true );
	SetDisplayValue( m_iCounter );
}

void CHudAliveCounterT::ApplySchemeSettings( IScheme *scheme )
{
	BaseClass::ApplySchemeSettings( scheme );

	GetPos( m_iOriginalXPos, m_iOriginalYPos );
}

void CHudAliveCounterT::Think()
{
	if ( m_bIsAtTheBottom != hud_playercount_pos.GetBool() )
	{
		m_bIsAtTheBottom = hud_playercount_pos.GetBool();

		if ( m_bIsAtTheBottom )
		{
			int ypos = ScreenHeight() - m_iOriginalYPos - GetTall(); // inverse its Y pos
			SetPos( m_iOriginalXPos, ypos );
		}
		else
		{
			SetPos( m_iOriginalXPos, m_iOriginalYPos );
		}
	}

	if ( g_PR )
	{
		// Count the players on the team.
		m_iCounter = 0;
		for ( int playerIndex = 1; playerIndex <= MAX_PLAYERS; playerIndex++ )
		{
			if ( g_PR->IsConnected( playerIndex ) && g_PR->GetTeam( playerIndex ) == TEAM_TERRORIST && g_PR->IsAlive( playerIndex ) )
				++m_iCounter;
		}
		SetDisplayValue( m_iCounter );
	}
}

bool CHudAliveCounterT::ShouldDraw()
{
	//necessary?
	C_CSPlayer *pPlayer = C_CSPlayer::GetLocalCSPlayer();
	if ( pPlayer && pPlayer->IsObserver() )
		return false;

	return true;
}