/*!
 * \file skywatcherAPIMount.cpp
 *
 * \author Roger James
 * \author Gerry Rozema
 * \author Jean-Luc Geehalel
 * \date 13th November 2013
 *
 * This file contains the implementation in C++ of a INDI telescope driver using the Skywatcher API.
 * It is based on work from three sources.
 * A C++ implementation of the API by Roger James.
 * The indi_eqmod driver by Jean-Luc Geehalel.
 * The synscanmount driver by Gerry Rozema.
 */

#include "skywatcherAPIMount.h"
#include "libs/indicom.h"

#include <memory>
#include <cmath>

// We declare an auto pointer to SkywatcherAPIMount.
std::auto_ptr<SkywatcherAPIMount> SkywatcherAPIMountPtr(0);

void ISPoll(void *p);

void ISInit()
{
   static int isInit =0;

   if (isInit == 1)
       return;

    isInit = 1;
    if(SkywatcherAPIMountPtr.get() == 0) SkywatcherAPIMountPtr.reset(new SkywatcherAPIMount());

}

void ISGetProperties(const char *dev)
{
    ISInit();
    SkywatcherAPIMountPtr->ISGetProperties(dev);
}

void ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int num)
{
    ISInit();
    SkywatcherAPIMountPtr->ISNewSwitch(dev, name, states, names, num);
}

void ISNewText(	const char *dev, const char *name, char *texts[], char *names[], int num)
{
    ISInit();
    SkywatcherAPIMountPtr->ISNewText(dev, name, texts, names, num);
}

void ISNewNumber(const char *dev, const char *name, double values[], char *names[], int num)
{
    ISInit();
    SkywatcherAPIMountPtr->ISNewNumber(dev, name, values, names, num);
}

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


SkywatcherAPIMount::SkywatcherAPIMount()
{
    // We add an additional debug level so we can log verbose scope status
    DBG_SCOPE = INDI::Logger::getInstance().addDebugLevel("Scope Verbose", "SCOPE");
    // Set up the logging pointer in SkyWatcherAPI
    pChildTelescope = this;
}

SkywatcherAPIMount::~SkywatcherAPIMount()
{
    //dtor
}

bool SkywatcherAPIMount::ISNewNumber (const char *dev, const char *name, double values[], char *names[], int n)
{
    if(strcmp(dev,getDeviceName())==0)
    {
        // It is for us
        ProcessAlignmentNumberProperties(this, name, values, names, n);
    }
    // Pass it up the chain
    return INDI::Telescope::ISNewNumber(dev,name,values,names,n);
}

bool SkywatcherAPIMount::ISNewSwitch (const char *dev, const char *name, ISState *states, char *names[], int n)
{
    if(strcmp(dev,getDeviceName())==0)
    {
        // It is for us
        ProcessAlignmentSwitchProperties(this, name, states, names, n);
    }
    // Pass it up the chain
    return INDI::Telescope::ISNewSwitch(dev, name, states, names, n);
}

bool SkywatcherAPIMount::ISNewBLOB (const char *dev, const char *name, int sizes[], int blobsizes[], char *blobs[], char *formats[], char *names[], int n)
{
    if(strcmp(dev,getDeviceName())==0)
    {
        // It is for us
        ProcessAlignmentBlobProperties(this, name, sizes, blobsizes, blobs, formats, names, n);
    }
    // Pass it up the chain
    return INDI::Telescope::ISNewBLOB(dev, name, sizes, blobsizes, blobs, formats, names, n);
}

const char * SkywatcherAPIMount::getDefaultName()
{
    //DEBUG(INDI::Logger::DBG_SESSION, "SkywatcherAPIMount::getDefaultName\n");
    return "skywatcherAPIMount";
}

bool SkywatcherAPIMount::initProperties()
{
    IDLog("SkywatcherAPIMount::initProperties\n");

    // Allow the base class to initialise its visible before connection properties
    INDI::Telescope::initProperties();

    // Add default properties
    addDebugControl();
    addConfigurationControl();

    // Add alignment properties
    InitAlignmentProperties(this);

    // Add my visible before connection properties if any


    return true;
}
void SkywatcherAPIMount::ISGetProperties (const char *dev)
{
    // This iverride is currently here for debugging purposes only
    IDLog("SkywatcherAPIMount::ISGetProperties\n");
    INDI::Telescope::ISGetProperties(dev);
    return;
}

bool SkywatcherAPIMount::ReadScopeStatus()
{
    DEBUG(INDI::Logger::DBG_SESSION, "SkywatcherAPIMount::ReadScopeStatus");

    // Horrible hack to get over the fact that the base class calls ReadScopeStatus from inside Connect
    // before I have a chanced to set up tth serial port
    SetSerialPort(PortFD);

    // leave the following stuff in for the time being it is mostly harmless

    // Quick check of the mount
    if (!InquireMotorBoardVersion(AXIS1))
        return false;

    if (!MCGetAxisStatus(AXIS1))
        return false;

    if (!MCGetAxisStatus(AXIS2))
        return false;

    if(TrackState==SCOPE_SLEWING)
    {
        if ((AxesStatus[AXIS1].FullStop) && (AxesStatus[AXIS2].FullStop))
            TrackState = SCOPE_IDLE;
    }

    // Update Axis Position
    if (!MCGetAxisPosition(AXIS1))
        return false;
    if (!MCGetAxisPosition(AXIS2))
        return false;

    return true;
}

bool  SkywatcherAPIMount::Connect()
{
    DEBUG(INDI::Logger::DBG_SESSION, "SkywatcherAPIMount::Connect");

	if (!INDI::Telescope::Connect())
		return false;

    // Tell SkywatcherAPI about the serial port
    //SetSerialPort(PortFD); Hacked in ReadScopeStatus

    DEBUG(INDI::Logger::DBG_SESSION, "SkywatcherAPIMount::Connect - Call MCInit");
	return MCInit();
}

bool SkywatcherAPIMount::Goto(double ra,double dec)
{
    DEBUG(INDI::Logger::DBG_SESSION, "SkywatcherAPIMount::Goto");
    return false;
}

bool SkywatcherAPIMount::Park()
{
    DEBUG(INDI::Logger::DBG_SESSION, "SkywatcherAPIMount::Park");
    return false;
}

bool  SkywatcherAPIMount::Abort()
{
    DEBUG(INDI::Logger::DBG_SESSION, "SkywatcherAPIMount::Abort");
    return false;
}

int SkywatcherAPIMount::skywatcher_tty_read(int fd, char *buf, int nbytes, int timeout, int *nbytes_read)
{
    return tty_read(fd, buf, nbytes, timeout, nbytes_read);
}

int SkywatcherAPIMount::skywatcher_tty_write(int fd, const char * buffer, int nbytes, int *nbytes_written)
{
    return tty_write(fd, buffer, nbytes, nbytes_written);
}
