//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef ModOptionsSubHUD_H
#define ModOptionsSubHUD_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/PropertyPage.h>
#include <vgui_controls/Label.h>

class CLabeledCommandComboBox;

class CModOptionsSubHUD;

//-----------------------------------------------------------------------------
// Purpose: crosshair options property page
//-----------------------------------------------------------------------------
class CModOptionsSubHUD: public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE( CModOptionsSubHUD, vgui::PropertyPage );

public:
	CModOptionsSubHUD( vgui::Panel *parent );
	~CModOptionsSubHUD();

	MESSAGE_FUNC( OnControlModified, "ControlModified" );

protected:
	// Called when page is loaded.  Data should be reloaded from document into controls.
	virtual void OnResetData();
	// Called when the OK / Apply button is pressed.  Changed data should be written into document.
	virtual void OnApplyChanges();

private:
	CLabeledCommandComboBox*	m_pPlayerCountPos;
	CLabeledCommandComboBox*	m_pHealthArmorStyle;
	CLabeledCommandComboBox*	m_pAccountStyle;
};

#endif // ModOptionsSubHUD_H
