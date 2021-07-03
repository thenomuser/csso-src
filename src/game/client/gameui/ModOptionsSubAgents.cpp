//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//


#if defined( WIN32 ) && !defined( _X360 )
#include <windows.h> // SRC only!!
#endif

#include "ModOptionsSubAgents.h"
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

struct Agents
{
	const char*		m_szUIName;
	const char*		m_szImage;
};

static Agents agentsCT[] =
{
	{ "#GameUI_Loadout_Agent_None",						"ct_none"					},
	{ "#GameUI_Loadout_Agent_ctm_fbi_variantf",			"ctm_fbi_variantf"			},
	{ "#GameUI_Loadout_Agent_ctm_fbi_variantf_legacy",	"ctm_fbi_variantf_legacy"	},
	{ "#GameUI_Loadout_Agent_ctm_fbi_variantg",			"ctm_fbi_variantg"			},
	{ "#GameUI_Loadout_Agent_ctm_fbi_varianth",			"ctm_fbi_varianth"			},
	{ "#GameUI_Loadout_Agent_ctm_fbi_variantb",			"ctm_fbi_variantb"			},
	{ "#GameUI_Loadout_Agent_ctm_sas_variantf",			"ctm_sas_variantf"			},
	{ "#GameUI_Loadout_Agent_ctm_st6_variantk",			"ctm_st6_variantk"			},
	{ "#GameUI_Loadout_Agent_ctm_st6_variantk_legacy",	"ctm_st6_variantk_legacy"	},
	{ "#GameUI_Loadout_Agent_ctm_st6_variante",			"ctm_st6_variante"			},
	{ "#GameUI_Loadout_Agent_ctm_st6_variante_legacy",	"ctm_st6_variante_legacy"	},
	{ "#GameUI_Loadout_Agent_ctm_st6_variantg",			"ctm_st6_variantg"			},
	{ "#GameUI_Loadout_Agent_ctm_st6_variantm",			"ctm_st6_variantm"			},
	{ "#GameUI_Loadout_Agent_ctm_st6_variantm_legacy",	"ctm_st6_variantm_legacy"	},
	{ "#GameUI_Loadout_Agent_ctm_st6_varianti",			"ctm_st6_varianti"			},
	{ "#GameUI_Loadout_Agent_ctm_st6_varianti_legacy",	"ctm_st6_varianti_legacy"	},
	{ "#GameUI_Loadout_Agent_ctm_swat_variantj",		"ctm_swat_variantj"			},
	{ "#GameUI_Loadout_Agent_ctm_swat_varianth",		"ctm_swat_varianth"			},
	{ "#GameUI_Loadout_Agent_ctm_st6_variantj",			"ctm_st6_variantj"			},
	{ "#GameUI_Loadout_Agent_ctm_swat_variantg",		"ctm_swat_variantg"			},
	{ "#GameUI_Loadout_Agent_ctm_swat_varianti",		"ctm_swat_varianti"			},
	{ "#GameUI_Loadout_Agent_ctm_swat_variantf",		"ctm_swat_variantf"			},
	{ "#GameUI_Loadout_Agent_ctm_st6_variantl",			"ctm_st6_variantl"			},
	{ "#GameUI_Loadout_Agent_ctm_swat_variante",		"ctm_swat_variante"			},
	{ "#GameUI_Loadout_Agent_ctm_sas_old",				"ctm_sas_old"				},
	{ "#GameUI_Loadout_Agent_ctm_fbi_old",				"ctm_fbi_old"				},
	{ "#GameUI_Loadout_Agent_ctm_jumpsuit_varianta",	"ctm_jumpsuit_varianta"		},
	{ "#GameUI_Loadout_Agent_ctm_jumpsuit_variantb",	"ctm_jumpsuit_variantb"		},
	{ "#GameUI_Loadout_Agent_ctm_jumpsuit_variantc",	"ctm_jumpsuit_variantc"		},
};
static Agents agentsT[] =
{
	{ "#GameUI_Loadout_Agent_None",							"t_none"						},
	{ "#GameUI_Loadout_Agent_tm_leet_variantg",				"tm_leet_variantg"				},
	{ "#GameUI_Loadout_Agent_tm_leet_variantg_legacy",		"tm_leet_variantg_legacy"		},
	{ "#GameUI_Loadout_Agent_tm_leet_varianth",				"tm_leet_varianth"				},
	{ "#GameUI_Loadout_Agent_tm_leet_varianti",				"tm_leet_varianti"				},
	{ "#GameUI_Loadout_Agent_tm_leet_varianti_legacy",		"tm_leet_varianti_legacy"		},
	{ "#GameUI_Loadout_Agent_tm_leet_variantf",				"tm_leet_variantf"				},
	{ "#GameUI_Loadout_Agent_tm_phoenix_varianth",			"tm_phoenix_varianth"			},
	{ "#GameUI_Loadout_Agent_tm_phoenix_variantf",			"tm_phoenix_variantf"			},
	{ "#GameUI_Loadout_Agent_tm_phoenix_variantf_legacy",	"tm_phoenix_variantf_legacy"	},
	{ "#GameUI_Loadout_Agent_tm_phoenix_variantg",			"tm_phoenix_variantg"			},
	{ "#GameUI_Loadout_Agent_tm_balkan_variantf",			"tm_balkan_variantf"			},
	{ "#GameUI_Loadout_Agent_tm_balkan_varianti",			"tm_balkan_varianti"			},
	{ "#GameUI_Loadout_Agent_tm_balkan_variantg",			"tm_balkan_variantg"			},
	{ "#GameUI_Loadout_Agent_tm_balkan_variantj",			"tm_balkan_variantj"			},
	{ "#GameUI_Loadout_Agent_tm_balkan_varianth",			"tm_balkan_varianth"			},
	{ "#GameUI_Loadout_Agent_tm_balkan_variantl",			"tm_balkan_variantl"			},
	{ "#GameUI_Loadout_Agent_tm_phoenix_varianti",			"tm_phoenix_varianti"			},
	{ "#GameUI_Loadout_Agent_tm_professional_varj",			"tm_professional_varj"			},
	{ "#GameUI_Loadout_Agent_tm_professional_varh",			"tm_professional_varh"			},
	{ "#GameUI_Loadout_Agent_tm_balkan_variantk",			"tm_balkan_variantk"			},
	{ "#GameUI_Loadout_Agent_tm_professional_varg",			"tm_professional_varg"			},
	{ "#GameUI_Loadout_Agent_tm_professional_vari",			"tm_professional_vari"			},
	{ "#GameUI_Loadout_Agent_tm_professional_varf",			"tm_professional_varf"			},
	{ "#GameUI_Loadout_Agent_tm_professional_varf1",		"tm_professional_varf1"			},
	{ "#GameUI_Loadout_Agent_tm_professional_varf2",		"tm_professional_varf2"			},
	{ "#GameUI_Loadout_Agent_tm_professional_varf3",		"tm_professional_varf3"			},
	{ "#GameUI_Loadout_Agent_tm_professional_varf4",		"tm_professional_varf4"			},
	{ "#GameUI_Loadout_Agent_tm_leet_old",					"tm_leet_old"					},
	{ "#GameUI_Loadout_Agent_tm_phoenix_old",				"tm_phoenix_old"				},
	{ "#GameUI_Loadout_Agent_tm_jumpsuit_varianta",			"tm_jumpsuit_varianta"			},
	{ "#GameUI_Loadout_Agent_tm_jumpsuit_variantb",			"tm_jumpsuit_variantb"			},
	{ "#GameUI_Loadout_Agent_tm_jumpsuit_variantc",			"tm_jumpsuit_variantc"			},
};

