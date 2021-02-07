//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//


#if defined( WIN32 ) && !defined( _X360 )
#include <windows.h> // SRC only!!
#endif

#include "OptionsSubMultiplayer.h"
#include "MultiplayerAdvancedDialog.h"
#include <stdio.h>

#include <vgui_controls/Button.h>
#include <vgui_controls/QueryBox.h>
#include <vgui_controls/CheckButton.h>
#include "tier1/KeyValues.h"
#include <vgui_controls/Label.h>
#include <vgui/ISystem.h>
#include <vgui/ISurface.h>
#include <vgui/Cursor.h>
#include <vgui_controls/RadioButton.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/FileOpenDialog.h>
#include <vgui_controls/MessageBox.h>
#include <vgui/IVGui.h>
#include <vgui/ILocalize.h>
#include <vgui/IPanel.h>
#include <vgui_controls/MessageBox.h>

#include "CvarTextEntry.h"
#include "CvarToggleCheckButton.h"
#include "cvarslider.h"
#include "LabeledCommandComboBox.h"
#include "filesystem.h"
#include "EngineInterface.h"
#include "BitmapImagePanel.h"
#include "tier1/utlbuffer.h"
#include "ModInfo.h"
#include "tier1/convar.h"
#include "tier0/icommandline.h"

#include "materialsystem/imaterial.h"
#include "materialsystem/imesh.h"
#include "materialsystem/imaterialvar.h"

// use the JPEGLIB_USE_STDIO define so that we can read in jpeg's from outside the game directory tree.  For Spray Import.
#define JPEGLIB_USE_STDIO
#include "jpeglib/jpeglib.h"
#undef JPEGLIB_USE_STDIO

#include <setjmp.h>

#include "bitmap/tgawriter.h"
#include "ivtex.h"
#ifdef WIN32
#include <io.h>
#endif

#if defined( _X360 )
#include "xbox/xbox_win32stubs.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;


#define DEFAULT_SUIT_HUE 30
#define DEFAULT_PLATE_HUE 6

void UpdateLogoWAD( void *hdib, int r, int g, int b );

struct ColorItem_t
{
	const char	*name;
	int			r, g, b;
};

static ColorItem_t itemlist[]=
{
	{ "#Valve_Orange", 255, 120, 24 },
	{ "#Valve_Yellow", 225, 180, 24 },
	{ "#Valve_Blue", 0, 60, 255 },
	{ "#Valve_Ltblue", 0, 167, 255 },
	{ "#Valve_Green", 0, 167, 0 },
	{ "#Valve_Red", 255, 43, 0 },
	{ "#Valve_Brown", 123, 73, 0 },
	{ "#Valve_Ltgray", 100, 100, 100 },
	{ "#Valve_Dkgray", 36, 36, 36 },
};


