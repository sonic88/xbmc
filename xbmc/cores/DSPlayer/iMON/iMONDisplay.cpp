/* 
 *  Copyright (C) 2010-2013 Eduard Kytmanov
 *  http://www.avmedia.su
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#ifdef HAS_DS_PLAYER

#include "iMONDisplay.h"

#include "guilib/LocalizeStrings.h"
#include "guilib/GUIWindowManager.h"
#include "Application.h"
#include "FileItem.h"
#include "utils/log.h"
#include "settings/GUISettings.h"
#include "settings/Settings.h"
#include "windowing/WindowingFactory.h"
#include "windows/GUIMediaWindow.h"
#include "cores/AudioEngine/AEFactory.h"
#include "cores/AudioEngine/Engines/SoftAE/SoftAE.h"
#include "utils/XMLUtils.h"

#define AE (*((CSoftAE*)CAEFactory::GetEngine()))

ThreadIdentifier CImonDisplay::m_threadID = 0;

CImonDisplay::CImonDisplay(HRESULT* phr)
	: m_bVfdConnected(false)
	, m_bLcdConnected(false)
	, m_bLcdScrollDone(true)
	, m_strLine1("")
	, m_strLine2("")
	, m_RotationTimer(NULL)
	, m_nRotatePosition(0)
{
	if(m_ImonDisplayDll.Load())
	{
		if(phr) *phr = S_OK;
	}
}

CImonDisplay::~CImonDisplay(void)
{
	SAFE_DELETE(m_RotationTimer);
}

void CImonDisplay::Initialize()
{
	if (!m_ImonDisplayDll.IsLoaded() || !g_guiSettings.GetBool("videoscreen.haslcd"))
		return;

	CStdString lcdPath;
	lcdPath = g_settings.GetUserDataItem("iMONDisplay.xml");

	// load settings
	CXBMCTinyXML doc;
	if (!doc.LoadFile(lcdPath))
	{
		CLog::Log(LOGERROR, "%s - Unable to load LCD skin file %s", __FUNCTION__, lcdPath.c_str());
		return;
	}
	TiXmlElement *element = doc.RootElement();
	if (!element || strcmp(element->Value(), "lcd") != 0)
	{
		return;
	}
	
	CStdString disableOnPlay;
	XMLUtils::GetBoolean(element, "discbottomcircle", m_Settings.DiscBottomCircle);
	XMLUtils::GetBoolean(element, "discrotate", m_Settings.DiscRotate);
	XMLUtils::GetInt(element, "discrotatespeed", m_Settings.DiscRotateSpeed);

	LoadSkin(lcdPath);

	Stop();
	CThread::Create();

	m_ImonDisplayDll.IMON_Display_Init(g_hWnd, WM_DSP_PLUGIN_NOTIFY);
}

void CImonDisplay::Stop()
{
	if(m_ImonDisplayDll.IsLoaded())
	{
		m_ImonDisplayDll.IMON_Display_Uninit();
		PostMessage(WM_QUIT, 0, 0);
	}
}

void CImonDisplay::PostMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (m_threadID)
	{
		PostThreadMessage(m_threadID, uMsg, wParam, lParam);
	}
}

//CThread
void CImonDisplay::OnStartup()
{
	if(!m_RotationTimer && m_Settings.DiscRotate)
		m_RotationTimer = new CTimer(this);

	m_threadID = CThread::GetCurrentThreadId();
}

void CImonDisplay::OnExit()
{
	m_threadID = 0;
}

void CImonDisplay::Process()
{
	BOOL bRet;
	MSG msg;
	while( (bRet = GetMessage( &msg, (HWND) -1, 0, 0 )) != 0)
	{ 
		if (bRet == -1)
		{
			return;
		}
		else if(msg.message == WM_DSP_PLUGIN_NOTIFY)
		{
			switch(msg.wParam)
			{
			case DSPNM_PLUGIN_SUCCEED:
			case DSPNM_IMON_RESTARTED:
			case DSPNM_HW_CONNECTED:
				{
					m_bVfdConnected = FALSE;
					m_bLcdConnected = FALSE;
					if((msg.lParam & DSPN_DSP_VFD) == DSPN_DSP_VFD)		m_bVfdConnected = TRUE;
					if((msg.lParam & DSPN_DSP_LCD) == DSPN_DSP_LCD)		m_bLcdConnected = TRUE;

					DisplayPluginMessage(msg.wParam, FALSE);
				}
				break;

			case DSPNM_PLUGIN_FAILED:
			case DSPNM_HW_DISCONNECTED:
			case DSPNM_IMON_CLOSED:
				{
					m_bVfdConnected = FALSE;
					m_bLcdConnected = FALSE;

					DisplayPluginMessage(msg.lParam, TRUE);
				}
				break;

			case DSPNM_LCD_TEXT_SCROLL_DONE:
				{
					m_bLcdScrollDone = true;
					CLog::Log(LOGDEBUG,"%s - LCD Text Scroll Finished.", __FUNCTION__);
				}
				break;
			}
		}

	}
}

void CImonDisplay::DisplayPluginMessage(UINT uErrCode, BOOL bError)
{
	CStdString strErrMsg;

	if(bError)
	{
		switch(uErrCode)
		{
		case DSPN_ERR_IN_USED:				strErrMsg = "Display Plug-in is Already Used by Other Application.";		break;
		case DSPN_ERR_HW_DISCONNECTED:		strErrMsg = "iMON HW is Not Connected.";									break;
		case DSPN_ERR_NOT_SUPPORTED_HW:		strErrMsg = "The Connected iMON HW doesn't Support Display Plug-in.";		break;
		case DSPN_ERR_PLUGIN_DISABLED:		strErrMsg = "Display Plug-in Mode Option is Disabled.";						break;
		case DSPN_ERR_IMON_NO_REPLY:		strErrMsg = "The Latest iMON is Not Installed or iMON Not Running.";		break;
		case DSPN_ERR_UNKNOWN:				strErrMsg = "Unknown Failure.";												break;
		}

		CLog::Log(LOGERROR, "%s - %s", __FUNCTION__, strErrMsg.c_str());
	}
	else
	{
		switch(uErrCode)
		{
		case DSPNM_PLUGIN_SUCCEED:			strErrMsg = "Plug-in Mode Inited Successfully.";		    break;
		case DSPNM_IMON_RESTARTED:			strErrMsg = "iMON Started and Plug-in Mode Inited.";		break;
		case DSPNM_HW_CONNECTED:			strErrMsg = "iMON HW Connected and Plug-in Mode Inited.";	break;
		}

		CLog::Log(LOGNOTICE, "%s - %s", __FUNCTION__, strErrMsg.c_str());
	}
}

bool CImonDisplay::IsConnected()
{
	return m_ImonDisplayDll.IsLoaded() ? (m_ImonDisplayDll.IMON_Display_IsInited() == DSP_S_INITED) : false;
}

void CImonDisplay::Render(LCD_MODE mode)
{
	BYTE data = 0;

	if((!g_application.IsPlaying() || g_application.IsPaused()) && m_RotationTimer && m_RotationTimer->IsRunning())
	{
		m_RotationTimer->Stop(true);
	}

	switch(mode)
	{
	case ILCD::LCD_MODE_MUSIC: data |= 0x80; /*music*/
	case ILCD::LCD_MODE_PVRTV: data |= 0x08; /*pvrtv*/
	case ILCD::LCD_MODE_VIDEO: data |= 0x40; /*video*/
							   if(g_application.IsPlaying())
							   {
								   SetHScroll();
								   SetLcdAudioCodecIcon();
 								   SetLcdVideoCodecIcon();
								   SetLcdSpeakerIcon();
								   SetLcdAspectRatioIcon();
								   SetLcdEtcIcon();
								   if(m_RotationTimer && !m_RotationTimer->IsRunning() && !g_application.IsPaused())
									   m_RotationTimer->Start(m_Settings.DiscRotateSpeed, true);
							   }
							   break;

	case ILCD::LCD_MODE_GENERAL:
	case ILCD::LCD_MODE_NAVIGATION:
		int id = g_windowManager.GetFocusedWindow();
		switch(id)
		{
		case WINDOW_PVR:			data |= 0x08; break; /*pvrtv*/
		case WINDOW_VIDEO_FILES:
		case WINDOW_VIDEO_NAV:		data |= 0x40; break; /*video*/
		case WINDOW_MUSIC_NAV:
		case WINDOW_MUSIC_FILES:	data |= 0x80; break; /*music*/
		case WINDOW_PICTURES:		data |= 0x20; break; /*photo*/
		case WINDOW_WEATHER:		data |= 0x02; break; /*news/weather*/
		}

		if(!g_application.IsPlaying())
		{
			SetLcdOrangeIcon(true);
			SetHScroll(true);
			SetLcdAudioCodecIcon(true);
			SetLcdVideoCodecIcon(true);
			SetLcdSpeakerIcon(true);
			SetLcdAspectRatioIcon(true);
			SetLcdEtcIcon(true);
		}

		break;
	}

	if(!g_application.IsPlaying() || data)
		m_ImonDisplayDll.IMON_Display_SetLcdMediaTypeIcon(data);

	ILCD::Render(mode);
}

