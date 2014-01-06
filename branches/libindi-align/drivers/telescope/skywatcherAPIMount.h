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

#include "indibase/alignment/AlignmentSubsystemForDrivers.h"

#include "skywatcherAPI.h"


class SkywatcherAPIMount : public SkywatcherAPI, public INDI::Telescope, public INDI::AlignmentSubsystem::AlignmentSubsystemForDrivers
{
public:
    SkywatcherAPIMount();
    virtual ~SkywatcherAPIMount();

    //  overrides of base class virtual functions
    virtual bool ISNewText (const char *dev, const char *name, char *texts[], char *names[], int n);
    virtual bool ISNewNumber (const char *dev, const char *name, double values[], char *names[], int n);
    virtual bool ISNewSwitch (const char *dev, const char *name, ISState *states, char *names[], int n);
    virtual bool ISNewBLOB (const char *dev, const char *name, int sizes[], int blobsizes[], char *blobs[], char *formats[], char *names[], int n);
    virtual bool initProperties();
    virtual void ISGetProperties (const char *dev);
    virtual bool updateProperties();
    virtual const char *getDefaultName();
    virtual bool Connect();
    virtual bool ReadScopeStatus();
    virtual bool Goto(double,double);
    virtual bool Park();
    virtual bool Abort();
    virtual bool saveConfigItems(FILE *fp);
    // For the time being stop the timer path being used
    virtual void TimerHit() {}

private:
    // Overrides for the pure virtual functions in SkyWatcherAPI
    int skywatcher_tty_read(int fd, char *buf, int nbytes, int timeout, int *nbytes_read);
    int skywatcher_tty_write(int fd, const char * buffer, int nbytes, int *nbytes_written);

    void UpdateDetailedMountInformation(bool InformClient);

    // Custom debug level
    unsigned int DBG_SCOPE;

    // Properties

    static const char* DetailedMountInfoPage;
    enum { MOTOR_CONTROL_FIRMWARE_VERSION, MOUNT_CODE, IS_DC_MOTOR };
    INumber BasicMountInfo[3];
    INumberVectorProperty BasicMountInfoV;
    enum { MT_EQ6, MT_HEQ5, MT_EQ5, MT_EQ3, MT_GT, MT_MF, MT_114GT, MT_DOB, MT_UNKNOWN };
    ISwitch MountType[9];
    ISwitchVectorProperty MountTypeV;
    enum { GEAR_RATIO, STEP_TIMER_FREQUENCY, HIGH_SPEED_RATIO, PE_PERIOD };
    INumber AxisOneInfo[4];
    INumberVectorProperty AxisOneInfoV;
    INumber AxisTwoInfo[4];
    INumberVectorProperty AxisTwoInfoV;
    enum { FULL_STOP, SLEWING, SLEWING_TO, SLEWING_FORWARD, HIGH_SPEED, NOT_INITIALISED };
    ISwitch AxisOneState[6];
    ISwitchVectorProperty AxisOneStateV;
    ISwitch AxisTwoState[6];
    ISwitchVectorProperty AxisTwoStateV;
    INumber EncoderValues[2];
    INumberVectorProperty EncoderValuesV;
};

#endif // SKYWATCHERAPIMOUNT_H
