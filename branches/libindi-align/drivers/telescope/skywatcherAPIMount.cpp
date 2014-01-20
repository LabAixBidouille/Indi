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
#include <limits>

using namespace INDI::AlignmentSubsystem;

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
    ISInit();
    SkywatcherAPIMountPtr->ISNewBLOB (dev, name, sizes, blobsizes, blobs, formats, names, n);
}

void ISSnoopDevice (XMLEle *root)
{
    INDI_UNUSED(root);
}

const char * SkywatcherAPIMount::DetailedMountInfoPage = "Detailed Mount Information";

// Constructor

SkywatcherAPIMount::SkywatcherAPIMount()
{
    // Set up the logging pointer in SkyWatcherAPI
    pChildTelescope = this;
    PreviousNSMotion = PREVIOUS_NS_MOTION_UNKNOWN;
    PreviousWEMotion = PREVIOUS_WE_MOTION_UNKNOWN;
}

// destructor

SkywatcherAPIMount::~SkywatcherAPIMount()
{
}

// Public methods

bool  SkywatcherAPIMount::Abort()
{
    DEBUG(DBG_SCOPE, "SkywatcherAPIMount::Abort");
    return false;
}

bool SkywatcherAPIMount::canSync()
{
    return true;
}

bool  SkywatcherAPIMount::Connect()
{
    DEBUG(DBG_SCOPE, "SkywatcherAPIMount::Connect");

	if (!INDI::Telescope::Connect())
		return false;

    // Tell SkywatcherAPI about the serial port
    //SetSerialPort(PortFD); Hacked in ReadScopeStatus

    DEBUG(DBG_SCOPE, "SkywatcherAPIMount::Connect - Call MCInit");
	return InitMount();
}

const char * SkywatcherAPIMount::getDefaultName()
{
    //DEBUG(DBG_SCOPE, "SkywatcherAPIMount::getDefaultName\n");
    return "skywatcherAPIMount";
}

