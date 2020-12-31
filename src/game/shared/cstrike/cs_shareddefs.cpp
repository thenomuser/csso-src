//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "cs_shareddefs.h"

const float CS_PLAYER_SPEED_RUN			= 260.0f;
const float CS_PLAYER_SPEED_VIP			= 227.0f;
//const float CS_PLAYER_SPEED_WALK		= 100.0f;
const float CS_PLAYER_SPEED_SHIELD		= 160.0f;
const float CS_PLAYER_SPEED_STOPPED		=   1.0f;
const float CS_PLAYER_SPEED_HAS_HOSTAGE	= 200.0f;
const float CS_PLAYER_SPEED_OBSERVER	= 900.0f;

const float CS_PLAYER_SPEED_DUCK_MODIFIER	= 0.34f;
const float CS_PLAYER_SPEED_WALK_MODIFIER	= 0.52f;
const float CS_PLAYER_SPEED_CLIMB_MODIFIER	= 0.34f;

const float CS_PLAYER_DUCK_SPEED_IDEAL = 8.0f;


CCSClassInfo g_ClassInfos[] =
{
	{ "None", "", "" },

	{ "Phoenix",		"Phoenix",		"models/weapons/arms/v_glove_fullfinger.mdl" },
	{ "Leet Crew",		"Leet",			"models/weapons/arms/v_glove_fingerless.mdl" },
	{ "Separatist",		"Separatist",	"models/weapons/arms/t_arms_separatist.mdl" },
	{ "Balkan",			"Balkan",		"models/weapons/arms/t_arms_balkan.mdl" },
	{ "Professional",	"Professional",	"models/weapons/arms/t_arms_professional.mdl" },
	{ "Anarchist",		"Anarchist",	"models/weapons/arms/t_arms_anarchist.mdl" },
	{ "Pirate",			"Pirate",		"models/weapons/arms/t_arms_pirate.mdl" },

	{ "Seal Team 6",	"ST6",			"models/weapons/arms/ct_arms_st6.mdl" },
	{ "GSG-9",			"GSG9",			"models/weapons/arms/ct_arms_gsg9.mdl" },
	{ "SAS",			"SAS",			"models/weapons/arms/ct_arms_sas.mdl" },
	{ "GIGN",			"GIGN",			"models/weapons/arms/ct_arms_gign.mdl" },
	{ "FBI",			"FBI",			"models/weapons/arms/ct_arms_fbi.mdl" },
	{ "IDF",			"IDF",			"models/weapons/arms/ct_arms_idf.mdl" },
	{ "SWAT",			"SWAT",			"models/weapons/arms/ct_arms_swat.mdl" }
};

const CCSClassInfo* GetCSClassInfo( int i )
{
	Assert( i >= 0 && i < ARRAYSIZE( g_ClassInfos ) );
	return &g_ClassInfos[i];
}

