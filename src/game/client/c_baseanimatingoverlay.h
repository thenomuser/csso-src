//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#ifndef C_BASEANIMATINGOVERLAY_H
#define C_BASEANIMATINGOVERLAY_H
#pragma once

#include "c_baseanimating.h"

// For shared code.
#define CBaseAnimatingOverlay C_BaseAnimatingOverlay


class C_BaseAnimatingOverlay : public C_BaseAnimating
{
public:
	DECLARE_CLASS( C_BaseAnimatingOverlay, C_BaseAnimating );
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_INTERPOLATION();


	C_BaseAnimatingOverlay();

	virtual CStudioHdr *OnNewModel();
	
	C_AnimationLayer* GetAnimOverlay( int i, bool bUseOrder = true );
	void SetNumAnimOverlays( int num );	// This makes sure there is space for this # of layers.
	int GetNumAnimOverlays() const;

	virtual void	GetRenderBounds( Vector& theMins, Vector& theMaxs );

	virtual bool UpdateDispatchLayer( CAnimationLayer *pLayer, CStudioHdr *pWeaponStudioHdr, int iSequence );
	void AccumulateDispatchedLayers( C_BaseAnimatingOverlay *pWeapon, CStudioHdr *pWeaponStudioHdr, IBoneSetup &boneSetup, Vector pos[], Quaternion q[], float currentTime );
	void RegenerateDispatchedLayers( IBoneSetup &boneSetup, Vector pos[], Quaternion q[], float currentTime );

	void AccumulateInterleavedDispatchedLayers( C_BaseAnimatingOverlay *pWeapon, IBoneSetup &boneSetup, Vector pos[], Quaternion q[], float currentTime, bool bSetupInvisibleWeapon = false );

	virtual void	NotifyOnLayerChangeSequence( const CAnimationLayer* pLayer, const int nNewSequence ) {};
	virtual void	NotifyOnLayerChangeWeight( const CAnimationLayer* pLayer, const float flNewWeight ) {};
	virtual void	NotifyOnLayerChangeCycle( const CAnimationLayer* pLayer, const float flNewCycle ) {};

	void			CheckForLayerChanges( CStudioHdr *hdr, float currentTime );

	virtual C_BaseAnimatingOverlay *GetBaseAnimatingOverlay() { return this; }

	// model specific
	virtual void	AccumulateLayers( IBoneSetup &boneSetup, Vector pos[], Quaternion q[], float currentTime );
	virtual void DoAnimationEvents( CStudioHdr *pStudioHdr );

	enum
	{
		MAX_OVERLAYS = 15,
	};

	CUtlVector < C_AnimationLayer >	m_AnimOverlay;

	CUtlVector < CInterpolatedVar< C_AnimationLayer > >	m_iv_AnimOverlay;

	float m_flOverlayPrevEventCycle[ MAX_OVERLAYS ];

private:
	C_BaseAnimatingOverlay( const C_BaseAnimatingOverlay & ); // not defined, not accessible
};


EXTERN_RECV_TABLE(DT_BaseAnimatingOverlay);


#endif // C_BASEANIMATINGOVERLAY_H




