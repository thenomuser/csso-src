#include "shaderlib/cshader.h"
class character_ps30_Static_Index
{
private:
	int m_nMASKS1;
#ifdef _DEBUG
	bool m_bMASKS1;
#endif
public:
	void SetMASKS1( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nMASKS1 = i;
#ifdef _DEBUG
		m_bMASKS1 = true;
#endif
	}
	void SetMASKS1( bool i )
	{
		m_nMASKS1 = i ? 1 : 0;
#ifdef _DEBUG
		m_bMASKS1 = true;
#endif
	}
private:
	int m_nMASKS2;
#ifdef _DEBUG
	bool m_bMASKS2;
#endif
public:
	void SetMASKS2( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nMASKS2 = i;
#ifdef _DEBUG
		m_bMASKS2 = true;
#endif
	}
	void SetMASKS2( bool i )
	{
		m_nMASKS2 = i ? 1 : 0;
#ifdef _DEBUG
		m_bMASKS2 = true;
#endif
	}
private:
	int m_nFRESNELRANGESTEXTURE;
#ifdef _DEBUG
	bool m_bFRESNELRANGESTEXTURE;
#endif
public:
	void SetFRESNELRANGESTEXTURE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nFRESNELRANGESTEXTURE = i;
#ifdef _DEBUG
		m_bFRESNELRANGESTEXTURE = true;
#endif
	}
	void SetFRESNELRANGESTEXTURE( bool i )
	{
		m_nFRESNELRANGESTEXTURE = i ? 1 : 0;
#ifdef _DEBUG
		m_bFRESNELRANGESTEXTURE = true;
#endif
	}
private:
	int m_nPHONGWARPTEXTURE;
#ifdef _DEBUG
	bool m_bPHONGWARPTEXTURE;
#endif
public:
	void SetPHONGWARPTEXTURE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nPHONGWARPTEXTURE = i;
#ifdef _DEBUG
		m_bPHONGWARPTEXTURE = true;
#endif
	}
	void SetPHONGWARPTEXTURE( bool i )
	{
		m_nPHONGWARPTEXTURE = i ? 1 : 0;
#ifdef _DEBUG
		m_bPHONGWARPTEXTURE = true;
#endif
	}
private:
	int m_nENVMAP;
#ifdef _DEBUG
	bool m_bENVMAP;
#endif
public:
	void SetENVMAP( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nENVMAP = i;
#ifdef _DEBUG
		m_bENVMAP = true;
#endif
	}
	void SetENVMAP( bool i )
	{
		m_nENVMAP = i ? 1 : 0;
#ifdef _DEBUG
		m_bENVMAP = true;
#endif
	}
private:
	int m_nAMBIENTREFLECTION;
#ifdef _DEBUG
	bool m_bAMBIENTREFLECTION;
#endif
public:
	void SetAMBIENTREFLECTION( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nAMBIENTREFLECTION = i;
#ifdef _DEBUG
		m_bAMBIENTREFLECTION = true;
#endif
	}
	void SetAMBIENTREFLECTION( bool i )
	{
		m_nAMBIENTREFLECTION = i ? 1 : 0;
#ifdef _DEBUG
		m_bAMBIENTREFLECTION = true;
#endif
	}
private:
	int m_nUSEBOUNCECOLOR;
#ifdef _DEBUG
	bool m_bUSEBOUNCECOLOR;
#endif
public:
	void SetUSEBOUNCECOLOR( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nUSEBOUNCECOLOR = i;
#ifdef _DEBUG
		m_bUSEBOUNCECOLOR = true;
#endif
	}
	void SetUSEBOUNCECOLOR( bool i )
	{
		m_nUSEBOUNCECOLOR = i ? 1 : 0;
#ifdef _DEBUG
		m_bUSEBOUNCECOLOR = true;
#endif
	}
private:
	int m_nANISOTROPY;
#ifdef _DEBUG
	bool m_bANISOTROPY;
#endif
public:
	void SetANISOTROPY( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nANISOTROPY = i;
#ifdef _DEBUG
		m_bANISOTROPY = true;
#endif
	}
	void SetANISOTROPY( bool i )
	{
		m_nANISOTROPY = i ? 1 : 0;
#ifdef _DEBUG
		m_bANISOTROPY = true;
#endif
	}
private:
	int m_nBASEALPHAPHONGMASK;
#ifdef _DEBUG
	bool m_bBASEALPHAPHONGMASK;
