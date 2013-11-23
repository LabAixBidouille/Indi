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
}

SkywatcherAPIMount::~SkywatcherAPIMount()
{
    //dtor
}

const char * SkywatcherAPIMount::getDefaultName()
{
    //DEBUG(INDI::Logger::DBG_SESSION, "SkywatcherAPIMount::getDefaultName\n");
    return "skywatcherAPIMount";
}

bool SkywatcherAPIMount::initProperties()
{
    IDLog("SkywatcherAPIMount::initProperties\n");

    INDI::Telescope::initProperties();

    addDebugControl();
    return true;
}
void SkywatcherAPIMount::ISGetProperties (const char *dev)
{
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
/*
    if(TrackState==SCOPE_PARKING)
    {
        //  ok, lets try read where we are
        //  and see if we have reached the park position
        memset(str,0,20);
        tty_write(PortFD,"Z",1, &bytesWritten);
        numread=tty_read(PortFD,str,10,2, &bytesRead);
        //DEBUG(INDI::Logger::DBG_SESSION, "PARK READ %s\n",str);
        if(strncmp((char *)str,"0000,4000",9)==0)
        {
            TrackState=SCOPE_PARKED;
            ParkSV.s=IPS_OK;
            IDSetSwitch(&ParkSV,NULL);
            IDMessage(getDeviceName(),"Telescope is Parked.");
        }

    }
*/
    // Update Axis Position
    if (!MCGetAxisPosition(AXIS1))
        return false;
    if (!MCGetAxisPosition(AXIS2))
        return false;


/*    ra=(double)n1/0x100000000*24.0;
    dec=(double)n2/0x100000000*360.0;
    NewRaDec(ra,dec);*/
    return true;
}

bool  SkywatcherAPIMount::Connect()
{
    DEBUG(INDI::Logger::DBG_SESSION, "SkywatcherAPIMount::Connect");

	if (!INDI::Telescope::Connect())
		return false;

    // Tell SkywatcherAPI about the serial port
    SetSerialPort(PortFD);

    DEBUG(INDI::Logger::DBG_SESSION, "SkywatcherAPIMount::Connect - Call MCInit");
	return MCInit();
}

bool SkywatcherAPIMount::Goto(double ra,double dec)
{
    DEBUG(INDI::Logger::DBG_SESSION, "SkywatcherAPIMount::Goto");
    char str[20];
    int n1,n2;
    int numread, bytesWritten, bytesRead;

    //  not fleshed in yet
    tty_write(PortFD,"Ka",2, &bytesWritten);  //  test for an echo
    tty_read(PortFD,str,2,2, &bytesRead);  //  Read 2 bytes of response
    if(str[1] != '#') {
        //  this is not a correct echo
        //  so we are not talking to a mount properly
        return false;
    }
    //  Ok, mount is alive and well
    //  so, lets format up a goto command
    n1=ra*0x1000000/24;
    n2=dec*0x1000000/360;
    n1=n1<<8;
    n2=n2<<8;
    sprintf((char *)str,"r%08X,%08X",n1,n2);
    tty_write(PortFD,str,18, &bytesWritten);
    TrackState=SCOPE_SLEWING;
    numread=tty_read(PortFD,str,1,60, &bytesRead);
    if (bytesRead!=1||str[0]!='#')
    {
        if (isDebug())
            DEBUG(INDI::Logger::DBG_SESSION, "Timeout waiting for scope to complete slewing.");
        return false;
    }

    return true;
}

bool SkywatcherAPIMount::Park()
{
    DEBUG(INDI::Logger::DBG_SESSION, "SkywatcherAPIMount::Park");
    char str[20];
    int numread, bytesWritten, bytesRead;


    memset(str,0,3);
    tty_write(PortFD,"Ka",2, &bytesWritten);  //  test for an echo
    tty_read(PortFD,str,2,2, &bytesRead);  //  Read 2 bytes of response
    if(str[1] != '#')
    {
        //  this is not a correct echo
        //  so we are not talking to a mount properly
        return false;
    }
    //  Now we stop tracking
    tty_write(PortFD,"T0",2, &bytesWritten);
    numread=tty_read(PortFD,str,1,60, &bytesRead);
    if (bytesRead!=1||str[0]!='#')
    {
        if (isDebug())
            DEBUG(INDI::Logger::DBG_SESSION, "Timeout waiting for scope to stop tracking.");
        return false;
    }

    //sprintf((char *)str,"b%08X,%08X",0x0,0x40000000);
    tty_write(PortFD,"B0000,4000",10, &bytesWritten);
    numread=tty_read(PortFD,str,1,60, &bytesRead);
    if (bytesRead!=1||str[0]!='#')
    {
        if (isDebug())
            DEBUG(INDI::Logger::DBG_SESSION, "Timeout waiting for scope to respond to park.");
        return false;
    }

    TrackState=SCOPE_PARKING;
    IDMessage(getDeviceName(),"Parking Telescope...");
    return true;
}

bool  SkywatcherAPIMount::Abort()
{
    DEBUG(INDI::Logger::DBG_SESSION, "SkywatcherAPIMount::Abort");
    char str[20];
    int bytesWritten, bytesRead;

    // Hmmm twice only stops it
    tty_write(PortFD,"M",1, &bytesWritten);
    tty_read(PortFD,str,1,1, &bytesRead);  //  Read 1 bytes of response


    tty_write(PortFD,"M",1, &bytesWritten);
    tty_read(PortFD,str,1,1, &bytesRead);  //  Read 1 bytes of response

    return true;
}

int SkywatcherAPIMount::skywatcher_tty_read(int fd, char *buf, int nbytes, int timeout, int *nbytes_read)
{
    return tty_read(fd, buf, nbytes, timeout, nbytes_read);
}

int SkywatcherAPIMount::skywatcher_tty_write(int fd, const char * buffer, int nbytes, int *nbytes_written)
{
    return tty_write(fd, buffer, nbytes, nbytes_written);
}


