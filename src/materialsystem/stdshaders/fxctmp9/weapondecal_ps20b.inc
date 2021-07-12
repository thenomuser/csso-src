#include "shaderlib/cshader.h"
class weapondecal_ps20b_Static_Index
{
private:
	int m_nPHONG;
#ifdef _DEBUG
	bool m_bPHONG;
#endif
public:
	void SetPHONG( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nPHONG = i;
#ifdef _DEBUG
		m_bPHONG = true;
#endif
	}
	void SetPHONG( bool i )
	{
		m_nPHONG = i ? 1 : 0;
#ifdef _DEBUG
		m_bPHONG = true;
#endif
	}
private:
	int m_nPHONGEXPONENTTEXTURE;
#ifdef _DEBUG
	bool m_bPHONGEXPONENTTEXTURE;
#endif
public:
	void SetPHONGEXPONENTTEXTURE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nPHONGEXPONENTTEXTURE = i;
#ifdef _DEBUG
		m_bPHONGEXPONENTTEXTURE = true;
#endif
	}
	void SetPHONGEXPONENTTEXTURE( bool i )
	{
		m_nPHONGEXPONENTTEXTURE = i ? 1 : 0;
#ifdef _DEBUG
		m_bPHONGEXPONENTTEXTURE = true;
#endif
	}
private:
	int m_nCUBEMAP;
#ifdef _DEBUG
	bool m_bCUBEMAP;
#endif
public:
	void SetCUBEMAP( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nCUBEMAP = i;
#ifdef _DEBUG
		m_bCUBEMAP = true;
#endif
	}
	void SetCUBEMAP( bool i )
	{
		m_nCUBEMAP = i ? 1 : 0;
#ifdef _DEBUG
		m_bCUBEMAP = true;
#endif
	}
private:
	int m_nDECALSTYLE;
#ifdef _DEBUG
	bool m_bDECALSTYLE;
#endif
public:
	void SetDECALSTYLE( int i )
	{
		Assert( i >= 0 && i <= 5 );
		m_nDECALSTYLE = i;
#ifdef _DEBUG
		m_bDECALSTYLE = true;
#endif
	}
	void SetDECALSTYLE( bool i )
	{
		m_nDECALSTYLE = i ? 1 : 0;
#ifdef _DEBUG
		m_bDECALSTYLE = true;
#endif
	}
private:
	int m_nTHIRDPERSON;
#ifdef _DEBUG
	bool m_bTHIRDPERSON;
#endif
public:
	void SetTHIRDPERSON( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nTHIRDPERSON = i;
#ifdef _DEBUG
		m_bTHIRDPERSON = true;
#endif
	}
	void SetTHIRDPERSON( bool i )
	{
		m_nTHIRDPERSON = i ? 1 : 0;
#ifdef _DEBUG
		m_bTHIRDPERSON = true;
#endif
	}
private:
	int m_nDESATBASETINT;
#ifdef _DEBUG
	bool m_bDESATBASETINT;
#endif
public:
	void SetDESATBASETINT( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDESATBASETINT = i;
#ifdef _DEBUG
		m_bDESATBASETINT = true;
#endif
	}
	void SetDESATBASETINT( bool i )
	{
		m_nDESATBASETINT = i ? 1 : 0;
#ifdef _DEBUG
		m_bDESATBASETINT = true;
#endif
	}
