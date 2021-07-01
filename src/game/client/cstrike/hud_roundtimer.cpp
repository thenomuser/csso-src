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
#include "hud_basetimer.h"
#include "hud_bitmapnumericdisplay.h"
#include "c_plantedc4.h"

#include <vgui_controls/AnimationController.h>

extern ConVar hud_playercount_pos;

class CHudRoundTimer : public CHudElement, public vgui::Panel
{
public:
	DECLARE_CLASS_SIMPLE( CHudRoundTimer, vgui::Panel );

	CHudRoundTimer( const char *name );

protected:	
	virtual void Paint();
	virtual void Think();
	virtual bool ShouldDraw();
	bool ShouldDrawText();
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

	void PaintTime(HFont font, int xpos, int ypos, int mins, int secs);

private:
	int m_iTimer;
	float m_flToggleTime;
	float m_flNextToggle;
	CHudTexture *m_pTimerIcon;
	bool m_bFlash;

	int m_iOriginalXPos;
	int m_iOriginalYPos;

	int m_iAdditiveWhiteID;

	CPanelAnimationVar( Color, m_FlashColor, "FlashColor", "Panel.FgColor" );

	CPanelAnimationVarAliasType( float, icon_xpos, "icon_xpos", "0", "proportional_float" );
	CPanelAnimationVarAliasType( float, icon_ypos, "icon_ypos", "0", "proportional_float" );

	CPanelAnimationVar( Color, m_TextColor, "TextColor", "Panel.FgColor" );
	CPanelAnimationVar( vgui::HFont, m_hNumberFont, "NumberFont", "HudNumbers" );
	CPanelAnimationVarAliasType( float, digit_xpos, "digit_xpos", "50", "proportional_float" );
	CPanelAnimationVarAliasType( float, digit_ypos, "digit_ypos", "2", "proportional_float" );

	float icon_tall;
	float icon_wide;

	bool m_bIsAtTheTop;
};


DECLARE_HUDELEMENT( CHudRoundTimer );


CHudRoundTimer::CHudRoundTimer( const char *pName ) :
	BaseClass( NULL, "HudRoundTimer" ), CHudElement( pName )
{
	m_iAdditiveWhiteID = vgui::surface()->CreateNewTextureID();
	vgui::surface()->DrawSetTextureFile( m_iAdditiveWhiteID, "vgui/white_additive" , true, false);

	SetHiddenBits( HIDEHUD_PLAYERDEAD );

	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	m_iTimer = -1;

	m_iOriginalXPos = -1;
	m_iOriginalYPos = -1;

	m_bIsAtTheTop = false;
}

void CHudRoundTimer::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	m_pTimerIcon = gHUD.GetIcon( "timer_icon" );

	if( m_pTimerIcon )
	{
		icon_tall = GetTall() - YRES(2);
		float scale = icon_tall / (float)m_pTimerIcon->Height();
		icon_wide = ( scale ) * (float)m_pTimerIcon->Width();
	}

	SetFgColor( m_TextColor );

	BaseClass::ApplySchemeSettings( pScheme );

	GetPos( m_iOriginalXPos, m_iOriginalYPos );
}

bool CHudRoundTimer::ShouldDraw()
{
	//necessary?
	C_CSPlayer *pPlayer = C_CSPlayer::GetLocalCSPlayer();
	if ( pPlayer && pPlayer->IsObserver() )
		return false;

	return true;
}

bool CHudRoundTimer::ShouldDrawText()
{
	if ( g_PlantedC4s.Count() > 0 )
		return false;

	if ( CSGameRules() && CSGameRules()->IsWarmupPeriodPaused() )
		return false;

	if ( CSGameRules() && CSGameRules()->IsMatchWaitingForResume() && CSGameRules()->IsFreezePeriod() && !CSGameRules()->IsTimeOutActive() )
		return false;

	return true;
}

