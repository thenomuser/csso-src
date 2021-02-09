//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef BUYMOUSEOVERPANELBUTTON_H
#define BUYMOUSEOVERPANELBUTTON_H
#ifdef _WIN32
#pragma once
#endif

#include <KeyValues.h>
#include <filesystem.h>
#include "mouseoverpanelbutton.h"
#include "hud.h"
#include "c_cs_player.h"
#include "cs_gamerules.h"
#include "cstrike/bot/shared_util.h"
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui_controls/ImagePanel.h>
#include "cs_loadout.h"

using namespace vgui;

extern ConVar loadout_slot_m4_weapon;
extern ConVar loadout_slot_hkp2000_weapon;
extern ConVar loadout_slot_fiveseven_weapon;
extern ConVar loadout_slot_tec9_weapon;
extern ConVar loadout_slot_mp7_weapon_ct;
extern ConVar loadout_slot_mp7_weapon_t;
extern ConVar loadout_slot_deagle_weapon_ct;
extern ConVar loadout_slot_deagle_weapon_t;

//-----------------------------------------------------------------------------
// Purpose: Triggers a new panel when the mouse goes over the button
//-----------------------------------------------------------------------------
class BuyMouseOverPanelButton : public MouseOverPanelButton
{
private:
	typedef MouseOverPanelButton BaseClass;
public:
	BuyMouseOverPanelButton(vgui::Panel *parent, const char *panelName, vgui::EditablePanel *panel) :
	  MouseOverPanelButton( parent, panelName, panel)	
	  {
		m_iPrice = 0;
		m_iPreviousPrice = 0;
		m_iASRestrict = 0;
		m_iDEUseOnly = 0;
		m_iDECSUseOnly = 0;
		m_command = NULL;
		m_bIsBargain = false;
		m_iCost0 = 0;
		m_iCost1 = 0;
		m_label_0 = NULL;
		m_label_1 = NULL;

		m_pBlackMarketPrice = NULL;//new EditablePanel( parent, "BlackMarket_Labels" );
		if ( m_pBlackMarketPrice )
		{
			m_pBlackMarketPrice->LoadControlSettings( "Resource/UI/BlackMarket_Labels.res" );

			int x,y,wide,tall;
			GetClassPanel()->GetBounds( x, y, wide, tall );
			m_pBlackMarketPrice->SetBounds( x, y, wide, tall );
			int px, py;
			GetClassPanel()->GetPinOffset( px, py );
			int rx, ry;
			GetClassPanel()->GetResizeOffset( rx, ry );
			// Apply pin settings from template, too
			m_pBlackMarketPrice->SetAutoResize( GetClassPanel()->GetPinCorner(), GetClassPanel()->GetAutoResize(), px, py, rx, ry );
		}
	  }
	
	virtual void ApplySettings( KeyValues *resourceData )
	{
		BaseClass::ApplySettings( resourceData );

		KeyValues *kv = resourceData->FindKey( "cost" );
		if ( kv )
			m_iPrice = kv->GetInt();

		kv = resourceData->FindKey( "labelText_0", false );
		if ( kv )
			m_label_0 = CloneString( kv->GetString() );

		kv = resourceData->FindKey( "labelText_1", false );
		if ( kv )
			m_label_1 = CloneString( kv->GetString() );

		kv = resourceData->FindKey( "cost_0" );
		if ( kv )
			m_iCost0 = kv->GetInt();

		kv = resourceData->FindKey( "cost_1" );
		if ( kv )
			m_iCost1 = kv->GetInt();

		kv = resourceData->FindKey( "as_restrict", false );
		if( kv ) // if this button has a map limitation for it
			m_iASRestrict = kv->GetInt(); // save the as_restrict away
		
		kv = resourceData->FindKey( "de_useonly", false );
		if( kv ) // if this button has a map limitation for it
			m_iDEUseOnly = kv->GetInt(); // save the de_useonly away
		
		kv = resourceData->FindKey( "de_cs_useonly", false );
		if( kv ) // if this button has a map limitation for it
			m_iDECSUseOnly = kv->GetInt(); // save the de_useonly away

		if ( m_command )
		{
			delete[] m_command;
			m_command = NULL;
		}
		kv = resourceData->FindKey( "command", false );
		if ( kv )
			m_command = CloneString( kv->GetString() );

		SetPriceState();
		SetMapTypeState();

		SetLabels();
	}

