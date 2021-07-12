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

		// Composite Preview
		SHADER_PARAM( PREVIEW, SHADER_PARAM_TYPE_BOOL, "", "" )

		// Composite Preview Samplers
		SHADER_PARAM( MATERIALMASK, SHADER_PARAM_TYPE_TEXTURE, "", "" )
		SHADER_PARAM( AO, SHADER_PARAM_TYPE_TEXTURE, "", "" )
		SHADER_PARAM( DETAIL, SHADER_PARAM_TYPE_TEXTURE, "", "" )
		SHADER_PARAM( DETAILNORMAL, SHADER_PARAM_TYPE_TEXTURE, "", "" )
		SHADER_PARAM( GRUNGE, SHADER_PARAM_TYPE_TEXTURE, "", "" )
		SHADER_PARAM( GRUNGETEXTURETRANSFORM, SHADER_PARAM_TYPE_MATRIX, "", "" )
		SHADER_PARAM( NOISE, SHADER_PARAM_TYPE_TEXTURE, "", "" )

		// Composite Preview Parameters Per-Material
		SHADER_PARAM( DETAILSCALE, SHADER_PARAM_TYPE_VEC4, "", "" )
		SHADER_PARAM( DETAILPHONGBOOST, SHADER_PARAM_TYPE_VEC4, "", "" )
		SHADER_PARAM( WEARDETAILPHONGBOOST, SHADER_PARAM_TYPE_VEC4, "", "" )
		SHADER_PARAM( DAMAGEDETAILPHONGBOOST, SHADER_PARAM_TYPE_VEC4, "", "" )
		SHADER_PARAM( DETAILENVBOOST, SHADER_PARAM_TYPE_VEC4, "", "" )
		SHADER_PARAM( WEARDETAILENVBOOST, SHADER_PARAM_TYPE_VEC4, "", "" )
		SHADER_PARAM( DAMAGEDETAILENVBOOST, SHADER_PARAM_TYPE_VEC4, "", "" )
		SHADER_PARAM( DETAILPHONGALBEDOTINT, SHADER_PARAM_TYPE_VEC4, "", "" )
		SHADER_PARAM( DAMAGEDETAILSATURATION, SHADER_PARAM_TYPE_VEC4, "", "" )
		SHADER_PARAM( DAMAGEDETAILBRIGHTNESSADJUSTMENT, SHADER_PARAM_TYPE_VEC4, "", "" )
		SHADER_PARAM( DETAILWARPINDEX, SHADER_PARAM_TYPE_VEC4, "", "" )
		SHADER_PARAM( DETAILMETALNESS, SHADER_PARAM_TYPE_VEC4, "", "" )
		SHADER_PARAM( DETAILNORMALDEPTH, SHADER_PARAM_TYPE_VEC4, "", "" )
		SHADER_PARAM( DAMAGENORMALEDGEDEPTH, SHADER_PARAM_TYPE_VEC4, "", "" )
		SHADER_PARAM( DAMAGEEDGEPHONGBOOST, SHADER_PARAM_TYPE_VEC4, "", "" )
		SHADER_PARAM( DAMAGEEDGEENVBOOST, SHADER_PARAM_TYPE_VEC4, "", "" )
		SHADER_PARAM( CURVATUREWEARBOOST, SHADER_PARAM_TYPE_VEC4, "", "" )
		SHADER_PARAM( CURVATUREWEARPOWER, SHADER_PARAM_TYPE_VEC4, "", "" )
		SHADER_PARAM( GRIME, SHADER_PARAM_TYPE_VEC4, "", "" )
		SHADER_PARAM( GRIMESATURATION, SHADER_PARAM_TYPE_VEC4, "", "" )
		SHADER_PARAM( GRIMEBRIGHTNESSADJUSTMENT, SHADER_PARAM_TYPE_VEC4, "", "" )
		SHADER_PARAM( DAMAGEGRUNGE, SHADER_PARAM_TYPE_VEC4, "", "" )
		SHADER_PARAM( DETAILGRUNGE, SHADER_PARAM_TYPE_VEC4, "", "" )
		SHADER_PARAM( GRUNGEMAX, SHADER_PARAM_TYPE_VEC4, "", "" )
		SHADER_PARAM( DAMAGELEVELS1, SHADER_PARAM_TYPE_VEC2, "", "" )
		SHADER_PARAM( DAMAGELEVELS2, SHADER_PARAM_TYPE_VEC2, "", "" )
		SHADER_PARAM( DAMAGELEVELS3, SHADER_PARAM_TYPE_VEC2, "", "" )
		SHADER_PARAM( DAMAGELEVELS4, SHADER_PARAM_TYPE_VEC2, "", "" )

		// Composite Preview Pattern Parameters
		SHADER_PARAM( PATTERN, SHADER_PARAM_TYPE_TEXTURE, "", "" )
		SHADER_PARAM( PATTERNTEXTURETRANSFORM, SHADER_PARAM_TYPE_MATRIX, "", "" )
		SHADER_PARAM( PATTERNREPLACEINDEX, SHADER_PARAM_TYPE_INTEGER, "", "" )
		SHADER_PARAM( PATTERNCOLORINDICES, SHADER_PARAM_TYPE_VEC4, "", "" )
		SHADER_PARAM( PATTERNDETAILINFLUENCE, SHADER_PARAM_TYPE_FLOAT, "", "" )
		SHADER_PARAM( PATTERNPHONGFACTOR, SHADER_PARAM_TYPE_FLOAT, "", "" )
		SHADER_PARAM( PATTERNPAINTTHICKNESS, SHADER_PARAM_TYPE_FLOAT, "", "" )

		// Composite Preview Parameters
		SHADER_PARAM( PALETTECOLOR1, SHADER_PARAM_TYPE_COLOR, "", "" )
		SHADER_PARAM( PALETTECOLOR2, SHADER_PARAM_TYPE_COLOR, "", "" )
		SHADER_PARAM( PALETTECOLOR3, SHADER_PARAM_TYPE_COLOR, "", "" )
		SHADER_PARAM( PALETTECOLOR4, SHADER_PARAM_TYPE_COLOR, "", "" )
		SHADER_PARAM( PALETTECOLOR5, SHADER_PARAM_TYPE_COLOR, "", "" )
		SHADER_PARAM( PALETTECOLOR6, SHADER_PARAM_TYPE_COLOR, "", "" )
		SHADER_PARAM( PALETTECOLOR7, SHADER_PARAM_TYPE_COLOR, "", "" )
		SHADER_PARAM( PALETTECOLOR8, SHADER_PARAM_TYPE_COLOR, "", "" )
		SHADER_PARAM( CUSTOMPAINTJOB, SHADER_PARAM_TYPE_BOOL, "", "" )
		SHADER_PARAM( ALLOVERPAINTJOB, SHADER_PARAM_TYPE_BOOL, "", "" )

		SHADER_PARAM( WEARPROGRESS, SHADER_PARAM_TYPE_FLOAT, "", "" )
		SHADER_PARAM( WEAREXPONENT, SHADER_PARAM_TYPE_FLOAT, "", "" )

		SHADER_PARAM(GRUNGETEXTUREROTATION, SHADER_PARAM_TYPE_MATRIX, "", "")
		SHADER_PARAM(PATTERNTEXTUREROTATION, SHADER_PARAM_TYPE_MATRIX, "", "")

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

		if ( !params[PREVIEW]->IsDefined() )
		{
			params[PREVIEW]->SetIntValue( 0 );
		}

		if ( !params[SHADOWRIMBOOST]->IsDefined() )
		{ 
			params[SHADOWRIMBOOST]->SetIntValue( 2.0f );
		}

		if ( params[PREVIEW]->GetIntValue() > 0 )
		{
			if ( !params[DETAILSCALE]->IsDefined() )
			{
				params[DETAILSCALE]->SetFloatValue( 4.0f );
			}

			if ( !params[DETAILPHONGBOOST]->IsDefined() )
			{
				params[DETAILPHONGBOOST]->SetVecValue( 1.0f, 1.0f, 1.0f, 1.0f );
			}

			if ( !params[WEARDETAILPHONGBOOST]->IsDefined() )
			{
				params[WEARDETAILPHONGBOOST]->SetVecValue( 0.0f, 0.0f, 0.0f, 0.0f );
			}

			if ( !params[DAMAGEDETAILPHONGBOOST]->IsDefined() )
			{
				params[DAMAGEDETAILPHONGBOOST]->SetVecValue( 1.0f, 1.0f, 1.0f, 1.0f );
			}

			if ( !params[DETAILENVBOOST]->IsDefined() )
			{
				params[DETAILENVBOOST]->SetVecValue( 1.0f, 1.0f, 1.0f, 1.0f );
			}

			if ( !params[WEARDETAILENVBOOST]->IsDefined() )
			{
				params[WEARDETAILENVBOOST]->SetVecValue( 0.0f, 0.0f, 0.0f, 0.0f );
			}

			if ( !params[DAMAGEDETAILENVBOOST]->IsDefined() )
			{
				params[DAMAGEDETAILENVBOOST]->SetVecValue( 0.0f, 0.0f, 0.0f, 0.0f );
			}

			if ( !params[DETAILPHONGALBEDOTINT]->IsDefined() )
			{
				params[DETAILPHONGALBEDOTINT]->SetVecValue( 0.0f, 0.0f, 0.0f, 0.0f );
			}

			if ( !params[DETAILWARPINDEX]->IsDefined() )
			{
				params[DETAILWARPINDEX]->SetVecValue( 0.0f, 0.0f, 0.0f, 0.0f );
			}

			// Metalness is just a multiply on the albedo, so invert the defined amount to get the multiplier
			if ( !params[DETAILMETALNESS]->IsDefined() )
			{
				params[DETAILMETALNESS]->SetVecValue( 1.0f, 1.0f, 1.0f, 1.0f );
			}

			if ( !params[DETAILNORMALDEPTH]->IsDefined() )
			{
				params[DETAILNORMALDEPTH]->SetVecValue( 1.0f, 1.0f, 1.0f, 1.0f );
			}

			if ( !params[DAMAGENORMALEDGEDEPTH]->IsDefined() )
			{
				params[DAMAGENORMALEDGEDEPTH]->SetVecValue( 1.0f, 1.0f, 1.0f, 1.0f );
			}

			if ( !params[DAMAGEEDGEPHONGBOOST]->IsDefined() )
			{
				params[DAMAGEEDGEPHONGBOOST]->SetVecValue( 0.0f, 0.0f, 0.0f, 0.0f );
			}

			if ( !params[DAMAGEEDGEENVBOOST]->IsDefined() )
			{
				params[DAMAGEEDGEENVBOOST]->SetVecValue( 0.0f, 0.0f, 0.0f, 0.0f );
			}

			if ( !params[DAMAGELEVELS1]->IsDefined() )
			{
				params[DAMAGELEVELS1]->SetVecValue( 0.0f, 1.0f );
			}

			if ( !params[DAMAGELEVELS2]->IsDefined() )
			{
				params[DAMAGELEVELS2]->SetVecValue( 0.0f, 1.0f );
			}

			if ( !params[DAMAGELEVELS3]->IsDefined() )
			{
				params[DAMAGELEVELS3]->SetVecValue( 0.0f, 1.0f );
			}

			if ( !params[DAMAGELEVELS4]->IsDefined() )
			{
				params[DAMAGELEVELS4]->SetVecValue( 0.0f, 1.0f );
			}

			if ( !params[CUSTOMPAINTJOB]->IsDefined() )
			{
				params[CUSTOMPAINTJOB]->SetIntValue( 0 );
			}

			if ( !params[PATTERNDETAILINFLUENCE]->IsDefined() )
			{
				params[PATTERNDETAILINFLUENCE]->SetFloatValue( 32.0f );
			}

			if ( !params[ALLOVERPAINTJOB]->IsDefined() )
			{
				params[ALLOVERPAINTJOB]->SetIntValue( 0 );
			}

			if ( !params[CURVATUREWEARBOOST]->IsDefined() )
			{
				params[CURVATUREWEARBOOST]->SetVecValue( 0.0f, 0.0f, 0.0f, 0.0f );
			}

			if ( !params[CURVATUREWEARPOWER]->IsDefined() )
			{
				params[CURVATUREWEARPOWER]->SetVecValue( 1.0f, 1.0f, 1.0f, 1.0f );
			}
		

			if ( !params[DAMAGEDETAILSATURATION]->IsDefined() )
			{
				params[DAMAGEDETAILSATURATION]->SetVecValue( 0.0f, 0.0f, 0.0f, 0.0f );
			}

			if (!params[DAMAGEDETAILBRIGHTNESSADJUSTMENT]->IsDefined())
			{
				params[DAMAGEDETAILBRIGHTNESSADJUSTMENT]->SetVecValue(0.0f, 0.0f, 0.0f, 0.0f);
			}

			if ( !params[GRIME]->IsDefined() )
			{
				params[GRIME]->SetVecValue( 0.25f, 0.25f, 0.25f, 0.25f );
			}

			if ( !params[GRIMESATURATION]->IsDefined() )
			{
				params[GRIMESATURATION]->SetVecValue( 0.0f, 0.0f, 0.0f, 0.0f );
			}

			if ( !params[GRIMEBRIGHTNESSADJUSTMENT]->IsDefined() )
			{
				params[GRIMEBRIGHTNESSADJUSTMENT]->SetVecValue( 0.0f, 0.0f, 0.0f, 0.0f );
			}

			if ( !params[DAMAGEGRUNGE]->IsDefined() )
			{
				params[DAMAGEGRUNGE]->SetVecValue( 0.25f, 0.25f, 0.25f, 0.25f );
			}

			if ( !params[DETAILGRUNGE]->IsDefined() )
			{
				params[DETAILGRUNGE]->SetVecValue( 0.0f, 0.0f, 0.0f, 0.0f );
			}

			if (!params[GRUNGEMAX]->IsDefined())
			{
				params[GRUNGEMAX]->SetVecValue( 1.0f, 1.0f, 1.0f, 1.0f );
			}

			if ( !params[WEARPROGRESS]->IsDefined() )
			{
				params[WEARPROGRESS]->SetFloatValue( 0.0f );
			}

			if ( !params[WEAREXPONENT]->IsDefined() )
			{
				params[WEAREXPONENT]->SetFloatValue( 1.0f );
			}

			if ( !params[PATTERNCOLORINDICES]->IsDefined() )
			{
				params[PATTERNCOLORINDICES]->SetVecValue( 1, 2, 3, 4 );
			}

			if ( !params[PATTERNREPLACEINDEX]->IsDefined() )
			{
				params[PATTERNREPLACEINDEX]->SetIntValue( 1 );
			}

			if ( !params[NOISE]->IsDefined() )
			{
				params[NOISE]->SetStringValue( "models/weapons/customization/materials/noise" );
			}

			if (!params[PATTERNPHONGFACTOR]->IsDefined())
			{
				params[PATTERNPHONGFACTOR]->SetFloatValue( 0.0f );
			}

			if (!params[PATTERNPAINTTHICKNESS]->IsDefined())
			{
				params[PATTERNPAINTTHICKNESS]->SetFloatValue(0.0f);
			}

			if (!params[PATTERNTEXTURETRANSFORM]->IsDefined())
			{
				params[PATTERNTEXTURETRANSFORM]->SetStringValue( "scale 1 1 translate 0 0 rotate 0" );
			}

			if (!params[GRUNGETEXTURETRANSFORM]->IsDefined())
			{
				params[GRUNGETEXTURETRANSFORM]->SetStringValue( "scale 1 1 translate 0 0 rotate 0" );
			}			

			if (!params[GRUNGETEXTUREROTATION]->IsDefined())
			{
				params[GRUNGETEXTUREROTATION]->SetStringValue("scale 1 1 translate 0 0 rotate 0");
			}

			if (!params[PATTERNTEXTUREROTATION]->IsDefined())
			{
				params[PATTERNTEXTUREROTATION]->SetStringValue("scale 1 1 translate 0 0 rotate 0");
			}
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

		bool bPreview = params[PREVIEW]->IsDefined() && ( params[PREVIEW]->GetIntValue() > 0 );

		if ( bPreview && params[MATERIALMASK]->IsDefined() )
		{
			LoadTexture( MATERIALMASK );
		}

		if ( bPreview && params[AO]->IsDefined() )
		{
			LoadTexture( AO );
		}

		if ( bPreview && params[GRUNGE]->IsDefined() )
		{
			LoadTexture( GRUNGE );
		}

		if ( bPreview && params[DETAIL]->IsDefined() )
		{
			LoadTexture( DETAIL );
		}

		if ( bPreview && params[DETAILNORMAL]->IsDefined() )
		{
			LoadTexture( DETAILNORMAL );
		}

		if ( bPreview && params[PATTERN]->IsDefined() )
		{
			LoadTexture( PATTERN );
		}

		if ( bPreview )
		{
			LoadTexture( NOISE );
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

		bool bPreview = params[PREVIEW]->IsDefined() && ( params[PREVIEW]->GetIntValue() > 0 );
		bool bPattern = bPreview && IsTextureSet( PATTERN, params );
		bool bHasCustomPaint = bPattern && params[CUSTOMPAINTJOB]->IsDefined() && ( params[CUSTOMPAINTJOB]->GetIntValue() > 0 );
		bool bHasAlloverPaint = bPattern && params[ALLOVERPAINTJOB]->IsDefined() && ( params[ALLOVERPAINTJOB]->GetIntValue() > 0 );

		bool bHasAO = bPreview && IsTextureSet( AO, params );
		bool bHasMaterialMask = bPreview && IsTextureSet( MATERIALMASK, params );
		bool bHasGrunge = bPreview && IsTextureSet( GRUNGE, params );
		bool bHasDetail = bPreview && IsTextureSet( DETAIL, params );
		bool bHasDetailNormal = bPreview && IsTextureSet( DETAILNORMAL, params );

		bool bHasFlashlight = UsingFlashlight( params );

		if ( bPreview )
		// need to turn off some features for shadercompile to complete, otherwise it runs out of memory before all combos are compiled
		{
			bHasBounceColor = false;
			bBaseAlphaSelfIllumMask = false;
			bHasFakeRim = false;
			bHasFlashlight = false;
		}

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
			SET_STATIC_PIXEL_SHADER_COMBO( DOPREVIEW, bPreview );
			SET_STATIC_PIXEL_SHADER_COMBO( USEPATTERN, bPattern + bHasCustomPaint + ( 2 * bHasAlloverPaint ) );
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
			// CSM sampler
			pShaderShadow->EnableTexture( SHADER_SAMPLER8, true );

			if ( bHasMaterialMask )
			{
				pShaderShadow->EnableTexture( SHADER_SAMPLER9, true );
			}
			if ( bPreview )
			{
				if ( bHasMasks1 )
				{
					pShaderShadow->EnableTexture(SHADER_SAMPLER2, true);
				}
				if ( bHasAO )
				{
					pShaderShadow->EnableTexture( SHADER_SAMPLER10, true );
				}
				if ( bHasGrunge )
				{
					pShaderShadow->EnableTexture( SHADER_SAMPLER11, true );
				}
				if ( bHasDetail )
				{
					pShaderShadow->EnableTexture( SHADER_SAMPLER12, true );
				}
				if ( bHasDetailNormal )
				{
					pShaderShadow->EnableTexture( SHADER_SAMPLER13, true );
				}
				if ( bPattern )
				{
					pShaderShadow->EnableTexture( SHADER_SAMPLER14, true );
				}
				// noise for color modulation
				pShaderShadow->EnableTexture( SHADER_SAMPLER15, true );
			}
			else
			{
				if ( bHasMasks1 )
				{
					pShaderShadow->EnableTexture( SHADER_SAMPLER10, true );
				}
				if ( bHasFlashlight )
				{
					pShaderShadow->EnableTexture( SHADER_SAMPLER11, true );
				}
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
			if ( bHasGrunge )
			{
				pShaderShadow->EnableSRGBRead( SHADER_SAMPLER11, true );
			}
			if ( bPattern )
			{
				pShaderShadow->EnableSRGBRead( SHADER_SAMPLER14, bHasCustomPaint ? true : false );
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
				if ( bPreview )
				{
					BindTexture( SHADER_SAMPLER2, MASKS1, -1 );
				}
				else
				{
					BindTexture(SHADER_SAMPLER10, MASKS1, -1);
				}
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
                    BindTexture(SHADER_SAMPLER11, pFlashlightDepthTexture, 0);
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


			if ( bHasMaterialMask )
			{
				BindTexture( SHADER_SAMPLER9, MATERIALMASK, -1 );
			}
			if ( bHasAO )
			{
				BindTexture( SHADER_SAMPLER10, AO, -1 );
			}
			if ( bHasGrunge )
			{
				BindTexture( SHADER_SAMPLER11, GRUNGE, -1 );
			}
			if ( bHasDetail )
			{
				BindTexture( SHADER_SAMPLER12, DETAIL, -1 );
			}
			if ( bHasDetailNormal )
			{
				BindTexture( SHADER_SAMPLER13, DETAILNORMAL, -1 );
			}
			if ( bPattern )
			{
				BindTexture( SHADER_SAMPLER14, PATTERN, -1 );
			}
			if ( bPreview )
			{
				BindTexture( SHADER_SAMPLER15, NOISE, -1 );
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
            
			// need to turn off some features for shadercompile to complete, otherwise it runs out of memory before all combos are compiled
			if ( bPreview )
				nNumLights = clamp( nNumLights, 0, 1 );

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

			if ( bPreview )
			{
				params[DETAILSCALE]->GetVecValue( vParams, 4 );
				pShaderAPI->SetPixelShaderConstant( 108, vParams, 1 );

				params[DETAILPHONGBOOST]->GetVecValue( vParams, 4 );
				pShaderAPI->SetPixelShaderConstant( 32, vParams, 1 );

				params[DAMAGEDETAILPHONGBOOST]->GetVecValue( vParams, 4 );
				pShaderAPI->SetPixelShaderConstant( 100, vParams, 1 );

				params[DETAILENVBOOST]->GetVecValue( vParams, 4 );
				pShaderAPI->SetPixelShaderConstant( 34, vParams, 1 );

				params[DAMAGEDETAILENVBOOST]->GetVecValue( vParams, 4 );
				pShaderAPI->SetPixelShaderConstant( 35, vParams, 1 );

				params[DETAILPHONGALBEDOTINT]->GetVecValue( vParams, 4 );
				pShaderAPI->SetPixelShaderConstant( 36, vParams, 1 );

				params[DETAILWARPINDEX]->GetVecValue( vParams, 4 );
				pShaderAPI->SetPixelShaderConstant( 37, vParams, 1 );

				params[DETAILMETALNESS]->GetVecValue( vParams, 4 );
				pShaderAPI->SetPixelShaderConstant( 38, vParams, 1 );

				params[DETAILNORMALDEPTH]->GetVecValue( vParams, 4 );
				pShaderAPI->SetPixelShaderConstant( 39, vParams, 1 );

				params[DAMAGENORMALEDGEDEPTH]->GetVecValue( vParams, 4 );
				pShaderAPI->SetPixelShaderConstant( 40, vParams, 1 );

				float vParams2[2] = { 0, 0 };
				params[DAMAGELEVELS1]->GetVecValue( vParams, 2 );
				params[DAMAGELEVELS2]->GetVecValue( vParams2, 2 );
				vParams[2] = vParams2[0];
				vParams[3] = vParams2[1];
				pShaderAPI->SetPixelShaderConstant( 41, vParams, 1 );

				params[DAMAGELEVELS3]->GetVecValue( vParams, 2 );
				params[DAMAGELEVELS4]->GetVecValue( vParams2, 2 );
				vParams[2] = vParams2[0];
				vParams[3] = vParams2[1];
				pShaderAPI->SetPixelShaderConstant( 42, vParams, 1 );

				SetPixelShaderTextureTransform( 43, PATTERNTEXTURETRANSFORM ); // 43-44

				params[PATTERNCOLORINDICES]->GetVecValue( vParams, 4 );
				vParams[0] = clamp( vParams[0] - 1, 0, 7 );
				vParams[1] = clamp( vParams[1] - 1, 0, 7 );
				vParams[2] = clamp( vParams[2] - 1, 0, 7 );
				vParams[3] = clamp( vParams[3] - 1, 0, 7 );
				pShaderAPI->SetPixelShaderConstant( 45, vParams, 1 );

				vParams[0] = pow( params[WEARPROGRESS]->GetFloatValue(),  params[WEAREXPONENT]->GetFloatValue() );
				vParams[1] = params[PATTERNDETAILINFLUENCE]->GetFloatValue();
				if ( bPattern )
				{
					vParams[2] = clamp( params[PATTERNREPLACEINDEX]->GetIntValue() - 1, 0, 7 );
				}
				else
				{
					vParams[2] = -1;
				}
				vParams[3] = 1.0f / 1024.0f; // assuming texture size of 1024.  Should be dynamic?
				pShaderAPI->SetPixelShaderConstant( 46, vParams, 1 );

				float vParams3[3] = { 0, 0, 0 };
				params[PALETTECOLOR4]->GetVecValue( vParams3, 3 );
				vParams3[0] = SrgbGammaToLinear( vParams3[0] / 255.0f );
				vParams3[1] = SrgbGammaToLinear( vParams3[1] / 255.0f );
				vParams3[2] = SrgbGammaToLinear( vParams3[2] / 255.0f );
				
				params[PALETTECOLOR1]->GetVecValue( vParams, 3 );
				vParams[0] = SrgbGammaToLinear( vParams[0] / 255.0f );
				vParams[1] = SrgbGammaToLinear( vParams[1] / 255.0f );
				vParams[2] = SrgbGammaToLinear( vParams[2] / 255.0f );
				vParams[3] = vParams3[0];
				pShaderAPI->SetPixelShaderConstant( 47, vParams, 1 );

				params[PALETTECOLOR2]->GetVecValue( vParams, 3 );
				vParams[0] = SrgbGammaToLinear( vParams[0] / 255.0f );
				vParams[1] = SrgbGammaToLinear( vParams[1] / 255.0f );
				vParams[2] = SrgbGammaToLinear( vParams[2] / 255.0f );
				vParams[3] = vParams3[1];
				pShaderAPI->SetPixelShaderConstant( 48, vParams, 1 );

				params[PALETTECOLOR3]->GetVecValue( vParams, 3 );
				vParams[0] = SrgbGammaToLinear( vParams[0] / 255.0f );
				vParams[1] = SrgbGammaToLinear( vParams[1] / 255.0f );
				vParams[2] = SrgbGammaToLinear( vParams[2] / 255.0f );
				vParams[3] = vParams3[2];
				pShaderAPI->SetPixelShaderConstant( 49, vParams, 1 );

				params[PALETTECOLOR8]->GetVecValue( vParams3, 3 );
				vParams3[0] = SrgbGammaToLinear( vParams3[0] / 255.0f );
				vParams3[1] = SrgbGammaToLinear( vParams3[1] / 255.0f );
				vParams3[2] = SrgbGammaToLinear( vParams3[2] / 255.0f );
				
				params[PALETTECOLOR5]->GetVecValue( vParams, 3 );
				vParams[0] = SrgbGammaToLinear( vParams[0] / 255.0f );
				vParams[1] = SrgbGammaToLinear( vParams[1] / 255.0f );
				vParams[2] = SrgbGammaToLinear( vParams[2] / 255.0f );
				vParams[3] = vParams3[0];
				pShaderAPI->SetPixelShaderConstant( 50, vParams, 1 );

				params[PALETTECOLOR6]->GetVecValue( vParams, 3 );
				vParams[0] = SrgbGammaToLinear( vParams[0] / 255.0f );
				vParams[1] = SrgbGammaToLinear( vParams[1] / 255.0f );
				vParams[2] = SrgbGammaToLinear( vParams[2] / 255.0f );
				vParams[3] = vParams3[1];
				pShaderAPI->SetPixelShaderConstant( 51, vParams, 1 );

				params[PALETTECOLOR7]->GetVecValue( vParams, 3 );
				vParams[0] = SrgbGammaToLinear( vParams[0] / 255.0f );
				vParams[1] = SrgbGammaToLinear( vParams[1] / 255.0f );
				vParams[2] = SrgbGammaToLinear( vParams[2] / 255.0f );
				vParams[3] = vParams3[2];
				pShaderAPI->SetPixelShaderConstant( 52, vParams, 1 );

				SetPixelShaderTextureTransform( 53, GRUNGETEXTURETRANSFORM ); // 53-54

				params[GRIME]->GetVecValue( vParams, 4);
				pShaderAPI->SetPixelShaderConstant( 55, vParams, 1 );

				params[WEARDETAILPHONGBOOST]->GetVecValue( vParams, 4 );
				pShaderAPI->SetPixelShaderConstant( 56, vParams, 1 );

				params[WEARDETAILENVBOOST]->GetVecValue( vParams, 4 );
				pShaderAPI->SetPixelShaderConstant( 57, vParams, 1 );

				params[DAMAGEEDGEPHONGBOOST]->GetVecValue( vParams, 4 );
				pShaderAPI->SetPixelShaderConstant( 58, vParams, 1 );

				params[DAMAGEEDGEENVBOOST]->GetVecValue( vParams, 4 );
				pShaderAPI->SetPixelShaderConstant( 59, vParams, 1 );

				params[CURVATUREWEARBOOST]->GetVecValue( vParams, 4 );
				pShaderAPI->SetPixelShaderConstant( 60, vParams, 1 );

				params[CURVATUREWEARPOWER]->GetVecValue( vParams, 4 );
				pShaderAPI->SetPixelShaderConstant( 61, vParams, 1 );

				params[DAMAGEDETAILSATURATION]->GetVecValue( vParams, 4 );
				pShaderAPI->SetPixelShaderConstant( 62, vParams, 1 );

				params[DAMAGEGRUNGE]->GetVecValue( vParams, 4);
				pShaderAPI->SetPixelShaderConstant( 63, vParams, 1 );

				params[DETAILGRUNGE]->GetVecValue(vParams, 4);
				pShaderAPI->SetPixelShaderConstant(90, vParams, 1);

				vParams[0] = params[PATTERNPHONGFACTOR]->GetFloatValue();
				vParams[1] = params[PATTERNPAINTTHICKNESS]->GetFloatValue();
				vParams[2] = 1;
				vParams[3] = 0;
				pShaderAPI->SetPixelShaderConstant(91, vParams, 1);

				params[DAMAGEDETAILBRIGHTNESSADJUSTMENT]->GetVecValue(vParams, 4);
				pShaderAPI->SetPixelShaderConstant(92, vParams, 1);

				params[GRUNGEMAX]->GetVecValue(vParams, 4);
				pShaderAPI->SetPixelShaderConstant(93, vParams, 1);

				// c68-89 used by csms

				params[GRIMESATURATION]->GetVecValue(vParams, 4);
				pShaderAPI->SetPixelShaderConstant(94, vParams, 1);

				params[GRIMEBRIGHTNESSADJUSTMENT]->GetVecValue(vParams, 4);
				pShaderAPI->SetPixelShaderConstant(95, vParams, 1);
	
				SetPixelShaderTextureTransform(96, GRUNGETEXTUREROTATION);				
				SetPixelShaderTextureTransform( 98, PATTERNTEXTUREROTATION );
			}

		}
		Draw();
	}

END_SHADER