public:
	weapondecal_ps20b_Static_Index( )
	{
#ifdef _DEBUG
		m_bPHONG = false;
#endif // _DEBUG
		m_nPHONG = 0;
#ifdef _DEBUG
		m_bPHONGEXPONENTTEXTURE = false;
#endif // _DEBUG
		m_nPHONGEXPONENTTEXTURE = 0;
#ifdef _DEBUG
		m_bCUBEMAP = false;
#endif // _DEBUG
		m_nCUBEMAP = 0;
#ifdef _DEBUG
		m_bDECALSTYLE = false;
#endif // _DEBUG
		m_nDECALSTYLE = 0;
#ifdef _DEBUG
		m_bTHIRDPERSON = false;
#endif // _DEBUG
		m_nTHIRDPERSON = 0;
#ifdef _DEBUG
		m_bDESATBASETINT = false;
#endif // _DEBUG
		m_nDESATBASETINT = 0;
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
		bool bAllStaticVarsDefined = m_bPHONG && m_bPHONGEXPONENTTEXTURE && m_bCUBEMAP && m_bDECALSTYLE && m_bTHIRDPERSON && m_bDESATBASETINT;
		Assert( bAllStaticVarsDefined );
#endif // _DEBUG
		return ( 20 * m_nPHONG ) + ( 40 * m_nPHONGEXPONENTTEXTURE ) + ( 80 * m_nCUBEMAP ) + ( 160 * m_nDECALSTYLE ) + ( 960 * m_nTHIRDPERSON ) + ( 1920 * m_nDESATBASETINT ) + 0;
	}
};
#define shaderStaticTest_weapondecal_ps20b psh_forgot_to_set_static_PHONG + psh_forgot_to_set_static_PHONGEXPONENTTEXTURE + psh_forgot_to_set_static_CUBEMAP + psh_forgot_to_set_static_DECALSTYLE + psh_forgot_to_set_static_THIRDPERSON + psh_forgot_to_set_static_DESATBASETINT + 0
class weapondecal_ps20b_Dynamic_Index
{
private:
	int m_nNUM_LIGHTS;
#ifdef _DEBUG
	bool m_bNUM_LIGHTS;
#endif
public:
	void SetNUM_LIGHTS( int i )
	{
		Assert( i >= 0 && i <= 4 );
		m_nNUM_LIGHTS = i;
#ifdef _DEBUG
		m_bNUM_LIGHTS = true;
#endif
	}
	void SetNUM_LIGHTS( bool i )
	{
		m_nNUM_LIGHTS = i ? 1 : 0;
#ifdef _DEBUG
		m_bNUM_LIGHTS = true;
#endif
	}
private:
	int m_nHIGHLIGHT;
#ifdef _DEBUG
	bool m_bHIGHLIGHT;
#endif
public:
	void SetHIGHLIGHT( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nHIGHLIGHT = i;
#ifdef _DEBUG
		m_bHIGHLIGHT = true;
#endif
	}
	void SetHIGHLIGHT( bool i )
	{
		m_nHIGHLIGHT = i ? 1 : 0;
#ifdef _DEBUG
		m_bHIGHLIGHT = true;
#endif
	}
private:
	int m_nPEEL;
#ifdef _DEBUG
	bool m_bPEEL;
#endif
public:
	void SetPEEL( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nPEEL = i;
#ifdef _DEBUG
		m_bPEEL = true;
#endif
	}
	void SetPEEL( bool i )
	{
		m_nPEEL = i ? 1 : 0;
#ifdef _DEBUG
		m_bPEEL = true;
#endif
	}
public:
	weapondecal_ps20b_Dynamic_Index()
	{
#ifdef _DEBUG
		m_bNUM_LIGHTS = false;
#endif // _DEBUG
		m_nNUM_LIGHTS = 0;
#ifdef _DEBUG
		m_bHIGHLIGHT = false;
#endif // _DEBUG
		m_nHIGHLIGHT = 0;
#ifdef _DEBUG
		m_bPEEL = false;
#endif // _DEBUG
		m_nPEEL = 0;
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
		bool bAllDynamicVarsDefined = m_bNUM_LIGHTS && m_bHIGHLIGHT && m_bPEEL;
		Assert( bAllDynamicVarsDefined );
#endif // _DEBUG
		return ( 1 * m_nNUM_LIGHTS ) + ( 5 * m_nHIGHLIGHT ) + ( 10 * m_nPEEL ) + 0;
	}
};
#define shaderDynamicTest_weapondecal_ps20b psh_forgot_to_set_dynamic_NUM_LIGHTS + psh_forgot_to_set_dynamic_HIGHLIGHT + psh_forgot_to_set_dynamic_PEEL + 0
