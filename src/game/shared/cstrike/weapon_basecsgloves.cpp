//========= Copyright PiMoNFeeD, CS:SO, All rights reserved. ==================//
//
// Purpose: Base class for all in-game gloves
//
//=============================================================================//

#include "cbase.h"
#include "weapon_basecsgloves.h"
#include "cs_loadout.h"
#ifdef CLIENT_DLL
#include "c_cs_player.h"
#else
#include "cs_player.h"
#endif

CBaseCSGloves::CBaseCSGloves( const char *pszModel )
{
#ifdef CLIENT_DLL
	InitializeAsClientEntity( pszModel, RENDER_GROUP_OPAQUE_ENTITY );
#endif
}

#ifdef CLIENT_DLL
bool CBaseCSGloves::ShouldDraw( void )
{
	CCSPlayer *pPlayerOwner = ToCSPlayer( GetOwnerEntity() );

	if ( !pPlayerOwner )
		return false;

	if ( !pPlayerOwner->ShouldDrawThisPlayer() )
		return false;

	if ( !pPlayerOwner->IsVisible() )
		return false;

	if ( pPlayerOwner->IsDormant() )
		return false;

	if ( !pPlayerOwner->IsAlive() )
		return false;
		
	return BaseClass::ShouldDraw();
}

const Vector& CBaseCSGloves::GetAbsOrigin( void ) const
{
	// if the player carrying this glove is in lod state (meaning outside the camera frustum)
	// we don't need to set up all the player's attachment bones just to find out where exactly
	// the gloves model wants to render. Just return the player's origin.

	CCSPlayer *pPlayerOwner = ToCSPlayer( GetOwnerEntity() );

	if ( pPlayerOwner )
	{
		if ( pPlayerOwner->IsDormant() || !pPlayerOwner->IsVisible() )
			return pPlayerOwner->GetAbsOrigin();

	}

	return BaseClass::GetAbsOrigin();
}
#endif

void CBaseCSGloves::Equip( CCSPlayer *pOwner )
{
	if ( !pOwner )
		return;

	if ( !pOwner->IsAlive() )
		return;

	UpdateGlovesModel();

	FollowEntity( pOwner, true );
	SetOwnerEntity( pOwner );

	// assuming that before equipping them, a DoesModelSupportGloves() check was made
	pOwner->SetBodygroup( pOwner->FindBodygroupByName( "gloves" ), 1 ); // hide default gloves
}

void CBaseCSGloves::UnEquip()
{
	CCSPlayer *pPlayerOwner = ToCSPlayer( GetOwnerEntity() );

	if ( !pPlayerOwner )
	{
		Remove();
		return;
	}

	pPlayerOwner->SetBodygroup( pPlayerOwner->FindBodygroupByName( "gloves" ), 0 ); // restore default gloves

	SetOwnerEntity( NULL );

	Remove();
}

void CBaseCSGloves::UpdateGlovesModel()
{
	CCSPlayer *pPlayerOwner = ToCSPlayer( GetOwnerEntity() );
	if ( !pPlayerOwner )
		return;

#ifdef CLIENT_DLL
	MDLCACHE_CRITICAL_SECTION();
#endif
	const char *pszModel = GetGlovesInfo( CSLoadout()->GetGlovesForPlayer( pPlayerOwner, pPlayerOwner->GetTeamNumber() ) )->szWorldModel;
	SetModel( pszModel );
}

