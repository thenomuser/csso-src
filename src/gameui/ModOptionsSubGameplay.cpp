//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//


#if defined( WIN32 ) && !defined( _X360 )
#include <windows.h> // SRC only!!
#endif

#include "ModOptionsSubGameplay.h"
#include <stdio.h>

#include <vgui_controls/Button.h>
#include "tier1/KeyValues.h"
#include <vgui_controls/Label.h>
#include <vgui/ISystem.h>
#include <vgui/ISurface.h>
#include <vgui_controls/ComboBox.h>

#include "CvarTextEntry.h"
#include "CvarToggleCheckButton.h"
#include "cvarslider.h"
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
CModOptionsSubGameplay::CModOptionsSubGameplay( vgui::Panel *parent ): vgui::PropertyPage( parent, "ModOptionsSubGameplay" )
{
	Button *cancel = new Button( this, "Cancel", "#GameUI_Cancel" );
	cancel->SetCommand( "Close" );

	Button *ok = new Button( this, "OK", "#GameUI_OK" );
	ok->SetCommand( "Ok" );

	Button *apply = new Button( this, "Apply", "#GameUI_Apply" );
	apply->SetCommand( "Apply" );

	//=========

	m_pCloseOnBuy = new CCvarToggleCheckButton( this, "CloseOnBuyCheckbox", "#GameUI_Gameplay_CloseOnBuy", "closeonbuy" );
	m_pUseOpensBuyMenu = new CCvarToggleCheckButton( this, "UseOpensBuyMenuCheckbox", "#GameUI_Gameplay_UseOpensBuyMenu", "cl_use_opens_buy_menu" );
	m_pAddBotPrefix = new CCvarToggleCheckButton( this, "AddBotPrefix", "#GameUI_Gameplay_AddBotPrefix", "cl_add_bot_prefix" );
	m_pViewmodelOffsetX = new CCvarSlider( this, "ViewmodelOffsetXSlider", "", -2.0f, 2.5f, "viewmodel_offset_x" );
	m_pViewmodelOffsetXLabel = new Label( this, "ViewmodelOffsetXLabel", "" );
	m_pViewmodelOffsetY = new CCvarSlider( this, "ViewmodelOffsetYSlider", "", -2.0f, 2.0f, "viewmodel_offset_y" );
	m_pViewmodelOffsetYLabel = new Label( this, "ViewmodelOffsetYLabel", "" );
	m_pViewmodelOffsetZ = new CCvarSlider( this, "ViewmodelOffsetZSlider", "", -2.0f, 2.0f, "viewmodel_offset_z" );
	m_pViewmodelOffsetZLabel = new Label( this, "ViewmodelOffsetZLabel", "" );
	m_pViewmodelOffsetPreset = new CLabeledCommandComboBox( this, "ViewmodelOffsetPreset" );
	m_pViewmodelFOV = new CCvarSlider( this, "ViewmodelFOVSlider", "", 54.0f, 68.0f, "viewmodel_fov" );
	m_pViewmodelFOVLabel = new Label( this, "ViewmodelFOVLabel", "" );
	m_pViewbobStyle = new CLabeledCommandComboBox( this, "ViewbobStyleComboBox" );
	m_pWeaponPos = new CLabeledCommandComboBox( this, "WeaponPositionComboBox" );

	m_pViewmodelOffsetPreset->AddItem( "#GameUI_Gameplay_Viewmodel_Preset_1", "viewmodel_presetpos 1" );
	m_pViewmodelOffsetPreset->AddItem( "#GameUI_Gameplay_Viewmodel_Preset_2", "viewmodel_presetpos 2" );
	m_pViewmodelOffsetPreset->AddItem( "#GameUI_Gameplay_Viewmodel_Preset_3", "viewmodel_presetpos 3" );

	m_pViewbobStyle->AddItem( "#GameUI_Gameplay_Viewbob_CSS", "cl_use_new_headbob 0" );
	m_pViewbobStyle->AddItem( "#GameUI_Gameplay_Viewbob_CSGO", "cl_use_new_headbob 1" );

	m_pWeaponPos->AddItem( "#GameUI_Gameplay_Hand_Left", "cl_righthand 0" );
	m_pWeaponPos->AddItem( "#GameUI_Gameplay_Hand_Right", "cl_righthand 1" );

	m_pCloseOnBuy->AddActionSignalTarget( this );
	m_pUseOpensBuyMenu->AddActionSignalTarget( this );
	m_pAddBotPrefix->AddActionSignalTarget( this );
	m_pViewmodelOffsetX->AddActionSignalTarget( this );
	m_pViewmodelOffsetY->AddActionSignalTarget( this );
	m_pViewmodelOffsetZ->AddActionSignalTarget( this );
	m_pViewmodelFOV->AddActionSignalTarget( this );
	m_pViewmodelOffsetPreset->AddActionSignalTarget( this );
	m_pViewbobStyle->AddActionSignalTarget( this );
	m_pWeaponPos->AddActionSignalTarget( this );

	LoadControlSettings( "Resource/ModOptionsSubGameplay.res" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CModOptionsSubGameplay::~CModOptionsSubGameplay()
{
}

void CModOptionsSubGameplay::UpdateViewmodelSliderLabels()
{
	char strValue[8];
	Q_snprintf( strValue, sizeof( strValue ), "%2.1f", m_pViewmodelOffsetX->GetSliderValue() );
	m_pViewmodelOffsetXLabel->SetText( strValue );
	Q_snprintf( strValue, sizeof( strValue ), "%2.1f", m_pViewmodelOffsetY->GetSliderValue() );
	m_pViewmodelOffsetYLabel->SetText( strValue );
	Q_snprintf( strValue, sizeof( strValue ), "%2.1f", m_pViewmodelOffsetZ->GetSliderValue() );
	m_pViewmodelOffsetZLabel->SetText( strValue );
	Q_snprintf( strValue, sizeof( strValue ), "%2.1f", m_pViewmodelFOV->GetSliderValue() );
	m_pViewmodelFOVLabel->SetText( strValue );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubGameplay::OnControlModified()
{
	PostMessage( GetParent(), new KeyValues( "ApplyButtonEnable" ) );
	InvalidateLayout();
}

void CModOptionsSubGameplay::OnTextChanged( vgui::Panel *panel )
{
	if ( panel == m_pViewmodelOffsetPreset )
	{
		if ( m_pViewmodelOffsetPreset->GetActiveItem() == 0 )
		{
			m_pViewmodelOffsetX->SetSliderValue(1);
			m_pViewmodelOffsetY->SetSliderValue(1);
			m_pViewmodelOffsetZ->SetSliderValue(-1);
			m_pViewmodelFOV->SetSliderValue(60);
			UpdateViewmodelSliderLabels();
		}
		if ( m_pViewmodelOffsetPreset->GetActiveItem() == 1 )
		{
			m_pViewmodelOffsetX->SetSliderValue(0);
			m_pViewmodelOffsetY->SetSliderValue(0);
			m_pViewmodelOffsetZ->SetSliderValue(0);
			m_pViewmodelFOV->SetSliderValue(54);
			UpdateViewmodelSliderLabels();
		}
		if ( m_pViewmodelOffsetPreset->GetActiveItem() == 2 )
		{
			m_pViewmodelOffsetX->SetSliderValue(2.5f);
			m_pViewmodelOffsetY->SetSliderValue(0);
			m_pViewmodelOffsetZ->SetSliderValue(-1.5f);
			m_pViewmodelFOV->SetSliderValue(68);
			UpdateViewmodelSliderLabels();
		}
	}
}

void CModOptionsSubGameplay::OnSliderMoved( KeyValues *data )
{
	vgui::Panel* pPanel = static_cast<vgui::Panel*>(data->GetPtr( "panel" ));

	if ( pPanel == m_pViewmodelOffsetX || pPanel == m_pViewmodelOffsetY || pPanel == m_pViewmodelOffsetZ || pPanel == m_pViewmodelFOV )
	{
		UpdateViewmodelSliderLabels();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubGameplay::OnResetData()
{
	m_pCloseOnBuy->Reset();
	m_pUseOpensBuyMenu->Reset();
	m_pAddBotPrefix->Reset();
	m_pViewmodelOffsetX->Reset();
	m_pViewmodelOffsetY->Reset();
	m_pViewmodelOffsetZ->Reset();
	m_pViewmodelFOV->Reset();
	UpdateViewmodelSliderLabels();
	
	ConVarRef viewmodel_presetpos( "viewmodel_presetpos" );
	if ( viewmodel_presetpos.IsValid() )
		m_pViewmodelOffsetPreset->SetInitialItem( viewmodel_presetpos.GetInt() - 1 );
	ConVarRef cl_use_new_headbob( "cl_use_new_headbob" );
	if ( cl_use_new_headbob.IsValid() )
		m_pViewbobStyle->SetInitialItem( cl_use_new_headbob.GetInt() );
	ConVarRef cl_righthand( "cl_righthand" );
	if ( cl_righthand.IsValid() )
		m_pWeaponPos->SetInitialItem( cl_righthand.GetInt() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubGameplay::OnApplyChanges()
{
	m_pCloseOnBuy->ApplyChanges();
	m_pUseOpensBuyMenu->ApplyChanges();
	m_pAddBotPrefix->ApplyChanges();
	m_pViewmodelOffsetPreset->ApplyChanges();
	m_pViewmodelOffsetX->ApplyChanges();
	m_pViewmodelOffsetY->ApplyChanges();
	m_pViewmodelOffsetZ->ApplyChanges();
	m_pViewmodelFOV->ApplyChanges();
	m_pViewbobStyle->ApplyChanges();
	m_pWeaponPos->ApplyChanges();
}
