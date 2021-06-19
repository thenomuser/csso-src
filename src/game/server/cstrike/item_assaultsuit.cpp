//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#include "cbase.h"
#include "items.h"
#include "cs_player.h"


class CItemAssaultSuit : public CItem
{
	void Spawn( void )
	{ 
		Precache( );
		CItem::Spawn( );
	}
	bool MyTouch( CBasePlayer *pBasePlayer )
	{
		CCSPlayer *pPlayer = dynamic_cast< CCSPlayer* >( pBasePlayer );
		if ( !pPlayer )
		{
			Assert( false );
			return false;
		}

		pPlayer->m_bHasHelmet = true;
		pPlayer->SetArmorValue( 100 );

		return true;		
	}
};

LINK_ENTITY_TO_CLASS( item_assaultsuit, CItemAssaultSuit );


