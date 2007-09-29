#if 0
    LX200 Generic
    Copyright (C) 2003 Jasem Mutlaq (mutlaqja@ikarustech.com)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#include "indicom.h"
#include "lx200driver.h"
#include "lx200gps.h"
#include "lx200classic.h"

LX200Generic *telescope = NULL;
int MaxReticleFlashRate = 3;

/* There is _one_ binary for all LX200 drivers, but each binary is renamed
** to its device name (i.e. lx200gps, lx200_16..etc). The main function will
** fetch from std args the binary name and ISInit will create the apporpiate
** device afterwards. If the binary name does not match any known devices,
** we simply create a generic device
*/
extern char* me;

#define COMM_GROUP	"Communication"
#define BASIC_GROUP	"Main Control"
#define MOTION_GROUP	"Motion Control"
#define DATETIME_GROUP	"Date/Time"
#define SITE_GROUP	"Site Management"
#define FOCUS_GROUP	"Focus Control"

#define LX200_SLEW	0
#define LX200_TRACK	1
#define LX200_SYNC	2
#define LX200_PARK	3

/* Simulation Parameters */
#define	SLEWRATE	1		/* slew rate, degrees/s */
#define SIDRATE		0.004178	/* sidereal rate, degrees/s */

static void ISPoll(void *);
static void retryConnection(void *);

/*INDI Propertries */

/**********************************************************************************************/
/************************************ GROUP: Communication ************************************/
/**********************************************************************************************/

/********************************************
 Property: Connection
*********************************************/
static ISwitch ConnectS[]          	= {{"CONNECT" , "Connect" , ISS_OFF, 0, 0},{"DISCONNECT", "Disconnect", ISS_ON, 0, 0}};
ISwitchVectorProperty ConnectSP		= { mydev, "CONNECTION" , "Connection", COMM_GROUP, IP_RW, ISR_1OFMANY, 0, IPS_IDLE, ConnectS, NARRAY(ConnectS), "", 0};

/********************************************
 Property: Device Port
*********************************************/
static IText PortT[]			= {{"PORT", "Port", 0, 0, 0, 0}};
static ITextVectorProperty PortTP	= { mydev, "DEVICE_PORT", "Ports", COMM_GROUP, IP_RW, 0, IPS_IDLE, PortT, NARRAY(PortT), "", 0};

/********************************************
 Property: Telescope Alignment Mode
*********************************************/
static ISwitch AlignmentS []		= {{"Polar", "", ISS_ON, 0, 0}, {"AltAz", "", ISS_OFF, 0, 0}, {"Land", "", ISS_OFF, 0, 0}};
static ISwitchVectorProperty AlignmentSw= { mydev, "Alignment", "", COMM_GROUP, IP_RW, ISR_1OFMANY, 0, IPS_IDLE, AlignmentS, NARRAY(AlignmentS), "", 0};

/**********************************************************************************************/
/************************************ GROUP: Main Control *************************************/
/**********************************************************************************************/

/********************************************
 Property: Equatorial Coordinates JNow
 Perm: Transient WO.
 Timeout: 120 seconds.
*********************************************/
INumber EquatorialCoordsWN[] 		= { {"RA",  "RA  H:M:S", "%10.6m",  0., 24., 0., 0., 0, 0, 0}, {"DEC", "Dec D:M:S", "%10.6m", -90., 90., 0., 0., 0, 0, 0} };
INumberVectorProperty EquatorialCoordsWNP= { mydev, "EQUATORIAL_EOD_COORD_REQUEST", "Equatorial JNow", BASIC_GROUP, IP_WO, 120, IPS_IDLE, EquatorialCoordsWN, NARRAY(EquatorialCoordsWN), "", 0};

/********************************************
 Property: Equatorial Coordinates JNow
 Perm: RO
*********************************************/
INumber EquatorialCoordsRN[]	 	= { {"RA",  "RA  H:M:S", "%10.6m",  0., 24., 0., 0., 0, 0, 0}, {"DEC", "Dec D:M:S", "%10.6m", -90., 90., 0., 0., 0, 0, 0}};
INumberVectorProperty EquatorialCoordsRNP= { mydev, "EQUATORIAL_EOD_COORD", "Equatorial JNow", BASIC_GROUP, IP_RO, 120, IPS_IDLE, EquatorialCoordsRN, NARRAY(EquatorialCoordsRN), "", 0};

/********************************************
 Property: On Coord Set
 Description: This property decides what happens
             when we receive a new equatorial coord
             value. We either slew, track, or sync
	     to the new coordinates.
*********************************************/
static ISwitch OnCoordSetS[]		 = {{"SLEW", "Slew", ISS_ON, 0, 0 }, {"TRACK", "Track", ISS_OFF, 0, 0}, {"SYNC", "Sync", ISS_OFF, 0 , 0}};
static ISwitchVectorProperty OnCoordSetSP= { mydev, "ON_COORD_SET", "On Set", BASIC_GROUP, IP_RW, ISR_1OFMANY, 0, IPS_IDLE, OnCoordSetS, NARRAY(OnCoordSetS), "", 0};

/********************************************
 Property: Abort telescope motion
*********************************************/
static ISwitch AbortSlewS[]		= {{"ABORT", "Abort", ISS_OFF, 0, 0 }};
static ISwitchVectorProperty AbortSlewSP= { mydev, "TELESCOPE_ABORT_MOTION", "Abort Slew/Track", BASIC_GROUP, IP_RW, ISR_1OFMANY, 0, IPS_IDLE, AbortSlewS, NARRAY(AbortSlewS), "", 0};

/********************************************
 Property: Park telescope to HOME
*********************************************/
static ISwitch ParkS[]		 	= { {"PARK", "Park", ISS_OFF, 0, 0} };
ISwitchVectorProperty ParkSP		= {mydev, "TELESCOPE_PARK", "Park Scope", BASIC_GROUP, IP_RW, ISR_ATMOST1, 0, IPS_IDLE, ParkS, NARRAY(ParkS), "", 0 };

/**********************************************************************************************/
/************************************** GROUP: Motion *****************************************/
/**********************************************************************************************/

/********************************************
 Property: Slew Speed
*********************************************/
static ISwitch SlewModeS[]		= {{"Max", "", ISS_ON, 0, 0}, {"Find", "", ISS_OFF, 0, 0}, {"Centering", "", ISS_OFF, 0, 0}, {"Guide", "", ISS_OFF, 0 , 0}};
static ISwitchVectorProperty SlewModeSP	= { mydev, "Slew rate", "", MOTION_GROUP, IP_RW, ISR_1OFMANY, 0, IPS_IDLE, SlewModeS, NARRAY(SlewModeS), "", 0};

/********************************************
 Property: Tracking Mode
*********************************************/
static ISwitch TrackModeS[]		= {{ "Default", "", ISS_ON, 0, 0} , { "Lunar", "", ISS_OFF, 0, 0}, {"Manual", "", ISS_OFF, 0, 0}};
static ISwitchVectorProperty TrackModeSP= { mydev, "Tracking Mode", "", MOTION_GROUP, IP_RW, ISR_1OFMANY, 0, IPS_IDLE, TrackModeS, NARRAY(TrackModeS), "", 0};

/********************************************
 Property: Tracking Frequency
*********************************************/
static INumber TrackFreqN[] 		 = {{ "trackFreq", "Freq", "%g", 56.4, 60.1, 0.1, 60.1, 0, 0, 0}};
static INumberVectorProperty TrackingFreqNP= { mydev, "Tracking Frequency", "", MOTION_GROUP, IP_RW, 0, IPS_IDLE, TrackFreqN, NARRAY(TrackFreqN), "", 0};

/********************************************
 Property: Movement (Arrow keys on handset). North/South
*********************************************/
static ISwitch MovementNSS[]       = {{"MOTION_NORTH", "North", ISS_OFF, 0, 0}, {"MOTION_SOUTH", "South", ISS_OFF, 0, 0}};
static ISwitchVectorProperty MovementNSSP      = { mydev, "TELESCOPE_MOTION_NS", "North/South", MOTION_GROUP, IP_RW, ISR_1OFMANY, 0, IPS_IDLE, MovementNSS, NARRAY(MovementNSS), "", 0};

/********************************************
 Property: Movement (Arrow keys on handset). West/East
*********************************************/
static ISwitch MovementWES[]       = {{"MOTION_WEST", "West", ISS_OFF, 0, 0}, {"MOTION_EAST", "East", ISS_OFF, 0, 0}};
static ISwitchVectorProperty MovementWESP      = { mydev, "TELESCOPE_MOTION_WE", "West/East", MOTION_GROUP, IP_RW, ISR_1OFMANY, 0, IPS_IDLE, MovementWES, NARRAY(MovementWES), "", 0};

/********************************************
 Property: Tracking Precision
 Desciption: How close the scope have to be with
	     respect to the requested coords for 
	     the tracking operation to be successull
	     i.e. returns OK
*********************************************/
INumber TrackingPrecisionN[] = {
    {"TrackRA",  "RA (arcmin)", "%g",  0., 60., 1., 3.0, 0, 0, 0},
    {"TrackDEC", "Dec (arcmin)", "%g", 0., 60., 1., 3.0, 0, 0, 0},
};
static INumberVectorProperty TrackingPrecisionNP = {mydev, "Tracking Precision", "", MOTION_GROUP, IP_RW, 0, IPS_IDLE, TrackingPrecisionN, NARRAY(TrackingPrecisionN), "", 0};

/********************************************
 Property: Slew precision
 Description: How close the scope have to be with
	     respect to the requested coords for 
	     the slew operation to be successull
	     i.e. returns OK
*********************************************/
INumber SlewPrecisionN[] = {
    {"SlewRA",  "RA (arcmin)", "%g",  0., 60., 1., 3.0, 0, 0, 0},
    {"SlewDEC", "Dec (arcmin)", "%g", 0., 60., 1., 3.0, 0, 0, 0},
};
static INumberVectorProperty SlewPrecisionNP = {mydev, "Slew Precision", "", MOTION_GROUP, IP_RW, 0, IPS_IDLE, SlewPrecisionN, NARRAY(SlewPrecisionN), "", 0};

