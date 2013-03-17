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

#include "../utils/LCD.h"
#include "iMONDisplayDll.h"
#include "threads/Timer.h"

#define WM_DSP_PLUGIN_NOTIFY	WM_APP + 1001

class CImonDisplay : public ILCD, protected ITimerCallback
{
public:
	CImonDisplay(HRESULT* phr);
	~CImonDisplay(void);

	virtual void Initialize();
	virtual bool IsConnected();
	virtual void Stop();
	virtual void Suspend() {};
	virtual void Resume() {};
	virtual void SetBackLight(int iLight){};
	virtual void SetContrast(int iContrast) {};
	virtual int  GetColumns() {return 0; };
	virtual int  GetRows() {return 0; };
	virtual void Render(LCD_MODE mode);
	virtual void SetLine(int iLine, const CStdString& strLine);

	static void PostMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
	// CThread
	virtual void OnStartup();
	virtual void OnExit();
	virtual void Process();

	// ITimerCallback
	virtual void OnTimeout();

	void DisplayPluginMessage(UINT uErrCode, BOOL bError);

	void SetLcdVideoCodecIcon(bool clear = false);
	void SetLcdAudioCodecIcon(bool clear = false);
	void SetLcdSpeakerIcon(bool clear = false);
	void SetLcdAspectRatioIcon(bool clear = false);
	void SetLcdEtcIcon(bool clear = false);
	void SetLcdOrangeIcon(bool clear = false);
	void SetHScroll(bool clear = false);

	class CSettings {

	public:
		CSettings()
			: DiscBottomCircle(true)
			, DiscRotate(true)
			, DiscRotateSpeed(300)
		{
		}

		bool DiscBottomCircle;
		bool DiscRotate;
		int DiscRotateSpeed;

	}m_Settings;

	int m_nRotatePosition;
	CTimer *m_RotationTimer;
	CImonDisplayDll m_ImonDisplayDll;
	static ThreadIdentifier m_threadID;
	bool m_bVfdConnected;
	bool m_bLcdConnected;
	bool m_bLcdScrollDone;
	CStdStringW m_strLine1;
	CStdStringW m_strLine2;
	
};
