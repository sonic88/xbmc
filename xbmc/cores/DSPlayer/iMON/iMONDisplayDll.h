#pragma once
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

#ifndef HAS_DS_PLAYER
#error DSPlayer's header file included without HAS_DS_PLAYER defined
#endif

#include "DynamicDll.h"
#include "iMONDisplayDefines.h"

class CImonDisplayDllInterface
{
public:
  virtual ~CImonDisplayDllInterface() {}
  	/////////////////////////////////////
	/////	Interfaces
	/**DSPResult IMON_Display_Init(HWND hwndNoti, UINT uMsgNotification)
	@brief	This function should be called to use other functions in iMON Display API.\n 
				When the caller application calls this function, API tries to request Display Plug-in Mode to iMON.
	@param	[in] hwndNoti	API will send/post message to this handle.
	@param	[in] uMsgNotification	API will send/post message to hwndNoti with this message identifier.
	@return	This function will return one of DSPResult enumeration value.\n
				DSP_SUCCEEDED will be returned if succeeded. DSP_E_INVALIDARG or DSP_E_OUTOFMEMORY can be returned when error occurs.*/
	virtual DSPResult IMON_Display_Init(HWND hwndNoti, UINT uMsgNotification) = 0;

	/**DSPResult IMON_Display_Uninit()
	@brief	This function should be called when the caller application need not use this API any more.\n 
				If this function call is missed, iMON can't display other information.\n
	@return	This function will return one of DSPResult enumeration value.\n
				DSP_SUCCEEDED will be returned if succeeded.*/
	virtual DSPResult IMON_Display_Uninit() = 0;

	/**DSPResult IMON_Display_IsInited()
	@brief	This function can be used when the caller application wants to know if API is initialized.\n 
	@return	This function will return one of DSPResult enumeration value.\n
				If API is initialized, this call will return DSP_S_INITED. Otherwise DSP_S_NOT_INITED will be returned.*/
	virtual DSPResult IMON_Display_IsInited() = 0;

	/**DSPResult IMON_Display_IsPluginModeEnabled()
	@brief	This function can be used when the caller application wants to know if API can control iMON display.\n
	@return	This function will return one of DSPResult enumeration value.\n
				If API can control iMON display, this call will return DSP_S_IN_PLUGIN_MODE. Otherwise DSP_S_NOT_IN_PLUGIN_MODE will be returned.*/
	virtual DSPResult IMON_Display_IsPluginModeEnabled() = 0;


	/**DSPResult IMON_Display_SetVfdText(LPCTSTR lpsz1stLine, LPCTSTR lpsz2ndLine)
	@brief	This function can be used when the caller application wants to display text data on VFD module.\n
	@param	[in] lpsz1stLine	This string data will be displayed on the 1st line of VFD module.\n
									It doesn't support multi-byte character and if string data is longer than 16 characters, it displays 16 characters from the first.\n
	@param	[in] lpsz2ndLine	This string data will be displayed on the 2nd line of VFD module.\n
									It doesn't support multi-byte character and if string data is longer than 16 characters, it displays 16 characters from the first.\n
	@return	This function will return one of DSPResult enumeration value.\n
				DSP_SUCCEEDED will be returned if succeeded. DSP_E_POINTER, DSP_E_NOT_INITED or DSP_E_FAIL can be returned if failed.*/
	virtual DSPResult IMON_Display_SetVfdText(LPCTSTR lpsz1stLine, LPCTSTR lpsz2ndLine) = 0;

	/**DSPResult IMON_Display_SetVfdEqData(PDSPEQDATA pEqData)
	@brief	This function can be used when the caller application wants to display equalizer data on VFD module.\n
	@param	[in] pEqData	Pointer of DSPEQDATA structure. The caller application should fill this structure with the equalizer data for 16 bands.\n
	@return	This function will return one of DSPResult enumeration value.\n
				DSP_SUCCEEDED will be returned if succeeded. DSP_E_POINTER, DSP_E_NOT_INITED or DSP_E_FAIL can be returned if failed.*/
	virtual DSPResult IMON_Display_SetVfdEqData(PDSPEQDATA pEqData) = 0;