/**********************************************************************************************/
/************************************** GROUP: Focus ******************************************/
/**********************************************************************************************/

/********************************************
 Property: Focus Direction
*********************************************/
ISwitch  FocusMotionS[]	 = { {"IN", "Focus in", ISS_OFF, 0, 0}, {"OUT", "Focus out", ISS_OFF, 0, 0}};
ISwitchVectorProperty	FocusMotionSP = {mydev, "FOCUS_MOTION", "Motion", FOCUS_GROUP, IP_RW, ISR_1OFMANY, 0, IPS_IDLE, FocusMotionS, NARRAY(FocusMotionS), "", 0};

/********************************************
 Property: Focus Timer
*********************************************/
INumber  FocusTimerN[]    = { {"TIMER", "Timer (ms)", "%g", 0., 10000., 1000., 50., 0, 0, 0 }};
INumberVectorProperty FocusTimerNP = { mydev, "FOCUS_TIMER", "Focus Timer", FOCUS_GROUP, IP_RW, 0, IPS_IDLE, FocusTimerN, NARRAY(FocusTimerN), "", 0};

/********************************************
 Property: Focus Mode
*********************************************/
static ISwitch  FocusModeS[]	 = { {"FOCUS_HALT", "Halt", ISS_ON, 0, 0},
				     {"FOCUS_SLOW", "Slow", ISS_OFF, 0, 0},
				     {"FOCUS_FAST", "Fast", ISS_OFF, 0, 0}};
static ISwitchVectorProperty FocusModeSP = {mydev, "FOCUS_MODE", "Mode", FOCUS_GROUP, IP_RW, ISR_1OFMANY, 0, IPS_IDLE, FocusModeS, NARRAY(FocusModeS), "", 0};

/**********************************************************************************************/
/*********************************** GROUP: Date & Time ***************************************/
/**********************************************************************************************/

/********************************************
 Property: UTC Time
*********************************************/
static IText TimeT[] = {{"UTC", "UTC", 0, 0, 0, 0}};
ITextVectorProperty TimeTP = { mydev, "TIME_UTC", "UTC Time", DATETIME_GROUP, IP_RW, 0, IPS_IDLE, TimeT, NARRAY(TimeT), "", 0};

/********************************************
 Property: DST Corrected UTC Offfset
*********************************************/
static INumber UTCOffsetN[] = {{"OFFSET", "Offset", "%0.3g" , -12.,12.,0.5,0., 0, 0, 0}};
INumberVectorProperty UTCOffsetNP = { mydev, "TIME_UTC_OFFSET", "UTC Offset", DATETIME_GROUP, IP_RW, 0, IPS_IDLE, UTCOffsetN , NARRAY(UTCOffsetN), "", 0};

/********************************************
 Property: Sidereal Time
*********************************************/
static INumber SDTimeN[] = {{"LST", "Sidereal time", "%10.6m" , 0.,24.,0.,0., 0, 0, 0}};
INumberVectorProperty SDTimeNP = { mydev, "TIME_LST", "Sidereal Time", DATETIME_GROUP, IP_RW, 0, IPS_IDLE, SDTimeN, NARRAY(SDTimeN), "", 0};

/**********************************************************************************************/
/************************************* GROUP: Sites *******************************************/
/**********************************************************************************************/

/********************************************
 Property: Site Management
*********************************************/
static ISwitch SitesS[]          = {{"Site 1", "", ISS_ON, 0, 0}, {"Site 2", "", ISS_OFF, 0, 0},  {"Site 3", "", ISS_OFF, 0, 0},  {"Site 4", "", ISS_OFF, 0 ,0}};
static ISwitchVectorProperty SitesSP  = { mydev, "Sites", "", SITE_GROUP, IP_RW, ISR_1OFMANY, 0, IPS_IDLE, SitesS, NARRAY(SitesS), "", 0};

/********************************************
 Property: Site Name
*********************************************/
static IText   SiteNameT[] = {{"Name", "", 0, 0, 0, 0}};
static ITextVectorProperty SiteNameTP = { mydev, "Site Name", "", SITE_GROUP, IP_RW, 0 , IPS_IDLE, SiteNameT, NARRAY(SiteNameT), "", 0};

/********************************************
 Property: Geographical Location
*********************************************/

static INumber geo[] = {
    {"LAT",  "Lat.  D:M:S +N", "%10.6m",  -90.,  90., 0., 0., 0, 0, 0},
    {"LONG", "Long. D:M:S +E", "%10.6m", 0., 360., 0., 0., 0, 0, 0},
};
static INumberVectorProperty geoNP = {
    mydev, "GEOGRAPHIC_COORD", "Geographic Location", SITE_GROUP, IP_RW, 0., IPS_IDLE,
    geo, NARRAY(geo), "", 0};

/*****************************************************************************************************/
/**************************************** END PROPERTIES *********************************************/
/*****************************************************************************************************/

void changeLX200GenericDeviceName(const char * newName)
{
  // COMM_GROUP
  strcpy(ConnectSP.device , newName);
  strcpy(PortTP.device , newName);
  strcpy(AlignmentSw.device, newName);

  // BASIC_GROUP
  strcpy(EquatorialCoordsWNP.device, newName);
  strcpy(EquatorialCoordsRNP.device, newName);
  strcpy(OnCoordSetSP.device , newName );
  strcpy(AbortSlewSP.device , newName );
  strcpy(ParkSP.device, newName);

  // MOTION_GROUP
  strcpy(SlewModeSP.device , newName );
  strcpy(TrackModeSP.device , newName );
  strcpy(TrackingFreqNP.device , newName );
  strcpy(MovementNSSP.device , newName );
  strcpy(MovementWESP.device , newName );
  strcpy(TrackingPrecisionNP.device, newName);
  strcpy(SlewPrecisionNP.device, newName);

  // FOCUS_GROUP
  strcpy(FocusModeSP.device , newName );
  strcpy(FocusMotionSP.device , newName );
  strcpy(FocusTimerNP.device, newName);

  // DATETIME_GROUP
  strcpy(TimeTP.device , newName );
  strcpy(UTCOffsetNP.device , newName );
  strcpy(SDTimeNP.device , newName );

  // SITE_GROUP
  strcpy(SitesSP.device , newName );
  strcpy(SiteNameTP.device , newName );
  strcpy(geoNP.device , newName );
  
}

void changeAllDeviceNames(const char *newName)
{
  changeLX200GenericDeviceName(newName);
  changeLX200AutostarDeviceName(newName);
  changeLX200_16DeviceName(newName);
  changeLX200ClassicDeviceName(newName);
  changeLX200GPSDeviceName(newName);
}


/* send client definitions of all properties */
void ISInit()
{
  static int isInit=0;

 if (isInit)
  return;

 isInit = 1;
 
  IUSaveText(&PortT[0], "/dev/ttyS0");
  IUSaveText(&TimeT[0], "YYYY-MM-DDTHH:MM:SS");

  // We need to check if UTCOffset has been set by user or not
  UTCOffsetN[0].aux0 = (int *) malloc(sizeof(int));
  *((int *) UTCOffsetN[0].aux0) = 0;
  
  
  if (strstr(me, "indi_lx200classic"))
  {
     fprintf(stderr , "initilizaing from LX200 classic device...\n");
     // 1. mydev = device_name
     changeAllDeviceNames("LX200 Classic");
     // 2. device = sub_class
     telescope = new LX200Classic();
     telescope->setCurrentDeviceName("LX200 Classic");

     MaxReticleFlashRate = 3;
  }

  else if (strstr(me, "indi_lx200gps"))
  {
     fprintf(stderr , "initilizaing from LX200 GPS device...\n");
     // 1. mydev = device_name
     changeAllDeviceNames("LX200 GPS");
     // 2. device = sub_class
     telescope = new LX200GPS();
     telescope->setCurrentDeviceName("LX200 GPS");

     MaxReticleFlashRate = 9;
  }
  else if (strstr(me, "indi_lx200_16"))
  {

    IDLog("Initilizaing from LX200 16 device...\n");
    // 1. mydev = device_name
    changeAllDeviceNames("LX200 16");
    // 2. device = sub_class
   telescope = new LX200_16();
   telescope->setCurrentDeviceName("LX200 16");

   MaxReticleFlashRate = 3;
 }
 else if (strstr(me, "indi_lx200autostar"))
 {
   fprintf(stderr , "initilizaing from autostar device...\n");
  
   // 1. change device name
   changeAllDeviceNames("LX200 Autostar");
   // 2. device = sub_class
   telescope = new LX200Autostar();
   telescope->setCurrentDeviceName("LX200 Autostar");

   MaxReticleFlashRate = 9;
 }
 // be nice and give them a generic device
 else
 {
  telescope = new LX200Generic();
  telescope->setCurrentDeviceName("LX200 Generic");
 }

}

void ISGetProperties (const char *dev)
{ ISInit(); telescope->ISGetProperties(dev); IEAddTimer (POLLMS, ISPoll, NULL);}
void ISNewSwitch (const char *dev, const char *name, ISState *states, char *names[], int n)
{ ISInit(); telescope->ISNewSwitch(dev, name, states, names, n);}
void ISNewText (const char *dev, const char *name, char *texts[], char *names[], int n)
{ ISInit(); telescope->ISNewText(dev, name, texts, names, n);}
void ISNewNumber (const char *dev, const char *name, double values[], char *names[], int n)
{ ISInit(); telescope->ISNewNumber(dev, name, values, names, n);}
void ISPoll (void *p) { telescope->ISPoll(); IEAddTimer (POLLMS, ISPoll, NULL); p=p;}
void ISNewBLOB (const char *dev, const char *name, int sizes[], int blobsizes[], char *blobs[], char *formats[], char *names[], int n)
{
  INDI_UNUSED(dev);
  INDI_UNUSED(name);
  INDI_UNUSED(sizes);
  INDI_UNUSED(blobsizes);
  INDI_UNUSED(blobs);
  INDI_UNUSED(formats);
  INDI_UNUSED(names);
  INDI_UNUSED(n);
}
void ISSnoopDevice (XMLEle *root)
{
  INDI_UNUSED(root);
}