CCSAgentInfo g_AgentInfosCT[MAX_AGENTS_CT + 1] =
{
	{ "", "", 0, 0 },
	// Shattered Web
	{ "models/player/custom_player/legacy/ctm_fbi_variantf.mdl",	"models/weapons/arms/ct_arms_fbi.mdl",			"FBI",			CS_CLASS_FBI,			3,	false	},
	{ "models/player/custom_player/legacy/ctm_fbi_variantg.mdl",	"models/weapons/arms/ct_arms_fbi.mdl",			"FBI",			CS_CLASS_FBI,			4,	false	},
	{ "models/player/custom_player/legacy/ctm_fbi_varianth.mdl",	"models/weapons/arms/ct_arms_fbi.mdl",			"FBI",			CS_CLASS_FBI,			2,	false	},
	{ "models/player/custom_player/legacy/ctm_fbi_variantb.mdl",	"models/weapons/arms/ct_arms_fbi.mdl",			"FBI_Epic",		CS_CLASS_FBI,			1,	true	},
	{ "models/player/custom_player/legacy/ctm_sas_variantf.mdl",	"models/weapons/arms/ct_arms_sas.mdl",			"SAS",			CS_CLASS_SAS,			1,	false	},
	{ "models/player/custom_player/legacy/ctm_st6_variantk.mdl",	"models/weapons/arms/ct_arms_st6_v2.mdl",		"GSG9",			CS_CLASS_SEAL_TEAM_6,	0,	false	},
	{ "models/player/custom_player/legacy/ctm_st6_variante.mdl",	"models/weapons/arms/ct_arms_st6_v2.mdl",		"ST6",			CS_CLASS_SEAL_TEAM_6,	1,	false	},
	{ "models/player/custom_player/legacy/ctm_st6_variantg.mdl",	"models/weapons/arms/ct_arms_st6_v2.mdl",		"ST6",			CS_CLASS_SEAL_TEAM_6,	2,	false	},
	{ "models/player/custom_player/legacy/ctm_st6_variantm.mdl",	"models/weapons/arms/ct_arms_st6_v2.mdl",		"ST6",			CS_CLASS_SEAL_TEAM_6,	3,	false	},
	{ "models/player/custom_player/legacy/ctm_st6_varianti.mdl",	"models/weapons/arms/v_glove_hardknuckle.mdl",	"ST6_Epic",		CS_CLASS_SEAL_TEAM_6,	0,	false	},
	// Broken Fang
	{ "models/player/custom_player/legacy/ctm_swat_variantj.mdl",	"models/weapons/arms/ct_arms_swat_leader.mdl",	"SWAT",			CS_CLASS_SWAT,			2,	false	},
	{ "models/player/custom_player/legacy/ctm_swat_varianth.mdl",	"models/weapons/arms/ct_arms_swat_leader.mdl",	"SWAT",			CS_CLASS_SWAT,			3,	false	},
	{ "models/player/custom_player/legacy/ctm_st6_variantj.mdl",	"models/weapons/arms/ct_arms_st6_v2.mdl",		"ST6",			CS_CLASS_SEAL_TEAM_6,	4,	false	},
	{ "models/player/custom_player/legacy/ctm_swat_variantg.mdl",	"models/weapons/arms/ct_arms_swat_leader.mdl",	"SWAT",			CS_CLASS_SWAT,			5,	false	},
	{ "models/player/custom_player/legacy/ctm_swat_varianti.mdl",	"models/weapons/arms/ct_arms_swat_leader.mdl",	"SWAT",			CS_CLASS_SWAT,			4,	false	},
	{ "models/player/custom_player/legacy/ctm_swat_variantf.mdl",	"models/weapons/arms/ct_arms_swat_leader.mdl",	"SWAT_Fem",		CS_CLASS_SWAT,			1,	true	},
	{ "models/player/custom_player/legacy/ctm_st6_variantl.mdl",	"models/weapons/arms/ct_arms_st6_v2.mdl",		"ST6",			CS_CLASS_SEAL_TEAM_6,	5,	false	},
	{ "models/player/custom_player/legacy/ctm_swat_variante.mdl",	"models/weapons/arms/ct_arms_swat_leader.mdl",	"SWAT_Epic",	CS_CLASS_SWAT,			0,	true	},
	// what?
	{ "models/player/ctm_sas_old.mdl",								"models/weapons/arms/ct_arms_sas_old.mdl",		"SAS",			CS_CLASS_SAS,			0, false	},
	{ "models/player/ctm_fbi_old.mdl",								"models/weapons/arms/ct_arms_fbi_old.mdl",		"FBI",			CS_CLASS_FBI,			0, false	},
};

const CCSAgentInfo* GetCSAgentInfoCT( int i )
{
	Assert( i >= 0 && i < ARRAYSIZE( g_AgentInfosCT ) );
	return &g_AgentInfosCT[i];
}

