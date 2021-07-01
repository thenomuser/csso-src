//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//


#if defined( WIN32 ) && !defined( _X360 )
#include <windows.h> // SRC only!!
#endif

#include "ModOptionsSubHUD.h"
#include <stdio.h>

#include <vgui/ISystem.h>
#include <vgui/ISurface.h>

#include "LabeledCommandComboBox.h"
#include "EngineInterface.h"
#include "tier1/convar.h"

#if defined( _X360 )
#include "xbox/xbox_win32stubs.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Basic help dialog
//-----------------------------------------------------------------------------
CModOptionsSubHUD::CModOptionsSubHUD( vgui::Panel *parent ): vgui::PropertyPage( parent, "ModOptionsSubHUD" )
{
	Button *cancel = new Button( this, "Cancel", "#GameUI_Cancel" );
	cancel->SetCommand( "Close" );

	Button *ok = new Button( this, "OK", "#GameUI_OK" );
	ok->SetCommand( "Ok" );

	Button *apply = new Button( this, "Apply", "#GameUI_Apply" );
	apply->SetCommand( "Apply" );

	//=========
	m_pPlayerCountPos = new CLabeledCommandComboBox( this, "PlayerCountPositionComboBox" );
	m_pHealthArmorStyle = new CLabeledCommandComboBox( this, "HealthArmorStyleComboBox" );
	m_pAccountStyle = new CLabeledCommandComboBox( this, "AccountStyleComboBox" );

	m_pPlayerCountPos->AddItem( "#GameUI_HUD_PlayerCount_Bottom", "hud_playercount_pos 0" );
	m_pPlayerCountPos->AddItem( "#GameUI_HUD_PlayerCount_Top", "hud_playercount_pos 1" );

	m_pHealthArmorStyle->AddItem( "#GameUI_HUD_HealthArmorStyle_0", "hud_healtharmor_style 0" );
	m_pHealthArmorStyle->AddItem( "#GameUI_HUD_HealthArmorStyle_1", "hud_healtharmor_style 1" );
	m_pHealthArmorStyle->AddItem( "#GameUI_HUD_HealthArmorStyle_2", "hud_healtharmor_style 2" );

	m_pAccountStyle->AddItem( "#GameUI_HUD_AccountStyle_0", "hud_account_style 0" );
	m_pAccountStyle->AddItem( "#GameUI_HUD_AccountStyle_1", "hud_account_style 1" );

	m_pPlayerCountPos->AddActionSignalTarget( this );
	m_pHealthArmorStyle->AddActionSignalTarget( this );
	m_pAccountStyle->AddActionSignalTarget( this );

	LoadControlSettings( "Resource/ModOptionsSubHUD.res" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CModOptionsSubHUD::~CModOptionsSubHUD()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubHUD::OnControlModified()
{
	PostMessage( GetParent(), new KeyValues( "ApplyButtonEnable" ) );
	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubHUD::OnResetData()
{
	ConVarRef hud_playercount_pos( "hud_playercount_pos" );
	if ( hud_playercount_pos.IsValid() )
		m_pPlayerCountPos->SetInitialItem( hud_playercount_pos.GetInt() );

	ConVarRef hud_healtharmor_style( "hud_healtharmor_style" );
	if ( hud_healtharmor_style.IsValid() )
		m_pHealthArmorStyle->SetInitialItem( hud_healtharmor_style.GetInt() );

	ConVarRef hud_account_style( "hud_account_style" );
	if ( hud_account_style.IsValid() )
		m_pAccountStyle->SetInitialItem( hud_account_style.GetInt() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubHUD::OnApplyChanges()
{
	m_pPlayerCountPos->ApplyChanges();
	m_pHealthArmorStyle->ApplyChanges();
	m_pAccountStyle->ApplyChanges();
}