bool SkywatcherAPIMount::Goto(double ra,double dec)
{
    DEBUG(DBG_SCOPE, "SkywatcherAPIMount::Goto");

    SlewTo(AXIS2, DegreesToMicrosteps(AXIS1, 30));


    return true;
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
    InitProperties(this);

    // Set up property variables
    IUFillNumber(&BasicMountInfo[MOTOR_CONTROL_FIRMWARE_VERSION], "MOTOR_CONTROL_FIRMWARE_VERSION",
                                                            "Motor control fimware version",
                                                            "%g",
                                                            0,
                                                            0xFFFFFF,
                                                            1,
                                                            0);
    IUFillNumber(&BasicMountInfo[MOUNT_CODE], "MOUNT_CODE",
                                                            "Mount code",
                                                            "%g",
                                                            0,
                                                            0xFF,
                                                            1,
                                                            0);
    IUFillNumber(&BasicMountInfo[IS_DC_MOTOR], "IS_DC_MOTOR",
                                                            "Is DC motor (boolean)",
                                                            "%g",
                                                            0,
                                                            1,
                                                            1,
                                                            0);
    IUFillNumberVector(&BasicMountInfoV, BasicMountInfo, 3, getDeviceName(), "BASIC_MOUNT_INFO", "Basic mount information",
                        DetailedMountInfoPage, IP_RO, 60, IPS_IDLE);

    IUFillSwitch(&MountType[MT_EQ6], "EQ6", "EQ6", ISS_OFF);
    IUFillSwitch(&MountType[MT_HEQ5], "HEQ5", "HEQ5", ISS_OFF);
    IUFillSwitch(&MountType[MT_EQ5], "EQ5", "EQ5", ISS_OFF);
    IUFillSwitch(&MountType[MT_EQ3], "EQ3", "EQ3", ISS_OFF);
    IUFillSwitch(&MountType[MT_GT], "GT", "GT", ISS_OFF);
    IUFillSwitch(&MountType[MT_MF], "MF", "MF", ISS_OFF);
    IUFillSwitch(&MountType[MT_114GT], "114GT", "114GT", ISS_OFF);
    IUFillSwitch(&MountType[MT_DOB], "DOB", "DOB", ISS_OFF);
    IUFillSwitch(&MountType[MT_UNKNOWN], "UNKNOWN", "UNKNOWN", ISS_ON);
    IUFillSwitchVector(&MountTypeV, MountType, 9, getDeviceName(), "MOUNT_TYPE", "Mount type", DetailedMountInfoPage, IP_RO,
                        ISR_ATMOST1, 60, IPS_IDLE);

    IUFillNumber(&AxisOneInfo[MICROSTEPS_PER_REVOLUTION], "MICROSTEPS_PER_REVOLUTION",
                                                            "Microsteps per revolution",
                                                            "%.0f",
                                                            0,
                                                            0xFFFFFF,
                                                            1,
                                                            0);
    IUFillNumber(&AxisOneInfo[STEPPER_CLOCK_FREQUENCY], "STEPPER_CLOCK_FREQUENCY",
                                                            "Stepper clock frequency",
                                                            "%.0f",
                                                            0,
                                                            0xFFFFFF,
                                                            1,
                                                            0);
    IUFillNumber(&AxisOneInfo[HIGH_SPEED_RATIO], "HIGH_SPEED_RATIO",
                                                            "High speed ratio",
                                                            "%.0f",
                                                            0,
                                                            0xFFFFFF,
                                                            1,
                                                            0);
    IUFillNumber(&AxisOneInfo[MICROSTEPS_PER_WORM_REVOLUTION], "MICROSTEPS_PER_WORM_REVOLUTION",
                                                            "Microsteps per worm revolution",
                                                            "%.0f",
                                                            0,
                                                            0xFFFFFF,
                                                            1,
                                                            0);

    IUFillNumberVector(&AxisOneInfoV, AxisOneInfo, 4, getDeviceName(), "AXIS_ONE_INFO", "Axis one information",
                        DetailedMountInfoPage, IP_RO, 60, IPS_IDLE);

    IUFillSwitch(&AxisOneState[FULL_STOP], "FULL_STOP", "FULL_STOP", ISS_OFF);
    IUFillSwitch(&AxisOneState[SLEWING], "SLEWING", "SLEWING", ISS_OFF);
    IUFillSwitch(&AxisOneState[SLEWING_TO], "SLEWING_TO", "SLEWING_TO", ISS_OFF);
    IUFillSwitch(&AxisOneState[SLEWING_FORWARD], "SLEWING_FORWARD", "SLEWING_FORWARD", ISS_OFF);
    IUFillSwitch(&AxisOneState[HIGH_SPEED], "HIGH_SPEED", "HIGH_SPEED", ISS_OFF);
    IUFillSwitch(&AxisOneState[NOT_INITIALISED], "NOT_INITIALISED", "NOT_INITIALISED", ISS_ON);
    IUFillSwitchVector(&AxisOneStateV, AxisOneState, 6, getDeviceName(), "AXIS_ONE_STATE", "Axis one state", DetailedMountInfoPage, IP_RO,
                        ISR_NOFMANY, 60, IPS_IDLE);

    IUFillNumber(&AxisTwoInfo[MICROSTEPS_PER_REVOLUTION], "MICROSTEPS_PER_REVOLUTION",
                                                            "Microsteps per revolution",
                                                            "%.0f",
                                                            0,
                                                            0xFFFFFF,
                                                            1,
                                                            0);
    IUFillNumber(&AxisTwoInfo[STEPPER_CLOCK_FREQUENCY], "STEPPER_CLOCK_FREQUENCY",
                                                            "Step timer frequency",
                                                            "%.0f",
                                                            0,
                                                            0xFFFFFF,
                                                            1,
                                                            0);
    IUFillNumber(&AxisTwoInfo[HIGH_SPEED_RATIO], "HIGH_SPEED_RATIO",
                                                            "High speed ratio",
                                                            "%.0f",
                                                            0,
                                                            0xFFFFFF,
                                                            1,
                                                            0);
    IUFillNumber(&AxisTwoInfo[MICROSTEPS_PER_WORM_REVOLUTION], "MICROSTEPS_PER_WORM_REVOLUTION",
                                                            "Mictosteps per worm revolution",
                                                            "%.0f",
                                                            0,
                                                            0xFFFFFF,
                                                            1,
                                                            0);

    IUFillNumberVector(&AxisTwoInfoV, AxisTwoInfo, 4, getDeviceName(), "AXIS_TWO_INFO", "Axis two information",
                        DetailedMountInfoPage, IP_RO, 60, IPS_IDLE);

    IUFillSwitch(&AxisTwoState[FULL_STOP], "FULL_STOP", "FULL_STOP", ISS_OFF);
    IUFillSwitch(&AxisTwoState[SLEWING], "SLEWING", "SLEWING", ISS_OFF);
    IUFillSwitch(&AxisTwoState[SLEWING_TO], "SLEWING_TO", "SLEWING_TO", ISS_OFF);
    IUFillSwitch(&AxisTwoState[SLEWING_FORWARD], "SLEWING_FORWARD", "SLEWING_FORWARD", ISS_OFF);
    IUFillSwitch(&AxisTwoState[HIGH_SPEED], "HIGH_SPEED", "HIGH_SPEED", ISS_OFF);
    IUFillSwitch(&AxisTwoState[NOT_INITIALISED], "NOT_INITIALISED", "NOT_INITIALISED", ISS_ON);
    IUFillSwitchVector(&AxisTwoStateV, AxisTwoState, 6, getDeviceName(), "AXIS_TWO_STATE", "Axis two state", DetailedMountInfoPage, IP_RO,
                        ISR_NOFMANY, 60, IPS_IDLE);

    IUFillNumber(&EncoderValues[0], "AXIS_ONE",
                                                            "Axis One",
                                                            "%.0f",
                                                            0,
                                                            0xFFFFFF,
                                                            1,
                                                            0);
    IUFillNumber(&EncoderValues[1], "AXIS_TWO",
                                                            "Axis Two",
                                                            "%.0f",
                                                            0,
                                                            0xFFFFFF,
                                                            1,
                                                            0);

    IUFillNumberVector(&EncoderValuesV, EncoderValues, 2, getDeviceName(), "ENCODER_VALUES", "Encoder values",
                        DetailedMountInfoPage, IP_RO, 60, IPS_IDLE);
    // Register any visible before connection properties

    return true;
}

