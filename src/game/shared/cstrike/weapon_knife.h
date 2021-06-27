//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef WEAPON_KNIFE_H
#define WEAPON_KNIFE_H
#ifdef _WIN32
#pragma once
#endif


#include "weapon_csbase.h"


#if defined( CLIENT_DLL )

	#define CKnife C_Knife
	#define CKnifeT C_KnifeT
	#define CKnifeCSS C_KnifeCSS
	#define CKnifeKarambit C_KnifeKarambit
	#define CKnifeFlip C_KnifeFlip
	#define CKnifeBayonet C_KnifeBayonet
	#define CKnifeM9Bayonet C_KnifeM9Bayonet
	#define CKnifeButterfly C_KnifeButterfly
	#define CKnifeGut C_KnifeGut
	#define CKnifeTactical C_KnifeTactical
	#define CKnifeFalchion C_KnifeFalchion
	#define CKnifeSurvivalBowie C_KnifeSurvivalBowie
	#define CKnifeCanis C_KnifeCanis
	#define CKnifeCord C_KnifeCord
	#define CKnifeGypsy C_KnifeGypsy
	#define CKnifeOutdoor C_KnifeOutdoor
	#define CKnifeSkeleton C_KnifeSkeleton
	#define CKnifeStiletto C_KnifeStiletto
	#define CKnifeUrsus C_KnifeUrsus
	#define CKnifeWidowmaker C_KnifeWidowmaker
	#define CKnifePush C_KnifePush

#endif


// ----------------------------------------------------------------------------- //
// CKnife class definition.
// ----------------------------------------------------------------------------- //

class CKnife : public CWeaponCSBase
{
public:
	DECLARE_CLASS( CKnife, CWeaponCSBase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	
	CKnife();

	// We say yes to this so the weapon system lets us switch to it.
	virtual bool HasPrimaryAmmo();
	virtual bool CanBeSelected();
	
	virtual void Precache();

	void Spawn();
	bool SwingOrStab( CSWeaponMode weaponMode );
	void PrimaryAttack();
	void SecondaryAttack();

	virtual bool Deploy();

	bool CanDrop();

	void WeaponIdle();

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_KNIFE; }

private:
	CKnife( const CKnife & ) {}

#ifndef CLIENT_DLL

	bool m_swingLeft;

#endif
};


class CKnifeT : public CKnife
{
public:
	DECLARE_CLASS( CKnifeT, CKnife );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_KNIFE_T; }
	
	CKnifeT() {}

private:
	CKnifeT( const CKnifeT & ) {}
};


class CKnifeCSS : public CKnife
{
public:
	DECLARE_CLASS( CKnifeCSS, CKnife );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_KNIFE_CSS; }
	
	CKnifeCSS() {}

private:
	CKnifeCSS( const CKnifeCSS & ) {}
};


class CKnifeKarambit : public CKnife
{
public:
	DECLARE_CLASS( CKnifeKarambit, CKnife );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_KNIFE_KARAMBIT; }
	
	CKnifeKarambit() {}

private:
	CKnifeKarambit( const CKnifeKarambit & ) {}
};


class CKnifeFlip : public CKnife
{
public:
	DECLARE_CLASS( CKnifeFlip, CKnife );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_KNIFE_FLIP; }

	CKnifeFlip() {}

private:
	CKnifeFlip( const CKnifeFlip & ) {}
};


class CKnifeBayonet : public CKnife
{
public:
	DECLARE_CLASS( CKnifeBayonet, CKnife );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_KNIFE_BAYONET; }

	CKnifeBayonet() {}

private:
	CKnifeBayonet( const CKnifeBayonet & ) {}
};


class CKnifeM9Bayonet : public CKnife
{
public:
	DECLARE_CLASS( CKnifeM9Bayonet, CKnife );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_KNIFE_M9_BAYONET; }

	CKnifeM9Bayonet() {}

private:
	CKnifeM9Bayonet( const CKnifeM9Bayonet & ) {}
};


