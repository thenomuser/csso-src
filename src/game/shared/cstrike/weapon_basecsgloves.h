//========= Copyright PiMoNFeeD, CS:SO, All rights reserved. ==================//
//
// Purpose: Base class for all in-game gloves
//
//=============================================================================//

#ifndef WEAPON_BASECSGLOVES_H
#define WEAPON_BASECSGLOVES_H
#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL
#define CBaseCSGloves C_BaseCSGloves
#define CCSPlayer C_CSPlayer

#include "c_baseanimating.h"
#else
#include "baseanimating.h"
#endif

class CCSPlayer;

class CBaseCSGloves: public CBaseAnimating
{
	DECLARE_CLASS( CBaseCSGloves, CBaseAnimating );

public:
	CBaseCSGloves( const char *pszModel );

#ifdef CLIENT_DLL
	virtual bool ShouldDraw( void );

	virtual const Vector& GetAbsOrigin( void ) const;
#endif

	void Equip( CCSPlayer *pOwner );
	void UnEquip();

	void UpdateGlovesModel();
};

#endif // WEAPON_BASECSGLOVES_H