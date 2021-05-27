//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Class to control 'aim-down-sights' aka "IronSight" weapon functionality
//
//=====================================================================================//


#include "cbase.h"
#include "cs_shareddefs.h"

#if IRONSIGHT
#include "weapon_csbase.h"
#include "weapon_ironsightcontroller.h"

#if defined( CLIENT_DLL )
	#include "c_cs_player.h"
	#include "view_scene.h"
	#include "shaderapi/ishaderapi.h"
	#include "materialsystem/imaterialvar.h"
	#include "prediction.h"
#else
	#include "cs_player.h"
#endif

// NOTE: This has to be the last file included!
#include "tier0/memdbgon.h"

#define IRONSIGHT_VIEWMODEL_BOB_MULT_X 0.05
#define IRONSIGHT_VIEWMODEL_BOB_PERIOD_X 6
#define IRONSIGHT_VIEWMODEL_BOB_MULT_Y 0.1
#define IRONSIGHT_VIEWMODEL_BOB_PERIOD_Y 10

#ifdef DEBUG
	ConVar ironsight_override(				"ironsight_override",			"0",		FCVAR_DEVELOPMENTONLY | FCVAR_CHEAT );
	ConVar ironsight_position(				"ironsight_position",			"0 0 0",	FCVAR_DEVELOPMENTONLY | FCVAR_CHEAT );
	ConVar ironsight_angle(					"ironsight_angle",				"0 0 0",	FCVAR_DEVELOPMENTONLY | FCVAR_CHEAT );
	ConVar ironsight_fov(					"ironsight_fov",				"60",		FCVAR_DEVELOPMENTONLY | FCVAR_CHEAT );
	ConVar ironsight_pivot_forward(			"ironsight_pivot_forward",		"0",		FCVAR_DEVELOPMENTONLY | FCVAR_CHEAT );
	ConVar ironsight_looseness(				"ironsight_looseness",			"0.1",		FCVAR_DEVELOPMENTONLY | FCVAR_CHEAT );
	ConVar ironsight_speed_bringup(			"ironsight_speed_bringup",		"4.0",		FCVAR_DEVELOPMENTONLY | FCVAR_CHEAT );
	ConVar ironsight_speed_putdown(			"ironsight_speed_putdown",		"2.0",		FCVAR_DEVELOPMENTONLY | FCVAR_CHEAT );
	
	ConVar ironsight_catchupspeed(			"ironsight_catchupspeed",		"60.0",		FCVAR_DEVELOPMENTONLY | FCVAR_CHEAT );
	ConVar ironsight_running_looseness(		"ironsight_running_looseness",	"0.3",		FCVAR_DEVELOPMENTONLY | FCVAR_CHEAT );

	ConVar ironsight_spew_amount(			"ironsight_spew_amount",		"0",		FCVAR_DEVELOPMENTONLY | FCVAR_CHEAT );
#else
	#define ironsight_catchupspeed			60.0f
#endif

CIronSightController::CIronSightController( void )
{
	m_bIronSightAvailable			= false;
	m_angPivotAngle.Init();
	m_vecEyePos.Init();
	m_flIronSightAmount				= 0.0f;
	m_flIronSightPullUpSpeed		= 8.0f;
	m_flIronSightPutDownSpeed		= 4.0f;
	m_flIronSightFOV				= 80.0f;
	m_flIronSightPivotForward		= 10.0f;
	m_flIronSightLooseness			= 0.5f;
	m_pAttachedWeapon				= NULL;	

#ifdef CLIENT_DLL
	m_angViewLast.Init();
	for ( int i=0; i<IRONSIGHT_ANGLE_AVERAGE_SIZE; i++ )
	{
		m_angDeltaAverage[i].Init();
	}
	m_vecDotCoords.Init();
	m_flDotBlur						= 0.0f;
	m_flSpeedRatio					= 0.0f;
#endif
}

void CIronSightController::SetState( CSIronSightMode newState )
{
	if ( newState == IronSight_viewmodel_is_deploying || newState == IronSight_weapon_is_dropped )
	{
		m_flIronSightAmount = 0.0f;
	}
	if ( m_pAttachedWeapon )
	{
		m_pAttachedWeapon->m_iIronSightMode = newState;
	}
}

bool CIronSightController::IsApproachingSighted(void)
{
	return (m_pAttachedWeapon && m_pAttachedWeapon->m_iIronSightMode == IronSight_should_approach_sighted);
}

bool CIronSightController::IsApproachingUnSighted(void)
{
	return (m_pAttachedWeapon && m_pAttachedWeapon->m_iIronSightMode == IronSight_should_approach_unsighted);
}

