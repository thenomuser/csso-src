//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "hudelement.h"
#include <vgui_controls/Panel.h>
#include <vgui/ISurface.h>
#include "clientmode_csnormal.h"
#include "cs_gamerules.h"
#include "hud_numericdisplay.h"
#include <vgui_controls/AnimationController.h>

extern ConVar hud_account_style;

class CHudShoppingCart : public CHudElement, public vgui::Panel
{
public:
	DECLARE_CLASS_SIMPLE( CHudShoppingCart, vgui::Panel );

	CHudShoppingCart( const char *name );

	virtual bool ShouldDraw();	
	virtual void Paint();
	virtual void Init();
	virtual void OnThink();

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );


private:
	CPanelAnimationVar( Color, m_clrIcon, "IconColor", "IconColor" );

	CPanelAnimationVarAliasType( int, legacy_xpos, "legacy_xpos", "0", "proportional_xpos" );
	CPanelAnimationVarAliasType( int, legacy_ypos, "legacy_ypos", "0", "proportional_ypos" );

	int m_iStyle;
	int m_iOriginalXPos;
	int m_iOriginalYPos;

	CHudTexture *m_pCartIcon;
	bool		m_bPrevState;
};

DECLARE_HUDELEMENT( CHudShoppingCart );

CHudShoppingCart::CHudShoppingCart( const char *pName ) :
	vgui::Panel( NULL, "HudShoppingCart" ), CHudElement( pName )
{
	SetParent( g_pClientMode->GetViewport() );
	m_pCartIcon = NULL;
	
	SetHiddenBits( HIDEHUD_PLAYERDEAD );

	// [tj] Add this to the render group that disappears when the scoreboard is up
	RegisterForRenderGroup( "hide_for_scoreboard" );

	m_iStyle = -1;
	m_iOriginalXPos = 0;
	m_iOriginalYPos = 0;
}

void CHudShoppingCart::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	GetPos( m_iOriginalXPos, m_iOriginalYPos );
}

void CHudShoppingCart::Init()
{
}

bool CHudShoppingCart::ShouldDraw()
{
	C_CSPlayer *pPlayer = C_CSPlayer::GetLocalCSPlayer();

	// [tj] Added base class call
	return ( pPlayer && pPlayer->IsInBuyZone() && pPlayer->GetTeamNumber() != TEAM_UNASSIGNED && !CSGameRules()->IsBuyTimeElapsed() && CHudElement::ShouldDraw() && CSGameRules()->GetGamemode() != GameModes::DEATHMATCH );
}

void CHudShoppingCart::OnThink()
{
	if ( m_iStyle != hud_account_style.GetInt() )
	{
		m_iStyle = hud_account_style.GetInt();

		if ( m_iStyle == 1 )
			SetPos( legacy_xpos, legacy_ypos );
		else
			SetPos( m_iOriginalXPos, m_iOriginalYPos );
	}
}

void CHudShoppingCart::Paint()
{
	if ( !m_pCartIcon )
	{
		m_pCartIcon = gHUD.GetIcon( "shopping_cart" );
	}
	
	if ( m_pCartIcon )
	{
		int x, y, w, h;
		GetBounds( x, y, w, h );

		m_pCartIcon->DrawSelf( 0, 0, w, h, m_clrIcon );
	}
}