void SkywatcherAPIMount::ISGetProperties (const char *dev)
{
    IDLog("SkywatcherAPIMount::ISGetProperties\n");
    INDI::Telescope::ISGetProperties(dev);

    if (isConnected())
    {
        // Fill in any real values now available MCInit should have been called already
        UpdateDetailedMountInformation(false);

        // Define our connected only properties to the base driver
        // e.g. defineNumber(MyNumberVectorPointer);
        // This will register our properties and send a IDDefXXXX mewssage to any connected clients
        defineNumber(&BasicMountInfoV);
        defineSwitch(&MountTypeV);
        defineNumber(&AxisOneInfoV);
        defineSwitch(&AxisOneStateV);
        defineNumber(&AxisTwoInfoV);
        defineSwitch(&AxisTwoStateV);
        defineNumber(&EncoderValuesV);
    }
    return;
}

bool SkywatcherAPIMount::ISNewBLOB (const char *dev, const char *name, int sizes[], int blobsizes[], char *blobs[], char *formats[], char *names[], int n)
{
    if(strcmp(dev,getDeviceName())==0)
    {
        // It is for us
        ProcessBlobProperties(this, name, sizes, blobsizes, blobs, formats, names, n);
    }
    // Pass it up the chain
    return INDI::Telescope::ISNewBLOB(dev, name, sizes, blobsizes, blobs, formats, names, n);
}