class CKnifeButterfly : public CKnife
{
public:
	DECLARE_CLASS( CKnifeButterfly, CKnife );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_KNIFE_BUTTERFLY; }

	CKnifeButterfly() {}

private:
	CKnifeButterfly( const CKnifeButterfly & ) {}
};


class CKnifeGut : public CKnife
{
public:
	DECLARE_CLASS( CKnifeGut, CKnife );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_KNIFE_GUT; }

	CKnifeGut() {}

private:
	CKnifeGut( const CKnifeGut & ) {}
};


class CKnifeTactical : public CKnife
{
public:
	DECLARE_CLASS( CKnifeTactical, CKnife );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_KNIFE_TACTICAL; }

	CKnifeTactical() {}

private:
	CKnifeTactical( const CKnifeTactical& ) {}
};


class CKnifeFalchion : public CKnife
{
public:
	DECLARE_CLASS( CKnifeFalchion, CKnife );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_KNIFE_FALCHION; }

	CKnifeFalchion() {}

private:
	CKnifeFalchion( const CKnifeFalchion& ) {}
};


class CKnifeSurvivalBowie : public CKnife
{
public:
	DECLARE_CLASS( CKnifeSurvivalBowie, CKnife );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_KNIFE_SURVIVAL_BOWIE; }

	CKnifeSurvivalBowie() {}

private:
	CKnifeSurvivalBowie( const CKnifeSurvivalBowie & ) {}
};


class CKnifeCanis : public CKnife
{
public:
	DECLARE_CLASS( CKnifeCanis, CKnife );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_KNIFE_CANIS; }

	CKnifeCanis() {}

private:
	CKnifeCanis( const CKnifeCanis & ) {}
};


class CKnifeCord: public CKnife
{
public:
	DECLARE_CLASS( CKnifeCord, CKnife );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_KNIFE_CORD; }

	CKnifeCord() {}

private:
	CKnifeCord( const CKnifeCord & ) {}
};


class CKnifeGypsy: public CKnife
{
public:
	DECLARE_CLASS( CKnifeGypsy, CKnife );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_KNIFE_GYPSY; }

	CKnifeGypsy() {}

private:
	CKnifeGypsy( const CKnifeGypsy & ) {}
};


class CKnifeOutdoor: public CKnife
{
public:
	DECLARE_CLASS( CKnifeOutdoor, CKnife );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_KNIFE_OUTDOOR; }

	CKnifeOutdoor() {}

private:
	CKnifeOutdoor( const CKnifeOutdoor & ) {}
};


class CKnifeSkeleton: public CKnife
{
public:
	DECLARE_CLASS( CKnifeSkeleton, CKnife );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_KNIFE_SKELETON; }

	CKnifeSkeleton() {}

private:
	CKnifeSkeleton( const CKnifeSkeleton & ) {}
};


class CKnifeStiletto: public CKnife
{
public:
	DECLARE_CLASS( CKnifeStiletto, CKnife );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_KNIFE_STILETTO; }

	CKnifeStiletto() {}

private:
	CKnifeStiletto( const CKnifeStiletto & ) {}
};


class CKnifeUrsus: public CKnife
{
public:
	DECLARE_CLASS( CKnifeUrsus, CKnife );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_KNIFE_URSUS; }

	CKnifeUrsus() {}

private:
	CKnifeUrsus( const CKnifeUrsus & ) {}
};


class CKnifeWidowmaker: public CKnife
{
public:
	DECLARE_CLASS( CKnifeWidowmaker, CKnife );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_KNIFE_WIDOWMAKER; }

	CKnifeWidowmaker() {}

private:
	CKnifeWidowmaker( const CKnifeWidowmaker & ) {}
};


class CKnifePush: public CKnife
{
public:
	DECLARE_CLASS( CKnifePush, CKnife );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_KNIFE_PUSH; }

	CKnifePush() {}

private:
	CKnifePush( const CKnifePush & ) {}
};


#endif // WEAPON_KNIFE_H