bool CIronSightController::IsDeploying(void)
{
	return (m_pAttachedWeapon && m_pAttachedWeapon->m_iIronSightMode == IronSight_viewmodel_is_deploying);
}

bool CIronSightController::IsDropped(void)
{
	return (m_pAttachedWeapon && m_pAttachedWeapon->m_iIronSightMode == IronSight_weapon_is_dropped);
}

void CIronSightController::UpdateIronSightAmount( void )
{

	if (!m_pAttachedWeapon || IsDeploying() || IsDropped())
	{
		//ignore and discard any lingering ironsight amount.
		m_flIronSightAmount = 0.0f;
		m_flIronSightAmountGained = 0.0f;
		return;
	}

	//first determine if we are going into or out of ironsights, and set m_flIronSightAmount accordingly
	float flIronSightAmountTarget = IsApproachingSighted() ? 1.0f : 0.0f;
	float flIronSightUpdOrDownSpeed = IsApproachingSighted() ? m_flIronSightPullUpSpeed : m_flIronSightPutDownSpeed;

#ifdef DEBUG
	if (ironsight_override.GetBool())
		flIronSightUpdOrDownSpeed = IsApproachingSighted() ? ironsight_speed_bringup.GetFloat() : ironsight_speed_putdown.GetFloat();
#endif

	m_flIronSightAmount = Approach(flIronSightAmountTarget, m_flIronSightAmount, gpGlobals->frametime * flIronSightUpdOrDownSpeed);

	m_flIronSightAmountGained = Gain( m_flIronSightAmount, 0.8f );
	m_flIronSightAmountBiased = Bias( m_flIronSightAmount, 0.2f );

#ifdef DEBUG
	if ( ironsight_spew_amount.GetBool() )
		DevMsg( "Ironsight amount: %f, Gained: %f, Biased: %f\n", m_flIronSightAmount, m_flIronSightAmountGained, m_flIronSightAmountBiased );
#endif

}

#ifdef CLIENT_DLL

void CIronSightController::IncreaseDotBlur(float flAmount)
{
	if ( IsInIronSight() && prediction->IsFirstTimePredicted() )
		m_flDotBlur = clamp(m_flDotBlur + flAmount, 0, 1);
}

float CIronSightController::GetDotBlur(void)
{
	return Bias(1.0f - Max(m_flDotBlur, m_flSpeedRatio * 0.5f), 0.2f);
}

float CIronSightController::GetDotWidth(void)
{
	return (32 + (256 * Max(m_flDotBlur, m_flSpeedRatio * 0.3f)));

}
Vector2D CIronSightController::GetDotCoords(void)
{
	return m_vecDotCoords;
}

bool CIronSightController::ShouldHideCrossHair( void )
{
	return ( (IsApproachingSighted() || IsApproachingUnSighted()) && GetIronSightAmount() > IRONSIGHT_HIDE_CROSSHAIR_THRESHOLD );
}

const char *CIronSightController::GetDotMaterial( void )
{
	if ( m_pAttachedWeapon && m_pAttachedWeapon->GetCSWpnData().m_szIronsightDotMaterial != 0 )
		return m_pAttachedWeapon->GetCSWpnData().m_szIronsightDotMaterial;

	return NULL;
}

QAngle CIronSightController::QAngleDiff( QAngle &angTarget, QAngle &angSrc )
{
	return QAngle(	AngleDiff( angTarget.x, angSrc.x ),
					AngleDiff( angTarget.y, angSrc.y ),
					AngleDiff( angTarget.z, angSrc.z ) );
}

QAngle CIronSightController::GetAngleAverage( void )
{
	QAngle temp;
	temp.Init();

	if (GetIronSightAmount() < 1.0f)
		return temp;

	for ( int i=0; i<IRONSIGHT_ANGLE_AVERAGE_SIZE; i++ )
	{
		temp += m_angDeltaAverage[i];
	}

	return ( temp * IRONSIGHT_ANGLE_AVERAGE_DIVIDE );
}

void CIronSightController::AddToAngleAverage( QAngle newAngle )
{

	if (GetIronSightAmount() < 1.0f)
		return;

	newAngle.x = clamp( newAngle.x, -2, 2 );
	newAngle.y = clamp( newAngle.y, -2, 2 );
	newAngle.z = clamp( newAngle.z, -2, 2 );

	for ( int i=IRONSIGHT_ANGLE_AVERAGE_SIZE-1; i>0; i-- )
	{
		m_angDeltaAverage[i] = m_angDeltaAverage[i-1];
	}

	m_angDeltaAverage[0] = newAngle;
}

