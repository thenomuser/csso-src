#include "shaderlib/cshader.h"
class splinerope_ps20b_Static_Index
{
private:
	int m_nSHADOWDEPTH;
#ifdef _DEBUG
	bool m_bSHADOWDEPTH;
#endif
public:
	void SetSHADOWDEPTH( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nSHADOWDEPTH = i;
#ifdef _DEBUG
		m_bSHADOWDEPTH = true;
#endif
	}
	void SetSHADOWDEPTH( bool i )
	{
		m_nSHADOWDEPTH = i ? 1 : 0;
#ifdef _DEBUG
		m_bSHADOWDEPTH = true;
#endif
	}
private:
	int m_nALPHATESTREF;
#ifdef _DEBUG
	bool m_bALPHATESTREF;
#endif
public:
	void SetALPHATESTREF( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nALPHATESTREF = i;
#ifdef _DEBUG
		m_bALPHATESTREF = true;
#endif
	}
	void SetALPHATESTREF( bool i )
	{
		m_nALPHATESTREF = i ? 1 : 0;
#ifdef _DEBUG
		m_bALPHATESTREF = true;
#endif
	}
public:
	splinerope_ps20b_Static_Index( )
	{
#ifdef _DEBUG
		m_bSHADOWDEPTH = false;
#endif // _DEBUG
		m_nSHADOWDEPTH = 0;
#ifdef _DEBUG
		m_bALPHATESTREF = false;
#endif // _DEBUG
		m_nALPHATESTREF = 0;
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
		bool bAllStaticVarsDefined = m_bSHADOWDEPTH && m_bALPHATESTREF;
		Assert( bAllStaticVarsDefined );
#endif // _DEBUG
		return ( 4 * m_nSHADOWDEPTH ) + ( 8 * m_nALPHATESTREF ) + 0;
	}
};
#define shaderStaticTest_splinerope_ps20b psh_forgot_to_set_static_SHADOWDEPTH + psh_forgot_to_set_static_ALPHATESTREF + 0
class splinerope_ps20b_Dynamic_Index
{
private:
	int m_nWRITE_DEPTH_TO_DESTALPHA;
#ifdef _DEBUG
	bool m_bWRITE_DEPTH_TO_DESTALPHA;
#endif
public:
	void SetWRITE_DEPTH_TO_DESTALPHA( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nWRITE_DEPTH_TO_DESTALPHA = i;
#ifdef _DEBUG
		m_bWRITE_DEPTH_TO_DESTALPHA = true;
#endif
	}
	void SetWRITE_DEPTH_TO_DESTALPHA( bool i )
	{
		m_nWRITE_DEPTH_TO_DESTALPHA = i ? 1 : 0;
#ifdef _DEBUG
		m_bWRITE_DEPTH_TO_DESTALPHA = true;
#endif
	}
private:
	int m_nPIXELFOGTYPE;
#ifdef _DEBUG
	bool m_bPIXELFOGTYPE;
#endif
public:
	void SetPIXELFOGTYPE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nPIXELFOGTYPE = i;
#ifdef _DEBUG
		m_bPIXELFOGTYPE = true;
#endif
	}
	void SetPIXELFOGTYPE( bool i )
	{
		m_nPIXELFOGTYPE = i ? 1 : 0;
#ifdef _DEBUG
		m_bPIXELFOGTYPE = true;
#endif
	}
public:
	splinerope_ps20b_Dynamic_Index()
	{
#ifdef _DEBUG
		m_bWRITE_DEPTH_TO_DESTALPHA = false;
#endif // _DEBUG
		m_nWRITE_DEPTH_TO_DESTALPHA = 0;
#ifdef _DEBUG
		m_bPIXELFOGTYPE = false;
#endif // _DEBUG
		m_nPIXELFOGTYPE = 0;
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
		bool bAllDynamicVarsDefined = m_bWRITE_DEPTH_TO_DESTALPHA && m_bPIXELFOGTYPE;
		Assert( bAllDynamicVarsDefined );
#endif // _DEBUG
		return ( 1 * m_nWRITE_DEPTH_TO_DESTALPHA ) + ( 2 * m_nPIXELFOGTYPE ) + 0;
	}
};
#define shaderDynamicTest_splinerope_ps20b psh_forgot_to_set_dynamic_WRITE_DEPTH_TO_DESTALPHA + psh_forgot_to_set_dynamic_PIXELFOGTYPE + 0
