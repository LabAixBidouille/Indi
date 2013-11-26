/*!
 * \file skywatcherAPIMount.h
 *
 * \author Roger James
 * \author Gerry Rozema
 * \author Jean-Luc Geehalel
 * \date 13th November 2013
 *
 * This file contains the definitions for a C++ implementatiom of a INDI telescope driver using the Skywatcher API.
 * It is based on work from three sources.
 * A C++ implementation of the API by Roger James.
 * The indi_eqmod driver by Jean-Luc Geehalel.
 * The synscanmount driver by Gerry Rozema.
 */

#ifndef SKYWATCHERAPIMOUNT_H
#define SKYWATCHERAPIMOUNT_H

#include "indibase/alignment/AlignmentSubsystem.h"

#include "skywatcherAPI.h"

class SkywatcherAPIMount : public SkywatcherAPI, public INDI::Telescope, public INDI::AlignmentSubsystemDriver
{
public:
    SkywatcherAPIMount();
    virtual ~SkywatcherAPIMount();

    //  overrides of base class virtual functions
    virtual bool ISNewNumber (const char *dev, const char *name, double values[], char *names[], int n);
    virtual bool ISNewSwitch (const char *dev, const char *name, ISState *states, char *names[], int n);
    virtual bool ISNewBLOB (const char *dev, const char *name, int sizes[], int blobsizes[], char *blobs[], char *formats[], char *names[], int n);
    virtual bool initProperties();
    virtual void ISGetProperties (const char *dev);
    virtual const char *getDefaultName();
    virtual bool Connect();
    virtual bool ReadScopeStatus();
    virtual bool Goto(double,double);
    virtual bool Park();
    virtual bool Abort();
    // For the time being stop the timer path being used
    virtual void TimerHit() {}

private:
    // Overrides for the pure virtual functions in SkyWatcherAPI
    int skywatcher_tty_read(int fd, char *buf, int nbytes, int timeout, int *nbytes_read);
    int skywatcher_tty_write(int fd, const char * buffer, int nbytes, int *nbytes_written);

    // Ovverides for the pure virtual functions in

    // Custom debug level
    unsigned int DBG_SCOPE;
};

#endif // SKYWATCHERAPIMOUNT_H
