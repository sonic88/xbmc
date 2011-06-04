/*
 *      Copyright (C) 2010 Marcel Groothuis
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "client.h"
//#include "timers.h"
#include "channel.h"
#include "activerecording.h"
#include "upcomingrecording.h"
#include "recordinggroup.h"
#include "recordingsummary.h"
#include "recording.h"
#include "epg.h"
#include "utils.h"
#include "pvrclient-fortherecord.h"
#include "fortherecordrpc.h"

#ifdef TSREADER
#include "TSReader.h"
#endif

using namespace std;

/************************************************************/
/** Class interface */

cPVRClientForTheRecord::cPVRClientForTheRecord()
{
  m_bConnected             = false;
  //m_bStop                  = true;
  m_bTimeShiftStarted      = false;
  m_BackendUTCoffset       = 0;
  m_BackendTime            = 0;
  m_tsreader               = NULL;
  m_channel_id_offset      = 0;
  m_epg_id_offset          = 0;
}

cPVRClientForTheRecord::~cPVRClientForTheRecord()
{
  XBMC->Log(LOG_DEBUG, "->~cPVRClientForTheRecord()");
  // Check if we are still reading a TV/Radio stream and close it here
  CloseLiveStream();
}


bool cPVRClientForTheRecord::Connect()
{
  string result;
  char buffer[256];

  snprintf(buffer, 256, "http://%s:%i/", g_szHostname.c_str(), g_iPort);
  g_szBaseURL = buffer;

  XBMC->Log(LOG_INFO, "Connect() - Connecting to %s", g_szBaseURL.c_str());

  int backendversion = FTR_REST_MAXIMUM_API_VERSION;
  int rc = ForTheRecord::Ping(backendversion);
  if (rc == 1)
  {
    backendversion = FTR_REST_MINIMUM_API_VERSION;
    rc = ForTheRecord::Ping(backendversion);
  }

  m_BackendVersion = backendversion;

  switch (rc)
  {
  case 0:
    XBMC->Log(LOG_INFO, "Ping Ok. The client and server are compatible, API version %d.\n", m_BackendVersion);
    break;
  case -1:
    XBMC->Log(LOG_NOTICE, "Ping Ok. The client is too old for the server.\n");
    return false;
  case 1:
    XBMC->Log(LOG_NOTICE, "Ping Ok. The client is too new for the server.\n");
    return false;
  default:
    XBMC->Log(LOG_ERROR, "Ping failed... No connection to ForTheRecord.\n");
    return false;
  }

  m_bConnected = true;
  return true;
}

void cPVRClientForTheRecord::Disconnect()
{
  string result;

  XBMC->Log(LOG_INFO, "Disconnect");

  if (m_bTimeShiftStarted)
  {
    //TODO: tell ForTheRecord that it should stop streaming
  }

  m_bConnected = false;
}

/************************************************************/
/** General handling */

// Used among others for the server name string in the "Recordings" view
const char* cPVRClientForTheRecord::GetBackendName(void)
{
  XBMC->Log(LOG_DEBUG, "->GetBackendName()");

  if(m_BackendName.length() == 0)
  {
    m_BackendName = "ForTheRecord (";
    m_BackendName += g_szHostname.c_str();
    m_BackendName += ")";
  }

  return m_BackendName.c_str();
}

const char* cPVRClientForTheRecord::GetBackendVersion(void)
{
  // Don't know how to fetch this from ForTheRecord
  return "0.0";
}

const char* cPVRClientForTheRecord::GetConnectionString(void)
{
  XBMC->Log(LOG_DEBUG, "->GetConnectionString()");

  return g_szBaseURL.c_str();
}

