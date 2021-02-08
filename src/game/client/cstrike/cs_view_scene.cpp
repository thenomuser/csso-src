//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Responsible for drawing the scene
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "view.h"
#include "iviewrender.h"
#include "view_shared.h"
#include "ivieweffects.h"
#include "iinput.h"
#include "model_types.h"
#include "clientsideeffects.h"
#include "particlemgr.h"
#include "viewrender.h"
#include "iclientmode.h"
#include "voice_status.h"
#include "radio_status.h"
#include "glow_overlay.h"
#include "materialsystem/imesh.h"
#include "materialsystem/itexture.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/imaterialsystemhardwareconfig.h"
#include "detailobjectsystem.h"
#include "tier0/vprof.h"
#include "engine/IEngineTrace.h"
#include "engine/ivmodelinfo.h"
#include "view_scene.h"
#include "particles_ez.h"
#include "engine/IStaticPropMgr.h"
#include "engine/ivdebugoverlay.h"
#include "cs_view_scene.h"
#include "c_cs_player.h"
#include "cs_gamerules.h"
#include "shake.h"
#include "clienteffectprecachesystem.h"
#include "engine/IEngineSound.h"
#include <vgui/ISurface.h>

CLIENTEFFECT_REGISTER_BEGIN( PrecacheCSViewScene )
CLIENTEFFECT_MATERIAL( "effects/flashbang" )
CLIENTEFFECT_MATERIAL( "effects/flashbang_white" )
CLIENTEFFECT_MATERIAL( "effects/nightvision" )
CLIENTEFFECT_REGISTER_END()

static CCSViewRender g_ViewRender;

CCSViewRender::CCSViewRender()
{
	view = ( IViewRender * )&g_ViewRender;
	m_pFlashTexture = NULL;
}

struct ConVarFlags
{
	const char *name;
	int flags;
};

ConVarFlags s_flaggedConVars[] =
{
	{ "r_screenfademinsize", FCVAR_CHEAT },
	{ "r_screenfademaxsize", FCVAR_CHEAT },
	{ "developer", FCVAR_CHEAT },
};

void CCSViewRender::Init( void )
{
	for ( int i=0; i<ARRAYSIZE( s_flaggedConVars ); ++i )
	{
		ConVar *flaggedConVar = cvar->FindVar( s_flaggedConVars[i].name );
		if ( flaggedConVar )
		{
			flaggedConVar->AddFlags( s_flaggedConVars[i].flags );
		}
	}

	CViewRender::Init();
}

//-----------------------------------------------------------------------------
// Purpose: Returns the min/max fade distances
//-----------------------------------------------------------------------------
void CCSViewRender::GetScreenFadeDistances( float *min, float *max )
{
	if ( min )
	{
		*min = 0.0f;
	}

	if ( max )
	{
		*max = 0.0f;
	}
}


void CCSViewRender::PerformNightVisionEffect( const CViewSetup &view )
{
	C_CSPlayer *pPlayer = C_CSPlayer::GetLocalCSPlayer();

	if ( !pPlayer )
		return;

	if (pPlayer->GetObserverMode() == OBS_MODE_IN_EYE)
	{
		CBaseEntity *target = pPlayer->GetObserverTarget();
		if (target && target->IsPlayer())
		{
			pPlayer = (C_CSPlayer *)target;
		}
	}

	if ( pPlayer && pPlayer->m_flNightVisionAlpha > 0 )
	{
		IMaterial *pMaterial = materials->FindMaterial( "effects/nightvision", TEXTURE_GROUP_CLIENT_EFFECTS, true );

		if ( pMaterial )
		{
			int iMaxValue = 255;
			byte overlaycolor[4] = { 0, 255, 0, 255 };
			
			if ( g_pMaterialSystemHardwareConfig->GetDXSupportLevel() >= 80 )
			{
				UpdateScreenEffectTexture( 0, view.x, view.y, view.width, view.height );
			}
			else
			{
				// In DX7, use the values CS:goldsrc uses.
				iMaxValue = 225;
				overlaycolor[0] = overlaycolor[2] = 50 / 2;
				overlaycolor[1] = 225 / 2;
			}

			if ( pPlayer->m_bNightVisionOn )
			{
				pPlayer->m_flNightVisionAlpha += 15;

				pPlayer->m_flNightVisionAlpha = MIN( pPlayer->m_flNightVisionAlpha, iMaxValue );
			}
			else 
			{
				pPlayer->m_flNightVisionAlpha -= 40;

				pPlayer->m_flNightVisionAlpha = MAX( pPlayer->m_flNightVisionAlpha, 0 );
				
			}

			overlaycolor[3] = pPlayer->m_flNightVisionAlpha;
	
			render->ViewDrawFade( overlaycolor, pMaterial );

			// Only one pass in DX7.
			if ( g_pMaterialSystemHardwareConfig->GetDXSupportLevel() >= 80 )
			{
				CMatRenderContextPtr pRenderContext( materials );
				pRenderContext->DrawScreenSpaceQuad( pMaterial );
				render->ViewDrawFade( overlaycolor, pMaterial );
				pRenderContext->DrawScreenSpaceQuad( pMaterial );
			}
		}
	}
}