bool SkywatcherAPIMount::ISNewNumber (const char *dev, const char *name, double values[], char *names[], int n)
{
    if(strcmp(dev,getDeviceName())==0)
    {
        // It is for us
        ProcessNumberProperties(this, name, values, names, n);
    }
    // Pass it up the chain
    return INDI::Telescope::ISNewNumber(dev, name, values, names, n);
}

bool SkywatcherAPIMount::ISNewSwitch (const char *dev, const char *name, ISState *states, char *names[], int n)
{
    if(strcmp(dev,getDeviceName())==0)
    {
        // It is for us
        ProcessSwitchProperties(this, name, states, names, n);
    }
    // Pass it up the chain
    return INDI::Telescope::ISNewSwitch(dev, name, states, names, n);
}

bool SkywatcherAPIMount::ISNewText (const char *dev, const char *name, char *texts[], char *names[], int n)
{
    if(strcmp(dev,getDeviceName())==0)
    {
        ProcessTextProperties(this, name, texts, names, n);
    }
    // Pass it up the chain
    return INDI::Telescope::ISNewText(dev, name, texts, names, n);
}

bool SkywatcherAPIMount::MoveNS(TelescopeMotionNS dir)
{
    DEBUG(DBG_SCOPE, "SkywatcherAPIMount::MoveNS");
    switch (dir)
    {
        case MOTION_NORTH:
            if (PreviousNSMotion != PREVIOUS_NS_MOTION_NORTH)
            {
                DEBUG(DBG_SCOPE, "Starting Slew North");
                Slew(AXIS2, LOW_SPEED_MARGIN / 2);
                PreviousNSMotion = PREVIOUS_NS_MOTION_NORTH;
            }
            else
            {
                DEBUG(DBG_SCOPE, "Stopping Slew North");
                Stop(AXIS2);
                PreviousNSMotion = PREVIOUS_NS_MOTION_UNKNOWN;
                IUResetSwitch(&MovementNSSP);
                MovementNSSP.s = IPS_IDLE;
                IDSetSwitch(&MovementNSSP, NULL);
            }
            break;

        case MOTION_SOUTH:
            if (PreviousNSMotion != PREVIOUS_NS_MOTION_SOUTH)
            {
                DEBUG(DBG_SCOPE, "Starting Slew South");
                Slew(AXIS2, -LOW_SPEED_MARGIN / 2);
                PreviousNSMotion = PREVIOUS_NS_MOTION_SOUTH;
            }
            else
            {
                DEBUG(DBG_SCOPE, "Stopping Slew South");
                Stop(AXIS2);
                PreviousNSMotion = PREVIOUS_NS_MOTION_UNKNOWN;
                IUResetSwitch(&MovementNSSP);
                MovementNSSP.s = IPS_IDLE;
                IDSetSwitch(&MovementNSSP, NULL);
            }
            break;
    }
    return false;
}

bool SkywatcherAPIMount::MoveWE(TelescopeMotionWE dir)
{
    DEBUG(DBG_SCOPE, "SkywatcherAPIMount::MoveWE");
    switch (dir)
    {
        case MOTION_WEST:
            if (PreviousWEMotion != PREVIOUS_WE_MOTION_WEST)
            {
                DEBUG(DBG_SCOPE, "Starting Slew West");
                Slew(AXIS1, LOW_SPEED_MARGIN / 2);
                PreviousWEMotion = PREVIOUS_WE_MOTION_WEST;
            }
            else
            {
                DEBUG(DBG_SCOPE, "Stopping Slew West");
                Stop(AXIS1);
                PreviousWEMotion = PREVIOUS_WE_MOTION_UNKNOWN;
                IUResetSwitch(&MovementWESP);
                MovementWESP.s = IPS_IDLE;
                IDSetSwitch(&MovementWESP, NULL);
            }
            break;

        case MOTION_EAST:
            if (PreviousWEMotion != PREVIOUS_WE_MOTION_EAST)
            {
                DEBUG(DBG_SCOPE, "Starting Slew East");
                Slew(AXIS1, -LOW_SPEED_MARGIN / 2);
                PreviousWEMotion = PREVIOUS_WE_MOTION_EAST;
            }
            else
            {
                DEBUG(DBG_SCOPE, "Stopping Slew East");
                Stop(AXIS1);
                PreviousWEMotion = PREVIOUS_WE_MOTION_UNKNOWN;
                IUResetSwitch(&MovementWESP);
                MovementWESP.s = IPS_IDLE;
                IDSetSwitch(&MovementWESP, NULL);
            }
            break;
    }
    return false;
}

