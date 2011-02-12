/*
 *      Copyright (C) 2011 Fred Hoogduin
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
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include <vector>
#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include "upcomingrecording.h"

cUpcomingRecording::cUpcomingRecording(void)
{
  channeldisplayname = "";
  channelid = "";
  date = 0;
  starttime = 0;
  stoptime = 0;
  title = "";
  isactive = false;
  isrecording = false;
}

cUpcomingRecording::~cUpcomingRecording(void)
{
}
bool cUpcomingRecording::Parse(const Json::Value& data)
{
  int offset;
  std::string t;

  channeldisplayname = "";
  channelid = "";
  date = 0;
  t = data["ActualStartTime"].asString();
  starttime = ForTheRecord::WCFDateToTimeT(t, offset);
  starttime += ((offset/100)*3600);
  t = data["ActualStopTime"].asString();
  stoptime = ForTheRecord::WCFDateToTimeT(t, offset);
  stoptime += ((offset/100)*3600);
  title = data["Title"].asString();
  isactive = true;
  isrecording = false;

  // Pickup the C# Program class
  Json::Value programobject;
  programobject = data["Program"];

  // From the Program class pickup the C# Channel class
  Json::Value channelobject;
  channelobject = programobject["Channel"];

  // And -finally- our channel id
  channelid = channelobject["ChannelId"].asString();

  return true;
}