//Adrian - Super Nifty Flashbang Effect(tm)
// this does the burn in for the flashbang effect.
void CCSViewRender::PerformFlashbangEffect( const CViewSetup &view )
{
	C_CSPlayer *pLocalPlayer = C_CSPlayer::GetLocalCSPlayer();
	if ( pLocalPlayer == NULL )
		 return;

	C_CSPlayer *pFlashBangPlayer = pLocalPlayer;
	//bool bReduceEffect = false;

	float flAlphaScale = 1.0f;
	if ( pLocalPlayer->GetObserverMode() != OBS_MODE_NONE )
	{
		// If spectating, use values from target
		CBaseEntity *pTarget = pLocalPlayer->GetObserverTarget();
		if ( pTarget )
		{
			C_CSPlayer *pPlayerTmp = ToCSPlayer(pTarget);
			if ( pPlayerTmp )
			{
				pFlashBangPlayer = pPlayerTmp;
			}
		}

		// reduce the effect
		if ( pLocalPlayer->GetObserverMode() == OBS_MODE_IN_EYE && CanSeeSpectatorOnlyTools() )
		{
			// Reduce alpha.
			// Note: the logic to reduce the duration of the effect is done when the effect is started (RecvProxy_FlashTime).
			flAlphaScale = 0.6f;
		}
		else if ( pLocalPlayer->GetObserverMode() == OBS_MODE_FIXED || pLocalPlayer->GetObserverMode() == OBS_MODE_ROAMING )
		{
			// Reduce alpha.
			// Note: the logic to reduce the duration of the effect is done when the effect is started (RecvProxy_FlashTime).
			flAlphaScale = 0.2f;
		}
		else if ( pLocalPlayer->GetObserverMode() != OBS_MODE_IN_EYE )
		{
			// Reduce alpha.
			// Note: the logic to reduce the duration of the effect is done when the effect is started (RecvProxy_FlashTime).
			flAlphaScale = 0.6f;
		}
	}

	if ( !pFlashBangPlayer->IsFlashBangActive() || CSGameRules()->IsIntermission() )
	{
		if ( !pLocalPlayer->m_bFlashDspHasBeenCleared )
		{
			CLocalPlayerFilter filter;
			enginesound->SetPlayerDSP( filter, 0, true );
			pLocalPlayer->m_bFlashDspHasBeenCleared = true;
		}
		return;
	}

	// bandaid to insure that flashbang dsp effect doesn't continue on indefinitely
	if( pFlashBangPlayer->GetFlashTimeElapsed() > 1.6f && !pLocalPlayer->m_bFlashDspHasBeenCleared )
	{
		CLocalPlayerFilter filter;
		enginesound->SetPlayerDSP( filter, 0, true );
		pLocalPlayer->m_bFlashDspHasBeenCleared = true;
	}

	byte overlaycolor[4] = { 255, 255, 255, 255 };

	// draw the screenshot overlay portion of the flashbang effect
	IMaterial *pMaterial = materials->FindMaterial( "effects/flashbang", TEXTURE_GROUP_CLIENT_EFFECTS, true );
	if ( pMaterial )
	{
		// This is for handling split screen where we could potentially enter this function more than once a frame.
		// Since this bit of code grabs both the left and right viewports of the buffer, it only needs to be done once per frame per flash.
		static float lastTimeGrabbed = 0.0f;
		if ( gpGlobals->curtime == lastTimeGrabbed )
		{
			pLocalPlayer->m_bFlashScreenshotHasBeenGrabbed = true;
			pFlashBangPlayer->m_bFlashScreenshotHasBeenGrabbed = true;
		}

		if ( !pFlashBangPlayer->m_bFlashScreenshotHasBeenGrabbed )
		{
			CMatRenderContextPtr pRenderContext( materials );
			int nScreenWidth, nScreenHeight;
			pRenderContext->GetRenderTargetDimensions( nScreenWidth, nScreenHeight );

			// update m_pFlashTexture
			lastTimeGrabbed = gpGlobals->curtime;
			bool foundVar;

			IMaterialVar* m_BaseTextureVar = pMaterial->FindVar( "$basetexture", &foundVar, false );
			m_pFlashTexture = GetFullFrameFrameBufferTexture( 1 );

			// When grabbing the texture for the super imposed frame, we grab the whole buffer, not just the viewport.
			// We were having issues with trying to grab only the right side of the buffer for the second player in split screen.
			Rect_t srcRect;
			srcRect.x = 0;
			srcRect.y = 0;
			srcRect.width = nScreenWidth;
			srcRect.height = nScreenHeight;
			m_BaseTextureVar->SetTextureValue( m_pFlashTexture );
			pRenderContext->CopyRenderTargetToTextureEx( m_pFlashTexture, 0, &srcRect, NULL );
			pRenderContext->SetFrameBufferCopyTexture( m_pFlashTexture );

			pFlashBangPlayer->m_bFlashScreenshotHasBeenGrabbed = true;
			pLocalPlayer->m_bFlashScreenshotHasBeenGrabbed = true;
		}

		if ( !CanSeeSpectatorOnlyTools() )
		{
			overlaycolor[0] = overlaycolor[1] = overlaycolor[2] = pFlashBangPlayer->m_flFlashScreenshotAlpha * flAlphaScale;
			if ( m_pFlashTexture != NULL )
			{
				static int NUM_AFTER_IMAGE_PASSES = 4;
				for ( int pass = 0; pass < NUM_AFTER_IMAGE_PASSES; ++pass )
				{
					render->ViewDrawFade( overlaycolor, pMaterial, false );
				}
			}
		}
	}

	// draw pure white overlay part of the flashbang effect.
	pMaterial = materials->FindMaterial( "effects/flashbang_white", TEXTURE_GROUP_CLIENT_EFFECTS, true );
	if ( pMaterial )
	{
		overlaycolor[0] = overlaycolor[1] = overlaycolor[2] = pFlashBangPlayer->m_flFlashOverlayAlpha * flAlphaScale;
		render->ViewDrawFade( overlaycolor, pMaterial );
	}
}


//-----------------------------------------------------------------------------
// Purpose: Renders extra 2D effects in derived classes while the 2D view is on the stack
//-----------------------------------------------------------------------------
void CCSViewRender::Render2DEffectsPreHUD( const CViewSetup &view )
{
	PerformNightVisionEffect( view );	// this needs to come before the HUD is drawn, or it will wash the HUD out
}


//-----------------------------------------------------------------------------
// Purpose: Renders extra 2D effects in derived classes while the 2D view is on the stack
//-----------------------------------------------------------------------------
void CCSViewRender::Render2DEffectsPostHUD( const CViewSetup &view )
{
	PerformFlashbangEffect( view );
}


//-----------------------------------------------------------------------------
// Purpose: Renders voice feedback and other sprites attached to players
// Input  : none
//-----------------------------------------------------------------------------
void CCSViewRender::RenderPlayerSprites()
{
	GetClientVoiceMgr()->SetHeadLabelOffset( 40 );

	CViewRender::RenderPlayerSprites();
	RadioManager()->DrawHeadLabels();
}

