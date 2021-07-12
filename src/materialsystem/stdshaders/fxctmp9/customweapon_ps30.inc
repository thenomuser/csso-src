#include "shaderlib/cshader.h"
class customweapon_ps30_Static_Index
{
private:
	int m_nPAINTSTYLE;
#ifdef _DEBUG
	bool m_bPAINTSTYLE;
#endif
public:
	void SetPAINTSTYLE( int i )
	{
		Assert( i >= 0 && i <= 9 );
		m_nPAINTSTYLE = i;
#ifdef _DEBUG
		m_bPAINTSTYLE = true;
#endif
	}
	void SetPAINTSTYLE( bool i )
	{
		m_nPAINTSTYLE = i ? 1 : 0;
#ifdef _DEBUG
		m_bPAINTSTYLE = true;
#endif
	}
private:
	int m_nEXPONENTMODE;
#ifdef _DEBUG
	bool m_bEXPONENTMODE;
#endif
public:
	void SetEXPONENTMODE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nEXPONENTMODE = i;
#ifdef _DEBUG
		m_bEXPONENTMODE = true;
#endif
	}
	void SetEXPONENTMODE( bool i )
	{
		m_nEXPONENTMODE = i ? 1 : 0;
#ifdef _DEBUG
		m_bEXPONENTMODE = true;
#endif
	}
private:
	int m_nCHEAPMODE;
#ifdef _DEBUG
	bool m_bCHEAPMODE;
#endif
public:
	void SetCHEAPMODE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nCHEAPMODE = i;
#ifdef _DEBUG
		m_bCHEAPMODE = true;
#endif
	}
	void SetCHEAPMODE( bool i )
	{
		m_nCHEAPMODE = i ? 1 : 0;
#ifdef _DEBUG
		m_bCHEAPMODE = true;
#endif
	}
private:
	int m_nPREVIEW;
#ifdef _DEBUG
	bool m_bPREVIEW;
#endif
public:
	void SetPREVIEW( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nPREVIEW = i;
#ifdef _DEBUG
		m_bPREVIEW = true;
#endif
	}
	void SetPREVIEW( bool i )
	{
		m_nPREVIEW = i ? 1 : 0;
#ifdef _DEBUG
		m_bPREVIEW = true;
#endif
	}
private:
	int m_nPREVIEWPHONGALBEDOTINT;
#ifdef _DEBUG
	bool m_bPREVIEWPHONGALBEDOTINT;
#endif
public:
	void SetPREVIEWPHONGALBEDOTINT( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nPREVIEWPHONGALBEDOTINT = i;
#ifdef _DEBUG
		m_bPREVIEWPHONGALBEDOTINT = true;
#endif
	}
	void SetPREVIEWPHONGALBEDOTINT( bool i )
	{
		m_nPREVIEWPHONGALBEDOTINT = i ? 1 : 0;
#ifdef _DEBUG
		m_bPREVIEWPHONGALBEDOTINT = true;
#endif
	}
private:
	int m_nPHONGALBEDOFACTORMODE;
#ifdef _DEBUG
	bool m_bPHONGALBEDOFACTORMODE;
#endif
public:
	void SetPHONGALBEDOFACTORMODE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nPHONGALBEDOFACTORMODE = i;
#ifdef _DEBUG
		m_bPHONGALBEDOFACTORMODE = true;
#endif
	}
	void SetPHONGALBEDOFACTORMODE( bool i )
	{
		m_nPHONGALBEDOFACTORMODE = i ? 1 : 0;
#ifdef _DEBUG
		m_bPHONGALBEDOFACTORMODE = true;
#endif
	}
public:
	customweapon_ps30_Static_Index( )
	{
#ifdef _DEBUG
		m_bPAINTSTYLE = false;
#endif // _DEBUG
		m_nPAINTSTYLE = 0;
#ifdef _DEBUG
		m_bEXPONENTMODE = false;
#endif // _DEBUG
		m_nEXPONENTMODE = 0;
#ifdef _DEBUG
		m_bCHEAPMODE = false;
#endif // _DEBUG
		m_nCHEAPMODE = 0;
#ifdef _DEBUG
		m_bPREVIEW = false;
#endif // _DEBUG
		m_nPREVIEW = 0;
#ifdef _DEBUG
		m_bPREVIEWPHONGALBEDOTINT = false;
#endif // _DEBUG
		m_nPREVIEWPHONGALBEDOTINT = 0;
#ifdef _DEBUG
		m_bPHONGALBEDOFACTORMODE = false;
#endif // _DEBUG
		m_nPHONGALBEDOFACTORMODE = 0;
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
		bool bAllStaticVarsDefined = m_bPAINTSTYLE && m_bEXPONENTMODE && m_bCHEAPMODE && m_bPREVIEW && m_bPREVIEWPHONGALBEDOTINT && m_bPHONGALBEDOFACTORMODE;
		Assert( bAllStaticVarsDefined );
#endif // _DEBUG
		return ( 5 * m_nPAINTSTYLE ) + ( 50 * m_nEXPONENTMODE ) + ( 100 * m_nCHEAPMODE ) + ( 200 * m_nPREVIEW ) + ( 400 * m_nPREVIEWPHONGALBEDOTINT ) + ( 800 * m_nPHONGALBEDOFACTORMODE ) + 0;
	}
};
#define shaderStaticTest_customweapon_ps30 psh_forgot_to_set_static_PAINTSTYLE + psh_forgot_to_set_static_EXPONENTMODE + psh_forgot_to_set_static_CHEAPMODE + psh_forgot_to_set_static_PREVIEW + psh_forgot_to_set_static_PREVIEWPHONGALBEDOTINT + psh_forgot_to_set_static_PHONGALBEDOFACTORMODE + 0
class customweapon_ps30_Dynamic_Index
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
public:
	customweapon_ps30_Dynamic_Index()
	{
#ifdef _DEBUG
		m_bNUM_LIGHTS = false;
#endif // _DEBUG
		m_nNUM_LIGHTS = 0;
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
		bool bAllDynamicVarsDefined = m_bNUM_LIGHTS;
		Assert( bAllDynamicVarsDefined );
#endif // _DEBUG
		return ( 1 * m_nNUM_LIGHTS ) + 0;
	}
};
#define shaderDynamicTest_customweapon_ps30 psh_forgot_to_set_dynamic_NUM_LIGHTS + 0
