//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include <vgui_controls/Panel.h>
#include <vgui/ISurface.h>
#include <vgui_controls/AnimationController.h>
#include "hud_base_account.h"

using namespace vgui;

ConVar hud_account_style( "hud_account_style", "0", FCVAR_ARCHIVE, "0 = default, 1 = old", true, 0, true, 1 );

CHudBaseAccount::CHudBaseAccount( const char *pName ) :
	CHudNumericDisplay( NULL, pName ), CHudElement( pName )
{
	SetHiddenBits( HIDEHUD_PLAYERDEAD );
	SetIndent( false ); // don't indent small numbers in the drawing code - we're doing it manually

	m_iStyle = -1;
	m_iOriginalXPos = 0;
	m_iOriginalYPos = 0;
	m_iOriginalWide = 0;
	m_iOriginalTall = 0;
}


void CHudBaseAccount::LevelInit( void )
{
	m_iPreviousAccount = -1;
	m_iPreviousDelta = 0;
	m_flLastAnimationEnd = 0.0f;
	m_pszLastAnimationName = NULL;
	m_pszQueuedAnimationName = NULL;

	if ( m_iStyle == 1 )
		GetAnimationController()->StartAnimationSequence("AccountMoneyLegacyInvisible");
	else
		GetAnimationController()->StartAnimationSequence("AccountMoneyInvisible");
}

void CHudBaseAccount::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_clrRed	= pScheme->GetColor( "HudIcon_Red", Color( 255, 16, 16, 255 ) );
	m_clrGreen	= pScheme->GetColor( "HudIcon_Green", Color( 16, 255, 16, 255 ) );

	m_pAccountIcon = gHUD.GetIcon( "dollar_sign" );
	m_pPlusIcon = gHUD.GetIcon( "plus_sign" );
	m_pMinusIcon = gHUD.GetIcon( "minus_sign" );

	if( m_pAccountIcon )
	{
		icon_tall = ( GetTall() / 2 ) - YRES(2);
		float scale = icon_tall / (float)m_pAccountIcon->Height();
		icon_wide = ( scale ) * (float)m_pAccountIcon->Width();
	}

	GetBounds( m_iOriginalXPos, m_iOriginalYPos, m_iOriginalWide, m_iOriginalTall );
}


bool CHudBaseAccount::ShouldDraw()
{
	// Deriving classes must implement
	Assert( 0 );
	return false;
}


void CHudBaseAccount::Reset( void )
{
	// Round is restarting
	if ( m_flLastAnimationEnd > gpGlobals->curtime && m_pszLastAnimationName )
	{
		// if we had an animation in progress, queue it to be kicked it off again
		m_pszQueuedAnimationName = m_pszLastAnimationName;
	}
}

void CHudBaseAccount::OnThink()
{
	if ( m_iStyle != hud_account_style.GetInt() )
	{
		m_iStyle = hud_account_style.GetInt();

		if ( m_iStyle == 1 )
			SetBounds( legacy_xpos, legacy_ypos, legacy_wide, legacy_tall );
		else
			SetBounds( m_iOriginalXPos, m_iOriginalYPos, m_iOriginalWide, m_iOriginalTall );
	}
}


void CHudBaseAccount::Paint()
{
	int account  = GetPlayerAccount();

	//don't show delta on initial money give
	if( m_iPreviousAccount < 0 )
		m_iPreviousAccount = account;

	if( m_iPreviousAccount != account )
	{
		m_iPreviousDelta = account - m_iPreviousAccount;
		m_pszQueuedAnimationName = NULL;

		if( m_iPreviousDelta < 0 )
		{
			if ( m_iStyle == 1 )
				m_pszLastAnimationName = "AccountMoneyLegacyRemoved";
			else
				m_pszLastAnimationName = "AccountMoneyRemoved";
		}
		else 
		{
			if ( m_iStyle == 1 )
				m_pszLastAnimationName = "AccountMoneyLegacyAdded";
			else
				m_pszLastAnimationName = "AccountMoneyAdded";
		}
		GetAnimationController()->StartAnimationSequence( m_pszLastAnimationName );
		m_flLastAnimationEnd = gpGlobals->curtime + GetAnimationController()->GetAnimationSequenceLength( m_pszLastAnimationName );

		m_iPreviousAccount = account;
	}
	else if ( m_pszQueuedAnimationName )
	{
		GetAnimationController()->StartAnimationSequence( m_pszQueuedAnimationName );
		m_pszQueuedAnimationName = NULL;
	}

	if( m_pAccountIcon )
	{
		if ( m_iStyle == 1 )
			m_pAccountIcon->DrawSelf( legacy_icon_xpos, legacy_icon_ypos, icon_wide, icon_tall, GetFgColor() );
		else
			m_pAccountIcon->DrawSelf( icon_xpos, icon_ypos, icon_wide, icon_tall, GetFgColor() );
	}

	int xpos;
	if ( m_iStyle == 1 )
		xpos = legacy_digit_xpos - GetNumberWidth( m_hNumberFont, account );
	else
		xpos = digit_xpos - GetNumberWidth( m_hNumberFont, account );

	// draw current account
	vgui::surface()->DrawSetTextColor(GetFgColor());
	if ( m_iStyle == 1 )
		PaintNumbers( m_hNumberFont, xpos, legacy_digit_ypos, account );
	else
		PaintNumbers( m_hNumberFont, xpos, digit_ypos, account );

	//draw account additions / subtractions
	if( m_iPreviousDelta < 0 )
	{
		if( m_pMinusIcon )
		{
			if ( m_iStyle == 1 )
				m_pMinusIcon->DrawSelf( legacy_icon2_xpos, legacy_icon2_ypos, icon_wide, icon_tall, m_Ammo2Color );
			else
				m_pMinusIcon->DrawSelf( icon2_xpos, icon2_ypos, icon_wide, icon_tall, m_Ammo2Color );
		}
	}
	else
	{
		if( m_pPlusIcon )
		{
			if ( m_iStyle == 1 )
				m_pPlusIcon->DrawSelf( legacy_icon2_xpos, legacy_icon2_ypos, icon_wide, icon_tall, m_Ammo2Color );
			else
				m_pPlusIcon->DrawSelf( icon2_xpos, icon2_ypos, icon_wide, icon_tall, m_Ammo2Color );
		}
	}

	int delta = abs(m_iPreviousDelta);

	if ( m_iStyle == 1 )
		xpos = legacy_digit2_xpos - GetNumberWidth( m_hNumberFont, delta );
	else
		xpos = digit2_xpos - GetNumberWidth( m_hNumberFont, delta );

	// draw delta
	vgui::surface()->DrawSetTextColor(m_Ammo2Color);
	if ( m_iStyle == 1 )
		PaintNumbers( m_hNumberFont, xpos, legacy_digit2_ypos, delta );
	else
		PaintNumbers( m_hNumberFont, xpos, digit2_ypos, delta );
}

int CHudBaseAccount::GetNumberWidth(HFont font, int number)
{
	int width = 0;

	surface()->DrawSetTextFont(font);
	wchar_t unicode[6];
	V_snwprintf(unicode, ARRAYSIZE(unicode), L"%d", number);

	for (wchar_t *ch = unicode; *ch != 0; ch++)
	{
		width += vgui::surface()->GetCharacterWidth( font, *ch );
	}

	return width;
}