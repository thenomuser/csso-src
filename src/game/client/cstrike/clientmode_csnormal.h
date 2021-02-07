//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef CS_CLIENTMODE_H
#define CS_CLIENTMODE_H
#ifdef _WIN32
#pragma once
#endif

#include "clientmode_shared.h"
#include "counterstrikeviewport.h"
#include "colorcorrectionmgr.h"

class ClientModeCSNormal : public ClientModeShared 
{
DECLARE_CLASS( ClientModeCSNormal, ClientModeShared );

private:

// IClientMode overrides.
public:

					ClientModeCSNormal();

	virtual void	Init();
	virtual void	InitViewport();
	virtual void	Update();

	virtual int		KeyInput( int down, ButtonCode_t keynum, const char *pszCurrentBinding );

	virtual float	GetViewModelFOV( void );

	int				GetDeathMessageStartHeight( void );

	virtual void	FireGameEvent( IGameEvent *event );
	virtual void	PostRenderVGui();

	virtual bool	ShouldDrawViewModel( void );

	virtual bool	CanRecordDemo( char *errorMsg, int length ) const;

	// [menglish] Save server information shown to the client in a persistent place
	virtual wchar_t* GetServerName() { return m_pServerName; }
	virtual void SetServerName(wchar_t* name);
	virtual wchar_t* GetMapName() { return m_pMapName; }
	virtual void SetMapName(wchar_t* name);

	virtual void	UpdateColorCorrectionWeights( void );
	virtual void	OnColorCorrectionWeightsReset( void );

private:
	wchar_t			m_pServerName[256];
	wchar_t			m_pMapName[256];
	
	ClientCCHandle_t	m_CCDeathHandle;	// handle to death cc effect
	float				m_CCDeathPercent;
	ClientCCHandle_t	m_CCFreezePeriodHandle_CT;
	float				m_CCFreezePeriodPercent_CT;
	ClientCCHandle_t	m_CCFreezePeriodHandle_T;
	float				m_CCFreezePeriodPercent_T;

	float				m_fDelayedCTWinTime;
};


extern IClientMode *GetClientModeNormal();
extern ClientModeCSNormal* GetClientModeCSNormal();


#endif // CS_CLIENTMODE_H