CCSAgentInfo g_AgentInfosT[MAX_AGENTS_T + 1] =
{
	{ "", "", 0, 0 },
	// Shattered Web
	{ "models/player/custom_player/legacy/tm_leet_variantg.mdl",		"models/weapons/arms/v_glove_fingerless.mdl",			"Leet",				CS_CLASS_L337_KREW,				0,	false	},
	{ "models/player/custom_player/legacy/tm_leet_varianth.mdl",		"models/weapons/arms/v_glove_fingerless.mdl",			"Leet",				CS_CLASS_L337_KREW,				1,	false	},
	{ "models/player/custom_player/legacy/tm_leet_varianti.mdl",		"models/weapons/arms/v_glove_fingerless.mdl",			"Leet",				CS_CLASS_L337_KREW,				0,	false	},
	{ "models/player/custom_player/legacy/tm_leet_variantf.mdl",		"models/weapons/arms/v_glove_fingerless.mdl",			"Leet_Epic",		CS_CLASS_L337_KREW,				0,	false	},
	{ "models/player/custom_player/legacy/tm_phoenix_varianth.mdl",		"models/weapons/arms/v_glove_fullfinger.mdl",			"Phoenix",			CS_CLASS_PHOENIX_CONNNECTION,	0,	false	},
	{ "models/player/custom_player/legacy/tm_phoenix_variantf.mdl",		"models/weapons/arms/v_glove_fullfinger.mdl",			"Phoenix",			CS_CLASS_PHOENIX_CONNNECTION,	0,	false	},
	{ "models/player/custom_player/legacy/tm_phoenix_variantg.mdl",		"models/weapons/arms/v_glove_fullfinger.mdl",			"Phoenix",			CS_CLASS_PHOENIX_CONNNECTION,	0,	false	},
	{ "models/player/custom_player/legacy/tm_balkan_variantf.mdl",		"models/weapons/arms/t_arms_balkan_v2_variantf.mdl",	"Balkan",			CS_CLASS_BALKAN,				0,	false	},
	{ "models/player/custom_player/legacy/tm_balkan_varianti.mdl",		"models/weapons/arms/t_arms_balkan_v2_variantf.mdl",	"Balkan",			CS_CLASS_BALKAN,				0,	false	},
	{ "models/player/custom_player/legacy/tm_balkan_variantg.mdl",		"models/weapons/arms/t_arms_balkan_v2_variantg.mdl",	"Balkan",			CS_CLASS_BALKAN,				0,	false	},
	{ "models/player/custom_player/legacy/tm_balkan_variantj.mdl",		"models/weapons/arms/t_arms_balkan_v2_variantj.mdl",	"Balkan",			CS_CLASS_BALKAN,				0,	false	},
	{ "models/player/custom_player/legacy/tm_balkan_varianth.mdl",		"models/weapons/arms/t_arms_balkan_v2_varianth.mdl",	"Balkan_Epic",		CS_CLASS_BALKAN,				0,	false	},
	// Broken Fang
	{ "models/player/custom_player/legacy/tm_balkan_variantl.mdl",		"models/weapons/arms/t_arms_balkan_v2_variantf.mdl",	"Balkan",			CS_CLASS_BALKAN,				1,	false	},
	{ "models/player/custom_player/legacy/tm_phoenix_varianti.mdl",		"models/weapons/arms/v_glove_fullfinger.mdl",			"Phoenix",			CS_CLASS_PHOENIX_CONNNECTION,	3,	false	},
	{ "models/player/custom_player/legacy/tm_professional_varj.mdl",	"models/weapons/arms/t_arms_professional.mdl",			"Professional_Fem",	CS_CLASS_PROFESSIONAL,			0,	true	},
	{ "models/player/custom_player/legacy/tm_professional_varh.mdl",	"models/weapons/arms/v_glove_fullfinger.mdl",			"Professional",		CS_CLASS_PROFESSIONAL,			0,	false	},
	{ "models/player/custom_player/legacy/tm_balkan_variantk.mdl",		"models/weapons/arms/t_arms_balkan_v2_variantg.mdl",	"Balkan",			CS_CLASS_BALKAN,				1,	false	},
	{ "models/player/custom_player/legacy/tm_professional_varg.mdl",	"models/weapons/arms/v_glove_fullfinger.mdl",			"Professional_Fem",	CS_CLASS_PROFESSIONAL,			0,	true	},
	{ "models/player/custom_player/legacy/tm_professional_vari.mdl",	"models/weapons/arms/t_arms_professional.mdl",			"Professional",		CS_CLASS_PROFESSIONAL,			0,	false	},
	{ "models/player/custom_player/legacy/tm_professional_varf.mdl",	"models/weapons/arms/t_arms_professional_watch.mdl",	"Professional_Epic",CS_CLASS_PROFESSIONAL,			0,	false	},
	{ "models/player/custom_player/legacy/tm_professional_varf1.mdl",	"models/weapons/arms/t_arms_professional_watch.mdl",	"Professional_Epic",CS_CLASS_PROFESSIONAL,			1,	false	},
	{ "models/player/custom_player/legacy/tm_professional_varf2.mdl",	"models/weapons/arms/t_arms_professional_watch.mdl",	"Professional_Epic",CS_CLASS_PROFESSIONAL,			0,	false	},
	{ "models/player/custom_player/legacy/tm_professional_varf3.mdl",	"models/weapons/arms/t_arms_professional_watch.mdl",	"Professional_Epic",CS_CLASS_PROFESSIONAL,			0,	false	},
	{ "models/player/custom_player/legacy/tm_professional_varf4.mdl",	"models/weapons/arms/t_arms_professional_watch.mdl",	"Professional_Epic",CS_CLASS_PROFESSIONAL,			0,	false	},
	// what?
	{ "models/player/tm_leet_old.mdl",									"models/weapons/arms/t_arms_leet_old.mdl",				"Leet",				CS_CLASS_L337_KREW,				0,	false	},
	{ "models/player/tm_phoenix_old.mdl",								"models/weapons/arms/t_arms_phoenix_old.mdl",			"Phoenix",			CS_CLASS_PHOENIX_CONNNECTION,	0,	false	},
};