//-----------------------------------------------------------------------------
// Purpose: Basic help dialog
//-----------------------------------------------------------------------------
COptionsSubMultiplayer::COptionsSubMultiplayer(vgui::Panel *parent) : vgui::PropertyPage(parent, "OptionsSubMultiplayer") 
{
	Button *cancel = new Button( this, "Cancel", "#GameUI_Cancel" );
	cancel->SetCommand( "Close" );

	Button *ok = new Button( this, "OK", "#GameUI_OK" );
	ok->SetCommand( "Ok" );

	Button *apply = new Button( this, "Apply", "#GameUI_Apply" );
	apply->SetCommand( "Apply" );

	Button *advanced = new Button( this, "Advanced", "#GameUI_AdvancedEllipsis" );
	advanced->SetCommand( "Advanced" );

	Button *importSprayImage = new Button( this, "ImportSprayImage", "#GameUI_ImportSprayEllipsis" );
	importSprayImage->SetCommand("ImportSprayImage");

	m_hImportSprayDialog = NULL;

	m_pPrimaryColorSlider = new CCvarSlider( this, "Primary Color Slider", "#GameUI_PrimaryColor",
		0.0f, 255.0f, "topcolor" );

	m_pSecondaryColorSlider = new CCvarSlider( this, "Secondary Color Slider", "#GameUI_SecondaryColor",
		0.0f, 255.0f, "bottomcolor" );

	m_pHighQualityModelCheckBox = new CCvarToggleCheckButton( this, "High Quality Models", "#GameUI_HighModels", "cl_himodels" );

	m_pModelList = new CLabeledCommandComboBox( this, "Player model" );
	m_ModelName[0] = 0;
	InitModelList( m_pModelList );

	m_pLogoList = new CLabeledCommandComboBox( this, "SpraypaintList" );
    m_LogoName[0] = 0;
	InitLogoList( m_pLogoList );

	m_pModelImage = new CBitmapImagePanel( this, "ModelImage", NULL );
	m_pModelImage->AddActionSignalTarget( this );

	m_pLogoImage = new ImagePanel( this, "LogoImage" );
	m_pLogoImage->AddActionSignalTarget( this );

	m_nLogoR = 255;
	m_nLogoG = 255;
	m_nLogoB = 255;

	m_pLockRadarRotationCheckbox = new CCvarToggleCheckButton( this, "LockRadarRotationCheckbox", "#Cstrike_RadarLocked", "cl_radar_locked" );

	m_pDownloadFilterCombo = new ComboBox( this, "DownloadFilterCheck", 4, false );
	m_pDownloadFilterCombo->AddItem( "#GameUI_DownloadFilter_ALL", NULL );
	m_pDownloadFilterCombo->AddItem( "#GameUI_DownloadFilter_NoSounds", NULL );
	m_pDownloadFilterCombo->AddItem( "#GameUI_DownloadFilter_MapsOnly", NULL );
	m_pDownloadFilterCombo->AddItem( "#GameUI_DownloadFilter_None", NULL );

	//=========

	LoadControlSettings("Resource/OptionsSubMultiplayer.res");

	// turn off model selection stuff if the mod specifies "nomodels" in the gameinfo.txt file
	if ( ModInfo().NoModels() )
	{
		Panel *pTempPanel = NULL;

		if ( m_pModelImage )
		{
			m_pModelImage->SetVisible( false );
		}

		if ( m_pModelList )
		{
			m_pModelList->SetVisible( false );
		}

		if ( m_pPrimaryColorSlider )
		{
			m_pPrimaryColorSlider->SetVisible( false );
		}

		if ( m_pSecondaryColorSlider )
		{
			m_pSecondaryColorSlider->SetVisible( false );
		}

		// #GameUI_PlayerModel (from "Resource/OptionsSubMultiplayer.res")
		pTempPanel = FindChildByName( "Label1" );

		if ( pTempPanel )
		{
			pTempPanel->SetVisible( false );
		}

		// #GameUI_ColorSliders (from "Resource/OptionsSubMultiplayer.res")
		pTempPanel = FindChildByName( "Colors" );

		if ( pTempPanel )
		{
			pTempPanel->SetVisible( false );
		}
	}

	// turn off the himodel stuff if the mod specifies "nohimodel" in the gameinfo.txt file
	if ( ModInfo().NoHiModel() )
	{
		if ( m_pHighQualityModelCheckBox )
		{
			m_pHighQualityModelCheckBox->SetVisible( false );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
COptionsSubMultiplayer::~COptionsSubMultiplayer()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void COptionsSubMultiplayer::OnCommand( const char *command )
{
	if ( !stricmp( command, "Advanced" ) )
	{
#ifndef _XBOX
		if (!m_hMultiplayerAdvancedDialog.Get())
		{
			m_hMultiplayerAdvancedDialog = new CMultiplayerAdvancedDialog( this );
		}
		m_hMultiplayerAdvancedDialog->Activate();
#endif
	}
	else if (!stricmp( command, "ImportSprayImage" ) )
	{
		if (m_hImportSprayDialog == NULL)
		{
			m_hImportSprayDialog = new FileOpenDialog(NULL, "#GameUI_ImportSprayImage", true);
#ifdef WIN32
			m_hImportSprayDialog->AddFilter("*.tga,*.jpg,*.bmp,*.vtf", "#GameUI_All_Images", true);
#else
			m_hImportSprayDialog->AddFilter("*.tga,*.jpg,*.vtf", "#GameUI_All_ImagesNoBmp", true);
#endif
			m_hImportSprayDialog->AddFilter("*.tga", "#GameUI_TGA_Images", false);
			m_hImportSprayDialog->AddFilter("*.jpg", "#GameUI_JPEG_Images", false);
#ifdef WIN32
			m_hImportSprayDialog->AddFilter("*.bmp", "#GameUI_BMP_Images", false);
#endif
			m_hImportSprayDialog->AddFilter("*.vtf", "#GameUI_VTF_Images", false);
			m_hImportSprayDialog->AddActionSignalTarget(this);
		}
		m_hImportSprayDialog->DoModal(false);
		m_hImportSprayDialog->Activate();
	}

	else if ( !stricmp( command, "ResetStats" ) )
	{
		QueryBox *box = new QueryBox("#GameUI_ConfirmResetStatsTitle", "#GameUI_ConfirmResetStatsText", this);
		box->SetOKButtonText("#GameUI_Reset");
		box->SetOKCommand(new KeyValues("Command", "command", "ResetStats_NoConfirm"));
		box->SetCancelCommand(new KeyValues("Command", "command", "ReleaseModalWindow"));
		box->AddActionSignalTarget(this);
		box->DoModal();
	}

	else if ( !stricmp( command, "ResetStats_NoConfirm" ) )
	{
		engine->ClientCmd_Unrestricted("stats_reset");
	}

	BaseClass::OnCommand( command );
}

void COptionsSubMultiplayer::ConversionError( ConversionErrorType nError )
{
	const char *pErrorText = NULL;

	switch ( nError )
	{
	case CE_MEMORY_ERROR:
		pErrorText = "#GameUI_Spray_Import_Error_Memory";
		break;

	case CE_CANT_OPEN_SOURCE_FILE:
		pErrorText = "#GameUI_Spray_Import_Error_Reading_Image";
		break;

	case CE_ERROR_PARSING_SOURCE:
		pErrorText = "#GameUI_Spray_Import_Error_Image_File_Corrupt";
		break;

	case CE_SOURCE_FILE_SIZE_NOT_SUPPORTED:
		pErrorText = "#GameUI_Spray_Import_Image_Wrong_Size";
		break;

	case CE_SOURCE_FILE_FORMAT_NOT_SUPPORTED:
		pErrorText = "#GameUI_Spray_Import_Image_Wrong_Size";
		break;

	case CE_SOURCE_FILE_TGA_FORMAT_NOT_SUPPORTED:
		pErrorText = "#GameUI_Spray_Import_Error_TGA_Format_Not_Supported";
		break;

	case CE_SOURCE_FILE_BMP_FORMAT_NOT_SUPPORTED:
		pErrorText = "#GameUI_Spray_Import_Error_BMP_Format_Not_Supported";
		break;

	case CE_ERROR_WRITING_OUTPUT_FILE:
		pErrorText = "#GameUI_Spray_Import_Error_Writing_Temp_Output";
		break;

	case CE_ERROR_LOADING_DLL:
		pErrorText = "#GameUI_Spray_Import_Error_Cant_Load_VTEX_DLL";
		break;
	}

	if ( pErrorText )
	{
		// Create the dialog
		vgui::MessageBox *pErrorDlg = new vgui::MessageBox("#GameUI_Spray_Import_Error_Title", pErrorText );	Assert( pErrorDlg );

		// Display
		if ( pErrorDlg )	// Check for a NULL just to be extra cautious...
		{
			pErrorDlg->DoModal();
		}
	}
}

void COptionsSubMultiplayer::OnFileSelected(const char *fullpath)
{
#ifndef _XBOX
	// this can take a while, put up a waiting cursor
	surface()->SetCursor(dc_hourglass);

	ConversionErrorType nErrorCode = ImgUtl_ConvertToVTFAndDumpVMT( fullpath, IsPosix() ? "/vgui/logos" : "\\vgui\\logos", 256, 256 );
	if ( nErrorCode == CE_SUCCESS )
	{
		// refresh the logo list so the new spray shows up.
		InitLogoList(m_pLogoList);

		// Get the filename
		char szRootFilename[MAX_PATH];
		V_FileBase( fullpath, szRootFilename, sizeof( szRootFilename ) );

		// automatically select the logo that was just imported.
		SelectLogo(szRootFilename);
	}
	else
	{
		ConversionError( nErrorCode );
	}

	// change the cursor back to normal
	surface()->SetCursor(dc_user);
#endif
}

struct ValveJpegErrorHandler_t 
{
	// The default manager
	struct jpeg_error_mgr	m_Base;
	// For handling any errors
	jmp_buf					m_ErrorContext;
};

//-----------------------------------------------------------------------------
// Purpose: We'll override the default error handler so we can deal with errors without having to exit the engine
//-----------------------------------------------------------------------------
static void ValveJpegErrorHandler( j_common_ptr cinfo )
{
	ValveJpegErrorHandler_t *pError = reinterpret_cast< ValveJpegErrorHandler_t * >( cinfo->err );

	char buffer[ JMSG_LENGTH_MAX ];

	/* Create the message */
	( *cinfo->err->format_message )( cinfo, buffer );

	Warning( "%s\n", buffer );

	// Bail
	longjmp( pError->m_ErrorContext, 1 );
}

//-----------------------------------------------------------------------------
// Purpose: Builds the list of logos
//-----------------------------------------------------------------------------
void COptionsSubMultiplayer::InitLogoList( CLabeledCommandComboBox *cb )
{
	// Find out images
	FileFindHandle_t fh;
	char directory[ 512 ];

	ConVarRef cl_logofile( "cl_logofile", true );
	if ( !cl_logofile.IsValid() )
		return;

	cb->DeleteAllItems();

	const char *logofile = cl_logofile.GetString();
	Q_snprintf( directory, sizeof( directory ), "materials/vgui/logos/*.vtf" );
	const char *fn = g_pFullFileSystem->FindFirst( directory, &fh );
	int i = 0, initialItem = 0; 
	while (fn)
	{
		char filename[ 512 ];
		Q_snprintf( filename, sizeof(filename), "materials/vgui/logos/%s", fn );
		if ( strlen( filename ) >= 4 )
		{
			filename[ strlen( filename ) - 4 ] = 0;
			Q_strncat( filename, ".vmt", sizeof( filename ), COPY_ALL_CHARACTERS );
			if ( g_pFullFileSystem->FileExists( filename ) )
			{
				// strip off the extension
				Q_strncpy( filename, fn, sizeof( filename ) );
				filename[ strlen( filename ) - 4 ] = 0;
				cb->AddItem( filename, "" );

				// check to see if this is the one we have set
				Q_snprintf( filename, sizeof(filename), "materials/vgui/logos/%s", fn );
				if (!Q_stricmp(filename, logofile))
				{
					initialItem = i;
				}

				++i;
			}
		}

		fn = g_pFullFileSystem->FindNext( fh );
	}

	g_pFullFileSystem->FindClose( fh );
	cb->SetInitialItem(initialItem);
}


//-----------------------------------------------------------------------------
// Purpose: Selects the given logo in the logo list.
//-----------------------------------------------------------------------------
void COptionsSubMultiplayer::SelectLogo(const char *logoName)
{
	int numEntries = m_pLogoList->GetItemCount();
	int index;
	wchar_t itemText[MAX_PATH];
	wchar_t itemToSelectText[MAX_PATH];

	// convert the logo filename to unicode
	g_pVGuiLocalize->ConvertANSIToUnicode(logoName, itemToSelectText, sizeof(itemToSelectText));

	// find the index of the spray we want.
	for (index = 0; index < numEntries; ++index)
	{
		m_pLogoList->GetItemText(index, itemText, sizeof(itemText));
		if (!wcscmp(itemText, itemToSelectText))
		{
			break;
		}
	}

	if (index < numEntries)
	{
		// select the logo.
		m_pLogoList->ActivateItem(index);
	}
}

#define MODEL_MATERIAL_BASE_FOLDER "materials/vgui/playermodels/"


void StripStringOutOfString( const char *pPattern, const char *pIn, char *pOut )
{
	int iLengthBase = strlen( pPattern );
	int iLengthString = strlen( pIn );

	int k = 0;

	for ( int j = iLengthBase; j < iLengthString; j++ )
	{
		pOut[k] = pIn[j];
		k++;
	}

	pOut[k] = 0;
}

void FindVMTFilesInFolder( const char *pFolder, const char *pFolderName, CLabeledCommandComboBox *cb, int &iCount, int &iInitialItem )
{
	ConVarRef cl_modelfile( "cl_playermodel", true );
	if ( !cl_modelfile.IsValid() )
		return;

	char directory[ 512 ];
	Q_snprintf( directory, sizeof( directory ), "%s/*.*", pFolder );

	FileFindHandle_t fh;

	const char *fn = g_pFullFileSystem->FindFirst( directory, &fh );
	const char *modelfile = cl_modelfile.GetString();

	while ( fn )
	{
		if ( !stricmp( fn, ".") || !stricmp( fn, "..") )
		{
			fn = g_pFullFileSystem->FindNext( fh );
			continue;
		}

		if ( g_pFullFileSystem->FindIsDirectory( fh ) )
		{
			char folderpath[512];

			Q_snprintf( folderpath, sizeof( folderpath ), "%s/%s", pFolder, fn );

			FindVMTFilesInFolder( folderpath, fn, cb, iCount, iInitialItem );
			fn = g_pFullFileSystem->FindNext( fh );
			continue;
		}

		if ( !strstr( fn, ".vmt" ) )
		{
			fn = g_pFullFileSystem->FindNext( fh );
			continue;
		}


		char filename[ 512 ];
		Q_snprintf( filename, sizeof(filename), "%s/%s", pFolder, fn );
		if ( strlen( filename ) >= 4 )
		{
			filename[ strlen( filename ) - 4 ] = 0;
			Q_strncat( filename, ".vmt", sizeof( filename ), COPY_ALL_CHARACTERS );
			if ( g_pFullFileSystem->FileExists( filename ) )
			{
				char displayname[ 512 ];
				char texturepath[ 512 ];
				// strip off the extension
				Q_strncpy( displayname, fn, sizeof( displayname ) );
				StripStringOutOfString( MODEL_MATERIAL_BASE_FOLDER, filename, texturepath );
				
				displayname[ strlen( displayname ) - 4 ] = 0;
				
				if ( CORRECT_PATH_SEPARATOR == texturepath[0] )
				    cb->AddItem( displayname, texturepath + 1 ); // ignore the initial "/" in texture path
				else
				    cb->AddItem( displayname, texturepath );


				char realname[ 512 ];
				Q_FileBase( modelfile, realname, sizeof( realname ) );
				Q_FileBase( filename, filename, sizeof( filename ) );
				
				if (!stricmp(filename, realname))
				{
					iInitialItem = iCount;
				}

				++iCount;
			}
		}

		fn = g_pFullFileSystem->FindNext( fh );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Builds model list
//-----------------------------------------------------------------------------
void COptionsSubMultiplayer::InitModelList( CLabeledCommandComboBox *cb )
{
	// Find out images
	int i = 0, initialItem = 0;

	cb->DeleteAllItems();
	FindVMTFilesInFolder( MODEL_MATERIAL_BASE_FOLDER, "", cb, i, initialItem );
	cb->SetInitialItem( initialItem );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void COptionsSubMultiplayer::RemapLogo()
{
	char logoname[256];

	m_pLogoList->GetText( logoname, sizeof( logoname ) );
	if( !logoname[ 0 ] )
		return;

	char fullLogoName[512];

	// make sure there is a version with the proper shader
	g_pFullFileSystem->CreateDirHierarchy( "materials/VGUI/logos/UI", "GAME" );
	Q_snprintf( fullLogoName, sizeof( fullLogoName ), "materials/VGUI/logos/UI/%s.vmt", logoname );
	if ( !g_pFullFileSystem->FileExists( fullLogoName ) )
	{
		FileHandle_t fp = g_pFullFileSystem->Open( fullLogoName, "wb" );
		if ( !fp )
			return;

		char data[1024];
		Q_snprintf( data, sizeof( data ), "\"UnlitGeneric\"\n\
{\n\
	// Original shader: BaseTimesVertexColorAlphaBlendNoOverbright\n\
	\"$translucent\" 1\n\
	\"$basetexture\" \"VGUI/logos/%s\"\n\
	\"$vertexcolor\" 1\n\
	\"$vertexalpha\" 1\n\
	\"$no_fullbright\" 1\n\
	\"$ignorez\" 1\n\
}\n\
", logoname );

		g_pFullFileSystem->Write( data, strlen( data ), fp );
		g_pFullFileSystem->Close( fp );
	}

	Q_snprintf( fullLogoName, sizeof( fullLogoName ), "logos/UI/%s", logoname );
	m_pLogoImage->SetImage( fullLogoName );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void COptionsSubMultiplayer::RemapModel()
{
	const char *pModelName = m_pModelList->GetActiveItemCommand();
	
	if( pModelName == NULL )
		return;

	char texture[ 256 ];
	Q_snprintf ( texture, sizeof( texture ), "vgui/playermodels/%s", pModelName );
	texture[ strlen( texture ) - 4 ] = 0;

	m_pModelImage->setTexture( texture );
}


//-----------------------------------------------------------------------------
// Purpose: Called whenever model name changes
//-----------------------------------------------------------------------------
void COptionsSubMultiplayer::OnTextChanged(vgui::Panel *panel)
{
	RemapModel();
	RemapLogo();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void COptionsSubMultiplayer::OnControlModified()
{
	PostMessage(GetParent(), new KeyValues("ApplyButtonEnable"));
	InvalidateLayout();
}

#define DIB_HEADER_MARKER   ((WORD) ('M' << 8) | 'B')
#define SUIT_HUE_START 192
#define SUIT_HUE_END 223
#define PLATE_HUE_START 160
#define PLATE_HUE_END 191

#ifdef POSIX 
typedef struct tagRGBQUAD { 
  uint8 rgbBlue;
  uint8 rgbGreen;
  uint8 rgbRed;
  uint8 rgbReserved;
} RGBQUAD;
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
static void PaletteHueReplace( RGBQUAD *palSrc, int newHue, int Start, int end )
{
	int i;
	float r, b, g;
	float maxcol, mincol;
	float hue, val, sat;

	hue = (float)(newHue * (360.0 / 255));

	for (i = Start; i <= end; i++)
	{
		b = palSrc[ i ].rgbBlue;
		g = palSrc[ i ].rgbGreen;
		r = palSrc[ i ].rgbRed;
		
		maxcol = max( max( r, g ), b ) / 255.0f;
		mincol = min( min( r, g ), b ) / 255.0f;
		
		val = maxcol;
		sat = (maxcol - mincol) / maxcol;

		mincol = val * (1.0f - sat);

		if (hue <= 120)
		{
			b = mincol;
			if (hue < 60)
			{
				r = val;
				g = mincol + hue * (val - mincol)/(120 - hue);
			}
			else
			{
				g = val;
				r = mincol + (120 - hue)*(val-mincol)/hue;
			}
		}
		else if (hue <= 240)
		{
			r = mincol;
			if (hue < 180)
			{
				g = val;
				b = mincol + (hue - 120)*(val-mincol)/(240 - hue);
			}
			else
			{
				b = val;
				g = mincol + (240 - hue)*(val-mincol)/(hue - 120);
			}
		}
		else
		{
			g = mincol;
			if (hue < 300)
			{
				b = val;
				r = mincol + (hue - 240)*(val-mincol)/(360 - hue);
			}
			else
			{
				r = val;
				b = mincol + (360 - hue)*(val-mincol)/(hue - 240);
			}
		}

		palSrc[ i ].rgbBlue = (unsigned char)(b * 255);
		palSrc[ i ].rgbGreen = (unsigned char)(g * 255);
		palSrc[ i ].rgbRed = (unsigned char)(r * 255);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void COptionsSubMultiplayer::ColorForName( char const *pszColorName, int&r, int&g, int&b )
{
	r = g = b = 0;
	
	int count = sizeof( itemlist ) / sizeof( itemlist[0] );

	for ( int i = 0; i < count; i++ )
	{
		if (!Q_strnicmp(pszColorName, itemlist[ i ].name, strlen(itemlist[ i ].name)))
		{
			r = itemlist[ i ].r;
			g = itemlist[ i ].g;
			b = itemlist[ i ].b;
			return;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void COptionsSubMultiplayer::OnResetData()
{
	// reset the DownloadFilter combo box
	if ( m_pDownloadFilterCombo )
	{
		// cl_downloadfilter
		ConVarRef cl_downloadfilter( "cl_downloadfilter");

		if ( Q_stricmp( cl_downloadfilter.GetString(), "none" ) == 0 )
		{
			m_pDownloadFilterCombo->ActivateItem( 3 );
		}
		else if ( Q_stricmp( cl_downloadfilter.GetString(), "nosounds" ) == 0 )
		{
			m_pDownloadFilterCombo->ActivateItem( 1 );
		}
		else if ( Q_stricmp( cl_downloadfilter.GetString(), "mapsonly" ) == 0 )
		{
			m_pDownloadFilterCombo->ActivateItem( 2 );
		}
		else
		{
			m_pDownloadFilterCombo->ActivateItem( 0 );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void COptionsSubMultiplayer::OnApplyChanges()
{
	m_pPrimaryColorSlider->ApplyChanges();
	m_pSecondaryColorSlider->ApplyChanges();
//	m_pModelList->ApplyChanges();
	m_pLogoList->ApplyChanges();
    m_pLogoList->GetText(m_LogoName, sizeof(m_LogoName));
	m_pHighQualityModelCheckBox->ApplyChanges();

	for ( int i=0; i<m_cvarToggleCheckButtons.GetCount(); ++i )
	{
		CCvarToggleCheckButton *toggleButton = m_cvarToggleCheckButtons[i];
		if( toggleButton->IsVisible() && toggleButton->IsEnabled() )
		{
			toggleButton->ApplyChanges();
		}
	}

	if ( m_pLockRadarRotationCheckbox != NULL )
	{
		m_pLockRadarRotationCheckbox->ApplyChanges();
	}

	// save the logo name
	char cmd[512];
	if ( m_LogoName[ 0 ] )
	{
		Q_snprintf(cmd, sizeof(cmd), "cl_logofile materials/vgui/logos/%s.vtf\n", m_LogoName);
	}
	else
	{
		Q_strncpy( cmd, "cl_logofile \"\"\n", sizeof( cmd ) );
	}
	engine->ClientCmd_Unrestricted(cmd);

	if ( m_pModelList && m_pModelList->IsVisible() && m_pModelList->GetActiveItemCommand() )
	{
		Q_strncpy( m_ModelName, m_pModelList->GetActiveItemCommand(), sizeof( m_ModelName ) );
		Q_StripExtension( m_ModelName, m_ModelName, sizeof ( m_ModelName ) );
		
		// save the player model name
		Q_snprintf(cmd, sizeof(cmd), "cl_playermodel models/%s.mdl\n", m_ModelName );
		engine->ClientCmd_Unrestricted(cmd);
	}
	else
	{
		m_ModelName[0] = 0;
	}

	// set the DownloadFilter cvar
	if ( m_pDownloadFilterCombo )
	{
		ConVarRef  cl_downloadfilter( "cl_downloadfilter" );
		
		switch ( m_pDownloadFilterCombo->GetActiveItem() )
		{
		default:
		case 0:
			cl_downloadfilter.SetValue( "all" );
			break;
		case 1:
			cl_downloadfilter.SetValue( "nosounds" );
			break;
		case 2:
			cl_downloadfilter.SetValue( "mapsonly" );
			break;
		case 3:
			cl_downloadfilter.SetValue( "none" );
			break;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Allow the res file to create controls on per-mod basis
//-----------------------------------------------------------------------------
Panel *COptionsSubMultiplayer::CreateControlByName( const char *controlName )
{
	if( !Q_stricmp( "CCvarToggleCheckButton", controlName ) )
	{
		CCvarToggleCheckButton *newButton = new CCvarToggleCheckButton( this, controlName, "", "" );
		m_cvarToggleCheckButtons.AddElement( newButton );
		return newButton;
	}
	else
	{
		return BaseClass::CreateControlByName( controlName );
	}
}