void CHudRoundTimer::Think()
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

	C_CSGameRules *pRules = CSGameRules();
	if ( !pRules )
		return;

	m_iTimer = (int)ceil( pRules->GetRoundRemainingTime() );

	if ( pRules->IsWarmupPeriod() && !pRules->IsWarmupPeriodPaused() )
	{
		m_iTimer = (int)ceil( pRules->GetWarmupRemainingTime() );
	}
	if ( pRules->IsFreezePeriod() )
	{
		if ( pRules->IsTimeOutActive() )
		{
			C_CSPlayer *pPlayer = C_CSPlayer::GetLocalCSPlayer();
			if ( pPlayer )
			{
				switch ( pPlayer->GetTeamNumber() )
				{
					case TEAM_CT:
						m_iTimer = (int) ceil( pRules->GetCTTimeOutRemaining() );
						break;
					case TEAM_TERRORIST:
						m_iTimer = (int) ceil( pRules->GetTerroristTimeOutRemaining() );
						break;
				}
			}
		}
		else
		{
			// in freeze period countdown to round start time
			m_iTimer = (int) ceil( pRules->GetRoundStartTime() - gpGlobals->curtime );
		}
	}
	
	if(m_iTimer > 30)
	{
		SetFgColor(m_TextColor);
		return;
	}
	
	if(m_iTimer <= 0)
	{
		m_iTimer = 0;
		SetFgColor(m_FlashColor);
		return;
	}

	if(gpGlobals->curtime > m_flNextToggle)
	{
		if( m_iTimer <= 0)
		{
			m_bFlash = true;
		}
		else if( m_iTimer <= 2)
		{
			m_flToggleTime = gpGlobals->curtime;
			m_flNextToggle = gpGlobals->curtime + 0.05;
			m_bFlash = !m_bFlash;
		}
		else if( m_iTimer <= 5)
		{
			m_flToggleTime = gpGlobals->curtime;
			m_flNextToggle = gpGlobals->curtime + 0.1;
			m_bFlash = !m_bFlash;
		}
		else if( m_iTimer <= 10)
		{
			m_flToggleTime = gpGlobals->curtime;
			m_flNextToggle = gpGlobals->curtime + 0.2;
			m_bFlash = !m_bFlash;
		}
		else if( m_iTimer <= 20)
		{
			m_flToggleTime = gpGlobals->curtime;
			m_flNextToggle = gpGlobals->curtime + 0.4;
			m_bFlash = !m_bFlash;
		}
		else if( m_iTimer <= 30)
		{
			m_flToggleTime = gpGlobals->curtime;
			m_flNextToggle = gpGlobals->curtime + 0.8;
			m_bFlash = !m_bFlash;
		}
		else
			m_bFlash = false;
	}

	Color startValue, endValue;
	Color interp_color;

	if( m_bFlash )
	{
		startValue = m_FlashColor;
		endValue = m_TextColor;
	}
	else
	{
		startValue = m_TextColor;
		endValue = m_FlashColor;
	}

	float pos = (gpGlobals->curtime - m_flToggleTime) / (m_flNextToggle - m_flToggleTime);
	pos = clamp( SimpleSpline( pos ), 0, 1 );

	interp_color[0] = ((endValue[0] - startValue[0]) * pos) + startValue[0];
	interp_color[1] = ((endValue[1] - startValue[1]) * pos) + startValue[1];
	interp_color[2] = ((endValue[2] - startValue[2]) * pos) + startValue[2];
	interp_color[3] = ((endValue[3] - startValue[3]) * pos) + startValue[3];

	SetFgColor(interp_color);
}

void CHudRoundTimer::Paint()
{
	// Update the time.
	C_CSGameRules *pRules = CSGameRules();
	if ( !pRules )
		return;
		
	if(m_iTimer < 0) 
		m_iTimer = 0;
		
	int minutes = m_iTimer / 60;
	int seconds = m_iTimer % 60;

	if ( ShouldDrawText() )
	{
		//Draw Timer icon
		if ( m_pTimerIcon )
		{
			m_pTimerIcon->DrawSelf( icon_xpos, icon_ypos, icon_wide, icon_tall, GetFgColor() );
		}

		PaintTime( m_hNumberFont, digit_xpos, digit_ypos, minutes, seconds );
	}
}

void CHudRoundTimer::PaintTime(HFont font, int xpos, int ypos, int mins, int secs)
{
	surface()->DrawSetTextFont(font);
	wchar_t unicode[6];
	V_snwprintf(unicode, ARRAYSIZE(unicode), L"%d:%.2d", mins, secs);
	
	surface()->DrawSetTextPos(xpos, ypos);
	surface()->DrawUnicodeString( unicode );
}
