/*
 *      Copyright (C) 2005-2012 Team XBMC
 *      http://www.xbmc.org
 *
 *		Copyright (C) 2010-2013 Eduard Kytmanov
 *		http://www.avmedia.su
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
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "LCDFactory.h"
#include "../linux/XLCDproc.h"
#ifdef HAS_DS_PLAYER
#include "iMON/iMONDisplay.h"
#endif

ILCD* g_lcd = NULL;
CLCDFactory::CLCDFactory(void)
{}

CLCDFactory::~CLCDFactory(void)
{}

ILCD* CLCDFactory::Create()
{
#ifdef _LINUX
  return new XLCDproc();
#endif
#ifdef HAS_DS_PLAYER
  HRESULT hr = E_FAIL;
  CImonDisplay *imon = new CImonDisplay(&hr);
  if(FAILED(hr))
	  SAFE_DELETE(imon);
  return imon;
#endif
}
