//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef MODOPTIONSSUBLOADOUT_H
#define MODOPTIONSSUBLOADOUT_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/PropertyPage.h>

class CLabeledCommandComboBox;

class CModOptionsSubLoadout;

//-----------------------------------------------------------------------------
// Purpose: crosshair options property page
//-----------------------------------------------------------------------------
class CModOptionsSubLoadout: public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE( CModOptionsSubLoadout, vgui::PropertyPage );

public:
	CModOptionsSubLoadout( vgui::Panel *parent );
	~CModOptionsSubLoadout();

	MESSAGE_FUNC( OnControlModified, "ControlModified" );

protected:
	// Called when page is loaded.  Data should be reloaded from document into controls.
	virtual void OnResetData();
	// Called when the OK / Apply button is pressed.  Changed data should be written into document.
	virtual void OnApplyChanges();

private:
	CLabeledCommandComboBox *m_pLoadoutM4ComboBox;
	CLabeledCommandComboBox *m_pLoadoutHKP2000ComboBox;
	CLabeledCommandComboBox *m_pLoadoutFiveSevenComboBox;
	CLabeledCommandComboBox *m_pLoadoutTec9ComboBox;
	CLabeledCommandComboBox *m_pLoadoutMP7CTComboBox;
	CLabeledCommandComboBox *m_pLoadoutMP7TComboBox;
	CLabeledCommandComboBox *m_pLoadoutDeagleCTComboBox;
	CLabeledCommandComboBox *m_pLoadoutDeagleTComboBox;
};

#endif // MODOPTIONSSUBLOADOUT_H