#endif
public:
	void SetBASEALPHAPHONGMASK( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nBASEALPHAPHONGMASK = i;
#ifdef _DEBUG
		m_bBASEALPHAPHONGMASK = true;
#endif
	}
	void SetBASEALPHAPHONGMASK( bool i )
	{
		m_nBASEALPHAPHONGMASK = i ? 1 : 0;
#ifdef _DEBUG
		m_bBASEALPHAPHONGMASK = true;
#endif
	}
private:
	int m_nBASEALPHAENVMASK;
#ifdef _DEBUG
	bool m_bBASEALPHAENVMASK;
#endif
public:
	void SetBASEALPHAENVMASK( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nBASEALPHAENVMASK = i;
#ifdef _DEBUG
		m_bBASEALPHAENVMASK = true;
#endif
	}
	void SetBASEALPHAENVMASK( bool i )
	{
		m_nBASEALPHAENVMASK = i ? 1 : 0;
#ifdef _DEBUG
		m_bBASEALPHAENVMASK = true;
#endif
	}
private:
	int m_nBUMPALPHAENVMASK;
#ifdef _DEBUG
	bool m_bBUMPALPHAENVMASK;
#endif
public:
	void SetBUMPALPHAENVMASK( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nBUMPALPHAENVMASK = i;
#ifdef _DEBUG
		m_bBUMPALPHAENVMASK = true;
#endif
	}
	void SetBUMPALPHAENVMASK( bool i )
	{
		m_nBUMPALPHAENVMASK = i ? 1 : 0;
#ifdef _DEBUG
		m_bBUMPALPHAENVMASK = true;
#endif
	}
private:
	int m_nSHADOWSATURATION;
#ifdef _DEBUG
	bool m_bSHADOWSATURATION;
#endif
public:
	void SetSHADOWSATURATION( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nSHADOWSATURATION = i;
#ifdef _DEBUG
		m_bSHADOWSATURATION = true;
#endif
	}
	void SetSHADOWSATURATION( bool i )
	{
		m_nSHADOWSATURATION = i ? 1 : 0;
#ifdef _DEBUG
		m_bSHADOWSATURATION = true;
#endif
	}
private:
	int m_nBASEALPHASELFILLUMMASK;
#ifdef _DEBUG
	bool m_bBASEALPHASELFILLUMMASK;
#endif
public:
	void SetBASEALPHASELFILLUMMASK( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nBASEALPHASELFILLUMMASK = i;
#ifdef _DEBUG
		m_bBASEALPHASELFILLUMMASK = true;
#endif
	}
	void SetBASEALPHASELFILLUMMASK( bool i )
	{
		m_nBASEALPHASELFILLUMMASK = i ? 1 : 0;
#ifdef _DEBUG
		m_bBASEALPHASELFILLUMMASK = true;
#endif
	}
private:
	int m_nFAKERIM;
#ifdef _DEBUG
	bool m_bFAKERIM;
#endif
public:
	void SetFAKERIM( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nFAKERIM = i;
#ifdef _DEBUG
		m_bFAKERIM = true;
#endif
	}
	void SetFAKERIM( bool i )
	{
		m_nFAKERIM = i ? 1 : 0;
#ifdef _DEBUG
		m_bFAKERIM = true;
#endif
	}
private:
	int m_nDOPREVIEW;
#ifdef _DEBUG
	bool m_bDOPREVIEW;
#endif
public:
	void SetDOPREVIEW( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDOPREVIEW = i;
#ifdef _DEBUG
		m_bDOPREVIEW = true;
#endif
	}
	void SetDOPREVIEW( bool i )
	{
		m_nDOPREVIEW = i ? 1 : 0;
#ifdef _DEBUG
		m_bDOPREVIEW = true;
#endif
	}
private:
	int m_nUSEPATTERN;
#ifdef _DEBUG
	bool m_bUSEPATTERN;
#endif
public:
	void SetUSEPATTERN( int i )
	{
		Assert( i >= 0 && i <= 4 );
		m_nUSEPATTERN = i;
#ifdef _DEBUG
		m_bUSEPATTERN = true;
#endif
	}
	void SetUSEPATTERN( bool i )
	{
		m_nUSEPATTERN = i ? 1 : 0;
#ifdef _DEBUG
		m_bUSEPATTERN = true;
#endif
	}
private:
	int m_nFLASHLIGHT;
#ifdef _DEBUG
	bool m_bFLASHLIGHT;
#endif
public:
	void SetFLASHLIGHT( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nFLASHLIGHT = i;
#ifdef _DEBUG
		m_bFLASHLIGHT = true;
#endif
	}
	void SetFLASHLIGHT( bool i )
	{
		m_nFLASHLIGHT = i ? 1 : 0;
#ifdef _DEBUG
		m_bFLASHLIGHT = true;
#endif
	}
private:
	int m_nFLASHLIGHTDEPTHFILTERMODE;
