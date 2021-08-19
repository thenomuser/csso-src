//=============== Copyright © Valve Corporation, All rights reserved. =================//
//
//=====================================================================================//

#include "BaseVSShader.h"

#include "character_vs30.inc"
#include "character_ps30.inc"
#include "commandbuilder.h"
#include "cpp_shader_constant_register_map.h"

#include "../materialsystem_global.h"

// NOTE: This has to be the last file included!
#include "tier0/memdbgon.h"

//#define CHARACTER_LIMIT_LIGHTS_WITH_PHONGWARP 1

ConVar mat_disable_character_shader( "mat_disable_character_shader", "0", FCVAR_ARCHIVE, "Makes Character shader automatically fallback to VertexLitGeneric" );

BEGIN_VS_SHADER( Character, "Help for Character Shader" )
	BEGIN_SHADER_PARAMS

		SHADER_PARAM( BUMPMAP, SHADER_PARAM_TYPE_TEXTURE, "", "" )
		SHADER_PARAM( MASKS1, SHADER_PARAM_TYPE_TEXTURE, "", "" )
		SHADER_PARAM( METALNESS, SHADER_PARAM_TYPE_FLOAT, "", "" )
		SHADER_PARAM( WARPINDEX, SHADER_PARAM_TYPE_FLOAT, "", "" )

		// Phong terms
		SHADER_PARAM( PHONGBOOST, SHADER_PARAM_TYPE_FLOAT, "", "" )
		SHADER_PARAM( PHONGALBEDOBOOST, SHADER_PARAM_TYPE_FLOAT, "", "" )
		SHADER_PARAM( PHONGTINT, SHADER_PARAM_TYPE_COLOR, "", "" )
		SHADER_PARAM( PHONGWARPTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "" )
		SHADER_PARAM( PHONGEXPONENT, SHADER_PARAM_TYPE_FLOAT, "", "" )
		SHADER_PARAM( BASEALPHAPHONGMASK, SHADER_PARAM_TYPE_BOOL, "", "" )

		// Envmap terms
		SHADER_PARAM( ENVMAP, SHADER_PARAM_TYPE_TEXTURE, "", "" )
		SHADER_PARAM( ENVMAPCONTRAST, SHADER_PARAM_TYPE_FLOAT, "", "" )
		SHADER_PARAM( ENVMAPSATURATION, SHADER_PARAM_TYPE_FLOAT, "", "" )
		SHADER_PARAM( ENVMAPTINT, SHADER_PARAM_TYPE_COLOR, "", "" )
		SHADER_PARAM( BASEALPHAENVMASK, SHADER_PARAM_TYPE_BOOL, "", "" )
		SHADER_PARAM( BUMPALPHAENVMASK, SHADER_PARAM_TYPE_BOOL, "", "" )
		SHADER_PARAM( ENVMAPLIGHTSCALE, SHADER_PARAM_TYPE_FLOAT, "", "" )
		SHADER_PARAM( ENVMAPLIGHTSCALEMINMAX, SHADER_PARAM_TYPE_VEC2, "", "" )

		// Phong and envmap
		SHADER_PARAM( FRESNELRANGESTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "" )
		SHADER_PARAM( FRESNELRANGES, SHADER_PARAM_TYPE_VEC3, "", "" )
		SHADER_PARAM( MASKS2, SHADER_PARAM_TYPE_TEXTURE, "", "" )
		SHADER_PARAM( ANISOTROPYAMOUNT, SHADER_PARAM_TYPE_FLOAT, "", "" )

		// Rim lighting terms
		SHADER_PARAM( RIMLIGHTEXPONENT, SHADER_PARAM_TYPE_FLOAT, "4.0", "Exponent for rim lights" )
		SHADER_PARAM( RIMLIGHTBOOST, SHADER_PARAM_TYPE_FLOAT, "1.0", "Boost for rim lights" )
		SHADER_PARAM( RIMLIGHTALBEDO, SHADER_PARAM_TYPE_FLOAT, "0.0", "" )
		SHADER_PARAM( RIMLIGHTTINT, SHADER_PARAM_TYPE_COLOR, "", "" )
		SHADER_PARAM( SHADOWRIMBOOST, SHADER_PARAM_TYPE_FLOAT, "2.0f", "Extra boost for rim lights in shadow" )
		SHADER_PARAM( FAKERIMBOOST, SHADER_PARAM_TYPE_FLOAT, "", "" )
		SHADER_PARAM( FAKERIMTINT, SHADER_PARAM_TYPE_COLOR, "", "" )
		SHADER_PARAM( RIMHALOBOOST, SHADER_PARAM_TYPE_FLOAT, "", "" )
		SHADER_PARAM( RIMHALOBOUNDS, SHADER_PARAM_TYPE_VEC4, "", "" )

		// Ambient reflection terms
		SHADER_PARAM( AMBIENTREFLECTIONBOOST, SHADER_PARAM_TYPE_FLOAT, "", "" )
		SHADER_PARAM( AMBIENTREFLECTIONBOUNCECOLOR, SHADER_PARAM_TYPE_COLOR, "", "" )
		SHADER_PARAM( AMBIENTREFLECTIONBOUNCECENTER, SHADER_PARAM_TYPE_VEC3, "", "" )

		// Diffuse shading params
		SHADER_PARAM( SHADOWSATURATION, SHADER_PARAM_TYPE_FLOAT, "", "" )
		SHADER_PARAM( SHADOWSATURATIONBOUNDS, SHADER_PARAM_TYPE_VEC4, "", "" )
		SHADER_PARAM( SHADOWTINT, SHADER_PARAM_TYPE_VEC4, "", "Color and alpha" )
		SHADER_PARAM( SHADOWCONTRAST, SHADER_PARAM_TYPE_FLOAT, "", "" )

		// Self-illum
		SHADER_PARAM( SELFILLUMBOOST, SHADER_PARAM_TYPE_FLOAT, "", "" )
		SHADER_PARAM( BASEALPHASELFILLUMMASK, SHADER_PARAM_TYPE_BOOL, "", "" )

		SHADER_PARAM( ENTITYORIGIN, SHADER_PARAM_TYPE_VEC3, "", "" )
	
	END_SHADER_PARAMS

	SHADER_INIT_PARAMS()
	{
		if ( !params[PHONGBOOST]->IsDefined() )
		{
			params[PHONGBOOST]->SetFloatValue( 1.0f );
		}

		if ( !params[PHONGEXPONENT]->IsDefined() )
		{
			params[PHONGEXPONENT]->SetFloatValue( 1.0f );
		}

		if ( !params[PHONGALBEDOBOOST]->IsDefined() )
		{
			params[PHONGALBEDOBOOST]->SetFloatValue( 1.0f );
		}

		if ( !params[PHONGTINT]->IsDefined() )
		{
			params[PHONGTINT]->SetVecValue( 1.0f, 1.0f, 1.0f );
		}

		if ( !params[BASEALPHAPHONGMASK]->IsDefined() )
		{
			params[BASEALPHAPHONGMASK]->SetIntValue( 0 );
		}

		if ( !params[ENVMAPCONTRAST]->IsDefined() )
		{
			params[ENVMAPCONTRAST]->SetFloatValue( 0.0f );
		}

		if ( !params[ENVMAPSATURATION]->IsDefined() )
		{
			params[ENVMAPSATURATION]->SetFloatValue( 0.0 );
		}

		if ( !params[ENVMAPTINT]->IsDefined() )
		{
			params[ENVMAPTINT]->SetVecValue( 1.0f, 1.0f, 1.0f );
		}

		if ( !params[BASEALPHAENVMASK]->IsDefined() )
		{
			params[BASEALPHAENVMASK]->SetIntValue( 0 );
		}

		if ( !params[BUMPALPHAENVMASK]->IsDefined() )
		{
			params[BUMPALPHAENVMASK]->SetIntValue( 0 );
		}

		if ( !params[SHADOWSATURATION]->IsDefined() )
		{
			params[SHADOWSATURATION]->SetFloatValue( 0.0f );
		}

		if ( !params[SHADOWSATURATIONBOUNDS]->IsDefined() )
		{
			params[SHADOWSATURATIONBOUNDS]->SetVecValue( 0.4f, 0.5f, 0.5f, 0.6f );
		}

		if ( !params[SHADOWTINT]->IsDefined() )
		{
			params[SHADOWTINT]->SetVecValue( 1.0f, 1.0f, 1.0f );
		}

		if ( !params[RIMLIGHTEXPONENT]->IsDefined() )
		{
			params[RIMLIGHTEXPONENT]->SetFloatValue( 4.0f );
		}

		if ( !params[RIMLIGHTBOOST]->IsDefined() )
		{
			params[RIMLIGHTBOOST]->SetFloatValue( 0.0f );
		}

		if ( !params[RIMLIGHTALBEDO]->IsDefined() )
		{
			params[RIMLIGHTALBEDO]->SetFloatValue( 0.0f );
		}

		if ( !params[AMBIENTREFLECTIONBOOST]->IsDefined() )
		{
			params[AMBIENTREFLECTIONBOOST]->SetFloatValue( 0.0f );
		}

		if ( !params[AMBIENTREFLECTIONBOUNCECOLOR]->IsDefined() )
		{
			params[AMBIENTREFLECTIONBOUNCECOLOR]->SetVecValue( 0.0f, 0.0f, 0.0f );
		}

		if ( !params[AMBIENTREFLECTIONBOUNCECENTER]->IsDefined() )
		{
			params[AMBIENTREFLECTIONBOUNCECENTER]->SetVecValue( 0.0f, 42.0f, 0.0f );
		}

		if ( !params[FRESNELRANGES]->IsDefined() )
		{
			params[FRESNELRANGES]->SetVecValue( 1.0f, 1.0f, 1.0f );
		}

		// Metalness is just a multiply on the albedo, so invert the defined amount to get the multiplier
		if ( !params[METALNESS]->IsDefined() )
		{
			params[METALNESS]->SetFloatValue( 1.0f );
		}
		else
		{
			params[METALNESS]->SetFloatValue( 1 - clamp( params[METALNESS]->GetFloatValue(), 0.0f, 1.0f ) );
		}

		if ( !params[ENVMAPLIGHTSCALE]->IsDefined() )
		{
			params[ENVMAPLIGHTSCALE]->SetFloatValue( 0.0f );
		}

		if ( !params[ENVMAPLIGHTSCALEMINMAX]->IsDefined() )
		{
			params[ENVMAPLIGHTSCALEMINMAX]->SetVecValue( 0.0f, 1.0f );
		}

		if ( !params[RIMLIGHTTINT]->IsDefined() )
		{
			params[RIMLIGHTTINT]->SetVecValue( 1.0f, 1.0f, 1.0f );
		}

		if ( !params[WARPINDEX]->IsDefined() )
		{
			params[WARPINDEX]->SetFloatValue( 0.0f );
		}

		if ( !params[SHADOWCONTRAST]->IsDefined() )
		{
			params[SHADOWCONTRAST]->SetFloatValue( 0.0f );
		}

		if ( !params[FAKERIMTINT]->IsDefined() )
		{
			params[FAKERIMTINT]->SetVecValue( 1.0f, 1.0f, 1.0f );
		}

		if ( !params[RIMHALOBOOST]->IsDefined() )
		{
			params[RIMHALOBOOST]->SetFloatValue( 0.0f );
		}

		if ( !params[RIMHALOBOUNDS]->IsDefined() )
		{
			params[RIMHALOBOUNDS]->SetVecValue( 0.4f, 0.5f, 0.5f, 0.6f );
		}

		if ( !params[SHADOWRIMBOOST]->IsDefined() )
		{ 
			params[SHADOWRIMBOOST]->SetIntValue( 2.0f );
		}

		SET_FLAGS2( MATERIAL_VAR2_NEEDS_TANGENT_SPACES );
		SET_FLAGS2( MATERIAL_VAR2_SUPPORTS_HW_SKINNING );
		SET_FLAGS2( MATERIAL_VAR2_LIGHTING_VERTEX_LIT );
	}

	SHADER_FALLBACK
	{
		// Character shader only works on shader model 3.0
		if (!g_pHardwareConfig->SupportsShaderModel_3_0())
			return "VertexLitGeneric";

		if ( mat_disable_character_shader.GetBool() )
			return "VertexLitGeneric";

		return 0;
	}

	SHADER_INIT
	{
		LoadTexture( BASETEXTURE );

		if ( params[BUMPMAP]->IsDefined() )
		{
			LoadTexture( BUMPMAP );
		}

		if ( params[MASKS1]->IsDefined() )
		{
			LoadTexture( MASKS1 );
		}

		if ( params[PHONGWARPTEXTURE]->IsDefined() )
		{
			LoadTexture( PHONGWARPTEXTURE );
		}

		if ( params[FRESNELRANGESTEXTURE]->IsDefined() )
		{
			LoadTexture( FRESNELRANGESTEXTURE );
		}

		if ( params[MASKS2]->IsDefined() )
		{
			LoadTexture( MASKS2 );
		}

		if ( params[ENVMAP]->IsDefined() )
		{
			LoadCubeMap( ENVMAP );
		}

	}

	SHADER_DRAW
	{
		bool bIsTranslucent = IS_FLAG_SET( MATERIAL_VAR_TRANSLUCENT ) != 0;
		
		bool bHasMasks1 = IsTextureSet( MASKS1, params );
		bool bHasFresnelRangesTexture = IsTextureSet( FRESNELRANGESTEXTURE, params );
		bool bHasPhongWarpTexture = IsTextureSet( PHONGWARPTEXTURE, params );
		bool bHasBumpMap = IsTextureSet( BUMPMAP, params );
		bool bHasEnvmap = params[ENVMAP]->IsDefined();

		bool bHasAmbientReflection = params[AMBIENTREFLECTIONBOOST]->IsDefined() && ( params[AMBIENTREFLECTIONBOOST]->GetFloatValue() > 0 );
		bool bHasBounceColor = bHasAmbientReflection && params[AMBIENTREFLECTIONBOUNCECOLOR]->IsDefined();
		
		//bool bHasRim = params[RIMLIGHTBOOST]->IsDefined() && ( params[RIMLIGHTBOOST]->GetFloatValue() > 0 );
		
		bool bBaseAlphaSelfIllumMask = !bIsTranslucent && params[BASEALPHASELFILLUMMASK]->IsDefined() && ( params[BASEALPHASELFILLUMMASK]->GetIntValue() > 0 );

		// Phong mask uses bump alpha by default, but can use base alpha if specified
		bool bBaseAlphaPhongMask = !bBaseAlphaSelfIllumMask && !bIsTranslucent && params[BASEALPHAPHONGMASK]->IsDefined() && ( params[BASEALPHAPHONGMASK]->GetIntValue() > 0 );

		bool bHasAnisotropy = params[ANISOTROPYAMOUNT]->IsDefined() && ( params[ANISOTROPYAMOUNT]->GetFloatValue() > 0 );
		bool bHasShadowSaturation = params[SHADOWSATURATION]->IsDefined() && ( params[SHADOWSATURATION]->GetFloatValue() > 0 );
		bool bHasMasks2 = IsTextureSet( MASKS2, params ) && ( bHasEnvmap || bHasShadowSaturation || bHasAnisotropy );

		// Envmap uses same mask as spec
		bool bBaseAlphaEnvMask = bHasEnvmap && bBaseAlphaPhongMask;
		bool bBumpAlphaEnvMask = bHasEnvmap && !bBaseAlphaPhongMask;
		// Unless we've specified it should be different.
			 bBaseAlphaEnvMask = !bBaseAlphaSelfIllumMask && ( bBaseAlphaEnvMask || ( bHasEnvmap && !bIsTranslucent && ( params[BASEALPHAENVMASK]->IsDefined() && ( params[BASEALPHAENVMASK]->GetIntValue() > 0 ) ) ) );
			 bBumpAlphaEnvMask = bHasEnvmap && bHasBumpMap && ( params[BUMPALPHAENVMASK]->IsDefined() && ( params[BUMPALPHAENVMASK]->GetIntValue() > 0 ) );
		// Can't have both.
			 bBaseAlphaEnvMask = bBaseAlphaEnvMask && !bBumpAlphaEnvMask;
		bool bHasFakeRim = params[FAKERIMBOOST]->IsDefined() && ( params[FAKERIMBOOST]->GetFloatValue() > 0 );

		bool bHasFlashlight = UsingFlashlight( params );

        bool bUseStaticControlFlow = g_pHardwareConfig->SupportsStaticControlFlow();
            
		SHADOW_STATE
		{
			SetInitialShadowState();

			// Set stream format
			int userDataSize = 4; // tangent S
			unsigned int flags = VERTEX_POSITION | VERTEX_NORMAL | VERTEX_FORMAT_COMPRESSED;

			
			int nTexCoordCount = 1;
			pShaderShadow->VertexShaderVertexFormat( flags, nTexCoordCount, NULL, userDataSize );


			int nShadowFilterMode = g_pHardwareConfig->GetShadowFilterMode();	// Based upon vendor and device dependent formats

			// Vertex Shader
			DECLARE_STATIC_VERTEX_SHADER( character_vs30 );
			SET_STATIC_VERTEX_SHADER_COMBO( USEBOUNCECOLOR, bHasBounceColor );
			SET_STATIC_VERTEX_SHADER_COMBO( FLASHLIGHT, bHasFlashlight );
            SET_STATIC_VERTEX_SHADER_COMBO( FLATTEN_STATIC_CONTROL_FLOW, !bUseStaticControlFlow );
			SET_STATIC_VERTEX_SHADER( character_vs30 );

			// Pixel Shader
			DECLARE_STATIC_PIXEL_SHADER( character_ps30 );
			SET_STATIC_PIXEL_SHADER_COMBO( MASKS1, bHasMasks1 );
			SET_STATIC_PIXEL_SHADER_COMBO( MASKS2, bHasMasks2 );
			SET_STATIC_PIXEL_SHADER_COMBO( FRESNELRANGESTEXTURE, bHasFresnelRangesTexture );
			SET_STATIC_PIXEL_SHADER_COMBO( PHONGWARPTEXTURE, bHasPhongWarpTexture );
			SET_STATIC_PIXEL_SHADER_COMBO( ENVMAP, bHasEnvmap );
			SET_STATIC_PIXEL_SHADER_COMBO( AMBIENTREFLECTION, bHasAmbientReflection );
			SET_STATIC_PIXEL_SHADER_COMBO( USEBOUNCECOLOR, bHasBounceColor );
			SET_STATIC_PIXEL_SHADER_COMBO( ANISOTROPY, bHasAnisotropy );
			SET_STATIC_PIXEL_SHADER_COMBO( BASEALPHAPHONGMASK, bBaseAlphaPhongMask );
			SET_STATIC_PIXEL_SHADER_COMBO( BASEALPHAENVMASK, bBaseAlphaEnvMask );
			SET_STATIC_PIXEL_SHADER_COMBO( BUMPALPHAENVMASK, bBumpAlphaEnvMask );
			SET_STATIC_PIXEL_SHADER_COMBO( SHADOWSATURATION, bHasShadowSaturation );
			SET_STATIC_PIXEL_SHADER_COMBO( BASEALPHASELFILLUMMASK, bBaseAlphaSelfIllumMask );
			SET_STATIC_PIXEL_SHADER_COMBO( FAKERIM, bHasFakeRim && !bHasFlashlight );
			SET_STATIC_PIXEL_SHADER_COMBO( FLASHLIGHT, bHasFlashlight );
			SET_STATIC_PIXEL_SHADER_COMBO( FLASHLIGHTDEPTHFILTERMODE, nShadowFilterMode );
			SET_STATIC_PIXEL_SHADER( character_ps30 );

			// Base texture
			pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
			// Bump map
			pShaderShadow->EnableTexture( SHADER_SAMPLER1, true );
			if ( bHasMasks2 )
			{
				pShaderShadow->EnableTexture( SHADER_SAMPLER3, true );
			}
			if ( bHasFresnelRangesTexture )
			{
				pShaderShadow->EnableTexture( SHADER_SAMPLER4, true );
			}
			if ( bHasPhongWarpTexture )
			{
				pShaderShadow->EnableTexture( SHADER_SAMPLER7, true );
			}
			if ( bHasEnvmap )
			{
				pShaderShadow->EnableTexture( SHADER_SAMPLER6, true );
			}
			// Normalize sampler
			pShaderShadow->EnableTexture( SHADER_SAMPLER5, true );
			if ( bHasFlashlight )
			{
				// Flashlight sampler
				pShaderShadow->EnableTexture( SHADER_SAMPLER8, true );
			}
			if ( bHasMasks1 )
			{
				pShaderShadow->EnableTexture( SHADER_SAMPLER2, true );
			}

			if ( bHasFlashlight )
			{
				pShaderShadow->EnableTexture( SHADER_SAMPLER9, true );
			}

			if ( bIsTranslucent || IsAlphaModulating() )
			{
				EnableAlphaBlending( SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA );
			}
			else
			{
				pShaderShadow->EnableAlphaWrites( false );
				pShaderShadow->EnableDepthWrites( true );
			}
			pShaderShadow->EnableAlphaTest( IS_FLAG_SET(MATERIAL_VAR_ALPHATEST) );

			pShaderShadow->EnableSRGBWrite( true );

			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER0, true );
			if ( bHasPhongWarpTexture )
			{
				pShaderShadow->EnableSRGBRead( SHADER_SAMPLER7, true );
			}
			if ( bHasEnvmap )
			{
				bool bHDR = (g_pHardwareConfig->GetHDRType() != HDR_TYPE_NONE);
				pShaderShadow->EnableSRGBRead( SHADER_SAMPLER6, bHDR ? false : true );
			}

			DefaultFog();
			bool bFullyOpaque = !bIsTranslucent && !( IS_FLAG_SET( MATERIAL_VAR_ALPHATEST ) );
			// HACK HACK HACK - enable alpha writes all the time so that we have them for underwater stuff
			pShaderShadow->EnableAlphaWrites( bFullyOpaque );

		}
		DYNAMIC_STATE
		{
			pShaderAPI->SetDefaultState();

			BindTexture( SHADER_SAMPLER0, BASETEXTURE, -1 );

			pShaderAPI->SetPixelShaderStateAmbientLightCube( PSREG_AMBIENT_CUBE );
			pShaderAPI->SetVertexShaderStateAmbientLightCube();
			pShaderAPI->CommitPixelShaderLighting( PSREG_LIGHT_INFO_ARRAY );
			SetModulationPixelShaderDynamicState_LinearColorSpace( 1 );

			if ( bHasBumpMap )
			{
				BindTexture( SHADER_SAMPLER1, BUMPMAP, -1 );
			}
			else
			{
				pShaderAPI->BindStandardTexture( SHADER_SAMPLER1, TEXTURE_NORMALMAP_FLAT );
			}
			if ( bHasMasks1 )
			{
				BindTexture( SHADER_SAMPLER2, MASKS1, -1 );
			}
			if ( bHasMasks2 )
			{
				BindTexture( SHADER_SAMPLER3, MASKS2, -1 );
			}
			if ( bHasFresnelRangesTexture )
			{
				BindTexture( SHADER_SAMPLER4, FRESNELRANGESTEXTURE, -1 );
			}
			if ( bHasPhongWarpTexture )
			{
				BindTexture( SHADER_SAMPLER7, PHONGWARPTEXTURE, -1 );
			}
			if ( bHasEnvmap )
			{
				BindTexture( SHADER_SAMPLER6, ENVMAP, -1 );
			}
			if (bHasFlashlight)
            {
                VMatrix worldToTexture;
                float atten[4], pos[4], tweaks[4];
                ITexture* pFlashlightDepthTexture;
                const FlashlightState_t& flashlightState = pShaderAPI->GetFlashlightStateEx(worldToTexture, &pFlashlightDepthTexture);
                SetFlashLightColorFromState(flashlightState, pShaderAPI, PSREG_FLASHLIGHT_COLOR);
                BindTexture(SHADER_SAMPLER8, flashlightState.m_pSpotlightTexture, flashlightState.m_nSpotlightTextureFrame);
                if (pFlashlightDepthTexture && g_pConfig->ShadowDepthTexture() && flashlightState.m_bEnableShadows)
                {
                    BindTexture(SHADER_SAMPLER9, pFlashlightDepthTexture, 0);
                    pShaderAPI->BindStandardTexture(SHADER_SAMPLER5, TEXTURE_SHADOW_NOISE_2D);
                }
                
                // Set the flashlight attenuation factors
                atten[0] = flashlightState.m_fConstantAtten;
                atten[1] = flashlightState.m_fLinearAtten;
                atten[2] = flashlightState.m_fQuadraticAtten;
                atten[3] = flashlightState.m_FarZ;
                pShaderAPI->SetPixelShaderConstant(PSREG_FLASHLIGHT_ATTENUATION, atten, 1);

                // Set the flashlight origin
                pos[0] = flashlightState.m_vecLightOrigin[0];
                pos[1] = flashlightState.m_vecLightOrigin[1];
                pos[2] = flashlightState.m_vecLightOrigin[2];
                pShaderAPI->SetPixelShaderConstant(PSREG_FLASHLIGHT_POSITION_RIM_BOOST, pos, 1);

                pShaderAPI->SetPixelShaderConstant(PSREG_FLASHLIGHT_TO_WORLD_TEXTURE, worldToTexture.Base(), 4, false);

                // Tweaks associated with a given flashlight
                tweaks[0] = ShadowFilterFromState(flashlightState);
                tweaks[1] = ShadowAttenFromState(flashlightState);
                HashShadow2DJitter(flashlightState.m_flShadowJitterSeed, &tweaks[2], &tweaks[3]);
                pShaderAPI->SetPixelShaderConstant(109, tweaks, 1, false);

                {
                    float vScreenScale[4] = { 1280.0f / 32.0f, 720.0f / 32.0f, 0, 0 };
                    int nWidth, nHeight;
                    pShaderAPI->GetBackBufferDimensions(nWidth, nHeight);
                    vScreenScale[0] = (float)nWidth / 32.0f;
                    vScreenScale[1] = (float)nHeight / 32.0f;
                    pShaderAPI->SetPixelShaderConstant(PSREG_FLASHLIGHT_SCREEN_SCALE, vScreenScale, 1);
                }
            }

			int numBones = pShaderAPI->GetCurrentNumBones();
			LightState_t lightState = { 0, false, false };
			pShaderAPI->GetDX9LightState( &lightState );

			MaterialFogMode_t fogType = pShaderAPI->GetSceneFogMode();
			bool bWriteDepthToAlpha;
			bool bWriteWaterFogToAlpha;
			bool bFullyOpaque = !bIsTranslucent && !( IS_FLAG_SET( MATERIAL_VAR_VERTEXALPHA ) ) && !( IS_FLAG_SET( MATERIAL_VAR_ALPHATEST ) );
			if( bFullyOpaque ) 
			{
				bWriteDepthToAlpha = false;
				bWriteWaterFogToAlpha = (fogType == MATERIAL_FOG_LINEAR_BELOW_FOG_Z);
				AssertMsg( !(bWriteDepthToAlpha && bWriteWaterFogToAlpha), "Can't write two values to alpha at the same time." );
			}
			else
			{
				//can't write a special value to dest alpha if we're actually using as-intended alpha
				bWriteDepthToAlpha = false;
				bWriteWaterFogToAlpha = false;
			}

			int nNumLights = lightState.m_nNumLights;
            
            nNumLights = bUseStaticControlFlow ? nNumLights : MIN( 2, nNumLights );

			#if defined( CHARACTER_LIMIT_LIGHTS_WITH_PHONGWARP )
				if ( bHasPhongWarpTexture ) 
					nNumLights = clamp( nNumLights, 0, 3 );
			#endif

			// Vertex Shader
			DECLARE_DYNAMIC_VERTEX_SHADER( character_vs30 );
			SET_DYNAMIC_VERTEX_SHADER_COMBO( SKINNING, numBones > 0 );
			SET_DYNAMIC_VERTEX_SHADER_COMBO( COMPRESSED_VERTS, (int)vertexCompression );
			SET_DYNAMIC_VERTEX_SHADER_COMBO( NUM_LIGHTS, nNumLights );
			SET_DYNAMIC_VERTEX_SHADER( character_vs30 );
			
			SetVertexShaderConstant( VERTEX_SHADER_SHADER_SPECIFIC_CONST_0, AMBIENTREFLECTIONBOUNCECENTER );
			SetVertexShaderConstant( VERTEX_SHADER_SHADER_SPECIFIC_CONST_1, ENTITYORIGIN );

			// Pixel Shader
			DECLARE_DYNAMIC_PIXEL_SHADER( character_ps30 );
			SET_DYNAMIC_PIXEL_SHADER_COMBO( NUM_LIGHTS, nNumLights );
			SET_DYNAMIC_PIXEL_SHADER_COMBO( WRITE_DEPTH_TO_DESTALPHA, bWriteDepthToAlpha );
			SET_DYNAMIC_PIXEL_SHADER_COMBO( WRITEWATERFOGTODESTALPHA, bWriteWaterFogToAlpha );
			SET_DYNAMIC_PIXEL_SHADER_COMBO( PIXELFOGTYPE,  pShaderAPI->GetPixelFogCombo() );
			SET_DYNAMIC_PIXEL_SHADER( character_ps30 );

			float vEyePos[4] = { 0, 0, 0, 0 };
			pShaderAPI->GetWorldSpaceCameraPosition( vEyePos );
			pShaderAPI->SetPixelShaderConstant( 3, vEyePos, 1 );

			pShaderAPI->SetPixelShaderFogParams( 19 );

			float vParams[4] = { 0, 0, 0, 0 };
			params[AMBIENTREFLECTIONBOUNCECOLOR]->GetVecValue( vParams, 3 );
			vParams[3] = params[AMBIENTREFLECTIONBOOST]->GetFloatValue();
			pShaderAPI->SetPixelShaderConstant( 0, vParams, 1 );

			vParams[0] = params[ENVMAPLIGHTSCALE]->GetFloatValue();
			vParams[1] = params[SHADOWSATURATION]->GetFloatValue();
			vParams[2] = params[METALNESS]->GetFloatValue();
			vParams[3] = params[RIMLIGHTALBEDO]->GetFloatValue();
			pShaderAPI->SetPixelShaderConstant( 101, vParams, 1 );

			SetPixelShaderConstant( 2, SHADOWSATURATIONBOUNDS );

			vParams[0] = params[PHONGBOOST]->GetFloatValue();
			vParams[1] = params[PHONGALBEDOBOOST]->GetFloatValue();
			vParams[2] = params[PHONGEXPONENT]->GetFloatValue();
			vParams[3] = params[ANISOTROPYAMOUNT]->GetFloatValue();
			pShaderAPI->SetPixelShaderConstant( 10, vParams, 1 );

			params[PHONGTINT]->GetVecValue(vParams, 3);
			vParams[3] = params[SHADOWRIMBOOST]->GetFloatValue();
			pShaderAPI->SetPixelShaderConstant( 11, vParams, 1 );

			SetPixelShaderConstant( 12, FRESNELRANGES );

			SetPixelShaderConstant( 102, SHADOWTINT );

			params[ENVMAPLIGHTSCALEMINMAX]->GetVecValue( vParams, 2 );
			vParams[2] = params[ENVMAPCONTRAST]->GetFloatValue();
			vParams[3] = params[ENVMAPSATURATION]->GetFloatValue();
			pShaderAPI->SetPixelShaderConstant( 103, vParams, 1 );

			SetPixelShaderConstant( 104, ENVMAPTINT );

			float fRimLightBoost = params[RIMLIGHTBOOST]->GetFloatValue();
			vParams[0] = params[RIMLIGHTEXPONENT]->GetFloatValue();
			vParams[1] = fRimLightBoost;
			vParams[2] = params[SELFILLUMBOOST]->GetFloatValue();
			vParams[3] = params[WARPINDEX]->GetFloatValue();
			pShaderAPI->SetPixelShaderConstant( 105, vParams, 1 );

			float fRimHaloBoost = params[RIMHALOBOOST]->GetFloatValue();
			params[RIMLIGHTTINT]->GetVecValue( vParams, 3 );
			vParams[3] = fRimHaloBoost * fRimLightBoost;
			pShaderAPI->SetPixelShaderConstant( 106, vParams, 1 );

			params[FAKERIMTINT]->GetVecValue( vParams, 3 );
			float fFakeRimBoost = params[FAKERIMBOOST]->GetFloatValue();
			vParams[0] *= fFakeRimBoost;
			vParams[1] *= fFakeRimBoost;
			vParams[2] *= fFakeRimBoost;
			vParams[3] = clamp( 1.0f - params[SHADOWCONTRAST]->GetFloatValue(), 0.0f, 1.0f );
			pShaderAPI->SetPixelShaderConstant( 107, vParams, 1 );

			//20-25 used

			//26-27 used

			SetPixelShaderConstant( 33, RIMHALOBOUNDS );
		}
		Draw();
	}

END_SHADER