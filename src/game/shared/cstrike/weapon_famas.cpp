//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"
#include "fx_cs_shared.h"


#if defined( CLIENT_DLL )

	#define CWeaponFamas C_WeaponFamas
	#include "c_cs_player.h"

#else

	#include "cs_player.h"

#endif


class CWeaponFamas : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponFamas, CWeaponCSBase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponFamas();

	virtual void PrimaryAttack();
	virtual void SecondaryAttack();
	virtual bool Deploy();

	virtual void ItemPostFrame();

	void FamasFire( float flSpread, bool bFireBurst );
	void FireRemaining();
	
	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_FAMAS; }

private:
	
	CWeaponFamas( const CWeaponFamas & );
	CNetworkVar( bool, m_bBurstMode );
	CNetworkVar( int, m_iBurstShotsRemaining );	
	float	m_fNextBurstShot;			// time to shoot the next bullet in burst fire mode
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponFamas, DT_WeaponFamas )

BEGIN_NETWORK_TABLE( CWeaponFamas, DT_WeaponFamas )
	#ifdef CLIENT_DLL
		RecvPropBool( RECVINFO( m_bBurstMode ) ),
		RecvPropInt( RECVINFO( m_iBurstShotsRemaining ) ),
	#else
		SendPropBool( SENDINFO( m_bBurstMode ) ),
		SendPropInt( SENDINFO( m_iBurstShotsRemaining ) ),
	#endif
END_NETWORK_TABLE()

#if defined(CLIENT_DLL)
BEGIN_PREDICTION_DATA( CWeaponFamas )
DEFINE_PRED_FIELD( m_iBurstShotsRemaining, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
DEFINE_PRED_FIELD( m_fNextBurstShot, FIELD_FLOAT, 0 ),
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_famas, CWeaponFamas );
PRECACHE_WEAPON_REGISTER( weapon_famas );


const float kFamasBurstCycleTime = 0.075f;


CWeaponFamas::CWeaponFamas()
{
	m_bBurstMode = false;
}


bool CWeaponFamas::Deploy( )
{
	m_iBurstShotsRemaining = 0;
	m_fNextBurstShot = 0.0f;

	return BaseClass::Deploy();
}


// Secondary attack could be three-round burst mode
void CWeaponFamas::SecondaryAttack()
{	
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	if ( m_bBurstMode )
	{
		ClientPrint( pPlayer, HUD_PRINTCENTER, "#Switch_To_FullAuto" );
		m_bBurstMode = false;
		m_weaponMode = Primary_Mode;
	}
	else
	{
		ClientPrint( pPlayer, HUD_PRINTCENTER, "#Switch_To_BurstFire" );
		m_bBurstMode = true;
		m_weaponMode = Secondary_Mode;
	}

#ifndef CLIENT_DLL
	pPlayer->EmitSound( "Weapon.AutoSemiAutoSwitch" );
#endif

	m_flNextSecondaryAttack = gpGlobals->curtime + 0.3;
}

void CWeaponFamas::ItemPostFrame()
{
	if ( m_iBurstShotsRemaining > 0 && gpGlobals->curtime >= m_fNextBurstShot )
		FireRemaining();

	BaseClass::ItemPostFrame();
}



// GOOSEMAN : FireRemaining used by Famas
void CWeaponFamas::FireRemaining()
{
	m_iClip1--;

	if (m_iClip1 < 0)
	{
		m_iClip1 = 0;
		m_iBurstShotsRemaining = 0;
		m_fNextBurstShot = 0.0f;
		return;
	}

	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		Error( "!pPlayer" );

	// Famas burst mode
	FX_FireBullets(
		pPlayer->entindex(),
		pPlayer->Weapon_ShootPosition(),
		pPlayer->GetFinalAimAngle(),
		GetWeaponID(),
		Secondary_Mode,
		CBaseEntity::GetPredictionRandomSeed() & 255,
		GetInaccuracy(),
		GetSpread(),
		m_fNextBurstShot);
	
	SendWeaponAnim( ACT_VM_PRIMARYATTACK );

	pPlayer->DoMuzzleFlash();
	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	pPlayer->m_iShotsFired++;

	--m_iBurstShotsRemaining;
	if ( m_iBurstShotsRemaining > 0 )
		m_fNextBurstShot += kFamasBurstCycleTime;
	else
		m_fNextBurstShot = 0.0f;

	// update accuracy
	m_fAccuracyPenalty += GetCSWpnData().m_fInaccuracyImpulseFire[Secondary_Mode];

	// table driven recoil
	Recoil( Secondary_Mode );

	m_flRecoilIndex += 1.0f;
}


void CWeaponFamas::PrimaryAttack()
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	// don't fire underwater
	if (pPlayer->GetWaterLevel() == 3)
	{
		PlayEmptySound( );
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.15;
		return;
	}

	pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	float flCycleTime = GetCSWpnData().m_flCycleTime[m_weaponMode];

	// change a few things if we're in burst mode
	if ( m_bBurstMode )
	{
		flCycleTime = 0.55f;
		m_iBurstShotsRemaining = 2;
		m_fNextBurstShot = gpGlobals->curtime + kFamasBurstCycleTime;
	}

	if ( !CSBaseGunFire( flCycleTime, m_weaponMode ) )
		return;
}