/**************************************************
*** LX200 Generic Implementation
***************************************************/

LX200Generic::LX200Generic()
{
   currentSiteNum = 1;
   trackingMode   = LX200_TRACK_DEFAULT;
   lastSet        = -1;
   fault          = false;
   simulation     = false;
   targetRA       = 0;
   targetDEC      = 0;
   currentRA      = 0;
   currentDEC     = 0;
   currentSet     = 0;
   fd             = -1;

   // Children call parent routines, this is the default
   IDLog("INDI Library v%g\n", INDI_LIBV);
   IDLog("initilizaing from generic LX200 device...\n");
   IDLog("Driver Version: 2007-05-11\n");
 
   //enableSimulation(true);  
}

LX200Generic::~LX200Generic()
{
}

void LX200Generic::setCurrentDeviceName(const char * devName)
{
  strcpy(thisDevice, devName);
}

void LX200Generic::ISGetProperties(const char *dev)
{

 if (dev && strcmp (thisDevice, dev))
    return;

  // COMM_GROUP
  IDDefSwitch (&ConnectSP, NULL);
  IDDefText   (&PortTP, NULL);
  IDDefSwitch (&AlignmentSw, NULL);

  // BASIC_GROUP
  IDDefNumber (&EquatorialCoordsWNP, NULL);
  IDDefNumber (&EquatorialCoordsRNP, NULL);
  IDDefSwitch (&OnCoordSetSP, NULL);
  IDDefSwitch (&AbortSlewSP, NULL);
  IDDefSwitch (&ParkSP, NULL);

  // MOTION_GROUP
  IDDefNumber (&TrackingFreqNP, NULL);
  IDDefSwitch (&SlewModeSP, NULL);
  IDDefSwitch (&TrackModeSP, NULL);
  IDDefSwitch (&MovementNSSP, NULL);
  IDDefSwitch (&MovementWESP, NULL);
  IDDefNumber (&TrackingPrecisionNP, NULL);
  IDDefNumber (&SlewPrecisionNP, NULL);

  // FOCUS_GROUP
  IDDefSwitch(&FocusModeSP, NULL);
  IDDefSwitch(&FocusMotionSP, NULL);
  IDDefNumber(&FocusTimerNP, NULL);

  // DATETIME_GROUP
  IDDefText   (&TimeTP, NULL);
  IDDefNumber(&UTCOffsetNP, NULL);
  IDDefNumber (&SDTimeNP, NULL);

  // SITE_GROUP
  IDDefSwitch (&SitesSP, NULL);
  IDDefText   (&SiteNameTP, NULL);
  IDDefNumber (&geoNP, NULL);
  
  /* Send the basic data to the new client if the previous client(s) are already connected. */		
   if (ConnectSP.s == IPS_OK)
       getBasicData();

}

void LX200Generic::ISNewText (const char *dev, const char *name, char *texts[], char *names[], int n)
{
	int err;
	IText *tp;

	// ignore if not ours //
	if (strcmp (dev, thisDevice))
	    return;

	// suppress warning
	n=n;

	if (!strcmp(name, PortTP.name) )
	{
	  PortTP.s = IPS_OK;
	  tp = IUFindText( &PortTP, names[0] );
	  if (!tp)
	   return;

	  IUSaveText(&PortTP.tp[0], texts[0]);
	  IDSetText (&PortTP, NULL);
	  return;
	}

	if (!strcmp (name, SiteNameTP.name) )
	{
	  if (checkPower(&SiteNameTP))
	   return;

	  if ( ( err = setSiteName(fd, texts[0], currentSiteNum) < 0) )
	  {
	     handleError(&SiteNameTP, err, "Setting site name");
	     return;
	  }
	     SiteNameTP.s = IPS_OK;
	     tp = IUFindText(&SiteNameTP, names[0]);
	     tp->text = new char[strlen(texts[0])+1];
	     strcpy(tp->text, texts[0]);
   	     IDSetText(&SiteNameTP , "Site name updated");
	     return;
       }

       if (!strcmp (name, TimeTP.name))
       {
	  if (checkPower(&TimeTP))
	   return;

	 if (simulation)
	 {
		TimeTP.s = IPS_OK;
		IUSaveText(&TimeTP.tp[0], texts[0]);
		IDSetText(&TimeTP, "Simulated time updated.");
		return;
	 }

	 struct tm utm;
	 struct tm ltm;
	 time_t time_epoch;

        if (*((int *) UTCOffsetN[0].aux0) == 0)
	{
		TimeTP.s = IPS_IDLE;
		IDSetText(&TimeTP, "You must set the UTC Offset property first.");
		return;
	}

	  if (extractISOTime(texts[0], &utm) < 0)
	  {
	    TimeTP.s = IPS_IDLE;
	    IDSetText(&TimeTP , "Time invalid");
	    return;
	  }

	 // update JD
         JD = UTtoJD(&utm);
	 IDLog("New JD is %f\n", (float) JD);

        // Get epoch since given UTC (we're assuming it's LOCAL for now, then we'll subtract UTC to get local)
	// Since mktime only returns epoch given a local calender time
	 time_epoch = mktime(&utm);

	 // Add UTC Offset to get local time. The offset is assumed to be DST corrected.
	time_epoch += (int) (UTCOffsetN[0].value * 60.0 * 60.0);

	// Now let's get the local time
	localtime_r(&time_epoch, &ltm);
		
	ltm.tm_mon +=1;
        ltm.tm_year += 1900;

	// Set Local Time
	if ( ( err = setLocalTime(fd, ltm.tm_hour, ltm.tm_min, ltm.tm_sec) < 0) )
	{
	          handleError(&TimeTP, err, "Setting local time");
        	  return;
	}

	// GPS needs UTC date?
	// TODO Test This!
	// Make it calender representation
	 utm.tm_mon  += 1;
	 utm.tm_year += 1900;

	if (!strcmp(dev, "LX200 GPS"))
	{
			if ( ( err = setCalenderDate(fd, utm.tm_mday, utm.tm_mon, utm.tm_year) < 0) )
	  		{
		  		handleError(&TimeTP, err, "Setting TimeT date.");
		  		return;
			}
	}
	else
	{
			if ( ( err = setCalenderDate(fd, ltm.tm_mday, ltm.tm_mon, ltm.tm_year) < 0) )
	  		{
		  		handleError(&TimeTP, err, "Setting local date.");
		  		return;
			}
	}
	
	// Everything Ok, save time value	
	if (IUUpdateText(&TimeTP, texts, names, n) < 0)
		return;

	TimeTP.s = IPS_OK;
 	IDSetText(&TimeTP , "Time updated to %s, updating planetary data...", texts[0]);

	// Also update telescope's sidereal time
	getSDTime(fd, &SDTimeN[0].value);
	IDSetNumber(&SDTimeNP, NULL);
	}
}