const CCSAgentInfo* GetCSAgentInfoT( int i )
{
	Assert( i >= 0 && i < ARRAYSIZE( g_AgentInfosT ) );
	return &g_AgentInfosT[i];
}

const char *pszWinPanelCategoryHeaders[] =
{
	"",
	"#winpanel_topdamage",
	"#winpanel_topheadshots",
	"#winpanel_kills"
};

// todo: rewrite this because it's TOO MASSIVE!
const char *TPhoenixPlayerModelStrings[] =
{
	"models/player/custom_player/legacy/tm_phoenix.mdl",
	"models/player/custom_player/legacy/tm_phoenix_varianta.mdl",
	"models/player/custom_player/legacy/tm_phoenix_variantb.mdl",
	"models/player/custom_player/legacy/tm_phoenix_variantc.mdl",
	"models/player/custom_player/legacy/tm_phoenix_variantd.mdl",
};
const char *TLeetPlayerModelStrings[] =
{
	"models/player/custom_player/legacy/tm_leet_variantA.mdl",
	"models/player/custom_player/legacy/tm_leet_variantB.mdl",
	"models/player/custom_player/legacy/tm_leet_variantC.mdl",
	"models/player/custom_player/legacy/tm_leet_variantD.mdl",
	"models/player/custom_player/legacy/tm_leet_variantE.mdl",
};
const char *TSeparatistPlayerModelStrings[] =
{
	"models/player/custom_player/legacy/tm_separatist.mdl",
	"models/player/custom_player/legacy/tm_separatist_varianta.mdl",
	"models/player/custom_player/legacy/tm_separatist_variantb.mdl",
	"models/player/custom_player/legacy/tm_separatist_variantc.mdl",
	"models/player/custom_player/legacy/tm_separatist_variantd.mdl",
};
const char *TBalkanPlayerModelStrings[] =
{
	"models/player/custom_player/legacy/tm_balkan_varianta.mdl",
	"models/player/custom_player/legacy/tm_balkan_variantb.mdl",
	"models/player/custom_player/legacy/tm_balkan_variantc.mdl",
	"models/player/custom_player/legacy/tm_balkan_variantd.mdl",
	"models/player/custom_player/legacy/tm_balkan_variante.mdl",
};
const char *TProfessionalPlayerModelStrings[] =
{
	"models/player/custom_player/legacy/tm_professional.mdl",
	"models/player/custom_player/legacy/tm_professional_var1.mdl",
	"models/player/custom_player/legacy/tm_professional_var2.mdl",
	"models/player/custom_player/legacy/tm_professional_var3.mdl",
	"models/player/custom_player/legacy/tm_professional_var4.mdl",
};
const char *TAnarchistPlayerModelStrings[] =
{
	"models/player/custom_player/legacy/tm_anarchist.mdl",
	"models/player/custom_player/legacy/tm_anarchist_varianta.mdl",
	"models/player/custom_player/legacy/tm_anarchist_variantb.mdl",
	"models/player/custom_player/legacy/tm_anarchist_variantc.mdl",
	"models/player/custom_player/legacy/tm_anarchist_variantd.mdl",
};
const char *TPiratePlayerModelStrings[] =
{
	"models/player/custom_player/legacy/tm_pirate.mdl",
	"models/player/custom_player/legacy/tm_pirate_varianta.mdl",
	"models/player/custom_player/legacy/tm_pirate_variantc.mdl",
	"models/player/custom_player/legacy/tm_pirate_variantb.mdl",
	"models/player/custom_player/legacy/tm_pirate_variantd.mdl",
};
CUtlVectorInitialized< const char * > TPhoenixPlayerModels( TPhoenixPlayerModelStrings, ARRAYSIZE( TPhoenixPlayerModelStrings ) );
CUtlVectorInitialized< const char * > TLeetPlayerModels( TLeetPlayerModelStrings, ARRAYSIZE( TLeetPlayerModelStrings ) );
CUtlVectorInitialized< const char * > TSeparatistPlayerModels( TSeparatistPlayerModelStrings, ARRAYSIZE( TSeparatistPlayerModelStrings ) );
CUtlVectorInitialized< const char * > TBalkanPlayerModels( TBalkanPlayerModelStrings, ARRAYSIZE( TBalkanPlayerModelStrings ) );
CUtlVectorInitialized< const char * > TProfessionalPlayerModels( TProfessionalPlayerModelStrings, ARRAYSIZE( TProfessionalPlayerModelStrings ) );
CUtlVectorInitialized< const char * > TAnarchistPlayerModels( TAnarchistPlayerModelStrings, ARRAYSIZE( TAnarchistPlayerModelStrings ) );
CUtlVectorInitialized< const char * > TPiratePlayerModels( TPiratePlayerModelStrings, ARRAYSIZE( TPiratePlayerModelStrings ) );