bool SkywatcherAPIMount::Park()
{
    DEBUG(DBG_SCOPE, "SkywatcherAPIMount::Park");
    return false;
}

bool SkywatcherAPIMount::ReadScopeStatus()
{
    DEBUG(DBG_SCOPE, "SkywatcherAPIMount::ReadScopeStatus");

    // Horrible hack to get over the fact that the base class calls ReadScopeStatus from inside Connect
    // before I have a chance to set up the serial port
    SetSerialPort(PortFD);

    // leave the following stuff in for the time being it is mostly harmless

    // Quick check of the mount
    if (!GetMotorBoardVersion(AXIS1))
        return false;

    if (!GetStatus(AXIS1))
        return false;

    if (!GetStatus(AXIS2))
        return false;


    if(TrackState==SCOPE_SLEWING)
    {
        if ((AxesStatus[AXIS1].FullStop) && (AxesStatus[AXIS2].FullStop))
            TrackState = SCOPE_IDLE;
    }

    // Update Axis Position
    if (!GetEncoder(AXIS1))
        return false;
    if (!GetEncoder(AXIS2))
        return false;

    UpdateDetailedMountInformation(true);

    // Calculate new RA DEC
    struct ln_hrz_posn AltAz;
    AltAz.alt = MicrostepsToDegrees(AXIS2, CurrentEncoders[AXIS2] - InitialEncoders[AXIS2]);
    DEBUGF(DBG_SCOPE, "Axis2 encoder %ld initial %ld alt(degrees) %lf", CurrentEncoders[AXIS2], InitialEncoders[AXIS2], AltAz.alt);
    AltAz.az = MicrostepsToDegrees(AXIS1, CurrentEncoders[AXIS1] - InitialEncoders[AXIS1]);
    DEBUGF(DBG_SCOPE, "Axis1 encoder %ld initial %ld az(degrees) %lf", CurrentEncoders[AXIS1], InitialEncoders[AXIS1], AltAz.az);
    TelescopeDirectionVector TDV = TelescopeDirectionVectorFromAltitudeAzimuth(AltAz);
    DEBUGF(DBG_SCOPE, "TDV x %lf y %lf z %lf", TDV.x, TDV.y, TDV.z);

    double RightAscension, Declination;
    if (TransformTelescopeToCelestial( TDV, RightAscension, Declination))
    {
        NewRaDec(RightAscension, Declination);
        DEBUGF(DBG_SCOPE, "Conversion OK RA %lf DEC %lf", RightAscension, Declination);
    }
    else
    {
        // Conversion failed just pull the coordinates back into RADEC and hope for the best
        struct ln_equ_posn EquatorialCoordinates;
        EquatorialCoordinatesFromTelescopeDirectionVector(TDV, EquatorialCoordinates);
        NewRaDec(EquatorialCoordinates.ra, EquatorialCoordinates.dec);
        DEBUGF(DBG_SCOPE, "Conversion failed RA %lf DEC %lf", EquatorialCoordinates.ra, EquatorialCoordinates.dec);
   }

    return true;
}

bool SkywatcherAPIMount::Sync(double ra, double dec)
{
    return false;
}

bool SkywatcherAPIMount::saveConfigItems(FILE *fp)
{
    SaveConfigProperties(fp);

    return INDI::Telescope::saveConfigItems(fp);
}

bool SkywatcherAPIMount::updateLocation(double latitude, double longitude, double elevation)
{
    DEBUG(DBG_SCOPE, "SkywatcherAPIMount::updateLocation");
    UpdateLocation(latitude, longitude, elevation);
}

