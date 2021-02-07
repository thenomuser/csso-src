//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef MODOPTIONSDIALOG_H
#define MODOPTIONSDIALOG_H
#ifdef _WIN32
#pragma once
#endif

#include "vgui_controls/PropertyDialog.h"
#include "vgui_controls/KeyRepeat.h"

//-----------------------------------------------------------------------------
// Purpose: Holds all the game option pages
//-----------------------------------------------------------------------------
class CModOptionsDialog : public vgui::PropertyDialog
{
	DECLARE_CLASS_SIMPLE( CModOptionsDialog, vgui::PropertyDialog );

public:
	CModOptionsDialog(vgui::Panel *parent);
	~CModOptionsDialog();

	void Run();
	virtual void Activate();

	void OnKeyCodePressed( vgui::KeyCode code );

	MESSAGE_FUNC( OnGameUIHidden, "GameUIHidden" );	// called when the GameUI is hidden
};

#endif // OPTIONSDIALOG_H
