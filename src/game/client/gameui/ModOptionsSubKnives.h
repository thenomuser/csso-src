//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef MODOPTIONSSUBKNIVES_H
#define MODOPTIONSSUBKNIVES_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/PropertyPage.h>

class CLabeledCommandComboBox;
class CBitmapImagePanel;

class CModOptionsSubKnives;

//-----------------------------------------------------------------------------
// Purpose: crosshair options property page
//-----------------------------------------------------------------------------
class CModOptionsSubKnives: public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE( CModOptionsSubKnives, vgui::PropertyPage );

public:
	CModOptionsSubKnives( vgui::Panel *parent );
	~CModOptionsSubKnives();

	MESSAGE_FUNC( OnControlModified, "ControlModified" );
	MESSAGE_FUNC_PTR( OnTextChanged, "TextChanged", panel );

protected:
	// Called when page is loaded.  Data should be reloaded from document into controls.
	virtual void OnResetData();
	// Called when the OK / Apply button is pressed.  Changed data should be written into document.
	virtual void OnApplyChanges();

private:
	void					RemapKnivesImage();
	CBitmapImagePanel		*m_pKnifeImageCT;
	CBitmapImagePanel		*m_pKnifeImageT;

	CLabeledCommandComboBox *m_pLoadoutKnifeCTComboBox;
	CLabeledCommandComboBox *m_pLoadoutKnifeTComboBox;
};

#endif // MODOPTIONSSUBKNIVES_H