PVR_ERROR cPVRClientForTheRecord::GetDriveSpace(long long *iTotal, long long *iUsed)
{
  *iTotal = 0;
  *iUsed = 0;

  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR cPVRClientForTheRecord::GetBackendTime(time_t *localTime, int *gmtOffset)
{
  return PVR_ERROR_SERVER_ERROR;
}

/************************************************************/
/** EPG handling */

PVR_ERROR cPVRClientForTheRecord::RequestEPGForChannel(const PVR_CHANNEL &channel, PVRHANDLE handle, time_t start, time_t end)
{
  XBMC->Log(LOG_DEBUG, "->RequestEPGForChannel(%i)", channel.uid);

  cChannel* ftrchannel = FetchChannel(channel.uid);

  struct tm* convert = localtime(&start);
  struct tm tm_start = *convert;
  convert = localtime(&end);
  struct tm tm_end = *convert;

  if(ftrchannel)
  {
    Json::Value response;
    int retval;

    retval = ForTheRecord::GetEPGData(m_BackendVersion, ftrchannel->GuideChannelID(), tm_start, tm_end, response);

    if (retval != E_FAILED)
    {
      XBMC->Log(LOG_DEBUG, "GetEPGData returned %i, response.type == %i, response.size == %i.", retval, response.type(), response.size());
      if( response.type() == Json::arrayValue)
      {
        int size = response.size();
        PVR_PROGINFO proginfo;
        cEpg epg;

        memset(&proginfo, NULL, sizeof(PVR_PROGINFO));

        // parse channel list
        for ( int index =0; index < size; ++index )
        {
          if (epg.Parse(response[index]))
          {
            m_epg_id_offset++;
            proginfo.channum         = channel.number;
            proginfo.uid             = m_epg_id_offset;
            proginfo.title           = epg.Title();
            proginfo.subtitle        = epg.Subtitle();
            proginfo.description     = epg.Description();
            proginfo.starttime       = epg.StartTime();
            proginfo.endtime         = epg.EndTime();
            proginfo.genre_type      = 0;
            proginfo.genre_sub_type  = 0;
            proginfo.genre_text      = "";
            proginfo.parental_rating = 0;
            PVR->TransferEpgEntry(handle, &proginfo);
          }
          epg.Reset();
        }
      }
    }
    else
    {
      XBMC->Log(LOG_ERROR, "GetEPGData failed for channel id:%i", channel.uid);
    }
  }
  else
  {
    XBMC->Log(LOG_ERROR, "Channel (%i) did not return a channel class.", channel.uid);
  }

  return PVR_ERROR_NO_ERROR;
}

bool cPVRClientForTheRecord::FetchGuideProgramDetails(std::string Id, cGuideProgram& guideprogram)
{ 
  bool fRc = false;
  Json::Value guideprogramresponse;

  int retval = ForTheRecord::GetProgramById(Id, guideprogramresponse);
  if (retval >= 0)
  {
    fRc = guideprogram.Parse(guideprogramresponse);
  }
  return fRc;
}

/************************************************************/
/** Channel handling */

int cPVRClientForTheRecord::GetNumChannels()
{
  // Not directly possible in ForTheRecord
  Json::Value response;

  XBMC->Log(LOG_DEBUG, "GetNumChannels()");

  // pick up the channellist for TV
  int retval = ForTheRecord::GetChannelList(ForTheRecord::Television, response);
  if (retval < 0) 
  {
    return 0;
  }

  int numberofchannels = response.size();

  // When radio is enabled, add the number of radio channels
  if (g_bRadioEnabled)
  {
    retval = ForTheRecord::GetChannelList(ForTheRecord::Radio, response);
    if (retval >= 0)
    {
      numberofchannels += response.size();
    }
  }

  return numberofchannels;
}

PVR_ERROR cPVRClientForTheRecord::RequestChannelList(PVRHANDLE handle, int radio)
{
  Json::Value response;
  int retval = -1;

  XBMC->Log(LOG_DEBUG, "RequestChannelList(%s)", radio ? "radio" : "television");
  if (!radio)
  {
    retval = ForTheRecord::GetChannelList(ForTheRecord::Television, response);
  }
  else
  {
    retval = ForTheRecord::GetChannelList(ForTheRecord::Radio, response);
  }

  if(retval >= 0)
  {           
    int size = response.size();

    // parse channel list
    for ( int index = 0; index < size; ++index )
    {

      cChannel channel;
      if( channel.Parse(response[index]) )
      {
        PVR_CHANNEL tag;
        memset(&tag, 0 , sizeof(tag));
        //Hack: assumes that the order of the channel list is fixed.
        //      We can't use the ForTheRecord channel id's. They are GUID strings (128 bit int).       
        //      But only if it isn't cached yet!
        if (FetchChannel(channel.Guid()) == NULL)
        {
          tag.number =  m_channel_id_offset + 1;
          m_channel_id_offset++;
        }
        else
        {
          tag.number = FetchChannel(channel.Guid())->ID();
        }
        tag.uid = tag.number;
        tag.name = channel.Name();
        tag.callsign = channel.Name(); //Used for automatic channel icon search
        tag.iconpath = "";
        tag.encryption = 0; //How to fetch this from ForTheRecord??
        tag.radio = (channel.Type() == ForTheRecord::Radio ? true : false);
        tag.hide = false;
        tag.recording = false;
        tag.bouquet = 0;
        tag.multifeed = false;
        //Use OpenLiveStream to read from the timeshift .ts file or an rtsp stream
        tag.stream_url = "";
        tag.input_format = "mpegts";

        if (!tag.radio)
        {
          XBMC->Log(LOG_DEBUG, "Found TV channel: %s\n", channel.Name());
        }
        else
        {
          XBMC->Log(LOG_DEBUG, "Found Radio channel: %s\n", channel.Name());
        }
        channel.SetID(tag.uid);
        if (FetchChannel(channel.Guid()) == NULL)
        {
          m_Channels.push_back(channel); //Local cache...
        }
        PVR->TransferChannelEntry(handle, &tag);
      }
    }

    return PVR_ERROR_NO_ERROR;
  }
  else
  {
    XBMC->Log(LOG_DEBUG, "RequestChannelList failed. Return value: %i\n", retval);
  }

  return PVR_ERROR_SERVER_ERROR;
}

/************************************************************/
/** Record handling **/

int cPVRClientForTheRecord::GetNumRecordings(void)
{
  Json::Value response;
  int retval = -1;
  int iNumRecordings = 0;

  XBMC->Log(LOG_DEBUG, "GetNumRecordings()");
  retval = ForTheRecord::GetRecordingGroupByTitle(response);
  if (retval >= 0)
  {
    int size = response.size();

    // parse channelgroup list
    for ( int index = 0; index < size; ++index )
    {
      cRecordingGroup recordinggroup;
      if (recordinggroup.Parse(response[index]))
      {
        iNumRecordings += recordinggroup.RecordingsCount();
      }
    }
  }
  return iNumRecordings;
}

PVR_ERROR cPVRClientForTheRecord::RequestRecordingsList(PVRHANDLE handle)
{
  Json::Value recordinggroupresponse;
  int retval = -1;
  int iNumRecordings = 0;

  XBMC->Log(LOG_DEBUG, "RequestRecordingsList()");
  retval = ForTheRecord::GetRecordingGroupByTitle(recordinggroupresponse);
  if(retval >= 0)
  {           
    // process list of recording groups
    int size = recordinggroupresponse.size();
    for ( int recordinggroupindex = 0; recordinggroupindex < size; ++recordinggroupindex )
    {
      cRecordingGroup recordinggroup;
      if (recordinggroup.Parse(recordinggroupresponse[recordinggroupindex]))
      {
        Json::Value recordingsbytitleresponse;
        retval = ForTheRecord::GetRecordingsForTitle(recordinggroup.ProgramTitle(), recordingsbytitleresponse);
        if (retval >= 0)
        {
          // process list of recording summaries for this group
          int nrOfRecordings = recordingsbytitleresponse.size();
          for (int recordingindex = 0; recordingindex < nrOfRecordings; recordingindex++)
          {
            cRecording recording;
            if (FetchRecordingDetails(recordingsbytitleresponse[recordingindex], recording))
            {
              PVR_RECORDINGINFO tag;
              memset(&tag, 0 , sizeof(tag));
              tag.index           = iNumRecordings;
              tag.channel_name    = recording.ChannelDisplayName();
              tag.lifetime        = MAXLIFETIME; //TODO: recording.Lifetime();
              tag.priority        = 0; //TODO? recording.Priority();
              tag.recording_time  = recording.RecordingStartTime();
              tag.duration        = recording.RecordingStopTime() - recording.RecordingStartTime();
              tag.description     = recording.Description();;
              tag.title           = recording.Title();
              tag.subtitle        = recording.SubTitle();
              if (nrOfRecordings > 1)
              {
                tag.directory     = recordinggroup.ProgramTitle().c_str(); //used in XBMC as directory structure below "Server X - hostname"
              }
              else
              {
                tag.directory     = "";
              }
              tag.stream_url      = recording.RecordingFileName();
              PVR->TransferRecordingEntry(handle, &tag);
              iNumRecordings++;
            }
          }
        }
      }
    }
  }
  return PVR_ERROR_NO_ERROR;
}

bool cPVRClientForTheRecord::FetchRecordingDetails(const Json::Value& data, cRecording& recording)
{ 
  bool fRc = false;
  Json::Value recordingresponse;

  cRecordingSummary recordingsummary;
  if (recordingsummary.Parse(data))
  {
    int retval = ForTheRecord::GetRecordingById(recordingsummary.RecordingId(), recordingresponse);
    if (retval >= 0)
    {
      if (recordingresponse.type() == Json::objectValue)
      {
        fRc = recording.Parse(recordingresponse);
      }
    }
  }
  return fRc;
}

PVR_ERROR cPVRClientForTheRecord::DeleteRecording(const PVR_RECORDINGINFO &recinfo)
{
  // JSONify the stream_url
  Json::Value recordingname (recinfo.stream_url);
  Json::StyledWriter writer;
  std::string jsonval = writer.write(recordingname);
  if (ForTheRecord::DeleteRecording(jsonval) >= 0) 
  {
    return PVR_ERROR_NO_ERROR;
  }
  else
  {
    return PVR_ERROR_NOT_DELETED;
  }
}

PVR_ERROR cPVRClientForTheRecord::RenameRecording(const PVR_RECORDINGINFO &recinfo, const char *newname)
{
  return PVR_ERROR_NO_ERROR;
}


/************************************************************/
/** Timer handling */

int cPVRClientForTheRecord::GetNumTimers(void)
{
  // Not directly possible in ForTheRecord
  Json::Value response;

  XBMC->Log(LOG_DEBUG, "GetNumTimers()");
  // pick up the schedulelist for TV
  int retval = ForTheRecord::GetUpcomingPrograms(response);
  if (retval < 0) 
  {
    return 0;
  }

  return response.size();
}

PVR_ERROR cPVRClientForTheRecord::RequestTimerList(PVRHANDLE handle)
{
  Json::Value activeRecordingsResponse, upcomingProgramsResponse;
  int         iNumberOfTimers = 0;
  PVR_TIMERINFO   tag;
  int         numberoftimers;

  XBMC->Log(LOG_DEBUG, "%s", __FUNCTION__);

  // retrieve the currently active recordings
  int retval = ForTheRecord::GetActiveRecordings(activeRecordingsResponse);

  // pick up the upcoming recordings
  retval = ForTheRecord::GetUpcomingPrograms(upcomingProgramsResponse);
  if (retval < 0) 
  {
    return PVR_ERROR_SERVER_ERROR;
  }

  memset(&tag, 0 , sizeof(tag));
  numberoftimers = upcomingProgramsResponse.size();

  for (int i = 0; i < numberoftimers; i++)
  {
    cUpcomingRecording upcomingrecording;
    if (upcomingrecording.Parse(upcomingProgramsResponse[i]))
    {
      tag.index       = iNumberOfTimers;
      cChannel* pChannel = FetchChannel(upcomingrecording.ChannelId());
      tag.channelNum  = pChannel->ID();
      tag.firstday    = 0;
      tag.starttime   = upcomingrecording.StartTime() - (tag.marginstart * 60);
      tag.endtime     = upcomingrecording.StopTime() + (tag.marginstop * 60);
      // build the XBMC PVR State
      tag.recording   = false;
      if (upcomingrecording.IsCancelled())
      {
        tag.active      = false;
      }
      else
      {
        tag.active      = true;
        if (activeRecordingsResponse.size() > 0)
        {
          // Is the this upcoming program in the list of active recordings?
          for (Json::Value::UInt j = 0; j < activeRecordingsResponse.size(); j++)
          {
            cActiveRecording activerecording;
            if (activerecording.Parse(activeRecordingsResponse[j]))
            {
              if (upcomingrecording.UpcomingProgramId() == activerecording.UpcomingProgramId())
              {
                tag.recording = true;
                break;
              }
            }
          }
        }
      }

      tag.title       = upcomingrecording.Title().c_str();
      tag.directory   = "";
      tag.priority    = 0;
      tag.lifetime    = 0;
      tag.repeat      = false;
      tag.repeatflags = 0;
      tag.marginstart = upcomingrecording.PreRecordSeconds() / 60;
      tag.marginstop  = upcomingrecording.PostRecordSeconds() / 60;


      PVR->TransferTimerEntry(handle, &tag);
      iNumberOfTimers++;
    }
  }

  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR cPVRClientForTheRecord::GetTimerInfo(unsigned int timernumber, PVR_TIMERINFO &tag)
{
  return PVR_ERROR_NOT_IMPLEMENTED;
}

PVR_ERROR cPVRClientForTheRecord::AddTimer(const PVR_TIMERINFO &timerinfo)
{
  XBMC->Log(LOG_DEBUG, "AddTimer()");

  // re-synthesize the FTR startime, stoptime and channel GUID
  time_t starttime = timerinfo.starttime + (timerinfo.marginstart * 60);
  cChannel* pChannel = FetchChannel(timerinfo.channelNum);

  Json::Value addScheduleResponse;
  int retval = ForTheRecord::AddOneTimeSchedule(pChannel->Guid(), timerinfo.starttime, timerinfo.title, timerinfo.marginstart * 60, timerinfo.marginstop * 60, addScheduleResponse);
  if (retval < 0) 
  {
    return PVR_ERROR_SERVER_ERROR;
  }

  std::string scheduleid = addScheduleResponse["ScheduleId"].asString();

  // Ok, we created a schedule, but did that lead to an upcoming recording?
  Json::Value upcomingProgramsResponse;
  retval = ForTheRecord::GetUpcomingProgramsForSchedule(addScheduleResponse, upcomingProgramsResponse);

  // We should have at least one upcoming program for this schedule, otherwise nothing will be recorded
  if (retval <= 0)
  {
    // remove the added (now stale) schedule, ignore failure (what are we to do anyway?)
    ForTheRecord::DeleteSchedule(scheduleid);

    // TODO: remove the added (now stale) schedule and add a manual recording
    return PVR_ERROR_SERVER_ERROR;
  }

  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR cPVRClientForTheRecord::DeleteTimer(const PVR_TIMERINFO &timerinfo, bool force)
{
  Json::Value response;

  XBMC->Log(LOG_DEBUG, "DeleteTimer()");

  // re-synthesize the FTR startime, stoptime and channel GUID
  time_t starttime = timerinfo.starttime + (timerinfo.marginstart * 60);
  time_t stoptime = timerinfo.endtime - (timerinfo.marginstop * 60);
  cChannel* pChannel = FetchChannel(timerinfo.channelNum);

  // pick up the upcoming recordings
  int retval = ForTheRecord::GetUpcomingPrograms(response);
  if (retval < 0) 
  {
    return PVR_ERROR_SERVER_ERROR;
  }

  // try to find the upcoming recording that matches this xbmc timer
  int numberoftimers = response.size();
  for (int i = 0; i < numberoftimers; i++)
  {
    cUpcomingRecording upcomingrecording;
    if (upcomingrecording.Parse(response[i]))
    {
      if (upcomingrecording.ChannelId() == pChannel->Guid())
      {
        if (upcomingrecording.StartTime() == starttime)
        {
          if (upcomingrecording.StopTime() == stoptime)
          {
            retval = ForTheRecord::CancelUpcomingProgram(upcomingrecording.ScheduleId(), upcomingrecording.ChannelId(),
              upcomingrecording.StartTime(), upcomingrecording.UpcomingProgramId());
            if (retval >= 0) return PVR_ERROR_NO_ERROR;
            else return PVR_ERROR_SERVER_ERROR;
          }
        }
      }
    }
  }
  return PVR_ERROR_NOT_POSSIBLE;
}

PVR_ERROR cPVRClientForTheRecord::RenameTimer(const PVR_TIMERINFO &timerinfo, const char *newname)
{
  return PVR_ERROR_NOT_IMPLEMENTED;
}

PVR_ERROR cPVRClientForTheRecord::UpdateTimer(const PVR_TIMERINFO &timerinfo)
{
  return PVR_ERROR_NOT_IMPLEMENTED;
}


/************************************************************/
/** Live stream handling */
cChannel* cPVRClientForTheRecord::FetchChannel(int channel_uid)
{
  // Search for this channel in our local channel list to find the original ChannelID back:
  vector<cChannel>::iterator it;

  for ( it=m_Channels.begin(); it < m_Channels.end(); it++ )
  {
    if (it->ID() == channel_uid)
    {
      return &*it;
      break;
    }
  }

  return NULL;
}

cChannel* cPVRClientForTheRecord::FetchChannel(std::string channelid)
{
  // Search for this channel in our local channel list to find the original ChannelID back:
  vector<cChannel>::iterator it;

  for ( it=m_Channels.begin(); it < m_Channels.end(); it++ )
  {
    if (it->Guid() == channelid)
    {
      return &*it;
      break;
    }
  }

  return NULL;
}


bool cPVRClientForTheRecord::OpenLiveStream(const PVR_CHANNEL &channelinfo)
{
  XBMC->Log(LOG_DEBUG, "->OpenLiveStream(%i)", channelinfo.uid);

  cChannel* channel = FetchChannel(channelinfo.uid);

  if (channel)
  {
    std::string filename;
    XBMC->Log(LOG_INFO, "Tune XBMC channel: %i", channelinfo.uid);
    XBMC->Log(LOG_INFO, "Corresponding ForTheRecord channel: %s", channel->Guid().c_str());

    ForTheRecord::TuneLiveStream(channel->Guid(), channel->Type(), filename);

    if (filename.length() == 0)
    {
      XBMC->Log(LOG_ERROR, "Could not start the timeshift for channel %i (%s)", channelinfo.uid, channel->Guid().c_str());
      return false;
    }

    XBMC->Log(LOG_INFO, "Live stream file: %s", filename.c_str());
    m_bTimeShiftStarted = true;

#ifdef TSREADER
    if (m_tsreader != NULL)
    {
      m_keepalive.StopThread(0);
      m_tsreader->Close();
      delete m_tsreader;
      m_tsreader = new CTsReader();
    } else {
      m_tsreader = new CTsReader();
    }

    // Open Timeshift buffer
    // TODO: rtsp support
    m_tsreader->Open(filename.c_str());
    m_keepalive.StartThread();
#endif
    return true;
  }

  return false;
}

int cPVRClientForTheRecord::ReadLiveStream(unsigned char* pBuffer, int iBufferSize)
{
#ifdef TSREADER
  unsigned long read_wanted = iBufferSize;
  unsigned long read_done   = 0;
  static int read_timeouts  = 0;
  unsigned char* bufptr = pBuffer;

  //XBMC->Log(LOG_DEBUG, "->ReadLiveStream(buf_size=%i)", buf_size);

  while (read_done < (unsigned long) iBufferSize)
  {
    read_wanted = iBufferSize - read_done;
    if (!m_tsreader)
      return -1;

    if (m_tsreader->Read(bufptr, read_wanted, &read_wanted) > 0)
    {
      usleep(20000);
      read_timeouts++;
      return read_wanted;
    }
    read_done += read_wanted;

    if ( read_done < (unsigned long) iBufferSize )
    {
      if (read_timeouts > 50)
      {
        XBMC->Log(LOG_INFO, "No data in 1 second");
        read_timeouts = 0;
        return read_done;
      }
      bufptr += read_wanted;
      read_timeouts++;
      usleep(20000);
    }
  }
  read_timeouts = 0;
  return read_done;
#else
  return 0;
#endif //TSREADER
}

void cPVRClientForTheRecord::CloseLiveStream()
{
  string result;

  if (m_bTimeShiftStarted)
  {
#ifdef TSREADER
    if (m_tsreader)
    {
      m_tsreader->Close();
      delete_null(m_tsreader);
    }
    m_keepalive.StopThread();
#endif
    ForTheRecord::StopLiveStream();
    XBMC->Log(LOG_INFO, "CloseLiveStream");
    m_bTimeShiftStarted = false;
  } else {
    XBMC->Log(LOG_DEBUG, "CloseLiveStream: Nothing to do.");
  }
}


bool cPVRClientForTheRecord::SwitchChannel(const PVR_CHANNEL &channelinfo)
{
  return OpenLiveStream(channelinfo);
}


int cPVRClientForTheRecord::GetCurrentClientChannel()
{
  return 0;
}

PVR_ERROR cPVRClientForTheRecord::SignalQuality(PVR_SIGNALQUALITY &qualityinfo)
{
  return PVR_ERROR_NO_ERROR;
}


/************************************************************/
/** Record stream handling */
bool cPVRClientForTheRecord::OpenRecordedStream(const PVR_RECORDINGINFO &recinfo)
{
  XBMC->Log(LOG_DEBUG, "->OpenRecordedStream(index=%i)", recinfo.index);

  return false;
}


void cPVRClientForTheRecord::CloseRecordedStream(void)
{
}

int cPVRClientForTheRecord::ReadRecordedStream(unsigned char* buf, int buf_size)
{
  return -1;
}

/*
 * \brief Request the stream URL for live tv/live radio.
 */
const char* cPVRClientForTheRecord::GetLiveStreamURL(const PVR_CHANNEL &channelinfo)
{
  return false;
}