	int GetASRestrict() { return m_iASRestrict; }

	int GetDEUseOnly() { return m_iDEUseOnly; }

	int GetDECSUseOnly() { return m_iDECSUseOnly; }

	virtual void PerformLayout()
	{
		BaseClass::PerformLayout();
		SetPriceState();
		SetMapTypeState();

		SetLabels();

#ifndef CS_SHIELD_ENABLED
		if ( !Q_stricmp( GetName(), "shield" ) )
		{
			SetVisible( false );
			SetEnabled( false );
		}
#endif
	}
	
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme )
	{
		BaseClass::ApplySchemeSettings( pScheme );

		m_avaliableColor = pScheme->GetColor( "BuyMenu.AvailableColor", Color( 0, 0, 0, 0 ) );
		m_unavailableColor = pScheme->GetColor( "BuyMenu.UnavailableColor", Color( 0, 0, 0, 0 ) );
		m_alreadyOwnColor = pScheme->GetColor( "BuyMenu.AlreadyOwnColor", Color( 0, 0, 0, 0 ) ); // Label.DisabledFgColor2
		m_bargainColor = Color( 0, 255, 0, 192 );
		m_defaultColor = pScheme->GetColor( "Label.TextColor", Color( 0, 0, 0, 0 ) );

		SetPriceState();
		SetMapTypeState();

		SetLabels();
	}

	void SetPriceState()
	{
		if ( CSGameRules() && CSGameRules()->IsBlackMarket() )
		{
			SetMarketState();
		}
		else
		{
			if ( GetParent() )
			{
				Panel *pPanel = dynamic_cast< Panel * >(GetParent()->FindChildByName( "MarketSticker" ) ); 

				if ( pPanel )
				{
					pPanel->SetVisible( false );
				}
			}

			m_bIsBargain = false;
		}

		SetCommand( m_command );

		C_CSPlayer *pPlayer = C_CSPlayer::GetLocalCSPlayer();
		if ( !pPlayer )
			return;

		if ( m_iPrice && ( m_iPrice > pPlayer->GetAccount() ) )
			SetFgColor( m_unavailableColor );
		else if ( m_iPrice && (m_iPrice <= pPlayer->GetAccount()) )
			SetFgColor( m_avaliableColor );
		else
			SetFgColor( m_bIsBargain ? m_bargainColor : m_defaultColor );

		// check if we already own the weapon
		if ( Q_strncmp( m_command, "buy ", 4 ) == 0 )
		{
			const char* weaponClassFromSlot = CSLoadout()->GetWeaponFromSlot( pPlayer, CSLoadout()->GetSlotFromWeapon( pPlayer, m_command + 4 ) );
			char weaponClass[64];
			Q_snprintf( weaponClass, sizeof( weaponClass ), "weapon_%s", weaponClassFromSlot ? weaponClassFromSlot : m_command + 4 );
			C_WeaponCSBase* pWeapon = dynamic_cast<C_WeaponCSBase*>(pPlayer->Weapon_OwnsThisType( weaponClass ));

			if ( pWeapon )
			{
				if ( pWeapon->GetWeaponType() != WEAPONTYPE_GRENADE )
					SetFgColor( m_alreadyOwnColor );
				else if ( pWeapon->GetReserveAmmoCount( AMMO_POSITION_PRIMARY ) >= pWeapon->GetReserveAmmoMax( AMMO_POSITION_PRIMARY ) )
					SetFgColor( m_alreadyOwnColor );
			}
		}
	}

	void SetMarketState( void )
	{
		Panel *pClassPanel = GetClassPanel();
		if ( pClassPanel )
		{
			pClassPanel->SetVisible( false );
		}

		if ( m_pBlackMarketPrice )
		{
			Label *pLabel = dynamic_cast< Label * >(m_pBlackMarketPrice->FindChildByName( "pricelabel" ) );

			if ( pLabel )
			{
				const int BufLen = 2048;
				wchar_t wbuf[BufLen] = L"";
				const wchar_t *formatStr = g_pVGuiLocalize->Find("#Cstrike_MarketPreviousPrice");

				if ( !formatStr )
					formatStr = L"%s1";

				char strPrice[16];
				wchar_t szPrice[64];
				Q_snprintf( strPrice, sizeof( strPrice ), "%d", m_iPreviousPrice );

				g_pVGuiLocalize->ConvertANSIToUnicode( strPrice, szPrice, sizeof(szPrice));

				g_pVGuiLocalize->ConstructString( wbuf, sizeof(wbuf), formatStr, 1, szPrice );
				pLabel->SetText( wbuf );
				pLabel->SetVisible( true );
			}

			pLabel = dynamic_cast< Label * >(m_pBlackMarketPrice->FindChildByName( "price" ) );

			if ( pLabel )
			{
				const int BufLen = 2048;
				wchar_t wbuf[BufLen] = L"";
				const wchar_t *formatStr = g_pVGuiLocalize->Find("#Cstrike_MarketCurrentPrice");

				if ( !formatStr )
					formatStr = L"%s1";

				char strPrice[16];
				wchar_t szPrice[64];
				Q_snprintf( strPrice, sizeof( strPrice ), "%d", m_iPrice );

				g_pVGuiLocalize->ConvertANSIToUnicode( strPrice, szPrice, sizeof(szPrice));

				g_pVGuiLocalize->ConstructString( wbuf, sizeof(wbuf), formatStr, 1, szPrice );
				pLabel->SetText( wbuf );
				pLabel->SetVisible( true );
			}

			pLabel = dynamic_cast< Label * >(m_pBlackMarketPrice->FindChildByName( "difference" ) );

			if ( pLabel )
			{
				const int BufLen = 2048;
				wchar_t wbuf[BufLen] = L"";
				const wchar_t *formatStr = g_pVGuiLocalize->Find("#Cstrike_MarketDeltaPrice");

				if ( !formatStr )
					formatStr = L"%s1";

				char strPrice[16];
				wchar_t szPrice[64];

				int iDifference = m_iPreviousPrice - m_iPrice;

				if ( iDifference >= 0 )
				{
					pLabel->SetFgColor( m_bargainColor );
				}
				else
				{
					pLabel->SetFgColor( Color( 192, 28, 0, 255 ) );
				}

				Q_snprintf( strPrice, sizeof( strPrice ), "%d", abs( iDifference ) );

				g_pVGuiLocalize->ConvertANSIToUnicode( strPrice, szPrice, sizeof(szPrice));

				g_pVGuiLocalize->ConstructString( wbuf, sizeof(wbuf), formatStr, 1, szPrice );
				pLabel->SetText( wbuf );
				pLabel->SetVisible( true );
			}

			ImagePanel *pImage = dynamic_cast< ImagePanel * >(m_pBlackMarketPrice->FindChildByName( "classimage" ) );	

			if ( pImage )
			{
				ImagePanel *pClassImage = dynamic_cast< ImagePanel * >(GetClassPanel()->FindChildByName( "classimage" ) );	

				if ( pClassImage )
				{
					pImage->SetSize( pClassImage->GetWide(), pClassImage->GetTall() );
					pImage->SetImage( pClassImage->GetImage() );
				}
			}

			if ( GetParent() )
			{
				Panel *pPanel = dynamic_cast< Panel * >(GetParent()->FindChildByName( "MarketSticker" ) ); 

				if ( pPanel )
				{
					if ( m_bIsBargain )
					{
						pPanel->SetVisible( true );
					}
					else
					{
						pPanel->SetVisible( false );
					}
				}
			}
		}
	}

	virtual void SetLabels()
	{
		if ( Q_strcmp( GetName(), "m4a4_m4a1" ) == 0 )
		{
			SetText( (loadout_slot_m4_weapon.GetInt() == 1) ? m_label_1 : m_label_0 );
			m_iPrice = (loadout_slot_m4_weapon.GetInt() == 1) ? m_iCost1 : m_iCost0;
		}
		else if ( Q_strcmp( GetName(), "hkp2000_usp" ) == 0 )
		{
			SetText( (loadout_slot_hkp2000_weapon.GetInt() == 1) ? m_label_1 : m_label_0 );
		}
		else if ( Q_strcmp( GetName(), "fiveseven_cz75" ) == 0 )
		{
			SetText( (loadout_slot_fiveseven_weapon.GetInt() == 1) ? m_label_1 : m_label_0 );
		}
		else if ( Q_strcmp( GetName(), "tec9_cz75" ) == 0 )
		{
			SetText( (loadout_slot_tec9_weapon.GetInt() == 1) ? m_label_1 : m_label_0 );
		}
		else if ( Q_strcmp( GetName(), "mp7_mp5sd_ct" ) == 0 )
		{
			SetText( (loadout_slot_mp7_weapon_ct.GetInt() == 1) ? m_label_1 : m_label_0 );
		}
		else if ( Q_strcmp( GetName(), "mp7_mp5sd_t" ) == 0 )
		{
			SetText( (loadout_slot_mp7_weapon_t.GetInt() == 1) ? m_label_1 : m_label_0 );
		}
		else if ( Q_strcmp( GetName(), "deagle_revolver_ct" ) == 0 )
		{
			SetText( (loadout_slot_deagle_weapon_ct.GetInt() == 1) ? m_label_1 : m_label_0 );
			m_iPrice = (loadout_slot_deagle_weapon_ct.GetInt() == 1) ? m_iCost1 : m_iCost0;
		}
		else if ( Q_strcmp( GetName(), "deagle_revolver_t" ) == 0 )
		{
			SetText( (loadout_slot_deagle_weapon_t.GetInt() == 1) ? m_label_1 : m_label_0 );
			m_iPrice = (loadout_slot_deagle_weapon_t.GetInt() == 1) ? m_iCost1 : m_iCost0;
		}

		if ( CSGameRules() )
		{
			if ( Q_strcmp( GetName(), "defuser" ) == 0 )
				SetText( (CSGameRules()->IsHostageRescueMap()) ? m_label_1 : m_label_0 );
		}
	}

	void SetMapTypeState()
	{
		CCSGameRules *pRules = CSGameRules();

		if ( pRules ) 
		{
			if( pRules->IsVIPMap() )
			{
				if ( m_iASRestrict )
				{
					SetFgColor( m_unavailableColor );
					SetCommand( "buy_unavailable" );
				}
			}

			if ( m_iDECSUseOnly )
			{
				if ( !pRules->IsBombDefuseMap() && !pRules->IsHostageRescueMap() )
				{
					SetFgColor( m_unavailableColor );
					SetCommand( "buy_unavailable" );
				}
			}

			if ( m_iDEUseOnly )
			{
				if ( !pRules->IsBombDefuseMap() )
				{
					SetFgColor( m_unavailableColor );
					SetCommand( "buy_unavailable" );
				}
			}
		}
	}

	void SetBargainButton( bool state )
	{
		m_bIsBargain = state;
	}

	void SetCurrentPrice( int iPrice )
	{
		m_iPrice = iPrice;
	}

	void SetPreviousPrice( int iPrice )
	{
		m_iPreviousPrice = iPrice;
	}

	const char *GetBuyCommand( void )
	{
		return m_command;
	}
	
	virtual void ShowPage()
	{
		if ( g_lastPanel )
		{
			for( int i = 0; i< g_lastPanel->GetParent()->GetChildCount(); i++ )
			{
				MouseOverPanelButton *buyButton = dynamic_cast<MouseOverPanelButton *>(g_lastPanel->GetParent()->GetChild(i));

				if ( buyButton )
				{
					buyButton->HidePage();
				}
			}
		}

		BaseClass::ShowPage();

		//SetUpWeaponSlots();

		if ( !Q_stricmp( m_command, "vguicancel" ) )
			return;

		if ( CSGameRules() && CSGameRules()->IsBlackMarket() )
		{
			if ( m_pBlackMarketPrice && !m_pBlackMarketPrice->IsVisible() )
			{
				m_pBlackMarketPrice->SetVisible( true );
			}
		}
	}

	virtual void HidePage()
	{
		BaseClass::HidePage();

		if ( m_pBlackMarketPrice && m_pBlackMarketPrice->IsVisible() )
		{
			m_pBlackMarketPrice->SetVisible( false );
		}
	}

private:

	int m_iPrice;
	int m_iPreviousPrice;
	int m_iASRestrict;
	int m_iDEUseOnly;
	int m_iDECSUseOnly;
	bool m_bIsBargain;

	Color m_avaliableColor;
	Color m_unavailableColor;
	Color m_alreadyOwnColor;
	Color m_bargainColor;
	Color m_defaultColor;

	char *m_command;
	
	int m_iCost0;
	int m_iCost1;
	char *m_label_0;
	char *m_label_1;
	
public:
		vgui::EditablePanel *m_pBlackMarketPrice;
};


#endif // BUYMOUSEOVERPANELBUTTON_H
