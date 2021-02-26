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
#include "vgui_controls/QueryBox.h"

#include "CvarTextEntry.h"
#include "CvarToggleCheckButton.h"
#include "cvarslider.h"
#include "LabeledCommandComboBox.h"
#include "EngineInterface.h"
#include "tier1/convar.h"

#include "GameUI_Interface.h"

#if defined( _X360 )
#include "xbox/xbox_win32stubs.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

const char* szMusicStrings[] =
{
	"valve_csgo_01", // the default one should be on top
	"amontobin_01",
	"austinwintory_01",
	"austinwintory_02",
	"awolnation_01",
	"beartooth_01",
	"beartooth_02",
	"blitzkids_01",
	"damjanmravunac_01",
	"danielsadowski_01",
	"danielsadowski_02",
	"danielsadowski_03",
	"danielsadowski_04",
	"darude_01",
	"dren_01",
	"dren_02",
	"feedme_01",
	"halflife_alyx_01",
	"halo_01",
	"hotlinemiami_01",
	"hundredth_01",
	"ianhultquist_01",
	"kellybailey_01",
	"kitheory_01",
	"lenniemoore_01",
	"mateomessina_01",
	"mattlange_01",
	"mattlevine_01",
	"michaelbross_01",
	"midnightriders_01",
	"mordfustang_01",
	"neckdeep_01",
	"newbeatfund_01",
	"noisia_01",
	"proxy_01",
	"roam_01",
	"robertallaire_01",
	"sammarshall_01",
	"sasha_01",
	"scarlxrd_01",
	"seanmurray_01",
	"skog_01",
	"skog_02",
	"skog_03",
	"theverkkars_01",
	"timhuling_01",
	"treeadams_benbromfield_01",
	"troelsfolmann_01",
	"twinatlantic_01"
};

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
	m_pViewmodelRecoil = new CCvarSlider( this, "ViewmodelRecoilSlider", "", 0.0f, 1.0f, "viewmodel_recoil" );
	m_pViewmodelRecoilLabel = new Label( this, "ViewmodelRecoilLabel", "" );
	m_pViewbobStyle = new CLabeledCommandComboBox( this, "ViewbobStyleComboBox" );
	m_pWeaponPos = new CLabeledCommandComboBox( this, "WeaponPositionComboBox" );
	m_pMusicSelection = new CLabeledCommandComboBox( this, "MusicSelectionComboBox" );

	m_pViewmodelOffsetPreset->AddItem( "#GameUI_Gameplay_Viewmodel_Preset_1", "viewmodel_presetpos 1" );
	m_pViewmodelOffsetPreset->AddItem( "#GameUI_Gameplay_Viewmodel_Preset_2", "viewmodel_presetpos 2" );
	m_pViewmodelOffsetPreset->AddItem( "#GameUI_Gameplay_Viewmodel_Preset_3", "viewmodel_presetpos 3" );

	m_pViewbobStyle->AddItem( "#GameUI_Gameplay_Viewbob_CSS", "cl_use_new_headbob 0" );
	m_pViewbobStyle->AddItem( "#GameUI_Gameplay_Viewbob_CSGO", "cl_use_new_headbob 1" );

	m_pWeaponPos->AddItem( "#GameUI_Gameplay_Hand_Left", "cl_righthand 0" );
	m_pWeaponPos->AddItem( "#GameUI_Gameplay_Hand_Right", "cl_righthand 1" );

	for ( int i = 0; i < ARRAYSIZE( szMusicStrings ); i++ )
	{
		char command[128];
		char string[128];
		Q_snprintf( command, sizeof( command ), "snd_music_selection %s", szMusicStrings[i] );
		Q_snprintf( string, sizeof( string ), "#GameUI_Gameplay_MusicKit_%d", i );
		m_pMusicSelection->AddItem( string, command );
	}

	m_pCloseOnBuy->AddActionSignalTarget( this );
	m_pUseOpensBuyMenu->AddActionSignalTarget( this );
	m_pAddBotPrefix->AddActionSignalTarget( this );
	m_pViewmodelOffsetX->AddActionSignalTarget( this );
	m_pViewmodelOffsetY->AddActionSignalTarget( this );
	m_pViewmodelOffsetZ->AddActionSignalTarget( this );
	m_pViewmodelFOV->AddActionSignalTarget( this );
	m_pViewmodelRecoil->AddActionSignalTarget( this );
	m_pViewmodelOffsetPreset->AddActionSignalTarget( this );
	m_pViewbobStyle->AddActionSignalTarget( this );
	m_pWeaponPos->AddActionSignalTarget( this );
	m_pMusicSelection->AddActionSignalTarget( this );

	LoadControlSettings( "Resource/ModOptionsSubGameplay.res" );

#if !INSTANT_MUSIC_CHANGE
	m_bNeedToWarnAboutMusic = true;
#endif
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
	Q_snprintf( strValue, sizeof( strValue ), "%2.1f", m_pViewmodelRecoil->GetSliderValue() );
	m_pViewmodelRecoilLabel->SetText( strValue );
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

	if ( pPanel == m_pViewmodelOffsetX || pPanel == m_pViewmodelOffsetY || pPanel == m_pViewmodelOffsetZ || pPanel == m_pViewmodelFOV || pPanel == m_pViewmodelRecoil )
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
	m_pViewmodelRecoil->Reset();
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

	ConVarRef snd_music_selection( "snd_music_selection" );
	const char *pMusicName = snd_music_selection.GetString();
	for ( int i = 0; i < ARRAYSIZE( szMusicStrings ); i++ )
	{
		if ( !Q_strcmp( pMusicName, szMusicStrings[i] ) )
		{
			m_pMusicSelection->SetInitialItem( i );
			break;
		}
	}
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
	m_pViewmodelRecoil->ApplyChanges();
	m_pViewbobStyle->ApplyChanges();
	m_pWeaponPos->ApplyChanges();
	m_pMusicSelection->ApplyChanges();

	ConVarRef snd_music_selection( "snd_music_selection" );
#if INSTANT_MUSIC_CHANGE
	if ( Q_strcmp( snd_music_selection.GetString(), szMusicStrings[m_pMusicSelection->GetActiveItem()] ) )
#else
	if ( m_bNeedToWarnAboutMusic && Q_strcmp( snd_music_selection.GetString(), szMusicStrings[m_pMusicSelection->GetActiveItem()] ) )
#endif
	{
		// Bring up the confirmation dialog
#if INSTANT_MUSIC_CHANGE
		m_pMusicSelection->ApplyChanges();
		GameUI().ReleaseBackgroundMusic();
#else
		MessageBox *box = new MessageBox( "#GameUI_OptionsRestartRequired_Title", "#GameUI_Gameplay_MusicRestartHint", this );
		box->MoveToFront();
		box->DoModal();
		m_bNeedToWarnAboutMusic = false;
#endif
	}
}
