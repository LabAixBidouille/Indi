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

#include "indibase/inditelescope.h"

#include "skywatcherAPI.h"
#include <cmath>

class SkywatcherAPIMount : public INDI::Telescope, public SkywatcherAPI
{
public:
    SkywatcherAPIMount();
    virtual ~SkywatcherAPIMount();

    //  overrides of base class virtual functions
    virtual bool initProperties();
    virtual void ISGetProperties (const char *dev);
    virtual const char *getDefaultName();
    virtual bool Connect();
    virtual bool ReadScopeStatus();
    virtual bool Goto(double,double);
    virtual bool Park();
    virtual bool Abort();

private:
    // Overrides for the pure virtual functions in SkyWatcherAPI
    int skywatcher_tty_read(int fd, char *buf, int nbytes, int timeout, int *nbytes_read);
    int skywatcher_tty_write(int fd, const char * buffer, int nbytes, int *nbytes_written);

    // Custom debug level
    unsigned int DBG_SCOPE;
};

#endif // SKYWATCHERAPIMOUNT_H