bool SkywatcherAPIMount::updateProperties()
{
    INDI::Telescope::updateProperties();

    if (isConnected())
    {
        // Fill in any real values now available MCInit should have been called already
        UpdateDetailedMountInformation(false);

        // Define our connected only properties to the base driver
        // e.g. defineNumber(MyNumberVectorPointer);
        // This will register our properties and send a IDDefXXXX mewssage to any connected clients
        // I have now idea why I have to do this here as well as in ISGetProperties. It makes me
        // concerned there is a design or implementation flaw somewhere.
        defineNumber(&BasicMountInfoV);
        defineSwitch(&MountTypeV);
        defineNumber(&AxisOneInfoV);
        defineSwitch(&AxisOneStateV);
        defineNumber(&AxisTwoInfoV);
        defineSwitch(&AxisTwoStateV);
        defineNumber(&EncoderValuesV);

        // Start the timer if we need one
        // SetTimer(POLLMS);
    }
    else
    {
        // Delete any connected only properties from the base driver's list
        // e.g. deleteProperty(MyNumberVector.name);
        deleteProperty(BasicMountInfoV.name);
        deleteProperty(MountTypeV.name);
        deleteProperty(AxisOneInfoV.name);
        deleteProperty(AxisOneStateV.name);
        deleteProperty(AxisTwoInfoV.name);
        deleteProperty(AxisTwoStateV.name);
        deleteProperty(EncoderValuesV.name);
    }
}

// Private methods

int SkywatcherAPIMount::skywatcher_tty_read(int fd, char *buf, int nbytes, int timeout, int *nbytes_read)
{
    return tty_read(fd, buf, nbytes, timeout, nbytes_read);
}

int SkywatcherAPIMount::skywatcher_tty_write(int fd, const char * buffer, int nbytes, int *nbytes_written)
{
    return tty_write(fd, buffer, nbytes, nbytes_written);
}