#ifdef _DEBUG
	bool m_bFLASHLIGHTDEPTHFILTERMODE;
#endif
public:
	void SetFLASHLIGHTDEPTHFILTERMODE( int i )
	{
		Assert( i >= 0 && i <= 3 );
		m_nFLASHLIGHTDEPTHFILTERMODE = i;
#ifdef _DEBUG
		m_bFLASHLIGHTDEPTHFILTERMODE = true;
#endif
	}
	void SetFLASHLIGHTDEPTHFILTERMODE( bool i )
	{
		m_nFLASHLIGHTDEPTHFILTERMODE = i ? 1 : 0;
#ifdef _DEBUG
		m_bFLASHLIGHTDEPTHFILTERMODE = true;
#endif
	}
public:
	character_ps30_Static_Index( )
	{
#ifdef _DEBUG
		m_bMASKS1 = false;
#endif // _DEBUG
		m_nMASKS1 = 0;
#ifdef _DEBUG
		m_bMASKS2 = false;
#endif // _DEBUG
		m_nMASKS2 = 0;
#ifdef _DEBUG
		m_bFRESNELRANGESTEXTURE = false;
#endif // _DEBUG
		m_nFRESNELRANGESTEXTURE = 0;
#ifdef _DEBUG
		m_bPHONGWARPTEXTURE = false;
#endif // _DEBUG
		m_nPHONGWARPTEXTURE = 0;
#ifdef _DEBUG
		m_bENVMAP = false;
#endif // _DEBUG
		m_nENVMAP = 0;
#ifdef _DEBUG
		m_bAMBIENTREFLECTION = false;
#endif // _DEBUG
		m_nAMBIENTREFLECTION = 0;
#ifdef _DEBUG
		m_bUSEBOUNCECOLOR = false;
#endif // _DEBUG
		m_nUSEBOUNCECOLOR = 0;
#ifdef _DEBUG
		m_bANISOTROPY = false;
#endif // _DEBUG
		m_nANISOTROPY = 0;
#ifdef _DEBUG
		m_bBASEALPHAPHONGMASK = false;
#endif // _DEBUG
		m_nBASEALPHAPHONGMASK = 0;
#ifdef _DEBUG
		m_bBASEALPHAENVMASK = false;
#endif // _DEBUG
		m_nBASEALPHAENVMASK = 0;
#ifdef _DEBUG
		m_bBUMPALPHAENVMASK = false;
#endif // _DEBUG
		m_nBUMPALPHAENVMASK = 0;
#ifdef _DEBUG
		m_bSHADOWSATURATION = false;
#endif // _DEBUG
		m_nSHADOWSATURATION = 0;
#ifdef _DEBUG
		m_bBASEALPHASELFILLUMMASK = false;
#endif // _DEBUG
		m_nBASEALPHASELFILLUMMASK = 0;
#ifdef _DEBUG
		m_bFAKERIM = false;
#endif // _DEBUG
		m_nFAKERIM = 0;
#ifdef _DEBUG
		m_bDOPREVIEW = false;
#endif // _DEBUG
		m_nDOPREVIEW = 0;
#ifdef _DEBUG
		m_bUSEPATTERN = false;
#endif // _DEBUG
		m_nUSEPATTERN = 0;
#ifdef _DEBUG
		m_bFLASHLIGHT = false;
#endif // _DEBUG
		m_nFLASHLIGHT = 0;
#ifdef _DEBUG
		m_bFLASHLIGHTDEPTHFILTERMODE = false;
#endif // _DEBUG
		m_nFLASHLIGHTDEPTHFILTERMODE = 0;
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
		bool bAllStaticVarsDefined = m_bMASKS1 && m_bMASKS2 && m_bFRESNELRANGESTEXTURE && m_bPHONGWARPTEXTURE && m_bENVMAP && m_bAMBIENTREFLECTION && m_bUSEBOUNCECOLOR && m_bANISOTROPY && m_bBASEALPHAPHONGMASK && m_bBASEALPHAENVMASK && m_bBUMPALPHAENVMASK && m_bSHADOWSATURATION && m_bBASEALPHASELFILLUMMASK && m_bFAKERIM && m_bDOPREVIEW && m_bUSEPATTERN && m_bFLASHLIGHT && m_bFLASHLIGHTDEPTHFILTERMODE;
		Assert( bAllStaticVarsDefined );
#endif // _DEBUG
		return ( 20 * m_nMASKS1 ) + ( 40 * m_nMASKS2 ) + ( 80 * m_nFRESNELRANGESTEXTURE ) + ( 160 * m_nPHONGWARPTEXTURE ) + ( 320 * m_nENVMAP ) + ( 640 * m_nAMBIENTREFLECTION ) + ( 1280 * m_nUSEBOUNCECOLOR ) + ( 2560 * m_nANISOTROPY ) + ( 5120 * m_nBASEALPHAPHONGMASK ) + ( 10240 * m_nBASEALPHAENVMASK ) + ( 20480 * m_nBUMPALPHAENVMASK ) + ( 40960 * m_nSHADOWSATURATION ) + ( 81920 * m_nBASEALPHASELFILLUMMASK ) + ( 163840 * m_nFAKERIM ) + ( 327680 * m_nDOPREVIEW ) + ( 655360 * m_nUSEPATTERN ) + ( 3276800 * m_nFLASHLIGHT ) + ( 6553600 * m_nFLASHLIGHTDEPTHFILTERMODE ) + 0;
	}
};
#define shaderStaticTest_character_ps30 psh_forgot_to_set_static_MASKS1 + psh_forgot_to_set_static_MASKS2 + psh_forgot_to_set_static_FRESNELRANGESTEXTURE + psh_forgot_to_set_static_PHONGWARPTEXTURE + psh_forgot_to_set_static_ENVMAP + psh_forgot_to_set_static_AMBIENTREFLECTION + psh_forgot_to_set_static_USEBOUNCECOLOR + psh_forgot_to_set_static_ANISOTROPY + psh_forgot_to_set_static_BASEALPHAPHONGMASK + psh_forgot_to_set_static_BASEALPHAENVMASK + psh_forgot_to_set_static_BUMPALPHAENVMASK + psh_forgot_to_set_static_SHADOWSATURATION + psh_forgot_to_set_static_BASEALPHASELFILLUMMASK + psh_forgot_to_set_static_FAKERIM + psh_forgot_to_set_static_DOPREVIEW + psh_forgot_to_set_static_USEPATTERN + psh_forgot_to_set_static_FLASHLIGHT + psh_forgot_to_set_static_FLASHLIGHTDEPTHFILTERMODE + 0
class character_ps30_Dynamic_Index
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
	int m_nWRITE_DEPTH_TO_DESTALPHA;