void LX200Generic::ISNewNumber (const char *dev, const char *name, double values[], char *names[], int n)
{
	int h =0, m =0, s=0, err;
	double newRA =0, newDEC =0;
	
	// ignore if not ours //
	if (strcmp (dev, thisDevice))
	    return;

        // Tracking Precision
        if (!strcmp (name, TrackingPrecisionNP.name))
	{
		if (!IUUpdateNumber(&TrackingPrecisionNP, values, names, n))
		{
			TrackingPrecisionNP.s = IPS_OK;
			IDSetNumber(&TrackingPrecisionNP, NULL);
			return;
		}
		
		TrackingPrecisionNP.s = IPS_ALERT;
		IDSetNumber(&TrackingPrecisionNP, "unknown error while setting tracking precision");
		return;
	}

	// Slew Precision
	if (!strcmp(name, SlewPrecisionNP.name))
	{
		IUUpdateNumber(&SlewPrecisionNP, values, names, n);
		{
			SlewPrecisionNP.s = IPS_OK;
			IDSetNumber(&SlewPrecisionNP, NULL);
			return;
		}
		
		SlewPrecisionNP.s = IPS_ALERT;
		IDSetNumber(&SlewPrecisionNP, "unknown error while setting slew precision");
		return;
	}

	// DST Correct TimeT Offset
	if (!strcmp (name, UTCOffsetNP.name))
	{
		if (strcmp(names[0], UTCOffsetN[0].name))
		{
			UTCOffsetNP.s = IPS_ALERT;
			IDSetNumber( &UTCOffsetNP , "Unknown element %s for property %s.", names[0], UTCOffsetNP.label);
			return;
		}

		if (!simulation)
			if ( ( err = setUTCOffset(fd, (values[0] * -1.0)) < 0) )
			{
		        	UTCOffsetNP.s = IPS_ALERT;
	        		IDSetNumber( &UTCOffsetNP , "Setting UTC Offset failed.");
				return;
			}
		
		*((int *) UTCOffsetN[0].aux0) = 1;
		IUUpdateNumber(&UTCOffsetNP, values, names, n);
		UTCOffsetNP.s = IPS_OK;
		IDSetNumber(&UTCOffsetNP, NULL);
		return;
	}

	if (!strcmp (name, EquatorialCoordsWNP.name))
	{
	  int i=0, nset=0;

	  if (checkPower(&EquatorialCoordsWNP))
	   return;

	    for (nset = i = 0; i < n; i++)
	    {
		INumber *eqp = IUFindNumber (&EquatorialCoordsWNP, names[i]);
		if (eqp == &EquatorialCoordsWN[0])
		{
                    newRA = values[i];
		    nset += newRA >= 0 && newRA <= 24.0;
		} else if (eqp == &EquatorialCoordsWN[1])
		{
		    newDEC = values[i];
		    nset += newDEC >= -90.0 && newDEC <= 90.0;
		}
	    }

	  if (nset == 2)
	  {
	   /*EquatorialCoordsWNP.s = IPS_BUSY;*/
	   char RAStr[32], DecStr[32];

	   fs_sexa(RAStr, newRA, 2, 3600);
	   fs_sexa(DecStr, newDEC, 2, 3600);
	  
           #ifdef INDI_DEBUG
	   IDLog("We received JNOW RA %g - DEC %g\n", newRA, newDEC);
	   IDLog("We received JNOW RA %s - DEC %s\n", RAStr, DecStr);
	   #endif

	  if (!simulation)
	   if ( (err = setObjectRA(fd, newRA)) < 0 || ( err = setObjectDEC(fd, newDEC)) < 0)
	   {
	     handleError(&EquatorialCoordsWNP, err, "Setting RA/DEC");
	     return;
	   } 
	   
           /*EquatorialCoordsWNP.s = IPS_BUSY;*/
	   targetRA  = newRA;
	   targetDEC = newDEC;
	   
	   if (MovementNSSP.s == IPS_BUSY || MovementWESP.s == IPS_BUSY)
	   {
	   	IUResetSwitch(&MovementNSSP);
		IUResetSwitch(&MovementWESP);
		MovementNSSP.s = MovementWESP.s = IPS_IDLE;
		IDSetSwitch(&MovementNSSP, NULL);
		IDSetSwitch(&MovementWESP, NULL);
	   }
	   
	   if (handleCoordSet())
	   {
	     EquatorialCoordsWNP.s = IPS_ALERT;
	     IDSetNumber(&EquatorialCoordsWNP, NULL);
	     
	   }
	} // end nset
	else
	{
		EquatorialCoordsWNP.s = IPS_ALERT;
		IDSetNumber(&EquatorialCoordsWNP, "RA or Dec missing or invalid");
	}

	    return;
     } /* end EquatorialCoordsWNP */

	// Update Sidereal Time
        if ( !strcmp (name, SDTimeNP.name) )
	{
	  if (checkPower(&SDTimeNP))
	   return;


	  if (values[0] < 0.0 || values[0] > 24.0)
	  {
	    SDTimeNP.s = IPS_IDLE;
	    IDSetNumber(&SDTimeNP , "Time invalid");
	    return;
	  }

	  getSexComponents(values[0], &h, &m, &s);
	  IDLog("Siderial Time is %02d:%02d:%02d\n", h, m, s);
	  
	  if ( ( err = setSDTime(fd, h, m, s) < 0) )
	  {
	    handleError(&SDTimeNP, err, "Setting siderial time"); 
            return;
	  }
	  
	  SDTimeNP.np[0].value = values[0];
	  SDTimeNP.s = IPS_OK;

	  IDSetNumber(&SDTimeNP , "Sidereal time updated to %02d:%02d:%02d", h, m, s);

	  return;
        }

	// Update Geographical Location
	if (!strcmp (name, geoNP.name))
	{
	    // new geographic coords
	    double newLong = 0, newLat = 0;
	    int i, nset;
	    char msg[128];

	  if (checkPower(&geoNP))
	   return;


	    for (nset = i = 0; i < n; i++)
	    {
		INumber *geop = IUFindNumber (&geoNP, names[i]);
		if (geop == &geo[0])
		{
		    newLat = values[i];
		    nset += newLat >= -90.0 && newLat <= 90.0;
		} else if (geop == &geo[1])
		{
		    newLong = values[i];
		    nset += newLong >= 0.0 && newLong < 360.0;
		}
	    }

	    if (nset == 2)
	    {
		char l[32], L[32];
		geoNP.s = IPS_OK;
		fs_sexa (l, newLat, 3, 3600);
		fs_sexa (L, newLong, 4, 3600);
		
		if (!simulation)
		if ( ( err = setSiteLongitude(fd, 360.0 - newLong) < 0) )
	        {
		   handleError(&geoNP, err, "Setting site coordinates");
		   return;
	         }
		
		setSiteLatitude(fd, newLat);
		geoNP.np[0].value = newLat;
		geoNP.np[1].value = newLong;
		snprintf (msg, sizeof(msg), "Site location updated to Lat %.32s - Long %.32s", l, L);
	    } else
	    {
		geoNP.s = IPS_IDLE;
		strcpy(msg, "Lat or Long missing or invalid");
	    }
	    IDSetNumber (&geoNP, "%s", msg);
	    return;
	}

	// Update Frequency
	if ( !strcmp (name, TrackingFreqNP.name) )
	{

	 if (checkPower(&TrackingFreqNP))
	  return;

	  IDLog("Trying to set track freq of: %f\n", values[0]);

	  if ( ( err = setTrackFreq(fd, values[0])) < 0) 
	  {
             handleError(&TrackingFreqNP, err, "Setting tracking frequency");
	     return;
	 }
	 
	 TrackingFreqNP.s = IPS_OK;
	 TrackingFreqNP.np[0].value = values[0];
	 IDSetNumber(&TrackingFreqNP, "Tracking frequency set to %04.1f", values[0]);
	 if (trackingMode != LX200_TRACK_MANUAL)
	 {
	      trackingMode = LX200_TRACK_MANUAL;
	      TrackModeS[0].s = ISS_OFF;
	      TrackModeS[1].s = ISS_OFF;
	      TrackModeS[2].s = ISS_ON;
	      TrackModeSP.s   = IPS_OK;
	      selectTrackingMode(fd, trackingMode);
	      IDSetSwitch(&TrackModeSP, NULL);
	 }
	 
	  return;
	}
	
	if (!strcmp(name, FocusTimerNP.name))
	{
	  if (checkPower(&FocusTimerNP))
	   return;
	   
	  // Don't update if busy
	  if (FocusTimerNP.s == IPS_BUSY)
	   return;
	   
	  IUUpdateNumber(&FocusTimerNP, values, names, n);
	  
	  FocusTimerNP.s = IPS_OK;
	  
	  IDSetNumber(&FocusTimerNP, NULL);
	  IDLog("Setting focus timer to %g\n", FocusTimerN[0].value);
	  
	  return;

	}

}