void SkywatcherAPIMount::UpdateDetailedMountInformation(bool InformClient)
{
    bool BasicMountInfoHasChanged = false;
    if (BasicMountInfo[MOTOR_CONTROL_FIRMWARE_VERSION].value != MCVersion)
    {
        BasicMountInfo[MOTOR_CONTROL_FIRMWARE_VERSION].value = MCVersion;
        BasicMountInfoHasChanged = true;
    }
    if (BasicMountInfo[MOUNT_CODE].value != MountCode)
    {
        BasicMountInfo[MOUNT_CODE].value = MountCode;
        // Also tell the alignment subsystem
        switch (MountCode)
        {
            case _114GT:
            case DOB:
                SetApproximateMountAlignmentFromMountType(ALTAZ);
                break;

            default:
                SetApproximateMountAlignmentFromMountType(EQUATORIAL);
                break;
        }
        BasicMountInfoHasChanged = true;
    }
    if (BasicMountInfo[IS_DC_MOTOR].value != IsDCMotor)
    {
        BasicMountInfo[IS_DC_MOTOR].value = IsDCMotor;
        BasicMountInfoHasChanged = true;
    }
    if (BasicMountInfoHasChanged && InformClient)
        IDSetNumber(&BasicMountInfoV, NULL);

    int OldMountType = IUFindOnSwitchIndex(&MountTypeV);
    int NewMountType;
    switch (MountCode)
    {
        case 0x00:
            NewMountType = MT_EQ6;
            break;
        case 0x01:
            NewMountType = MT_HEQ5;
            break;
        case 0x02:
            NewMountType = MT_EQ5;
            break;
        case 0x03:
            NewMountType = MT_EQ3;
            break;
        case 0x80:
            NewMountType = MT_GT;
            break;
        case 0x81:
            NewMountType = MT_MF;
            break;
        case 0x82:
            NewMountType = MT_114GT;
            break;
        case 0x90:
            NewMountType = MT_DOB;
            break;
        default:
            NewMountType = MT_UNKNOWN;
            break;
    }
    if (OldMountType != NewMountType)
    {
        IUResetSwitch(&MountTypeV);
        MountType[NewMountType].s = ISS_ON;
        if (InformClient)
            IDSetSwitch(&MountTypeV, NULL);
    }

    bool AxisOneInfoHasChanged = false;
    if (AxisOneInfo[MICROSTEPS_PER_REVOLUTION].value != MicrostepsPerRevolution[0])
    {
        AxisOneInfo[MICROSTEPS_PER_REVOLUTION].value = MicrostepsPerRevolution[0];
        AxisOneInfoHasChanged = true;
    }
    if (AxisOneInfo[STEPPER_CLOCK_FREQUENCY].value != StepperClockFrequency[0])
    {
        AxisOneInfo[STEPPER_CLOCK_FREQUENCY].value = StepperClockFrequency[0];
        AxisOneInfoHasChanged = true;
    }
    if (AxisOneInfo[HIGH_SPEED_RATIO].value != HighSpeedRatio[0])
    {
        AxisOneInfo[HIGH_SPEED_RATIO].value = HighSpeedRatio[0];
        AxisOneInfoHasChanged = true;
    }
    if (AxisOneInfo[MICROSTEPS_PER_WORM_REVOLUTION].value != MicrostepsPerWormRevolution[0])
    {
        AxisOneInfo[MICROSTEPS_PER_WORM_REVOLUTION].value = MicrostepsPerWormRevolution[0];
        AxisOneInfoHasChanged = true;
    }
    if (AxisOneInfoHasChanged && InformClient)
        IDSetNumber(&AxisOneInfoV, NULL);

    bool AxisOneStateHasChanged = false;
    if (AxisOneState[FULL_STOP].s != (AxesStatus[0].FullStop ? ISS_ON : ISS_OFF))
    {
        AxisOneState[FULL_STOP].s = AxesStatus[0].FullStop ? ISS_ON : ISS_OFF;
        AxisOneStateHasChanged = true;
    }
    if (AxisOneState[SLEWING].s != (AxesStatus[0].Slewing ? ISS_ON : ISS_OFF))
    {
        AxisOneState[SLEWING].s = AxesStatus[0].Slewing ? ISS_ON : ISS_OFF;
        AxisOneStateHasChanged = true;
    }
    if (AxisOneState[SLEWING_TO].s != (AxesStatus[0].SlewingTo ? ISS_ON : ISS_OFF))
    {
        AxisOneState[SLEWING_TO].s = AxesStatus[0].SlewingTo ? ISS_ON : ISS_OFF;
        AxisOneStateHasChanged = true;
    }
    if (AxisOneState[SLEWING_FORWARD].s != (AxesStatus[0].SlewingForward ? ISS_ON : ISS_OFF))
    {
        AxisOneState[SLEWING_FORWARD].s = AxesStatus[0].SlewingForward ? ISS_ON : ISS_OFF;
        AxisOneStateHasChanged = true;
    }
    if (AxisOneState[HIGH_SPEED].s != (AxesStatus[0].HighSpeed ? ISS_ON : ISS_OFF))
    {
        AxisOneState[HIGH_SPEED].s = AxesStatus[0].HighSpeed ? ISS_ON : ISS_OFF;
        AxisOneStateHasChanged = true;
    }
    if (AxisOneState[NOT_INITIALISED].s != (AxesStatus[0].NotInitialized ? ISS_ON : ISS_OFF))
    {
        AxisOneState[NOT_INITIALISED].s = AxesStatus[0].NotInitialized ? ISS_ON : ISS_OFF;
        AxisOneStateHasChanged = true;
    }
    if (AxisOneStateHasChanged && InformClient)
            IDSetSwitch(&AxisOneStateV, NULL);

    bool AxisTwoInfoHasChanged = false;
    if (AxisTwoInfo[MICROSTEPS_PER_REVOLUTION].value != MicrostepsPerRevolution[1])
    {
        AxisTwoInfo[MICROSTEPS_PER_REVOLUTION].value = MicrostepsPerRevolution[1];
        AxisTwoInfoHasChanged = true;
    }
    if (AxisTwoInfo[STEPPER_CLOCK_FREQUENCY].value != StepperClockFrequency[1])
    {
        AxisTwoInfo[STEPPER_CLOCK_FREQUENCY].value = StepperClockFrequency[1];
        AxisTwoInfoHasChanged = true;
    }
    if (AxisTwoInfo[HIGH_SPEED_RATIO].value != HighSpeedRatio[1])
    {
        AxisTwoInfo[HIGH_SPEED_RATIO].value = HighSpeedRatio[1];
        AxisTwoInfoHasChanged = true;
    }
    if (AxisTwoInfo[MICROSTEPS_PER_WORM_REVOLUTION].value != MicrostepsPerWormRevolution[1])
    {
        AxisTwoInfo[MICROSTEPS_PER_WORM_REVOLUTION].value = MicrostepsPerWormRevolution[1];
        AxisTwoInfoHasChanged = true;
    }
    if (AxisTwoInfoHasChanged && InformClient)
        IDSetNumber(&AxisTwoInfoV, NULL);

    bool AxisTwoStateHasChanged = false;
    if (AxisTwoState[FULL_STOP].s != (AxesStatus[1].FullStop ? ISS_ON : ISS_OFF))
    {
        AxisTwoState[FULL_STOP].s = AxesStatus[1].FullStop ? ISS_ON : ISS_OFF;
        AxisTwoStateHasChanged = true;
    }
    if (AxisTwoState[SLEWING].s != (AxesStatus[1].Slewing ? ISS_ON : ISS_OFF))
    {
        AxisTwoState[SLEWING].s = AxesStatus[1].Slewing ? ISS_ON : ISS_OFF;
        AxisTwoStateHasChanged = true;
    }
    if (AxisTwoState[SLEWING_TO].s != (AxesStatus[1].SlewingTo ? ISS_ON : ISS_OFF))
    {
        AxisTwoState[SLEWING_TO].s = AxesStatus[1].SlewingTo ? ISS_ON : ISS_OFF;
        AxisTwoStateHasChanged = true;
    }
    if (AxisTwoState[SLEWING_FORWARD].s != (AxesStatus[1].SlewingForward ? ISS_ON : ISS_OFF))
    {
        AxisTwoState[SLEWING_FORWARD].s = AxesStatus[1].SlewingForward ? ISS_ON : ISS_OFF;
        AxisTwoStateHasChanged = true;
    }
    if (AxisTwoState[HIGH_SPEED].s != (AxesStatus[1].HighSpeed ? ISS_ON : ISS_OFF))
    {
        AxisTwoState[HIGH_SPEED].s = AxesStatus[1].HighSpeed ? ISS_ON : ISS_OFF;
        AxisTwoStateHasChanged = true;
    }
    if (AxisTwoState[NOT_INITIALISED].s != (AxesStatus[1].NotInitialized ? ISS_ON : ISS_OFF))
    {
        AxisTwoState[NOT_INITIALISED].s = AxesStatus[1].NotInitialized ? ISS_ON : ISS_OFF;
        AxisTwoStateHasChanged = true;
    }
    if (AxisTwoStateHasChanged && InformClient)
            IDSetSwitch(&AxisTwoStateV, NULL);

    bool EncoderValuesHasChanged = false;
    if (EncoderValues[0].value != CurrentEncoders[AXIS1])
    {
        EncoderValues[0].value = CurrentEncoders[AXIS1];
        EncoderValuesHasChanged = true;
    }
    if (EncoderValues[1].value != CurrentEncoders[AXIS2])
    {
        EncoderValues[1].value = CurrentEncoders[AXIS2];
        EncoderValuesHasChanged = true;
    }
    if (EncoderValuesHasChanged && InformClient)
        IDSetNumber(&EncoderValuesV, NULL);

}