#ifdef _DEBUG
	bool m_bWRITE_DEPTH_TO_DESTALPHA;
#endif
public:
	void SetWRITE_DEPTH_TO_DESTALPHA( int i )
	{
		Assert( i >= 0 && i <= 0 );
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
	int m_nWRITEWATERFOGTODESTALPHA;
#ifdef _DEBUG
	bool m_bWRITEWATERFOGTODESTALPHA;
#endif
public:
	void SetWRITEWATERFOGTODESTALPHA( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nWRITEWATERFOGTODESTALPHA = i;
#ifdef _DEBUG
		m_bWRITEWATERFOGTODESTALPHA = true;
#endif
	}
	void SetWRITEWATERFOGTODESTALPHA( bool i )
	{
		m_nWRITEWATERFOGTODESTALPHA = i ? 1 : 0;
#ifdef _DEBUG
		m_bWRITEWATERFOGTODESTALPHA = true;
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
	character_ps30_Dynamic_Index()
	{
#ifdef _DEBUG
		m_bNUM_LIGHTS = false;
#endif // _DEBUG
		m_nNUM_LIGHTS = 0;
#ifdef _DEBUG
		m_bWRITE_DEPTH_TO_DESTALPHA = false;
#endif // _DEBUG
		m_nWRITE_DEPTH_TO_DESTALPHA = 0;
#ifdef _DEBUG
		m_bWRITEWATERFOGTODESTALPHA = false;
#endif // _DEBUG
		m_nWRITEWATERFOGTODESTALPHA = 0;
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
		bool bAllDynamicVarsDefined = m_bNUM_LIGHTS && m_bWRITE_DEPTH_TO_DESTALPHA && m_bWRITEWATERFOGTODESTALPHA && m_bPIXELFOGTYPE;
		Assert( bAllDynamicVarsDefined );
#endif // _DEBUG
		return ( 1 * m_nNUM_LIGHTS ) + ( 5 * m_nWRITE_DEPTH_TO_DESTALPHA ) + ( 5 * m_nWRITEWATERFOGTODESTALPHA ) + ( 10 * m_nPIXELFOGTYPE ) + 0;
	}
};
#define shaderDynamicTest_character_ps30 psh_forgot_to_set_dynamic_NUM_LIGHTS + psh_forgot_to_set_dynamic_WRITE_DEPTH_TO_DESTALPHA + psh_forgot_to_set_dynamic_WRITEWATERFOGTODESTALPHA + psh_forgot_to_set_dynamic_PIXELFOGTYPE + 0