void CImonDisplay::SetLine(int iLine, const CStdString& strLine)
{
	int bufSize = MultiByteToWideChar(CP_ACP, 0, strLine.c_str(), -1, NULL, 0);
	CStdStringW strW (L"", bufSize);
	if ( bufSize == 0 || MultiByteToWideChar(CP_ACP, 0, strLine.c_str(), -1, strW.GetBuf(bufSize), bufSize) != bufSize )
		strW.clear();
	strW.RelBuf();

	bool bUpdate = false;
	if(iLine == 0 && m_strLine1.Compare(strW) != 0)
	{
		bUpdate = true;
		m_strLine1 = strW;
	}
	else if (iLine == 1 && m_bVfdConnected && m_strLine2.Compare(strW) != 0) 
	{
		bUpdate = true;
		m_strLine2 = strW;
	}

	if(m_bVfdConnected && bUpdate)	
	{
		m_ImonDisplayDll.IMON_Display_SetVfdText((LPCTSTR)m_strLine1.c_str(), (LPCTSTR)m_strLine2.c_str());
	} 
	else if(m_bLcdConnected && (m_bLcdScrollDone || bUpdate)) 
	{
		m_ImonDisplayDll.IMON_Display_SetLcdText((LPCTSTR)m_strLine1.c_str());
		m_bLcdScrollDone = false;
	}

}

