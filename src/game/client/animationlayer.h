//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef ANIMATIONLAYER_H
#define ANIMATIONLAYER_H
#ifdef _WIN32
#pragma once
#endif


#include "rangecheckedvar.h"
#include "lerp_functions.h"
#include "networkvar.h"
#include "ai_activity.h"

#ifdef CLIENT_DLL
class C_BaseAnimatingOverlay;
#endif

class C_AnimationLayer
{
public:

	// This allows the datatables to access private members.
	ALLOW_DATATABLES_PRIVATE_ACCESS();

	C_AnimationLayer();
	void Reset();

	void SetOwner( C_BaseAnimatingOverlay *pOverlay );
	C_BaseAnimatingOverlay *GetOwner() const;

	void SetOrder( int order );

public:

	bool IsActive( void );

	float GetFadeout( float flCurTime );

	void SetSequence( int nSequence );
	void SetCycle( float flCycle );
	void SetPrevCycle( float flCycle );
	void SetPlaybackRate( float flPlaybackRate );
	void SetWeight( float flWeight );
	void SetWeightDeltaRate( float flDelta );

	int   GetOrder() const;
	int   GetSequence() const;
	float GetCycle() const;
	float GetPrevCycle() const;
	float GetPlaybackRate() const;
	float GetWeight() const;
	float GetWeightDeltaRate() const;

	void BlendWeight();

	float	m_flLayerAnimtime;
	float	m_flLayerFadeOuttime;

	// dispatch flags
	CStudioHdr	*m_pDispatchedStudioHdr;
	int		m_nDispatchedSrc;
	int		m_nDispatchedDst;

	float   m_flBlendIn;
	float   m_flBlendOut;

	bool    m_bClientBlend;

private:

	CRangeCheckedVar<int, -1, 65535, 0>	m_nSequence;
	CRangeCheckedVar<float, -2, 2, 0>	m_flPrevCycle;
	CRangeCheckedVar<float, -5, 5, 0>	m_flWeight;
	CRangeCheckedVar<float, -5, 5, 0>	m_flWeightDeltaRate;
	int		m_nOrder;

	// used for automatic crossfades between sequence changes
	CRangeCheckedVar<float, -50, 50, 1>		m_flPlaybackRate;
	CRangeCheckedVar<float, -2, 2, 0>		m_flCycle;

	C_BaseAnimatingOverlay	*m_pOwner;

	friend C_AnimationLayer LoopingLerp( float flPercent, C_AnimationLayer& from, C_AnimationLayer& to );
	friend C_AnimationLayer Lerp( float flPercent, const C_AnimationLayer& from, const C_AnimationLayer& to );
	friend C_AnimationLayer LoopingLerp_Hermite( float flPercent, C_AnimationLayer& prev, C_AnimationLayer& from, C_AnimationLayer& to );
	friend C_AnimationLayer Lerp_Hermite( float flPercent, const C_AnimationLayer& prev, const C_AnimationLayer& from, const C_AnimationLayer& to );
	friend void Lerp_Clamp( C_AnimationLayer &val );

};
#ifdef CLIENT_DLL
	#define CAnimationLayer C_AnimationLayer
#endif

inline C_AnimationLayer::C_AnimationLayer()
{
	m_pOwner = NULL;
	m_pDispatchedStudioHdr = NULL;
	m_nDispatchedSrc = ACT_INVALID;
	m_nDispatchedDst = ACT_INVALID;

	Reset();
}

#ifdef GAME_DLL
inline void C_AnimationLayer::SetSequence( int nSequence )
{
	m_nSequence = nSequence;
}

inline void C_AnimationLayer::SetCycle( float flCycle )
{
	m_flCycle = flCycle;
}

inline void C_AnimationLayer::SetWeight( float flWeight )
{
	m_flWeight = flWeight;
}
#endif

FORCEINLINE void C_AnimationLayer::SetPrevCycle( float flPrevCycle )
{
	m_flPrevCycle = flPrevCycle;
}

FORCEINLINE void C_AnimationLayer::SetPlaybackRate( float flPlaybackRate )
{
	m_flPlaybackRate = flPlaybackRate;
}

FORCEINLINE void C_AnimationLayer::SetWeightDeltaRate( float flDelta )
{
	m_flWeightDeltaRate = flDelta;
}

FORCEINLINE int	C_AnimationLayer::GetSequence( ) const
{
	return m_nSequence;
}

FORCEINLINE float C_AnimationLayer::GetCycle( ) const
{
	return m_flCycle;
}

FORCEINLINE float C_AnimationLayer::GetPrevCycle() const
{
	return m_flPrevCycle;
}

FORCEINLINE float C_AnimationLayer::GetPlaybackRate( ) const
{
	return m_flPlaybackRate;
}

FORCEINLINE float C_AnimationLayer::GetWeight( ) const
{
	return m_flWeight;
}

FORCEINLINE float C_AnimationLayer::GetWeightDeltaRate() const
{
	return m_flWeightDeltaRate;
}

FORCEINLINE int C_AnimationLayer::GetOrder() const
{
	return m_nOrder;
}


