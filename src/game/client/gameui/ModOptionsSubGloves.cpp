//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//


#if defined( WIN32 ) && !defined( _X360 )
#include <windows.h> // SRC only!!
#endif

#include "ModOptionsSubGloves.h"
#include <stdio.h>

#include <vgui_controls/Button.h>
#include "tier1/KeyValues.h"

#include "LabeledCommandComboBox.h"
#include "tier1/convar.h"
#include "BitmapImagePanel.h"

#if defined( _X360 )
#include "xbox/xbox_win32stubs.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

struct Gloves
{
	const char*		m_szUIName;
	const char*		m_szImage;
};

static Gloves gloveNames[] =
{
	{ "#GameUI_Loadout_Glove_Default",					NULL							},
	{ "#GameUI_Loadout_Glove_Bloodhound",				"glove_bloodhound"				},
	{ "#GameUI_Loadout_Glove_Bloodhound_BrokenFang",	"glove_bloodhound_brokenfang"	},
	{ "#GameUI_Loadout_Glove_Bloodhound_Hydra",			"glove_bloodhound_hydra"		},
	{ "#GameUI_Loadout_Glove_Handwrap_Leathery",		"glove_handwrap_leathery"		},
	{ "#GameUI_Loadout_Glove_Motorcycle",				"glove_motorcycle"				},
	{ "#GameUI_Loadout_Glove_Slick",					"glove_slick"					},
	{ "#GameUI_Loadout_Glove_Specialist",				"glove_specialist"				},
	{ "#GameUI_Loadout_Glove_Sporty",					"glove_sporty"					},
};

//-----------------------------------------------------------------------------
// Purpose: Basic help dialog
//-----------------------------------------------------------------------------
CModOptionsSubGloves::CModOptionsSubGloves(vgui::Panel *parent) : vgui::PropertyPage(parent, "ModOptionsSubGloves") 
{
	Button *cancel = new Button( this, "Cancel", "#GameUI_Cancel" );
	cancel->SetCommand( "Close" );

	Button *ok = new Button( this, "OK", "#GameUI_OK" );
	ok->SetCommand( "Ok" );

	Button *apply = new Button( this, "Apply", "#GameUI_Apply" );
	apply->SetCommand( "Apply" );

	//=========

	m_pLoadoutGloveCTComboBox = new CLabeledCommandComboBox( this, "GloveCTComboBox" );
	m_pLoadoutGloveTComboBox = new CLabeledCommandComboBox( this, "GloveTComboBox" );

	int i;
	char command[64];
	for ( i = 0; i < ARRAYSIZE( gloveNames ); i++ )
	{
		Q_snprintf( command, sizeof( command ), "loadout_slot_gloves_ct %d", i );
		m_pLoadoutGloveCTComboBox->AddItem( gloveNames[i].m_szUIName, command );
		Q_snprintf( command, sizeof( command ), "loadout_slot_gloves_t %d", i );
		m_pLoadoutGloveTComboBox->AddItem( gloveNames[i].m_szUIName, command );
	}

	m_pGloveImageCT = new CBitmapImagePanel( this, "GloveImageCT", NULL );
	m_pGloveImageCT->AddActionSignalTarget( this );
	m_pGloveImageT = new CBitmapImagePanel( this, "GloveImageT", NULL );
	m_pGloveImageT->AddActionSignalTarget( this );

	m_pLoadoutGloveCTComboBox->AddActionSignalTarget( this );
	m_pLoadoutGloveTComboBox->AddActionSignalTarget( this );

	LoadControlSettings("Resource/ModOptionsSubGloves.res");
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CModOptionsSubGloves::~CModOptionsSubGloves()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubGloves::RemapGlovesImage()
{
	const char *pImageNameCT = gloveNames[m_pLoadoutGloveCTComboBox->GetActiveItem()].m_szImage;
	const char *pImageNameT = gloveNames[m_pLoadoutGloveTComboBox->GetActiveItem()].m_szImage;

	char texture[256];
	if ( pImageNameCT != NULL )
	{
		Q_snprintf( texture, sizeof( texture ), "vgui/gloves/%s", pImageNameCT );
		m_pGloveImageCT->setTexture( texture );
	}
	else
	{
		m_pGloveImageCT->setTexture( "vgui/gloves/ct_none" );
	}

	if ( pImageNameT != NULL )
	{
		Q_snprintf( texture, sizeof( texture ), "vgui/gloves/%s", pImageNameT );
		m_pGloveImageT->setTexture( texture );
	}
	else
	{
		m_pGloveImageT->setTexture( "vgui/gloves/t_none" );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubGloves::OnControlModified()
{
	PostMessage(GetParent(), new KeyValues("ApplyButtonEnable"));
	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubGloves::OnTextChanged( vgui::Panel *panel )
{
	RemapGlovesImage();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubGloves::OnResetData()
{
	ConVarRef loadout_slot_gloves_ct( "loadout_slot_gloves_ct" );
	if ( loadout_slot_gloves_ct.IsValid() )
		m_pLoadoutGloveCTComboBox->SetInitialItem( loadout_slot_gloves_ct.GetInt() );

	ConVarRef loadout_slot_gloves_t( "loadout_slot_gloves_t" );
	if ( loadout_slot_gloves_t.IsValid() )
		m_pLoadoutGloveTComboBox->SetInitialItem( loadout_slot_gloves_t.GetInt() );

	RemapGlovesImage();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubGloves::OnApplyChanges()
{
	m_pLoadoutGloveCTComboBox->ApplyChanges();
	m_pLoadoutGloveTComboBox->ApplyChanges();
}
