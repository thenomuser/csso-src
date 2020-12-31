//========= Copyright © Valve Corporation, All rights reserved. ============//
#ifndef VPHYSICS2_PARAM_TYPES
#define VPHYSICS2_PARAM_TYPES

#include "mathlib/mathlib.h"
#include "tier1/utlvector.h"
#include "rubikon/serializehelpers.h"

struct RnMaterial_t;


#if _MSC_VER >= 1300 // msvc 7.1
#define RN_DEPRECATED __declspec( deprecated )
#pragma warning(1 : 4996) // deprecated warning
#else
#define RN_DEPRECATED
#endif

//-------------------------------------------------------------------------------------------------
// Shape material
//-------------------------------------------------------------------------------------------------
struct RnMaterial_t
{
	RnMaterial_t()
	{

	}

	// Default material	(density of water in kg/inch^3)
	RnMaterial_t( float flFriction, float flRestitution )
	: m_flDensity( 0.015625f )
	, m_flFriction( flFriction )
	, m_flRestitution( flRestitution )
	{

	}

	float m_flDensity;
	float m_flFriction;
	float m_flRestitution;
	uintp m_pUserData AUTO_SERIALIZE_AS( CPhysSurfacePropertiesHandle );
};


//-------------------------------------------------------------------------------------------------
// Shape cast result
//-------------------------------------------------------------------------------------------------
struct CShapeCastResult
{
	float m_flHitTime; 
	Vector m_vHitPoint;
	Vector m_vHitNormal;
	const RnMaterial_t *m_pMaterial;
	bool m_bStartInSolid;

	CShapeCastResult( void )
	{
		m_flHitTime = 1.0f; 
		m_vHitPoint = Vector( 0, 0, 0 );
		m_vHitNormal = Vector( 0, 0, 0 );
		m_bStartInSolid = false;
		m_pMaterial = NULL;
	}

	bool DidHit( void )
	{
		return m_flHitTime < 1.0f;
	}
};

#endif