const char *CTST6PlayerModelStrings[] =
{
	"models/player/custom_player/legacy/ctm_st6.mdl",
	"models/player/custom_player/legacy/ctm_st6_varianta.mdl",
	"models/player/custom_player/legacy/ctm_st6_variantb.mdl",
	"models/player/custom_player/legacy/ctm_st6_variantc.mdl",
	"models/player/custom_player/legacy/ctm_st6_variantd.mdl",
};
const char *CTGSG9PlayerModelStrings[] =
{
	"models/player/custom_player/legacy/ctm_gsg9.mdl",
	"models/player/custom_player/legacy/ctm_gsg9_varianta.mdl",
	"models/player/custom_player/legacy/ctm_gsg9_variantb.mdl",
	"models/player/custom_player/legacy/ctm_gsg9_variantc.mdl",
	"models/player/custom_player/legacy/ctm_gsg9_variantd.mdl",
};
const char *CTSASPlayerModelStrings[] =
{
	"models/player/custom_player/legacy/ctm_sas.mdl",
};
const char *CTGIGNPlayerModelStrings[] =
{
	"models/player/custom_player/legacy/ctm_gign.mdl",
	"models/player/custom_player/legacy/ctm_gign_varianta.mdl",
	"models/player/custom_player/legacy/ctm_gign_variantb.mdl",
	"models/player/custom_player/legacy/ctm_gign_variantc.mdl",
	"models/player/custom_player/legacy/ctm_gign_variantd.mdl",
};
const char *CTFBIPlayerModelStrings[] =
{
	"models/player/custom_player/legacy/ctm_fbi.mdl",
	"models/player/custom_player/legacy/ctm_fbi_varianta.mdl",
	"models/player/custom_player/legacy/ctm_fbi_variantc.mdl",
	"models/player/custom_player/legacy/ctm_fbi_variantd.mdl",
	"models/player/custom_player/legacy/ctm_fbi_variante.mdl",
};
const char *CTIDFPlayerModelStrings[] =
{
	"models/player/custom_player/legacy/ctm_idf.mdl",
	"models/player/custom_player/legacy/ctm_idf_variantb.mdl",
	"models/player/custom_player/legacy/ctm_idf_variantc.mdl",
	"models/player/custom_player/legacy/ctm_idf_variantd.mdl",
	"models/player/custom_player/legacy/ctm_idf_variante.mdl",
	"models/player/custom_player/legacy/ctm_idf_variantf.mdl",
};
const char *CTSWATPlayerModelStrings[] =
{
	"models/player/custom_player/legacy/ctm_swat.mdl",
	"models/player/custom_player/legacy/ctm_swat_varianta.mdl",
	"models/player/custom_player/legacy/ctm_swat_variantb.mdl",
	"models/player/custom_player/legacy/ctm_swat_variantc.mdl",
	"models/player/custom_player/legacy/ctm_swat_variantd.mdl",
};
CUtlVectorInitialized< const char * > CTST6PlayerModels( CTST6PlayerModelStrings, ARRAYSIZE( CTST6PlayerModelStrings ) );
CUtlVectorInitialized< const char * > CTGSG9PlayerModels( CTGSG9PlayerModelStrings, ARRAYSIZE( CTGSG9PlayerModelStrings ) );
CUtlVectorInitialized< const char * > CTSASPlayerModels( CTSASPlayerModelStrings, ARRAYSIZE( CTSASPlayerModelStrings ) );
CUtlVectorInitialized< const char * > CTGIGNPlayerModels( CTGIGNPlayerModelStrings, ARRAYSIZE( CTGIGNPlayerModelStrings ) );
CUtlVectorInitialized< const char * > CTFBIPlayerModels( CTFBIPlayerModelStrings, ARRAYSIZE( CTFBIPlayerModelStrings ) );
CUtlVectorInitialized< const char * > CTIDFPlayerModels( CTIDFPlayerModelStrings, ARRAYSIZE( CTIDFPlayerModelStrings ) );
CUtlVectorInitialized< const char * > CTSWATPlayerModels( CTSWATPlayerModelStrings, ARRAYSIZE( CTSWATPlayerModelStrings ) );

// any new knives? add them here
const char *KnivesEntitiesStrings[] =
{
	"weapon_knife",
	"weapon_knife_t",
	"weapon_knife_css",
	"weapon_knife_karambit",
	"weapon_knife_flip",
	"weapon_knife_bayonet",
	"weapon_knife_m9_bayonet",
	"weapon_knife_butterfly",
	"weapon_knife_gut",
	"weapon_knife_tactical",
	"weapon_knife_falchion",
	"weapon_knife_survival_bowie",
	"weapon_knife_canis",
	"weapon_knife_cord",
	"weapon_knife_gypsy_jackknife",
	"weapon_knife_outdoor",
	"weapon_knife_skeleton",
	"weapon_knife_stiletto",
	"weapon_knife_ursus",
	"weapon_knife_widowmaker",
	"weapon_knife_push",
};
CUtlVectorInitialized< const char * > KnivesEntities( KnivesEntitiesStrings, ARRAYSIZE( KnivesEntitiesStrings ) );