void LX200Generic::ISNewSwitch (const char *dev, const char *name, ISState *states, char *names[], int n)
{
	int index;
	int dd, mm, err;

	// suppress warning
	names = names;

	//IDLog("in new Switch with Device= %s and Property= %s and #%d items\n", dev, name,n);
	//IDLog("SolarSw name is %s\n", SolarSw.name);

	//IDLog("The device name is %s\n", dev);
	// ignore if not ours //
	if (strcmp (thisDevice, dev))
	    return;

	// FIRST Switch ALWAYS for power
	if (!strcmp (name, ConnectSP.name))
	{
	 if (IUUpdateSwitch(&ConnectSP, states, names, n) < 0) return;
   	 connectTelescope();
	 return;
	}

	// Coord set
	if (!strcmp(name, OnCoordSetSP.name))
	{
  	  if (checkPower(&OnCoordSetSP))
	   return;

	  if (IUUpdateSwitch(&OnCoordSetSP, states, names, n) < 0) return;
	  currentSet = getOnSwitch(&OnCoordSetSP);
	  OnCoordSetSP.s = IPS_OK;
	  IDSetSwitch(&OnCoordSetSP, NULL);
	}
	
	// Parking
	if (!strcmp(name, ParkSP.name))
	{
	  if (checkPower(&ParkSP))
	    return;
           
	   ParkSP.s = IPS_IDLE;
	   

	   if ( (err = getSDTime(fd, &SDTimeN[0].value)) < 0)
	   {
  	  	handleError(&ParkSP, err, "Get siderial time");
	  	return;
	   }
	   
	   if (AlignmentS[0].s == ISS_ON)
	   {
	     targetRA  = SDTimeN[0].value;
	     targetDEC = 0;
	     setObjectRA(fd, targetRA);
	     setObjectDEC(fd, targetDEC);
	   }
	   
	   else if (AlignmentS[1].s == ISS_ON)
	   {
	     targetRA  = calculateRA(SDTimeN[0].value);
	     targetDEC = calculateDec(geo[0].value, SDTimeN[0].value);
	     setObjectRA(fd, targetRA);
	     setObjectDEC(fd, targetDEC);
	     IDLog("Parking the scope in AltAz (0,0) which corresponds to (RA,DEC) of (%g,%g)\n", targetRA, targetDEC);
	     IDLog("Current Sidereal time is: %g\n", SDTimeN[0].value);
	     IDSetNumber(&SDTimeNP, NULL);
	   }
	   else
	   {
	     IDSetSwitch(&ParkSP, "You can only park the telescope in Polar or AltAz modes.");
	     return;
	   }
	   
	   IDSetNumber(&SDTimeNP, NULL);
	   
	   currentSet = LX200_PARK;
	   handleCoordSet();
	}
	  
	// Abort Slew
	if (!strcmp (name, AbortSlewSP.name))
	{
	  if (checkPower(&AbortSlewSP))
	  {
	    AbortSlewSP.s = IPS_IDLE;
	    IDSetSwitch(&AbortSlewSP, NULL);
	    return;
	  }
	  
	  IUResetSwitch(&AbortSlewSP);
	  if (abortSlew(fd) < 0)
	  {
		AbortSlewSP.s = IPS_ALERT;
		IDSetSwitch(&AbortSlewSP, NULL);
		return;
	  }

	    if (EquatorialCoordsWNP.s == IPS_BUSY)
	    {
		AbortSlewSP.s = IPS_OK;
		EquatorialCoordsWNP.s       = IPS_IDLE;
		EquatorialCoordsRNP.s       = IPS_IDLE;
		IDSetSwitch(&AbortSlewSP, "Slew aborted.");
		IDSetNumber(&EquatorialCoordsRNP, NULL);
		IDSetNumber(&EquatorialCoordsWNP, NULL);
            }
	    else if (MovementNSSP.s == IPS_BUSY || MovementWESP.s == IPS_BUSY)
	    {
		MovementNSSP.s  = MovementWESP.s =  IPS_IDLE; 
	
		AbortSlewSP.s = IPS_OK;		
		EquatorialCoordsRNP.s       = IPS_IDLE;
		EquatorialCoordsWNP.s       = IPS_IDLE;
		IUResetSwitch(&MovementNSSP);
		IUResetSwitch(&MovementWESP);
		IUResetSwitch(&AbortSlewSP);

		IDSetSwitch(&AbortSlewSP, "Slew aborted.");
		IDSetSwitch(&MovementNSSP, NULL);
		IDSetSwitch(&MovementWESP, NULL);
		IDSetNumber(&EquatorialCoordsRNP, NULL);
		IDSetNumber(&EquatorialCoordsWNP, NULL);
	    }
	    else
	    {
	        AbortSlewSP.s = IPS_OK;
	        IDSetSwitch(&AbortSlewSP, NULL);
	    }

	    return;
	}

	// Alignment
	if (!strcmp (name, AlignmentSw.name))
	{
	  if (checkPower(&AlignmentSw))
	   return;

	  if (IUUpdateSwitch(&AlignmentSw, states, names, n) < 0) return;
	  index = getOnSwitch(&AlignmentSw);

	  if ( ( err = setAlignmentMode(fd, index) < 0) )
	  {
	     handleError(&AlignmentSw, err, "Setting alignment");
             return;
	  }
	  
	  AlignmentSw.s = IPS_OK;
          IDSetSwitch (&AlignmentSw, NULL);
	  return;

	}

        // Sites
	if (!strcmp (name, SitesSP.name))
	{
	  if (checkPower(&SitesSP))
	   return;

	  if (IUUpdateSwitch(&SitesSP, states, names, n) < 0) return;
	  currentSiteNum = getOnSwitch(&SitesSP) + 1;
	  
	  if ( ( err = selectSite(fd, currentSiteNum) < 0) )
	  {
   	      handleError(&SitesSP, err, "Selecting sites");
	      return;
	  }

	  if ( ( err = getSiteLatitude(fd, &dd, &mm) < 0))
	  {
	      handleError(&SitesSP, err, "Selecting sites");
	      return;
	  }

	  if (dd > 0) geoNP.np[0].value = dd + mm / 60.0;
	  else geoNP.np[0].value = dd - mm / 60.0;
	  
	  if ( ( err = getSiteLongitude(fd, &dd, &mm) < 0))
	  {
	        handleError(&SitesSP, err, "Selecting sites");
		return;
	  }
	  
	  if (dd > 0) geoNP.np[1].value = 360.0 - (dd + mm / 60.0);
	  else geoNP.np[1].value = (dd - mm / 60.0) * -1.0;
	  
	  getSiteName(fd, SiteNameTP.tp[0].text, currentSiteNum);

	  IDLog("Selecting site %d\n", currentSiteNum);
	  
	  geoNP.s = SiteNameTP.s = SitesSP.s = IPS_OK;

	  IDSetNumber (&geoNP, NULL);
	  IDSetText   (&SiteNameTP, NULL);
          IDSetSwitch (&SitesSP, NULL);
	  return;
	}

	// Focus Motion
	if (!strcmp (name, FocusMotionSP.name))
	{
	  if (checkPower(&FocusMotionSP))
	   return;

	  // If mode is "halt"
	  if (FocusModeS[0].s == ISS_ON)
	  {
	    FocusMotionSP.s = IPS_IDLE;
	    IDSetSwitch(&FocusMotionSP, NULL);
	    return;
	  }
	  
	  if (IUUpdateSwitch(&FocusMotionSP, states, names, n) < 0) return;
	  index = getOnSwitch(&FocusMotionSP);
	  
	  if ( ( err = setFocuserMotion(fd, index) < 0) )
	  {
	     handleError(&FocusMotionSP, err, "Setting focuser speed");
             return;
	  }

	  FocusMotionSP.s = IPS_BUSY;
	  
	  // with a timer 
	  if (FocusTimerN[0].value > 0)  
	  {
	     FocusTimerNP.s  = IPS_BUSY;
	     IEAddTimer(50, LX200Generic::updateFocusTimer, this);
	  }
	  
	  IDSetSwitch(&FocusMotionSP, NULL);
	  return;
	}

	// Slew mode
	if (!strcmp (name, SlewModeSP.name))
	{
	  if (checkPower(&SlewModeSP))
	   return;

	  if (IUUpdateSwitch(&SlewModeSP, states, names, n) < 0) return;
	  index = getOnSwitch(&SlewModeSP);
	   
	  if ( ( err = setSlewMode(fd, index) < 0) )
	  {
              handleError(&SlewModeSP, err, "Setting slew mode");
              return;
	  }
	  
          SlewModeSP.s = IPS_OK;
	  IDSetSwitch(&SlewModeSP, NULL);
	  return;
	}

	// Movement (North/South)
	if (!strcmp (name, MovementNSSP.name))
	{
	  if (checkPower(&MovementNSSP))
	   return;

	 int last_move=-1;
         int current_move = -1;

	// -1 means all off previously
	 last_move = getOnSwitch(&MovementNSSP);

	 if (IUUpdateSwitch(&MovementNSSP, states, names, n) < 0)
		return;

	current_move = getOnSwitch(&MovementNSSP);

	// Previosuly active switch clicked again, so let's stop.
	if (current_move == last_move)
	{
		HaltMovement(fd, (current_move == 0) ? LX200_NORTH : LX200_SOUTH);
		IUResetSwitch(&MovementNSSP);
	    	MovementNSSP.s = IPS_IDLE;
	    	IDSetSwitch(&MovementNSSP, NULL);
		return;
	}

	#ifdef INDI_DEBUG
        IDLog("Current Move: %d - Previous Move: %d\n", current_move, last_move);
	#endif

	// 0 (North) or 1 (South)
	last_move      = current_move;

	// Correction for LX200 Driver: North 0 - South 3
	current_move = (current_move == 0) ? LX200_NORTH : LX200_SOUTH;

        if ( ( err = MoveTo(fd, current_move) < 0) )
	{
	        	 handleError(&MovementNSSP, err, "Setting motion direction");
 		 	return;
	}
	
	  MovementNSSP.s = IPS_BUSY;
	  IDSetSwitch(&MovementNSSP, "Moving toward %s", (current_move == LX200_NORTH) ? "North" : "South");
	  return;
	}

	// Movement (West/East)
	if (!strcmp (name, MovementWESP.name))
	{
	  if (checkPower(&MovementWESP))
	   return;

	 int last_move=-1;
         int current_move = -1;

	// -1 means all off previously
	 last_move = getOnSwitch(&MovementWESP);

	 if (IUUpdateSwitch(&MovementWESP, states, names, n) < 0)
		return;

	current_move = getOnSwitch(&MovementWESP);

	// Previosuly active switch clicked again, so let's stop.
	if (current_move == last_move)
	{
		HaltMovement(fd, (current_move ==0) ? LX200_WEST : LX200_EAST);
		IUResetSwitch(&MovementWESP);
	    	MovementWESP.s = IPS_IDLE;
	    	IDSetSwitch(&MovementWESP, NULL);
		return;
	}

	#ifdef INDI_DEBUG
        IDLog("Current Move: %d - Previous Move: %d\n", current_move, last_move);
	#endif

	// 0 (West) or 1 (East)
	last_move      = current_move;

	// Correction for LX200 Driver: West 1 - East 2
	current_move = (current_move == 0) ? LX200_WEST : LX200_EAST;

        if ( ( err = MoveTo(fd, current_move) < 0) )
	{
	        	 handleError(&MovementWESP, err, "Setting motion direction");
 		 	return;
	}
	
	  MovementWESP.s = IPS_BUSY;
	  IDSetSwitch(&MovementWESP, "Moving toward %s", (current_move == LX200_WEST) ? "West" : "East");
	  return;
	}

	// Tracking mode
	if (!strcmp (name, TrackModeSP.name))
	{
	  if (checkPower(&TrackModeSP))
	   return;

	  IUResetSwitch(&TrackModeSP);
	  IUUpdateSwitch(&TrackModeSP, states, names, n);
	  trackingMode = getOnSwitch(&TrackModeSP);
	  
	  if ( ( err = selectTrackingMode(fd, trackingMode) < 0) )
	  {
	         handleError(&TrackModeSP, err, "Setting tracking mode.");
		 return;
	  }
	  
          getTrackFreq(fd, &TrackFreqN[0].value);
	  TrackModeSP.s = IPS_OK;
	  IDSetNumber(&TrackingFreqNP, NULL);
	  IDSetSwitch(&TrackModeSP, NULL);
	  return;
	}

        // Focus speed
	if (!strcmp (name, FocusModeSP.name))
	{
	  if (checkPower(&FocusModeSP))
	   return;

	  IUResetSwitch(&FocusModeSP);
	  IUUpdateSwitch(&FocusModeSP, states, names, n);

	  index = getOnSwitch(&FocusModeSP);

	  /* disable timer and motion */
	  if (index == 0)
	  {
	    IUResetSwitch(&FocusMotionSP);
	    FocusMotionSP.s = IPS_IDLE;
	    FocusTimerNP.s  = IPS_IDLE;
	    IDSetSwitch(&FocusMotionSP, NULL);
	    IDSetNumber(&FocusTimerNP, NULL);
	  }
	    
	  setFocuserSpeedMode(fd, index);
	  FocusModeSP.s = IPS_OK;
	  IDSetSwitch(&FocusModeSP, NULL);
	  return;
	}


}

