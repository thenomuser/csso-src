//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef SMOKEGRENADE_PROJECTILE_H
#define SMOKEGRENADE_PROJECTILE_H
#ifdef _WIN32
#pragma once
#endif


#include "basecsgrenade_projectile.h"


class CSmokeGrenadeProjectile : public CBaseCSGrenadeProjectile
{
public:
	DECLARE_CLASS( CSmokeGrenadeProjectile, CBaseCSGrenadeProjectile );
	DECLARE_DATADESC();

// Overrides.
public:
	
	virtual void Spawn();
	virtual void Precache();
	virtual void Detonate();
	virtual void OnBounced( void );
	virtual void BounceSound( void );

	void Think_Detonate();
	void Think_Fade();
	void Think_Remove();

	void SmokeDetonate( void );

// Grenade stuff.
public:

	static CSmokeGrenadeProjectile* Create( 
		const Vector &position, 
		const QAngle &angles, 
		const Vector &velocity, 
		const AngularImpulse &angVelocity, 
		CBaseCombatCharacter *pOwner );

	void SetTimer( float timer );

	EHANDLE m_hSmokeEffect;
	bool m_bDidSmokeEffect;
	float m_flLastBounce;
};


#endif // SMOKEGRENADE_PROJECTILE_H
