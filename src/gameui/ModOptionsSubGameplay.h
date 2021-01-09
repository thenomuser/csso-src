//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef MODOPTIONSSUBGAMEPLAY_H
#define MODOPTIONSSUBGAMEPLAY_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/PropertyPage.h>
#include <vgui_controls/Label.h>

class CLabeledCommandComboBox;


class CCvarToggleCheckButton;
class CCvarSlider;

class CModOptionsSubGameplay;

//-----------------------------------------------------------------------------
// Purpose: crosshair options property page
//-----------------------------------------------------------------------------
class CModOptionsSubGameplay: public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE( CModOptionsSubGameplay, vgui::PropertyPage );

public:
	CModOptionsSubGameplay( vgui::Panel *parent );
	~CModOptionsSubGameplay();

	MESSAGE_FUNC( OnControlModified, "ControlModified" );
	MESSAGE_FUNC_PTR( OnTextChanged, "TextChanged", panel );

	void UpdateViewmodelSliderLabels();

protected:
	MESSAGE_FUNC_PARAMS( OnSliderMoved, "SliderMoved", data );

	// Called when page is loaded.  Data should be reloaded from document into controls.
	virtual void OnResetData();
	// Called when the OK / Apply button is pressed.  Changed data should be written into document.
	virtual void OnApplyChanges();

private:
	CCvarToggleCheckButton*		m_pCloseOnBuy;
	CCvarToggleCheckButton*		m_pUseOpensBuyMenu;
	CCvarToggleCheckButton*		m_pAddBotPrefix;
	CCvarSlider*				m_pViewmodelOffsetX;
	vgui::Label*				m_pViewmodelOffsetXLabel;
	CCvarSlider*				m_pViewmodelOffsetY;
	vgui::Label*				m_pViewmodelOffsetYLabel;
	CCvarSlider*				m_pViewmodelOffsetZ;
	vgui::Label*				m_pViewmodelOffsetZLabel;
	CLabeledCommandComboBox*	m_pViewmodelOffsetPreset;
	CCvarSlider*				m_pViewmodelFOV;
	vgui::Label*				m_pViewmodelFOVLabel;
	CLabeledCommandComboBox*	m_pViewbobStyle;
	CLabeledCommandComboBox*	m_pWeaponPos;
};

#endif // MODOPTIONSSUBGAMEPLAY_H