//-----------------------------------------------------------------------------
// Purpose: Basic help dialog
//-----------------------------------------------------------------------------
CModOptionsSubAgents::CModOptionsSubAgents(vgui::Panel *parent) : vgui::PropertyPage(parent, "ModOptionsSubAgents") 
{
	Button *cancel = new Button( this, "Cancel", "#GameUI_Cancel" );
	cancel->SetCommand( "Close" );

	Button *ok = new Button( this, "OK", "#GameUI_OK" );
	ok->SetCommand( "Ok" );

	Button *apply = new Button( this, "Apply", "#GameUI_Apply" );
	apply->SetCommand( "Apply" );

	//=========

	m_pLoadoutAgentCTComboBox = new CLabeledCommandComboBox( this, "AgentCTComboBox" );
	m_pLoadoutAgentTComboBox = new CLabeledCommandComboBox( this, "AgentTComboBox" );

	m_pAgentImageCT = new CBitmapImagePanel( this, "AgentImageCT", NULL );
	m_pAgentImageCT->AddActionSignalTarget( this );
	m_pAgentImageT = new CBitmapImagePanel( this, "AgentImageT", NULL );
	m_pAgentImageT->AddActionSignalTarget( this );

	char command[64];
	int i;
	for ( i = 0; i < ARRAYSIZE( agentsCT ); i++ )
	{
		Q_snprintf( command, sizeof( command ), "loadout_slot_agent_ct %d", i );
		m_pLoadoutAgentCTComboBox->AddItem( agentsCT[i].m_szUIName, command );
	}
	for ( i = 0; i < ARRAYSIZE( agentsT ); i++ )
	{
		Q_snprintf( command, sizeof( command ), "loadout_slot_agent_t %d", i );
		m_pLoadoutAgentTComboBox->AddItem( agentsT[i].m_szUIName, command );
	}
	m_pLoadoutAgentCTComboBox->AddActionSignalTarget( this );
	m_pLoadoutAgentTComboBox->AddActionSignalTarget( this );

	LoadControlSettings("Resource/ModOptionsSubAgents.res");
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CModOptionsSubAgents::~CModOptionsSubAgents()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubAgents::RemapAgentsImage()
{
	const char *pImageNameCT = agentsCT[m_pLoadoutAgentCTComboBox->GetActiveItem()].m_szImage;
	const char *pImageNameT = agentsT[m_pLoadoutAgentTComboBox->GetActiveItem()].m_szImage;

	char texture[256];
	if ( pImageNameCT != NULL )
	{
		Q_snprintf( texture, sizeof( texture ), "vgui/agents/%s", pImageNameCT );
		m_pAgentImageCT->setTexture( texture );
	}
	else
	{
		m_pAgentImageCT->setTexture( "vgui/agents/ct_none" );
	}

	if ( pImageNameT != NULL )
	{
		Q_snprintf( texture, sizeof( texture ), "vgui/agents/%s", pImageNameT );
		m_pAgentImageT->setTexture( texture );
	}
	else
	{
		m_pAgentImageT->setTexture( "vgui/agents/t_none" );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubAgents::OnControlModified()
{
	PostMessage(GetParent(), new KeyValues("ApplyButtonEnable"));
	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubAgents::OnTextChanged( vgui::Panel *panel )
{
	RemapAgentsImage();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubAgents::OnResetData()
{
	ConVarRef loadout_slot_agent_ct( "loadout_slot_agent_ct" );
	if ( loadout_slot_agent_ct.IsValid() )
		m_pLoadoutAgentCTComboBox->SetInitialItem( loadout_slot_agent_ct.GetInt() );

	ConVarRef loadout_slot_agent_t( "loadout_slot_agent_t" );
	if ( loadout_slot_agent_t.IsValid() )
		m_pLoadoutAgentTComboBox->SetInitialItem( loadout_slot_agent_t.GetInt() );

	RemapAgentsImage();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubAgents::OnApplyChanges()
{
	m_pLoadoutAgentCTComboBox->ApplyChanges();
	m_pLoadoutAgentTComboBox->ApplyChanges();
}