extern ConVar cl_righthand;
void CIronSightController::ApplyIronSightPositioning( Vector &vecPosition, QAngle &angAngle, const Vector &vecBobbedEyePosition, const QAngle &angBobbedEyeAngle )
{
	UpdateIronSightAmount();

	if ( m_flIronSightAmount == 0 )
		return;

	//check if the player is moving and save off a usable movement speed ratio for later
	if ( m_pAttachedWeapon->GetOwner() )
	{
		CBaseCombatCharacter *pPlayer = m_pAttachedWeapon->GetOwner();
		m_flSpeedRatio = Approach( pPlayer->GetAbsVelocity().Length() / m_pAttachedWeapon->GetMaxSpeed(), m_flSpeedRatio, gpGlobals->frametime * 10.0f );
	}

	//if we're more than 10% ironsighted, apply looseness.
	if ( m_flIronSightAmount > 0.1f )
	{
		//get the difference between current angles and last angles
		QAngle angDelta = QAngleDiff( m_angViewLast, angAngle );
	
		//dampen the delta to simulate 'looseness', but the faster we move, the more looseness approaches ironsight_running_looseness.GetFloat(), which is as waggly as possible
#ifdef DEBUG
		if ( ironsight_override.GetBool() )
		{
			AddToAngleAverage( angDelta * Lerp(m_flSpeedRatio, ironsight_looseness.GetFloat(), ironsight_running_looseness.GetFloat()) );
		}
		else
#endif
		{
			AddToAngleAverage( angDelta * Lerp(m_flSpeedRatio, m_flIronSightLooseness, 0.3f ) );
		}

		//m_angViewLast tries to catch up to angAngle
#ifdef DEBUG
		m_angViewLast -= angDelta * clamp( gpGlobals->frametime * ironsight_catchupspeed.GetFloat(), 0, 1 );
#else
		m_angViewLast -= angDelta * clamp( gpGlobals->frametime * ironsight_catchupspeed, 0, 1 );
#endif

	}
	else
	{
		m_angViewLast = angAngle;
	}
		
	
	//now the fun part - move the viewmodel to look down the sights


	//create a working matrix at the current eye position and angles
	VMatrix matIronSightMatrix = SetupMatrixOrgAngles( vecPosition, angAngle );


	//offset the matrix by the ironsight eye position
#ifdef DEBUG
	if ( ironsight_override.GetBool() )
	{
		Vector vecTemp; //when overridden use convar values instead of schema driven ones so we can iterate on the values quickly while authoring
		if ( sscanf( ironsight_position.GetString(), "%f %f %f", &vecTemp.x, &vecTemp.y, &vecTemp.z ) == 3 )
			MatrixTranslate( matIronSightMatrix, (-vecTemp) * GetIronSightAmountGained() );
	}
	else
#endif
	{
		//use schema defined offset
		MatrixTranslate( matIronSightMatrix, (-m_vecEyePos) * GetIronSightAmountGained() );
	}
	
	//additionally offset by the ironsight origin of rotation, the weapon will pivot around this offset from the eye
#ifdef DEBUG
	if ( ironsight_override.GetBool() )
	{
		MatrixTranslate( matIronSightMatrix, Vector( ironsight_pivot_forward.GetFloat(), 0, 0 ) );
	}
	else
#endif
	{
		MatrixTranslate( matIronSightMatrix, Vector( m_flIronSightPivotForward, 0, 0 ) );
	}

	QAngle angDeltaAverage = GetAngleAverage();

	if ( !cl_righthand.GetBool() )
		angDeltaAverage = -angDeltaAverage;

	//apply ironsight eye rotation
#ifdef DEBUG
	if ( ironsight_override.GetBool() )
	{
		QAngle angTemp;  //when overridden use convar values instead of schema driven ones so we can iterate on the values quickly while authoring
		if ( sscanf( ironsight_angle.GetString(), "%f %f %f", &angTemp.x, &angTemp.y, &angTemp.z ) == 3 )
		{
			MatrixRotate( matIronSightMatrix, Vector( 1, 0, 0 ), (angDeltaAverage.z + angTemp.z) * GetIronSightAmountGained() );
			MatrixRotate( matIronSightMatrix, Vector( 0, 1, 0 ), (angDeltaAverage.x + angTemp.x) * GetIronSightAmountGained() );
			MatrixRotate( matIronSightMatrix, Vector( 0, 0, 1 ), (angDeltaAverage.y + angTemp.y) * GetIronSightAmountGained() );
		}
	}
	else
#endif
	{
		
		//use schema defined angles
		MatrixRotate( matIronSightMatrix, Vector( 1, 0, 0 ), (angDeltaAverage.z + m_angPivotAngle.z) * GetIronSightAmountGained() );
		MatrixRotate( matIronSightMatrix, Vector( 0, 1, 0 ), (angDeltaAverage.x + m_angPivotAngle.x) * GetIronSightAmountGained() );
		MatrixRotate( matIronSightMatrix, Vector( 0, 0, 1 ), (angDeltaAverage.y + m_angPivotAngle.y) * GetIronSightAmountGained() );

	}
	
	if ( !cl_righthand.GetBool() )
		angDeltaAverage = -angDeltaAverage;

	//move the weapon back to the ironsight eye position
#ifdef DEBUG
	if ( ironsight_override.GetBool() )
	{
		MatrixTranslate( matIronSightMatrix, Vector( -ironsight_pivot_forward.GetFloat(), 0, 0 ) );
	}
	else
#endif
	{
		MatrixTranslate( matIronSightMatrix, Vector( -m_flIronSightPivotForward, 0, 0 ) );
	}	


	//if the player is moving, pull down and re-bob the weapon
	if ( m_pAttachedWeapon->GetOwner() )
	{
		//magic bob value, replace me
		Vector vecIronSightBob = Vector(
			1,
			IRONSIGHT_VIEWMODEL_BOB_MULT_X * sin( gpGlobals->curtime * IRONSIGHT_VIEWMODEL_BOB_PERIOD_X ),			
			IRONSIGHT_VIEWMODEL_BOB_MULT_Y * sin( gpGlobals->curtime * IRONSIGHT_VIEWMODEL_BOB_PERIOD_Y ) - IRONSIGHT_VIEWMODEL_BOB_MULT_Y
			);
	
		m_vecDotCoords.x = -vecIronSightBob.y;
		m_vecDotCoords.y = -vecIronSightBob.z;
		m_vecDotCoords *= 0.1f;
		m_vecDotCoords.x -= angDeltaAverage.y * 0.03f;
		m_vecDotCoords.y += angDeltaAverage.x * 0.03f;
		m_vecDotCoords *= m_flSpeedRatio;

		if ( !cl_righthand.GetBool() )
			vecIronSightBob.y = -vecIronSightBob.y;

		MatrixTranslate( matIronSightMatrix, vecIronSightBob * m_flSpeedRatio );
	}


	//extract the final position and angles and apply them as differences from the passed in values
	vecPosition -= ( vecPosition - matIronSightMatrix.GetTranslation() );

	QAngle angIronSightAngles;
	MatrixAngles( matIronSightMatrix.As3x4(), angIronSightAngles );
	angAngle -= QAngleDiff( angAngle, angIronSightAngles );

	//dampen dot blur
	m_flDotBlur = Approach(0.0f, m_flDotBlur, gpGlobals->frametime * 2.0f);

}