void CImonDisplay::SetLcdVideoCodecIcon(bool clear /* = false */)
{
	BYTE data = 0;
	if(!clear)
	{
		CStdString name = g_application.m_pPlayer->GetVideoCodecName();

		if(name.CompareNoCase("mpg") == 0){
			data |= 0x80;
		}else if(name.CompareNoCase("divx") == 0){
			data |= 0x40;
		}else if(name.CompareNoCase("xvid") == 0){
			data |= 0x20;
		}else if(name.CompareNoCase("wmv") == 0){
			data |= 0x10;
		}

		name = g_application.m_pPlayer->GetAudioCodecName();

		if(name.CompareNoCase("mpa") == 0){
			data |= 0x08;
		}else if(name.CompareNoCase("ac3") == 0){
			data |= 0x04;
		}else if(name.CompareNoCase("dts") == 0){
			data |= 0x02;
		}else if(name.CompareNoCase("wma") == 0){
			data |= 0x01;
		}
	}
	m_ImonDisplayDll.IMON_Display_SetLcdVideoCodecIcon(data);
}

void CImonDisplay::SetLcdAudioCodecIcon(bool clear /* = false */)
{
	BYTE data = 0;
	if(!clear)
	{
		CStdString name = g_application.m_pPlayer->GetAudioCodecName();

		if(name.CompareNoCase("mp3") == 0){
			data |= 0x80;
		}else if(name.CompareNoCase("ogg") == 0){
			data |= 0x40;
		}else if(name.CompareNoCase("wma") == 0){
			data |= 0x20;
		}else if(name.CompareNoCase("wav") == 0){
			data |= 0x10;
		}
	}
	m_ImonDisplayDll.IMON_Display_SetLcdAudioCodecIcon(data);
}