void LX200Generic::handleError(ISwitchVectorProperty *svp, int err, const char *msg)
{
  
  svp->s = IPS_ALERT;
  
  /* First check to see if the telescope is connected */
    if (check_lx200_connection(fd))
    {
      /* The telescope is off locally */
      ConnectS[0].s = ISS_OFF;
      ConnectS[1].s = ISS_ON;
      ConnectSP.s = IPS_BUSY;
      IDSetSwitch(&ConnectSP, "Telescope is not responding to commands, will retry in 10 seconds.");
      
      IDSetSwitch(svp, NULL);
      IEAddTimer(10000, retryConnection, &fd);
      return;
    }
    
   /* If the error is a time out, then the device doesn't support this property or busy*/
      if (err == -2)
      {
       svp->s = IPS_ALERT;
       IDSetSwitch(svp, "Device timed out. Current device may be busy or does not support %s. Will retry again.", msg);
      }
      else
    /* Changing property failed, user should retry. */
       IDSetSwitch( svp , "%s failed.", msg);
       
       fault = true;
}

void LX200Generic::handleError(INumberVectorProperty *nvp, int err, const char *msg)
{
  
  nvp->s = IPS_ALERT;
  
  /* First check to see if the telescope is connected */
    if (check_lx200_connection(fd))
    {
      /* The telescope is off locally */
      ConnectS[0].s = ISS_OFF;
      ConnectS[1].s = ISS_ON;
      ConnectSP.s = IPS_BUSY;
      IDSetSwitch(&ConnectSP, "Telescope is not responding to commands, will retry in 10 seconds.");
      
      IDSetNumber(nvp, NULL);
      IEAddTimer(10000, retryConnection, &fd);
      return;
    }
    
   /* If the error is a time out, then the device doesn't support this property */
      if (err == -2)
      {
       nvp->s = IPS_ALERT;
       IDSetNumber(nvp, "Device timed out. Current device may be busy or does not support %s. Will retry again.", msg);
      }
      else
    /* Changing property failed, user should retry. */
       IDSetNumber( nvp , "%s failed.", msg);
       
       fault = true;
}

void LX200Generic::handleError(ITextVectorProperty *tvp, int err, const char *msg)
{
  
  tvp->s = IPS_ALERT;
  
  /* First check to see if the telescope is connected */
    if (check_lx200_connection(fd))
    {
      /* The telescope is off locally */
      ConnectS[0].s = ISS_OFF;
      ConnectS[1].s = ISS_ON;
      ConnectSP.s = IPS_BUSY;
      IDSetSwitch(&ConnectSP, "Telescope is not responding to commands, will retry in 10 seconds.");
      
      IDSetText(tvp, NULL);
      IEAddTimer(10000, retryConnection, &fd);
      return;
    }
    
   /* If the error is a time out, then the device doesn't support this property */
      if (err == -2)
      {
       tvp->s = IPS_ALERT;
       IDSetText(tvp, "Device timed out. Current device may be busy or does not support %s. Will retry again.", msg);
      }
       
      else
    /* Changing property failed, user should retry. */
       IDSetText( tvp , "%s failed.", msg);
       
       fault = true;
}

 void LX200Generic::correctFault()
 {
 
   fault = false;
   IDMessage(thisDevice, "Telescope is online.");
   
 }

bool LX200Generic::isTelescopeOn(void)
{
  //if (simulation) return true;
  
  return (ConnectSP.sp[0].s == ISS_ON);
}

static void retryConnection(void * p)
{
  int fd = *( (int *) p);
  
  if (check_lx200_connection(fd))
  {
    ConnectSP.s = IPS_IDLE;
    IDSetSwitch(&ConnectSP, "The connection to the telescope is lost.");
    return;
  }
  
  ConnectS[0].s = ISS_ON;
  ConnectS[1].s = ISS_OFF;
  ConnectSP.s = IPS_OK;
   
  IDSetSwitch(&ConnectSP, "The connection to the telescope has been resumed.");

}

void LX200Generic::updateFocusTimer(void *p)
{
   int err=0;

    switch (FocusTimerNP.s)
    {

      case IPS_IDLE:
	   break;
	     
      case IPS_BUSY:
      IDLog("Focus Timer Value is %g\n", FocusTimerN[0].value);
	    FocusTimerN[0].value-=50;
	    
	    if (FocusTimerN[0].value <= 0)
	    {
	      IDLog("Focus Timer Expired\n");
	      if ( ( err = setFocuserSpeedMode(telescope->fd, 0) < 0) )
              {
	        telescope->handleError(&FocusModeSP, err, "setting focuser mode");
                IDLog("Error setting focuser mode\n");
                return;
	      } 
         
	      
	      FocusMotionSP.s = IPS_IDLE;
	      FocusTimerNP.s  = IPS_OK;
	      FocusModeSP.s   = IPS_OK;
	      
              IUResetSwitch(&FocusMotionSP);
              IUResetSwitch(&FocusModeSP);
	      FocusModeS[0].s = ISS_ON;
	      
	      IDSetSwitch(&FocusModeSP, NULL);
	      IDSetSwitch(&FocusMotionSP, NULL);
	    }
	    
         IDSetNumber(&FocusTimerNP, NULL);

	  if (FocusTimerN[0].value > 0)
		IEAddTimer(50, LX200Generic::updateFocusTimer, p);
	    break;
	    
       case IPS_OK:
	    break;
	    
	case IPS_ALERT:
	    break;
     }

}

void LX200Generic::ISPoll()
{
        double dx, dy;
	/*static int okCounter = 3;*/
	int err=0;
	
	if (!isTelescopeOn())
	 return;

	if (simulation)
	{
		mountSim();
		return;
        }

	switch (EquatorialCoordsWNP.s)
	{
	case IPS_IDLE:
        if ( fabs (currentRA - lastRA) > 0.01 || fabs (currentDEC - lastDEC) > 0.01)
	{
	        EquatorialCoordsRNP.np[0].value = lastRA = currentRA;
		EquatorialCoordsRNP.np[1].value = lastDEC = currentDEC;
		IDSetNumber (&EquatorialCoordsRNP, NULL);
	}
        break;

        case IPS_BUSY:
	    getLX200RA(fd, &currentRA);
	    getLX200DEC(fd, &currentDEC);
	    dx = targetRA - currentRA;
	    dy = targetDEC - currentDEC;

	    /*IDLog("targetRA is %g, currentRA is %g\n", targetRA, currentRA);
	    IDLog("targetDEC is %g, currentDEC is %g\n*************************\n", targetDEC, currentDEC);*/

	    EquatorialCoordsRNP.np[0].value = currentRA;
	    EquatorialCoordsRNP.np[1].value = currentDEC;

	    // Wait until acknowledged or within threshold
	    if ( fabs(dx) <= (SlewPrecisionN[0].value/(60.0*15.0)) && fabs(dy) <= (SlewPrecisionN[1].value/60.0))
	    {
	      /* Don't set current to target. This might leave residual cumulative error 
		currentRA = targetRA;
		currentDEC = targetDEC;
	      */
		
	       EquatorialCoordsRNP.np[0].value = lastRA  = currentRA;
	       EquatorialCoordsRNP.np[1].value = lastDEC = currentDEC;
	       //IUResetSwitch(&OnCoordSetSP);
	       //OnCoordSetSP.s = IPS_OK;
	       
	       EquatorialCoordsWNP.s = IPS_OK;
	       IDSetNumber(&EquatorialCoordsWNP, NULL);
	       

		switch (currentSet)
		{
		  case LX200_SLEW:
			EquatorialCoordsRNP.s = IPS_IDLE;
			IDSetNumber(&EquatorialCoordsRNP, "Slew is complete.");
		  	break;
		  
		  case LX200_TRACK:
			EquatorialCoordsRNP.s = IPS_OK;
			IDSetNumber(&EquatorialCoordsRNP, "Slew is complete. Tracking...");
			break;
		  
		  case LX200_SYNC:
		  	break;
		  
		  case LX200_PARK:
		        if (setSlewMode(fd, LX200_SLEW_GUIDE) < 0)
			{ 
			  handleError(&SlewModeSP, err, "Setting slew mode");
			  return;
			}
			
			IUResetSwitch(&SlewModeSP);
			SlewModeS[LX200_SLEW_GUIDE].s = ISS_ON;
			IDSetSwitch(&SlewModeSP, NULL);
			
			MoveTo(fd, LX200_EAST);
			IUResetSwitch(&MovementWESP);
			MovementWES[1].s = ISS_ON;
			MovementWESP.s = IPS_BUSY;
			IDSetSwitch(&MovementWESP, NULL);
			
			ParkSP.s = IPS_OK;
		  	IDSetSwitch (&ParkSP, "Park is complete. Turn off the telescope now.");
		  	break;
		}

	    } else
	    {
		EquatorialCoordsRNP.np[0].value = currentRA;
		EquatorialCoordsRNP.np[1].value = currentDEC;
		IDSetNumber (&EquatorialCoordsRNP, NULL);
	    }
	    break;

	case IPS_OK:
	  
	if ( (err = getLX200RA(fd, &currentRA)) < 0 || (err = getLX200DEC(fd, &currentDEC)) < 0)
	{
	  handleError(&EquatorialCoordsRNP, err, "Getting RA/DEC");
	  return;
	}
	
	if (fault)
	  correctFault();
	
	if ( (currentRA != lastRA) || (currentDEC != lastDEC))
	{
	  	EquatorialCoordsRNP.np[0].value = lastRA  = currentRA;
		EquatorialCoordsRNP.np[1].value = lastDEC = currentDEC;
		IDSetNumber (&EquatorialCoordsRNP, NULL);
	}
        break;


	case IPS_ALERT:
	    break;
	}

	switch (MovementNSSP.s)
	{
	  case IPS_IDLE:
	   break;
	 case IPS_BUSY:
	   getLX200RA(fd, &currentRA);
	   getLX200DEC(fd, &currentDEC);
	   EquatorialCoordsRNP.np[0].value = currentRA;
	   EquatorialCoordsRNP.np[1].value = currentDEC;

	   IDSetNumber (&EquatorialCoordsRNP, NULL);
	     break;
	 case IPS_OK:
	   break;
	 case IPS_ALERT:
	   break;
	 }

	 switch (MovementWESP.s)
	{
	  case IPS_IDLE:
	   break;
	 case IPS_BUSY:
	   getLX200RA(fd, &currentRA);
	   getLX200DEC(fd, &currentDEC);
	   EquatorialCoordsRNP.np[0].value = currentRA;
	   EquatorialCoordsRNP.np[1].value = currentDEC;

	   IDSetNumber (&EquatorialCoordsRNP, NULL);
	     break;
	 case IPS_OK:
	   break;
	 case IPS_ALERT:
	   break;
	 }

}