inline float C_AnimationLayer::GetFadeout( float flCurTime )
{
	float s;

    if (m_flLayerFadeOuttime <= 0.0f)
	{
		s = 0;
	}
	else
	{
		// blend in over 0.2 seconds
		s = 1.0 - (flCurTime - m_flLayerAnimtime) / m_flLayerFadeOuttime;
		if (s > 0 && s <= 1.0)
		{
			// do a nice spline curve
			s = 3 * s * s - 2 * s * s * s;
		}
		else if ( s > 1.0f )
		{
			// Shouldn't happen, but maybe curtime is behind animtime?
			s = 1.0f;
		}
	}
	return s;
}


inline C_AnimationLayer LoopingLerp( float flPercent, C_AnimationLayer& from, C_AnimationLayer& to )
{
	Assert( from.GetOwner() == to.GetOwner() );

	C_AnimationLayer output;
	
	output.m_nSequence = to.m_nSequence;
	output.m_flCycle = LoopingLerp( flPercent, (float)from.m_flCycle, (float)to.m_flCycle );
	output.m_flPrevCycle = to.m_flPrevCycle;
	output.m_flWeight = Lerp( flPercent, from.m_flWeight, to.m_flWeight );
	output.m_nOrder = to.m_nOrder;

	output.m_flLayerAnimtime = to.m_flLayerAnimtime;
	output.m_flLayerFadeOuttime = to.m_flLayerFadeOuttime;
	output.SetOwner( to.GetOwner() );
	return output;
}

inline C_AnimationLayer Lerp( float flPercent, const C_AnimationLayer& from, const C_AnimationLayer& to )
{
	Assert( from.GetOwner() == to.GetOwner() );

	C_AnimationLayer output;

	output.m_nSequence = to.m_nSequence;
	output.m_flCycle = Lerp( flPercent, from.m_flCycle, to.m_flCycle );
	output.m_flPrevCycle = to.m_flPrevCycle;
	output.m_flWeight = Lerp( flPercent, from.m_flWeight, to.m_flWeight );
	output.m_nOrder = to.m_nOrder;

	output.m_flLayerAnimtime = to.m_flLayerAnimtime;
	output.m_flLayerFadeOuttime = to.m_flLayerFadeOuttime;
	output.SetOwner( to.GetOwner() );
	return output;
}

inline C_AnimationLayer LoopingLerp_Hermite( float flPercent, C_AnimationLayer& prev, C_AnimationLayer& from, C_AnimationLayer& to )
{
	Assert( prev.GetOwner() == from.GetOwner() );
	Assert( from.GetOwner() == to.GetOwner() );

	C_AnimationLayer output;

	output.m_nSequence = to.m_nSequence;
	output.m_flCycle = LoopingLerp_Hermite( flPercent, (float)prev.m_flCycle, (float)from.m_flCycle, (float)to.m_flCycle );
	output.m_flPrevCycle = to.m_flPrevCycle;
	output.m_flWeight = Lerp( flPercent, from.m_flWeight, to.m_flWeight );
	output.m_nOrder = to.m_nOrder;

	output.m_flLayerAnimtime = to.m_flLayerAnimtime;
	output.m_flLayerFadeOuttime = to.m_flLayerFadeOuttime;
	output.SetOwner( to.GetOwner() );
	return output;
}

// YWB:  Specialization for interpolating euler angles via quaternions...
inline C_AnimationLayer Lerp_Hermite( float flPercent, const C_AnimationLayer& prev, const C_AnimationLayer& from, const C_AnimationLayer& to )
{
	Assert( prev.GetOwner() == from.GetOwner() );
	Assert( from.GetOwner() == to.GetOwner() );

	C_AnimationLayer output;

	output.m_nSequence = to.m_nSequence;
	output.m_flCycle = Lerp_Hermite( flPercent, prev.m_flCycle, from.m_flCycle, to.m_flCycle );
	output.m_flPrevCycle = to.m_flPrevCycle;
	output.m_flWeight = Lerp( flPercent, from.m_flWeight, to.m_flWeight );
	output.m_nOrder = to.m_nOrder;

	output.m_flLayerAnimtime = to.m_flLayerAnimtime;
	output.m_flLayerFadeOuttime = to.m_flLayerFadeOuttime;
	output.SetOwner( to.GetOwner() );
	return output;
}

inline void Lerp_Clamp( C_AnimationLayer &val )
{
	Lerp_Clamp( val.m_nSequence );
	Lerp_Clamp( val.m_flCycle );
	Lerp_Clamp( val.m_flPrevCycle );
	Lerp_Clamp( val.m_flWeight );
	Lerp_Clamp( val.m_nOrder );
	Lerp_Clamp( val.m_flLayerAnimtime );
	Lerp_Clamp( val.m_flLayerFadeOuttime );
}

inline void C_AnimationLayer::BlendWeight()
{
	if ( !m_bClientBlend )
		return;

	m_flWeight = 1;

	// blend in?
	if ( m_flBlendIn != 0.0f )
	{
		if (m_flCycle < m_flBlendIn)
		{
			m_flWeight = m_flCycle / m_flBlendIn;
		}
	}

	// blend out?
	if ( m_flBlendOut != 0.0f )
	{
		if (m_flCycle > 1.0 - m_flBlendOut)
		{
			m_flWeight = (1.0 - m_flCycle) / m_flBlendOut;
		}
	}

	m_flWeight = 3.0 * m_flWeight * m_flWeight - 2.0 * m_flWeight * m_flWeight * m_flWeight;
	if (m_nSequence == 0)
		m_flWeight = 0;
}

#endif // ANIMATIONLAYER_H