void CImonDisplay::SetLcdSpeakerIcon(bool clear /* = false */)
{
	BYTE data[2];
	memset(data, 0, sizeof(BYTE)*2);

	int channels = 0;

	if(clear)
	{
		AEStdChLayout layout = AE.GetStdChLayout();
		if(layout != AE_CH_LAYOUT_INVALID)
		{
			static const UINT map[AE_CH_LAYOUT_MAX] =
			{
				1 /*1.0*/,
				2 /*2.0*/, 3 /*2.1*/, 3 /*3.0*/, 4 /*3.1*/, 4 /*4.0*/,
				5 /*4.1*/, 5 /*5.0*/, 6 /*5.1*/, 7 /*7.0*/, 8 /*7.1*/
			};

			channels = map[layout];
		}
	}
	else
	{
		channels = g_application.m_pPlayer->GetChannels();
	}

	if(channels == 1)
	{
		data[0] |= 0x40; /*center*/
	}
	else if(channels > 1)
	{
		data[0] |= 0x80; /*front left*/
		data[0] |= 0x20; /*front right*/

		switch(channels)
		{
		case 3: /*Stereo 2 1*/
			data[0] |= 0x08; /*LFE*/
			break;

		case 4: /*Quad 4 0*/
			data[0] |= 0x02; /*speaker rear left*/
			data[1] |= 0x80; /*speaker rear right*/
			break;

		case 5: /*Surround 5 0*/
			data[0] |= 0x02; /*speaker rear left*/
			data[1] |= 0x80; /*speaker rear right*/
			data[0] |= 0x40; /*speaker center*/
			break;

		case 6: /*Surround 5 1*/
			data[0] |= 0x02; /*speaker rear left*/
			data[1] |= 0x80; /*speaker rear right*/
			data[0] |= 0x40; /*speaker center*/
			data[0] |= 0x08; /*LFE*/
			break;

		case 8: /*Surround 7 1*/
			data[0] |= 0x10; /*speaker side left*/
			data[0] |= 0x04; /*speaker side right*/
			data[0] |= 0x02; /*speaker rear left*/
			data[1] |= 0x80; /*speaker rear right*/
			data[0] |= 0x40; /*speaker center*/
			data[0] |= 0x08; /*LFE*/
		}
	}

	m_ImonDisplayDll.IMON_Display_SetLcdSpeakerIcon(data[0], data[1]);
}

void CImonDisplay::SetLcdAspectRatioIcon(bool clear /* = false */)
{
	BYTE data = 0;
	if(!clear)
	{
		if(g_application.m_pPlayer->GetPictureHeight() >= 720)
			data |= 0x10; /*HDTV*/
		else
			data |= 0x20; /*TV*/
	}

	m_ImonDisplayDll.IMON_Display_SetLcdAspectRatioIcon(data);
}

void CImonDisplay::SetLcdEtcIcon(bool clear /* = false */)
{
	BYTE data = 0;
	if(!clear)
	{
		PLAYLIST::REPEAT_STATE state = g_playlistPlayer.GetRepeat(g_playlistPlayer.GetCurrentPlaylist());
		if(state == PLAYLIST::REPEAT_ALL || state == PLAYLIST::REPEAT_ONE)
			data |= 0x80; /*REPEAT*/
		if(g_playlistPlayer.IsShuffled(g_playlistPlayer.GetCurrentPlaylist())) 
			data |= 0x40; /*SHUFFLE*/
		if(g_application.m_pPlayer->IsRecording()) 
			data |= 0x10; /*REC*/
	}

	m_ImonDisplayDll.IMON_Display_SetLcdEtcIcon(data);
}

void CImonDisplay::SetLcdOrangeIcon(bool clear /* = false */)
{
	BYTE data[2];
	memset(data, 0, sizeof(BYTE) * 2);
	if(!clear)
	{
		const UINT count = 4;
		if(count != m_nRotatePosition)
		{
			m_nRotatePosition++;
		}
		else
		{
			m_nRotatePosition = 1;
		}

		static const BYTE rotate_list[count] = 
		{
			{ 	0x11 }, /*8-4*/
			{ 	0x22 }, /*7-3*/
			{ 	0x44 },	/*6-2*/
			{   0x88 }, /*5-1*/
		};

		data[0] |= rotate_list[m_nRotatePosition - 1];

		if(m_Settings.DiscBottomCircle)
			data[1] |= 0x80; /*circle*/
	}
	m_ImonDisplayDll.IMON_Display_SetLcdOrangeIcon(data[0], data[1]);
}

void CImonDisplay::SetHScroll(bool clear /* = false */)
{
	int pos = 0;
	if(!clear)
	{
		pos = (int)g_application.GetPercentage();
	}

	m_ImonDisplayDll.IMON_Display_SetLcdProgress(pos, 100);
}

void CImonDisplay::OnTimeout()
{
	SetLcdOrangeIcon();
}
#endif