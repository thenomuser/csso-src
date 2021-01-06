//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef MODOPTIONSSUBGLOVES_H
#define MODOPTIONSSUBGLOVES_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/PropertyPage.h>

class CLabeledCommandComboBox;
class CBitmapImagePanel;

class CModOptionsSubGloves;

//-----------------------------------------------------------------------------
// Purpose: crosshair options property page
//-----------------------------------------------------------------------------
class CModOptionsSubGloves: public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE( CModOptionsSubGloves, vgui::PropertyPage );

public:
	CModOptionsSubGloves( vgui::Panel *parent );
	~CModOptionsSubGloves();

	MESSAGE_FUNC( OnControlModified, "ControlModified" );
	MESSAGE_FUNC_PTR( OnTextChanged, "TextChanged", panel );

protected:
	// Called when page is loaded.  Data should be reloaded from document into controls.
	virtual void OnResetData();
	// Called when the OK / Apply button is pressed.  Changed data should be written into document.
	virtual void OnApplyChanges();

private:
	void					RemapGlovesImage();
	CBitmapImagePanel		*m_pGloveImageCT;
	CBitmapImagePanel		*m_pGloveImageT;

	CLabeledCommandComboBox *m_pLoadoutGloveCTComboBox;
	CLabeledCommandComboBox *m_pLoadoutGloveTComboBox;
};

#endif // MODOPTIONSSUBGLOVES_H
