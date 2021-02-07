//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef MODOPTIONSSUBCROSSHAIR_H
#define MODOPTIONSSUBCROSSHAIR_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/PropertyPage.h>
#include <vgui_controls/ImagePanel.h>
#include "imageutils.h"

class CLabeledCommandComboBox;

class CCvarToggleCheckButton;
class CCvarTextEntry;
class CCvarSlider;

class CModOptionsSubCrosshair;
 
class CrosshairImagePanelBase : public vgui::ImagePanel
{
	DECLARE_CLASS_SIMPLE( CrosshairImagePanelBase, vgui::ImagePanel );
public:
	CrosshairImagePanelBase( Panel *parent, const char *name ) : BaseClass(parent, name) {}
	virtual void ResetData() {}
	virtual void ApplyChanges() {}
	virtual void UpdateVisibility() {}
};

//-----------------------------------------------------------------------------
// Purpose: crosshair options property page
//-----------------------------------------------------------------------------
class CModOptionsSubCrosshair: public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE( CModOptionsSubCrosshair, vgui::PropertyPage );

public:
	CModOptionsSubCrosshair( vgui::Panel *parent );
	~CModOptionsSubCrosshair();

	MESSAGE_FUNC( OnControlModified, "ControlModified" );

protected:
	// Called when page is loaded.  Data should be reloaded from document into controls.
	virtual void OnResetData();
	// Called when the OK / Apply button is pressed.  Changed data should be written into document.
	virtual void OnApplyChanges();

private:
	CrosshairImagePanelBase *m_pCrosshairImage;
};

#endif // MODOPTIONSSUBCROSSHAIR_H
