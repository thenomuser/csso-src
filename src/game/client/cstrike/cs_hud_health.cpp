//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//
//=============================================================================//
//
// Health.cpp
//
// implementation of CHudHealth class
//
#include "cbase.h"
#include "hud.h"
#include "hud_macros.h"
#include "view.h"

#include "iclientmode.h"

#define PAIN_NAME "sprites/%d_pain.vmt"

#include <KeyValues.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/AnimationController.h>

using namespace vgui;

#include "hudelement.h"
#include "hud_numericdisplay.h"
#include "cs_gamerules.h"

#include "convar.h"

ConVar hud_healtharmor_style( "hud_healtharmor_style", "0", FCVAR_ARCHIVE, "0 = default, 1 = simple, 2 = old", true, 0, true, HUD_STYLE_MAX );

//-----------------------------------------------------------------------------
// Purpose: Health panel
//-----------------------------------------------------------------------------
class CHudHealth : public CHudElement, public CHudNumericDisplay
{
	DECLARE_CLASS_SIMPLE( CHudHealth, CHudNumericDisplay );

public:
	CHudHealth( const char *pElementName );
	virtual void Init( void );
	virtual void VidInit( void );
	virtual void Reset( void );
	virtual void OnThink();

	virtual void Paint( void );
	virtual void ApplySchemeSettings( IScheme *scheme );

private:
	// old variables
	int		m_iHealth;

	int		m_bitsDamage;

	CHudTexture *m_pHealthIcon;

	CPanelAnimationVarAliasType( float, icon_xpos, "icon_xpos", "0", "proportional_float" );
	CPanelAnimationVarAliasType( float, icon_ypos, "icon_ypos", "0", "proportional_float" );

	CPanelAnimationVarAliasType( int, legacy_xpos, "legacy_xpos", "0", "proportional_xpos" );
	CPanelAnimationVarAliasType( int, legacy_ypos, "legacy_ypos", "0", "proportional_ypos" );
	CPanelAnimationVarAliasType( int, legacy_wide, "legacy_wide", "0", "proportional_width" );
	CPanelAnimationVarAliasType( int, legacy_tall, "legacy_tall", "0", "proportional_height" );

	CPanelAnimationVarAliasType( int, simple_xpos, "simple_xpos", "0", "proportional_xpos" );
	CPanelAnimationVarAliasType( int, simple_ypos, "simple_ypos", "0", "proportional_ypos" );
	CPanelAnimationVarAliasType( int, simple_wide, "simple_wide", "0", "proportional_width" );
	CPanelAnimationVarAliasType( int, simple_tall, "simple_tall", "0", "proportional_height" );

	CPanelAnimationVarAliasType( float, progress_xpos, "progress_xpos", "0", "proportional_float" );
	CPanelAnimationVarAliasType( float, progress_ypos, "progress_ypos", "0", "proportional_float" );
	CPanelAnimationVarAliasType( float, progress_wide, "progress_wide", "0", "proportional_float" );
	CPanelAnimationVarAliasType( float, progress_tall, "progress_tall", "0", "proportional_float" );
	CPanelAnimationVar( Color, m_ProgressFgColor, "ProgressFgColor", "FgColor" );
	CPanelAnimationVar( Color, m_ProgressBgColor, "ProgressBgColor", "BgColor" );

	int m_iStyle;
	int m_iOriginalXPos;
	int m_iOriginalYPos;
	int m_iOriginalWide;
	int m_iOriginalTall;

	float icon_tall;
	float icon_wide;

};

DECLARE_HUDELEMENT( CHudHealth );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudHealth::CHudHealth( const char *pElementName ) : CHudElement( pElementName ), CHudNumericDisplay(NULL, "HudHealth")
{
	SetHiddenBits( HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD );

	m_iStyle = -1;
	m_iOriginalXPos = 0;
	m_iOriginalYPos = 0;
	m_iOriginalWide = 0;
	m_iOriginalTall = 0;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudHealth::Init()
{
	m_iHealth		= 100;
	m_bitsDamage	= 0;
	icon_tall		= 0;
	icon_wide		= 0;
	SetIndent(true);
	SetDisplayValue(m_iHealth);
}

void CHudHealth::ApplySchemeSettings( IScheme *scheme )
{
	BaseClass::ApplySchemeSettings( scheme );

	if( !m_pHealthIcon )
	{
		m_pHealthIcon = gHUD.GetIcon( "health_icon" );
	}

	if( m_pHealthIcon )
	{

		icon_tall = GetTall() - YRES(2);
		float scale = icon_tall / (float)m_pHealthIcon->Height();
		icon_wide = ( scale ) * (float)m_pHealthIcon->Width();
	}

	GetBounds( m_iOriginalXPos, m_iOriginalYPos, m_iOriginalWide, m_iOriginalTall );
}

//-----------------------------------------------------------------------------
// Purpose: reset health to normal color at round restart
//-----------------------------------------------------------------------------
void CHudHealth::Reset()
{
	g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HealthRestored");
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudHealth::VidInit()
{
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudHealth::OnThink()
{
	if ( m_iStyle != hud_healtharmor_style.GetInt() )
	{
		m_iStyle = hud_healtharmor_style.GetInt();

		switch ( m_iStyle )
		{
			case HUD_STYLE_DEFAULT:
				SetBounds( m_iOriginalXPos, m_iOriginalYPos, m_iOriginalWide, m_iOriginalTall );
				break;

			case HUD_STYLE_SIMPLE:
				SetBounds( simple_xpos, simple_ypos, simple_wide, simple_tall );
				break;

			case HUD_STYLE_LEGACY:
				SetBounds( legacy_xpos, legacy_ypos, legacy_wide, legacy_tall );
				break;
		}
	}

	int realHealth = 0;
	C_BasePlayer *local = C_BasePlayer::GetLocalPlayer();
	if ( local )
	{
		// Never below zero
		realHealth = MAX( local->GetHealth(), 0 );
	}

	// Only update the fade if we've changed health
	if ( realHealth == m_iHealth )
	{
		return;
	}

	if( realHealth > m_iHealth)
	{
		// round restarted, we have 100 again
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HealthRestored");
	}
	else if ( realHealth <= 25 )
	{
		// we are badly injured
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HealthLow");
	}
	else if( realHealth < m_iHealth )
	{
		// took a hit
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HealthTookDamage");
	}

	m_iHealth = realHealth;

	SetDisplayValue(m_iHealth);
}

void CHudHealth::Paint( void )
{
	if( m_pHealthIcon )
	{
		m_pHealthIcon->DrawSelf( icon_xpos, icon_ypos, icon_wide, icon_tall, GetFgColor() );
	}

	if ( hud_healtharmor_style.GetInt() == HUD_STYLE_DEFAULT )
	{
		// draw the progress bar
		DrawBox( progress_xpos, progress_ypos, progress_wide, progress_tall, m_ProgressBgColor, 1.0f );
		if ( m_iHealth > 0 )
			DrawBox( progress_xpos, progress_ypos, progress_wide * Clamp( m_iHealth / 100.0f, 0.0f, 1.0f ), progress_tall, m_ProgressFgColor, 1.0f );
	}

	//draw the health icon
	BaseClass::Paint();
}