	/**DSPResult IMON_Display_SetLcdText(LPCTSTR lpszText)
	@brief	This function can be used when the caller application wants to display text data on LCD module.\n
	@param	[in] lpszText	This string data will be displayed on the LCD module.\n
									It supports multi-byte character and if string data is longer than display area, it will start to scroll.\n
									When text scrolling is finished, API will notify it with DSPNotifyCode enumeration value, DSPNM_LCD_TEXT_SCROLL_DONE.\n
	@return	This function will return one of DSPResult enumeration value.\n
				DSP_SUCCEEDED will be returned if succeeded. DSP_E_POINTER, DSP_E_NOT_INITED or DSP_E_FAIL can be returned if failed.*/
	virtual DSPResult IMON_Display_SetLcdText(LPCTSTR lpszText) = 0;

	/**DSPResult IMON_Display_SetLcdEqData(PDSPEQDATA pEqDataL, PDSPEQDATA pEqDataR)
	@brief	This function can be used when the caller application wants to display equalizer data on LCD module.\n
	@param	[in] pEqDataL    Pointer of DSPEQDATA structure. This parameter represents equalizer data of left channel.\n
								 The caller application should fill this structure with the equalizer data of left channel for 16 bands.\n
	@param	[in] pEqDataR    Pointer of DSPEQDATA structure. This parameter represents equalizer data of right channel.\n
								 The caller application should fill this structure with the equalizer data of right channel for 16 bands.\n
	@return	This function will return one of DSPResult enumeration value.\n
				DSP_SUCCEEDED will be returned if succeeded. DSP_E_POINTER, DSP_E_NOT_INITED or DSP_E_FAIL can be returned if failed.*/
	virtual DSPResult IMON_Display_SetLcdEqData(PDSPEQDATA pEqDataL, PDSPEQDATA pEqDataR) = 0;


	/**DSPResult IMON_Display_SetLcdAllIcons(BOOL bOn)
	@brief	This function can be used when the caller application wants to turn on/off all icons on LCD module.\n
	@param	[in] bOn    If this value is TRUE, iMON will turn on all icons. Otherwise, iMON will turn off all icons.\n
	@return	This function will return one of DSPResult enumeration value.\n
				DSP_SUCCEEDED will be returned if succeeded. DSP_E_NOT_INITED or DSP_E_FAIL can be returned if failed.*/
	virtual DSPResult IMON_Display_SetLcdAllIcons(BOOL bOn) = 0;

	/**DSPResult IMON_Display_SetLcdOrangeIcon(BYTE btIconData1, BYTE btIconData2)
	@brief	This function can be used when the caller application wants to turn on/off orange shaped disk icons on the upper left part of LCD module.\n
				Disk icons consist of 8 pieces of orange and orange peel.\n
	@param	[in] btIconData1    Each bit represents one of icons shaped the piece of orange.\n
									MSB is used for the piece placed on top and the remaining bits are for the piece placed in CCW from top.\n
	@param	[in] btIconData2    MSB represents the orange peel shaped icon. Other bits are not used.\n
	@return	This function will return one of DSPResult enumeration value.\n
				DSP_SUCCEEDED will be returned if succeeded. DSP_E_NOT_INITED or DSP_E_FAIL can be returned if failed.*/
	virtual DSPResult IMON_Display_SetLcdOrangeIcon(BYTE btIconData1, BYTE btIconData2) = 0;

	/**DSPResult IMON_Display_SetLcdMediaTypeIcon(BYTE btIconData)
	@brief	This function can be used when the caller application wants to turn on/off media type icons on the upper part of LCD module.\n
	@param	[in] btIconData    Each bit represents one of media type icons. From MSB each bit represents MUSIC, MOVIE, PHOTO, CD/DVD, TV, WEBCASTING and NEWS/WEATHER icon.\n
	@return	This function will return one of DSPResult enumeration value.\n
				DSP_SUCCEEDED will be returned if succeeded. DSP_E_NOT_INITED or DSP_E_FAIL can be returned if failed.*/
	virtual DSPResult IMON_Display_SetLcdMediaTypeIcon(BYTE btIconData) = 0;