#ifdef DEBUG
ConVar r_ironsight_scope_effect("r_ironsight_scope_effect", "1", FCVAR_DEVELOPMENTONLY | FCVAR_CHEAT);
ConVar ironsight_laser_dot_render_tweak1("ironsight_laser_dot_render_tweak1", "61", FCVAR_DEVELOPMENTONLY | FCVAR_CHEAT);
ConVar ironsight_laser_dot_render_tweak2("ironsight_laser_dot_render_tweak2", "64", FCVAR_DEVELOPMENTONLY | FCVAR_CHEAT);
#endif

void CIronSightController::RenderScopeEffect( int x, int y, int w, int h, CViewSetup *pViewSetup )
{

#ifdef DEBUG
	if ( !r_ironsight_scope_effect.GetBool() )
		return;
#endif

	if ( !IsInIronSight() )
		return;

	CMatRenderContextPtr pRenderContext(materials);

	// now draw the laser dot

	Vector2D dotCoords = GetDotCoords();
	dotCoords.x *= engine->GetScreenAspectRatio();

	//CMatRenderContextPtr pRenderContext(materials);
	IMaterial *pMatDot = materials->FindMaterial(GetDotMaterial(), TEXTURE_GROUP_OTHER, true);

	int iWidth = GetDotWidth();

	IMaterialVar *pAlphaVar2 = pMatDot->FindVar("$alpha", 0);
	if (pAlphaVar2 != NULL)
	{
		pAlphaVar2->SetFloatValue(GetDotBlur());
	}

	dotCoords.x += 0.5f;
	dotCoords.y += 0.5f;

	if ( !IsErrorMaterial(pMatDot) )
	{
#ifdef DEBUG
		pRenderContext->DrawScreenSpaceRectangle( pMatDot, (w * dotCoords.x) - (iWidth / 2), (h * dotCoords.y) - (iWidth / 2), iWidth, iWidth,
												  0, 0, ironsight_laser_dot_render_tweak1.GetInt(), ironsight_laser_dot_render_tweak1.GetInt(),
												  ironsight_laser_dot_render_tweak2.GetInt(), ironsight_laser_dot_render_tweak2.GetInt() );
#else
		pRenderContext->DrawScreenSpaceRectangle( pMatDot, (w * dotCoords.x) - (iWidth / 2), (h * dotCoords.y) - (iWidth / 2), iWidth, iWidth,
												  0, 0, 61, 61,
												  64, 64 );
#endif
	}
}

