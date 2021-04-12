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
#include "c_cs_player.h"
#include "cs_gamerules.h"

#include "c_cs_hostage.h"
#include "c_plantedc4.h"

class CHudScenarioC4Icon : public CHudElement, public vgui::Panel
{
public:
	DECLARE_CLASS_SIMPLE( CHudScenarioC4Icon, vgui::Panel );

	CHudScenarioC4Icon( const char *name );

	virtual bool ShouldDraw();	
	virtual void Paint();

private:
	CPanelAnimationVar( Color, m_clrIcon, "IconColor", "IconColor" );	

	CHudTexture *m_pIcon;
};


DECLARE_HUDELEMENT( CHudScenarioC4Icon );


CHudScenarioC4Icon::CHudScenarioC4Icon( const char *pName ) :
	vgui::Panel( NULL, "HudScenarioC4Icon" ), CHudElement( pName )
{
	SetParent( g_pClientMode->GetViewport() );
	m_pIcon = NULL;

	SetHiddenBits( HIDEHUD_PLAYERDEAD );
}

bool CHudScenarioC4Icon::ShouldDraw()
{
	C_CSPlayer *pPlayer = C_CSPlayer::GetLocalCSPlayer();
	return pPlayer && pPlayer->IsAlive();
}

void CHudScenarioC4Icon::Paint()
{
	// If there is a bomb planted, draw that
	if( g_PlantedC4s.Count() > 0 )
	{
		if ( !m_pIcon )
		{
			m_pIcon = gHUD.GetIcon( "scenario_c4" );
		}

		if ( m_pIcon )
		{
			int x, y, w, h;
			GetBounds( x, y, w, h );

			C_PlantedC4 *pC4 = g_PlantedC4s[0];

			Color c = m_clrIcon;

			c[3] = 80;

			if( pC4->m_flNextGlow - gpGlobals->curtime < 0.1 )
			{
				c[3] = 255;
			}

			if( pC4->IsBombActive() )
				m_pIcon->DrawSelf( 0, 0, h, h, c );	//draw it square!
		}
	}
}



class CHudScenarioHostageIcon : public CHudElement, public vgui::Panel
{
public:
	DECLARE_CLASS_SIMPLE( CHudScenarioHostageIcon, vgui::Panel );

	CHudScenarioHostageIcon( const char *name );

	virtual bool ShouldDraw();	
	virtual void Paint();

private:
	CPanelAnimationVar( Color, m_clrIcon, "IconColor", "IconColor" );	

	CHudTexture *m_pIcon;
};


DECLARE_HUDELEMENT( CHudScenarioHostageIcon );


CHudScenarioHostageIcon::CHudScenarioHostageIcon( const char *pName ) :
	vgui::Panel( NULL, "HudScenarioHostageIcon" ), CHudElement( pName )
{
	SetParent( g_pClientMode->GetViewport() );
	m_pIcon = NULL;

	SetHiddenBits( HIDEHUD_PLAYERDEAD );
}

bool CHudScenarioHostageIcon::ShouldDraw()
{
	C_CSPlayer *pPlayer = C_CSPlayer::GetLocalCSPlayer();
	return pPlayer && pPlayer->IsAlive();
}

void CHudScenarioHostageIcon::Paint()
{
	CCSGameRules *pRules = CSGameRules();

	// If there are hostages, draw how many there are
	if( pRules && pRules->GetNumHostagesRemaining() )
	{
		if ( !m_pIcon )
		{
			m_pIcon = gHUD.GetIcon( "scenario_hostage" );
		}

		if( m_pIcon )
		{
			int xpos = 0;
			int iconWidth = m_pIcon->Width();

			for(int i=0;i<pRules->GetNumHostagesRemaining();i++)
			{
				m_pIcon->DrawSelf( xpos, 0, m_clrIcon );
				xpos += iconWidth;
			}
		}
	}
}