	/**DSPResult IMON_Display_SetLcdSpeakerIcon(BYTE btIconData1, BYTE btIconData2)
	@brief	This function can be used when the caller application wants to turn on/off speaker icons on the upper right part of LCD module.\n
	@param	[in] btIconData1    Each bit represents one of speaker icons.\nFrom MSB each bit represents L, C, R, SL, LFE, SR, RL and SPDIF icon.
	@param	[in] btIconData2    MSB represents RR icon. Other bits are not used.\n
	@return	This function will return one of DSPResult enumeration value.\n
				DSP_SUCCEEDED will be returned if succeeded. DSP_E_NOT_INITED or DSP_E_FAIL can be returned if failed.*/
	virtual DSPResult IMON_Display_SetLcdSpeakerIcon(BYTE btIconData1, BYTE btIconData2) = 0;

	/**DSPResult IMON_Display_SetLcdVideoCodecIcon(BYTE btIconData)
	@brief	This function can be used when the caller application wants to turn on/off codec icons for video file on the lower part of LCD module.\n
	@param	[in] btIconData    Each bit represents one of video codec icons. From MSB each bit represents MPG, DIVX, XVID, WMV, MPG, AC3, DTS and WMA icon.\n
	@return	This function will return one of DSPResult enumeration value.\n
				DSP_SUCCEEDED will be returned if succeeded. DSP_E_NOT_INITED or DSP_E_FAIL can be returned if failed.*/
	virtual DSPResult IMON_Display_SetLcdVideoCodecIcon(BYTE btIconData) = 0;

	/**DSPResult IMON_Display_SetLcdAudioCodecIcon(BYTE btIconData)
	@brief	This function can be used when the caller application wants to turn on/off codec icons for audio file on the lower part of LCD module.\n
	@param	[in] btIconData    Each bit represents one of audio codec icons. From MSB each bit represents MP3, OGG, WMA and WAV icon.\n
	@return	This function will return one of DSPResult enumeration value.\n
				DSP_SUCCEEDED will be returned if succeeded. DSP_E_NOT_INITED or DSP_E_FAIL can be returned if failed.*/
	virtual DSPResult IMON_Display_SetLcdAudioCodecIcon(BYTE btIconData) = 0;

	/**DSPResult IMON_Display_SetLcdAspectRatioIcon(BYTE btIconData)
	@brief	This function can be used when the caller application wants to turn on/off aspect ratio icons on the lower right part of LCD module.\n
	@param	[in] btIconData    Each bit represents one of aspect ratio icons. From MSB each bit represents SRC, FIT, TV, HDTV, SCR1 and SCR2 icon.\n
	@return	This function will return one of DSPResult enumeration value.\n
				DSP_SUCCEEDED will be returned if succeeded. DSP_E_NOT_INITED or DSP_E_FAIL can be returned if failed.*/
	virtual DSPResult IMON_Display_SetLcdAspectRatioIcon(BYTE btIconData) = 0;

	/**DSPResult IMON_Display_SetLcdEtcIcon(BYTE btIconData)
	@brief	This function can be used when the caller application wants to turn on/off icons on the lower left part of LCD module.\n
	@param	[in] btIconData    Each bit represents icon. From MSB each bit represents REPEAT, SHUFFLE, ALARM, REC, VOL and TIME icon.\n
	@return	This function will return one of DSPResult enumeration value.\n
				DSP_SUCCEEDED will be returned if succeeded. DSP_E_NOT_INITED or DSP_E_FAIL can be returned if failed.*/
	virtual DSPResult IMON_Display_SetLcdEtcIcon(BYTE btIconData) = 0;