#endif //CLIENT_DLL

bool CIronSightController::IsInIronSight( void )
{
	if ( m_pAttachedWeapon )
	{
		if ( IsDeploying() ||
			 IsDropped() ||
			 m_pAttachedWeapon->m_bInReload ||
			 m_pAttachedWeapon->IsSwitchingSilencer() )
			return false;

		CCSPlayer *pPlayer = ToCSPlayer( m_pAttachedWeapon->GetOwner() );
		if ( pPlayer && pPlayer->IsLookingAtWeapon() )
			return false;

#if defined ( CLIENT_DLL )
		C_CSPlayer *pLocalPlayer = C_CSPlayer::GetLocalCSPlayer();
		if ( pLocalPlayer && pLocalPlayer->GetObserverTarget() == pPlayer && pLocalPlayer->GetObserverInterpState() == C_CSPlayer::OBSERVER_INTERP_TRAVELING )
			return false;
#endif

		if ( GetIronSightAmount() > 0  && (IsApproachingSighted() || IsApproachingUnSighted()) )
			return true;
	}

	return false;
}

float CIronSightController::GetIronSightFOV( float flDefaultFOV, bool bUseBiasedValue /*= false*/ )
{
	//sets biased value between the current FOV and the ideal IronSight FOV based on how 'ironsighted' the weapon currently is
	
	if ( !IsInIronSight() )
		return flDefaultFOV;

	float flIronSightFOVAmount = bUseBiasedValue ? GetIronSightAmountBiased() : GetIronSightAmount();

#ifdef DEBUG
	if ( ironsight_override.GetBool() )
	{
		return Lerp( flIronSightFOVAmount, flDefaultFOV, ironsight_fov.GetFloat() );
	}
#endif
	
	return Lerp( flIronSightFOVAmount, flDefaultFOV, GetIronSightIdealFOV() );
}

bool CIronSightController::Init( CWeaponCSBase *pWeaponToMonitor )
{
	if ( IsInitializedAndAvailable() )
	{
		return true;
	}
	else if( pWeaponToMonitor )
	{
		if (!pWeaponToMonitor->GetCSWpnData().m_bIronsightCapable)
		{
			return false;
		}

		m_pAttachedWeapon = pWeaponToMonitor;

		m_bIronSightAvailable			= pWeaponToMonitor->GetCSWpnData().m_bIronsightCapable;
		m_flIronSightLooseness			= pWeaponToMonitor->GetCSWpnData().m_flIronsightLooseness;
		m_flIronSightPullUpSpeed		= pWeaponToMonitor->GetCSWpnData().m_flIronsightSpeedUp;
		m_flIronSightPutDownSpeed		= pWeaponToMonitor->GetCSWpnData().m_flIronsightSpeedDown;
		m_flIronSightFOV				= pWeaponToMonitor->GetCSWpnData().m_flIronsightFOV;
		m_flIronSightPivotForward		= pWeaponToMonitor->GetCSWpnData().m_flIronsightPivotForward;
		m_vecEyePos						= Vector(pWeaponToMonitor->GetCSWpnData().m_vecIronsightEyePos.x,
												 pWeaponToMonitor->GetCSWpnData().m_vecIronsightEyePos.y,
												 pWeaponToMonitor->GetCSWpnData().m_vecIronsightEyePos.z);
		m_angPivotAngle					= QAngle(pWeaponToMonitor->GetCSWpnData().m_angIronsightPivotAngle.x,
												 pWeaponToMonitor->GetCSWpnData().m_angIronsightPivotAngle.y,
												 pWeaponToMonitor->GetCSWpnData().m_angIronsightPivotAngle.z);
		return true;
	}
	
	return false;
}

#endif //IRONSIGHT