void LX200Generic::mountSim ()
{
	static struct timeval ltv;
	struct timeval tv;
	double dt, da, dx;
	int nlocked;

	/* update elapsed time since last poll, don't presume exactly POLLMS */
	gettimeofday (&tv, NULL);
	
	if (ltv.tv_sec == 0 && ltv.tv_usec == 0)
	    ltv = tv;
	    
	dt = tv.tv_sec - ltv.tv_sec + (tv.tv_usec - ltv.tv_usec)/1e6;
	ltv = tv;
	da = SLEWRATE*dt;

	/* Process per current state. We check the state of EQUATORIAL_COORDS and act acoordingly */
	switch (EquatorialCoordsWNP.s)
	{
	
	/* #1 State is idle, update telesocpe at sidereal rate */
	case IPS_IDLE:
	    /* RA moves at sidereal, Dec stands still */
	    currentRA += (SIDRATE*dt/15.);
	    
	   EquatorialCoordsRNP.np[0].value = currentRA;
	   EquatorialCoordsRNP.np[1].value = currentDEC;
	   IDSetNumber(&EquatorialCoordsRNP, NULL);

	    break;

	case IPS_BUSY:
	    /* slewing - nail it when both within one pulse @ SLEWRATE */
	    nlocked = 0;

	    dx = targetRA - currentRA;
	    
	    if (fabs(dx) <= da)
	    {
		currentRA = targetRA;
		nlocked++;
	    }
	    else if (dx > 0)
	    	currentRA += da/15.;
	    else 
	    	currentRA -= da/15.;
	    

	    dx = targetDEC - currentDEC;
	    if (fabs(dx) <= da)
	    {
		currentDEC = targetDEC;
		nlocked++;
	    }
	    else if (dx > 0)
	      currentDEC += da;
	    else
	      currentDEC -= da;

	   EquatorialCoordsRNP.np[0].value = currentRA;
	   EquatorialCoordsRNP.np[1].value = currentDEC;

	    if (nlocked == 2)
	    {
		EquatorialCoordsRNP.s = IPS_OK;
		EquatorialCoordsWNP.s = IPS_OK;
		IDSetNumber(&EquatorialCoordsWNP, "Now tracking");
		IDSetNumber(&EquatorialCoordsRNP, NULL);
	    } else
		IDSetNumber(&EquatorialCoordsRNP, NULL);

	    break;

	case IPS_OK:
	    /* tracking */
	   EquatorialCoordsRNP.np[0].value = currentRA;
	   EquatorialCoordsRNP.np[1].value = currentDEC;

	   IDSetNumber(&EquatorialCoordsRNP, NULL);
	    break;

	case IPS_ALERT:
	    break;
	}

}

void LX200Generic::getBasicData()
{

  int err;
  struct tm *timep;
  time_t ut;
  time (&ut);
  timep = gmtime (&ut);
  strftime (TimeTP.tp[0].text, strlen(TimeTP.tp[0].text), "%Y-%m-%dT%H:%M:%S", timep);

  IDLog("PC UTC time is %s\n", TimeTP.tp[0].text);

  getAlignment();
  
  checkLX200Format(fd);
  
  if ( (err = getTimeFormat(fd, &timeFormat)) < 0)
     IDMessage(thisDevice, "Failed to retrieve time format from device.");
  else
  {
    timeFormat = (timeFormat == 24) ? LX200_24 : LX200_AM;
    // We always do 24 hours
    if (timeFormat != LX200_24)
      err = toggleTimeFormat(fd);
  }

  getLX200RA(fd, &targetRA);
  getLX200DEC(fd, &targetDEC);

  EquatorialCoordsRNP.np[0].value = targetRA;
  EquatorialCoordsRNP.np[1].value = targetDEC;
  
  IDSetNumber (&EquatorialCoordsRNP, NULL);  
  
  SiteNameT[0].text = new char[64];
  
  if ( (err = getSiteName(fd, SiteNameT[0].text, currentSiteNum)) < 0)
    IDMessage(thisDevice, "Failed to get site name from device");
  else
    IDSetText   (&SiteNameTP, NULL);
  
  if ( (err = getTrackFreq(fd, &TrackFreqN[0].value)) < 0)
     IDMessage(thisDevice, "Failed to get tracking frequency from device.");
  else
     IDSetNumber (&TrackingFreqNP, NULL);
     
  /*updateLocation();
  updateTime();*/
  
}

int LX200Generic::handleCoordSet()
{

  int  err;
  char syncString[256];
  char RAStr[32], DecStr[32];
  double dx, dy;
  
  //IDLog("In Handle Coord Set()\n");

  switch (currentSet)
  {
    // Slew
    case LX200_SLEW:
          lastSet = LX200_SLEW;
	  if (EquatorialCoordsWNP.s == IPS_BUSY)
	  {
	     #ifdef INDI_DEBUG
	     IDLog("Aboring Slew\n");
	     #endif

	     abortSlew(fd);

	     // sleep for 100 mseconds
	     usleep(100000);
	  }

	if (!simulation)
	  if ((err = Slew(fd)))
	  {
	    slewError(err);
	    return (-1);
	  }

	  EquatorialCoordsWNP.s = IPS_BUSY;
	  EquatorialCoordsRNP.s = IPS_BUSY;
	  fs_sexa(RAStr, targetRA, 2, 3600);
	  fs_sexa(DecStr, targetDEC, 2, 3600);
	  IDSetNumber(&EquatorialCoordsWNP, "Slewing to JNow RA %s - DEC %s", RAStr, DecStr);
	  IDSetNumber(&EquatorialCoordsRNP, NULL);
	  #ifdef INDI_DEBUG
	  IDLog("Slewing to JNow RA %s - DEC %s\n", RAStr, DecStr);
          #endif
	  break;

     // Track
     case LX200_TRACK:
          if (EquatorialCoordsWNP.s == IPS_BUSY)
	  {
	     #ifdef INDI_DEBUG
	     IDLog("Aboring Slew\n");
	     #endif

	     abortSlew(fd);

	     // sleep for 200 mseconds
	     usleep(200000);
	  }

	  dx = fabs ( targetRA - currentRA );
	  dy = fabs (targetDEC - currentDEC);

	  
	  if (dx >= (TrackingPrecisionN[0].value/(60.0*15.0)) || (dy >= TrackingPrecisionN[1].value/60.0)) 
	  {
	        /*IDLog("Exceeded Tracking threshold, will attempt to slew to the new target.\n");
		IDLog("targetRA is %g, currentRA is %g\n", targetRA, currentRA);
	        IDLog("targetDEC is %g, currentDEC is %g\n*************************\n", targetDEC, currentDEC);*/

	      if (!simulation)
          	if ((err = Slew(fd)))
	  	{
	    		slewError(err);
	    		return (-1);
	  	}

		fs_sexa(RAStr, targetRA, 2, 3600);
	        fs_sexa(DecStr, targetDEC, 2, 3600);
		EquatorialCoordsWNP.s = IPS_BUSY;
		EquatorialCoordsRNP.s = IPS_BUSY;
		IDSetNumber(&EquatorialCoordsWNP, "Slewing to JNow RA %s - DEC %s", RAStr, DecStr);
		IDSetNumber(&EquatorialCoordsRNP, NULL);

		#ifdef INDI_DEBUG
		IDLog("Slewing to JNow RA %s - DEC %s\n", RAStr, DecStr);
		#endif
	  }
	  else
	  {
	    #ifdef INDI_DEBUG
	    IDLog("Tracking called, but tracking threshold not reached yet.\n");
	    #endif
	    
	    EquatorialCoordsWNP.s = IPS_OK;
	    EquatorialCoordsRNP.s = IPS_OK;
	    EquatorialCoordsRNP.np[0].value = currentRA;
	    EquatorialCoordsRNP.np[1].value = currentDEC;

	    if (lastSet != LX200_TRACK)
	    {
	      IDSetNumber(&EquatorialCoordsWNP, "Tracking...");
	      IDSetNumber(&EquatorialCoordsRNP, NULL);
	    }
	    else
	    {
	      IDSetNumber(&EquatorialCoordsWNP, NULL);
	      IDSetNumber(&EquatorialCoordsRNP, NULL);
	    }
	  }

	  lastSet = LX200_TRACK;
      break;

    // Sync
    case LX200_SYNC:
          lastSet = LX200_SYNC;
	   
	if (!simulation)
	  if ( ( err = Sync(fd, syncString) < 0) )
	  {
		EquatorialCoordsWNP.s = IPS_ALERT;
	        IDSetNumber(&EquatorialCoordsWNP , "Synchronization failed.");
		return (-1);
	  }

	  EquatorialCoordsWNP.s = IPS_OK;
	  EquatorialCoordsRNP.s = IPS_IDLE;
	  IDLog("Synchronization successful %s\n", syncString);
	  IDSetNumber(&EquatorialCoordsWNP, "Synchronization successful.");
	  IDSetNumber(&EquatorialCoordsRNP, NULL);
	  break;

   // PARK
   // Set RA to LST and DEC to 0 degrees, slew, then change to 'guide' slew after slew is complete.
   case LX200_PARK:
          if (EquatorialCoordsWNP.s == IPS_BUSY)
	  {
	     abortSlew(fd);

	     // sleep for 200 mseconds
	     usleep(200000);
	  }

	  if ((err = Slew(fd)))
	  {
	    	slewError(err);
	    	return (-1);
	  }
		
	  ParkSP.s = IPS_BUSY;
	  EquatorialCoordsRNP.s = IPS_BUSY;
	  IDSetNumber(&EquatorialCoordsRNP, NULL);
	  IDSetSwitch(&ParkSP, "The telescope is slewing to park position. Turn off the telescope after park is complete.");
	  
	  break;
	  
   }
   
   return (0);

}

