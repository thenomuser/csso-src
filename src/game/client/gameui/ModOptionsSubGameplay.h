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

// PiMoN: change this to zero if you want to
// show a MessageBox to player that they need
// to restart the game or disconnect from a server
#define INSTANT_MUSIC_CHANGE 1

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
	CCvarSlider*				m_pViewmodelRecoil;
	vgui::Label*				m_pViewmodelRecoilLabel;
	CLabeledCommandComboBox*	m_pViewbobStyle;
	CLabeledCommandComboBox*	m_pWeaponPos;
	CLabeledCommandComboBox*	m_pMusicSelection;

#if !INSTANT_MUSIC_CHANGE
	bool						m_bNeedToWarnAboutMusic;
#endif
};

#endif // MODOPTIONSSUBGAMEPLAY_H