	/**DSPResult IMON_Display_SetLcdProgress(int nCurPos, int nTotal)
	@brief	This function can be used when the caller application wants to display progress bar on the upper and lower left part of text area of LCD module.\n
	@param	[in] nCurPos   It represents the current position of progress bar.\n
	@param	[in] nTotal    It represents the total length of progress bar.\n
	@return	This function will return one of DSPResult enumeration value.\n
				DSP_SUCCEEDED will be returned if succeeded. DSP_E_NOT_INITED or DSP_E_FAIL can be returned if failed.*/
	virtual DSPResult IMON_Display_SetLcdProgress(int nCurPos, int nTotal) = 0;
};



class CImonDisplayDll : public DllDynamic, CImonDisplayDllInterface
{
  DECLARE_DLL_WRAPPER(CImonDisplayDll, "iMONDisplay.dll")
  
  DEFINE_METHOD2(DSPResult, IMON_Display_Init, (HWND p1, UINT p2))

  DEFINE_METHOD0(DSPResult, IMON_Display_Uninit)

  DEFINE_METHOD0(DSPResult, IMON_Display_IsInited)

  DEFINE_METHOD0(DSPResult, IMON_Display_IsPluginModeEnabled)

  DEFINE_METHOD2(DSPResult, IMON_Display_SetVfdText, (LPCTSTR p1, LPCTSTR p2))

  DEFINE_METHOD1(DSPResult, IMON_Display_SetVfdEqData, (PDSPEQDATA p1))

  DEFINE_METHOD1(DSPResult, IMON_Display_SetLcdText, (LPCTSTR p1))

  DEFINE_METHOD2(DSPResult, IMON_Display_SetLcdEqData, (PDSPEQDATA p1, PDSPEQDATA p2))

  DEFINE_METHOD1(DSPResult, IMON_Display_SetLcdAllIcons, (BOOL p1))

  DEFINE_METHOD2(DSPResult, IMON_Display_SetLcdOrangeIcon, (BYTE p1, BYTE p2))

  DEFINE_METHOD1(DSPResult, IMON_Display_SetLcdMediaTypeIcon, (BYTE p1))

  DEFINE_METHOD2(DSPResult, IMON_Display_SetLcdSpeakerIcon, (BYTE p1, BYTE p2))

  DEFINE_METHOD1(DSPResult, IMON_Display_SetLcdVideoCodecIcon, (BYTE p1))

  DEFINE_METHOD1(DSPResult, IMON_Display_SetLcdAudioCodecIcon, (BYTE p1))

  DEFINE_METHOD1(DSPResult, IMON_Display_SetLcdAspectRatioIcon, (BYTE p1))

  DEFINE_METHOD1(DSPResult, IMON_Display_SetLcdEtcIcon, (BYTE p1))

  DEFINE_METHOD2(DSPResult, IMON_Display_SetLcdProgress, (int p1, int p2))
  
  BEGIN_METHOD_RESOLVE()
  RESOLVE_METHOD(IMON_Display_Init)
  RESOLVE_METHOD(IMON_Display_Uninit)
  RESOLVE_METHOD(IMON_Display_IsInited)
  RESOLVE_METHOD(IMON_Display_IsPluginModeEnabled)
  RESOLVE_METHOD(IMON_Display_SetVfdText)
  RESOLVE_METHOD(IMON_Display_SetVfdEqData)
  RESOLVE_METHOD(IMON_Display_SetLcdText)
  RESOLVE_METHOD(IMON_Display_SetLcdEqData)
  RESOLVE_METHOD(IMON_Display_SetLcdAllIcons)
  RESOLVE_METHOD(IMON_Display_SetLcdOrangeIcon)
  RESOLVE_METHOD(IMON_Display_SetLcdMediaTypeIcon)
  RESOLVE_METHOD(IMON_Display_SetLcdSpeakerIcon)
  RESOLVE_METHOD(IMON_Display_SetLcdVideoCodecIcon)
  RESOLVE_METHOD(IMON_Display_SetLcdAudioCodecIcon)
  RESOLVE_METHOD(IMON_Display_SetLcdAspectRatioIcon)
  RESOLVE_METHOD(IMON_Display_SetLcdEtcIcon)
  RESOLVE_METHOD(IMON_Display_SetLcdProgress)
  END_METHOD_RESOLVE()
};