int LX200Generic::getOnSwitch(ISwitchVectorProperty *sp)
{
 for (int i=0; i < sp->nsp ; i++)
     if (sp->sp[i].s == ISS_ON)
      return i;

 return -1;
}


int LX200Generic::checkPower(ISwitchVectorProperty *sp)
{
  if (simulation) return 0;
  
  if (ConnectSP.s != IPS_OK)
  {
    if (!strcmp(sp->label, ""))
    	IDMessage (thisDevice, "Cannot change property %s while the telescope is offline.", sp->name);
    else
        IDMessage (thisDevice, "Cannot change property %s while the telescope is offline.", sp->label);
	
    sp->s = IPS_IDLE;
    IDSetSwitch(sp, NULL);
    return -1;
  }

  return 0;
}

int LX200Generic::checkPower(INumberVectorProperty *np)
{
  if (simulation) return 0;
  
  if (ConnectSP.s != IPS_OK)
  {
    
    if (!strcmp(np->label, ""))
    	IDMessage (thisDevice, "Cannot change property %s while the telescope is offline.", np->name);
    else
        IDMessage (thisDevice, "Cannot change property %s while the telescope is offline.", np->label);
	
    np->s = IPS_IDLE;
    IDSetNumber(np, NULL);
    return -1;
  }

  return 0;

}

int LX200Generic::checkPower(ITextVectorProperty *tp)
{

  if (simulation) return 0;
  
  if (ConnectSP.s != IPS_OK)
  {
    if (!strcmp(tp->label, ""))
    	IDMessage (thisDevice, "Cannot change property %s while the telescope is offline.", tp->name);
    else
        IDMessage (thisDevice, "Cannot change property %s while the telescope is offline.", tp->label);
	
    tp->s = IPS_IDLE;
    IDSetText(tp, NULL);
    return -1;
  }

  return 0;

}

void LX200Generic::connectTelescope()
{
     switch (ConnectSP.sp[0].s)
     {
      case ISS_ON:  
	
        if (simulation)
	{
	  ConnectSP.s = IPS_OK;
	  IDSetSwitch (&ConnectSP, "Simulated telescope is online.");
	  //updateTime();
	  return;
	}
	
	 if (tty_connect(PortTP.tp[0].text, 9600, 8, 0, 1, &fd) != TTY_OK)
	 {
	   ConnectS[0].s = ISS_OFF;
	   ConnectS[1].s = ISS_ON;
	   IDSetSwitch (&ConnectSP, "Error connecting to port %s. Make sure you have BOTH write and read permission to your port.\n", PortTP.tp[0].text);
	   return;
	 }
	 if (check_lx200_connection(fd))
	 {   
	   ConnectS[0].s = ISS_OFF;
	   ConnectS[1].s = ISS_ON;
	   IDSetSwitch (&ConnectSP, "Error connecting to Telescope. Telescope is offline.");
	   return;
	 }

        #ifdef INDI_DEBUG
        IDLog("Telescope test successfful.\n");
	#endif

        *((int *) UTCOffsetN[0].aux0) = 0;
	ConnectSP.s = IPS_OK;
	IDSetSwitch (&ConnectSP, "Telescope is online. Retrieving basic data...");
	getBasicData();
	break;

     case ISS_OFF:
         ConnectS[0].s = ISS_OFF;
	 ConnectS[1].s = ISS_ON;
         ConnectSP.s = IPS_IDLE;
         IDSetSwitch (&ConnectSP, "Telescope is offline.");
	 IDLog("Telescope is offline.");
	 tty_disconnect(fd);
	 break;

    }

}

void LX200Generic::slewError(int slewCode)
{
    ParkSP.s = IPS_IDLE;
    EquatorialCoordsWNP.s = IPS_ALERT;
    IDSetSwitch(&ParkSP, NULL);

    if (slewCode == 1)
	IDSetNumber(&EquatorialCoordsWNP, "Object below horizon.");
    else if (slewCode == 2)
	IDSetNumber(&EquatorialCoordsWNP, "Object below the minimum elevation limit.");
    else
	IDSetNumber(&EquatorialCoordsWNP, "Slew failed.");

}

void LX200Generic::getAlignment()
{

   if (ConnectSP.s != IPS_OK)
    return;

   signed char align = ACK(fd);
   if (align < 0)
   {
     IDSetSwitch (&AlignmentSw, "Failed to get telescope alignment.");
     return;
   }

   AlignmentS[0].s = ISS_OFF;
   AlignmentS[1].s = ISS_OFF;
   AlignmentS[2].s = ISS_OFF;

    switch (align)
    {
      case 'P': AlignmentS[0].s = ISS_ON;
       		break;
      case 'A': AlignmentS[1].s = ISS_ON;
      		break;
      case 'L': AlignmentS[2].s = ISS_ON;
            	break;
    }

    AlignmentSw.s = IPS_OK;
    IDSetSwitch (&AlignmentSw, NULL);
    IDLog("ACK success %c\n", align);
}

void LX200Generic::enableSimulation(bool enable)
{
   simulation = enable;
   
   if (simulation)
     IDLog("Warning: Simulation is activated.\n");
   else
     IDLog("Simulation is disabled.\n");
}

void LX200Generic::updateTime()
{

  char cdate[32];
  double ctime;
  int h, m, s, lx200_utc_offset=0;
  int day, month, year, result;
  struct tm ltm;
  struct tm utm;
  time_t time_epoch;
  
  if (simulation)
  {
    sprintf(TimeT[0].text, "%d-%02d-%02dT%02d:%02d:%02d", 1979, 6, 25, 3, 30, 30);
    IDLog("Telescope ISO date and time: %s\n", TimeT[0].text);
    IDSetText(&TimeTP, NULL);
    return;
  }

  getUTCOffset(fd, &lx200_utc_offset);

  // LX200 TimeT Offset is defined at the number of hours added to LOCAL TIME to get TimeT. This is contrary to the normal definition.
  UTCOffsetN[0].value = lx200_utc_offset*-1;

  // We got a valid value for UTCOffset now
  *((int *) UTCOffsetN[0].aux0) = 1;  

  #ifdef INDI_DEBUG
  IDLog("Telescope TimeT Offset: %g\n", UTCOffsetN[0].value);
  #endif

  getLocalTime24(fd, &ctime);
  getSexComponents(ctime, &h, &m, &s);

  if ( (result = getSDTime(fd, &SDTimeN[0].value)) < 0)
    IDMessage(thisDevice, "Failed to retrieve siderial time from device.");
  
  getCalenderDate(fd, cdate);
  result = sscanf(cdate, "%d/%d/%d", &year, &month, &day);
  if (result != 3) return;

  // Let's fill in the local time
  ltm.tm_sec = s;
  ltm.tm_min = m;
  ltm.tm_hour = h;
  ltm.tm_mday = day;
  ltm.tm_mon = month - 1;
  ltm.tm_year = year - 1900;

  // Get time epoch
  time_epoch = mktime(&ltm);

  // Convert to TimeT
  time_epoch -= (int) (UTCOffsetN[0].value * 60.0 * 60.0);

  // Get UTC (we're using localtime_r, but since we shifted time_epoch above by UTCOffset, we should be getting the real UTC time)
  localtime_r(&time_epoch, &utm);

  /* Format it into ISO 8601 */
  strftime(cdate, 32, "%Y-%m-%dT%H:%M:%S", &utm);
  IUSaveText(&TimeT[0], cdate);
  
  #ifdef INDI_DEBUG
  IDLog("Telescope Local Time: %02d:%02d:%02d\n", h, m , s);
  IDLog("Telescope SD Time is: %g\n", SDTimeN[0].value);
  IDLog("Telescope UTC Time: %s\n", TimeT[0].text);
  #endif

  // Let's send everything to the client
  IDSetText(&TimeTP, NULL);
  IDSetNumber(&SDTimeNP, NULL);
  IDSetNumber(&UTCOffsetNP, NULL);

}

void LX200Generic::updateLocation()
{

 int dd = 0, mm = 0, err = 0;

 if (simulation)
	return;
 
 if ( (err = getSiteLatitude(fd, &dd, &mm)) < 0)
    IDMessage(thisDevice, "Failed to get site latitude from device.");
  else
  {
    if (dd > 0)
    	geoNP.np[0].value = dd + mm/60.0;
    else
        geoNP.np[0].value = dd - mm/60.0;
  
      IDLog("Autostar Latitude: %d:%d\n", dd, mm);
      IDLog("INDI Latitude: %g\n", geoNP.np[0].value);
  }
  
  if ( (err = getSiteLongitude(fd, &dd, &mm)) < 0)
    IDMessage(thisDevice, "Failed to get site longitude from device.");
  else
  {
    if (dd > 0) geoNP.np[1].value = 360.0 - (dd + mm/60.0);
    else geoNP.np[1].value = (dd - mm/60.0) * -1.0;
    
    IDLog("Autostar Longitude: %d:%d\n", dd, mm);
    IDLog("INDI Longitude: %g\n", geoNP.np[1].value);
  }
  
  IDSetNumber (&geoNP, NULL);

}

