/*!
 * \file skywatcherAPI.cpp
 *
 * \author Roger James
 * \author Gerry Rozema
 * \author Jean-Luc Geehalel
 * \date 13th November 2013
 *
 * This file contains the implementation in C++ of the Skywatcher API.
 * It is based on work from three sources.
 * A C++ implementation of the API by Roger James.
 * The indi_eqmod driver by Jean-Luc Geehalel.
 * The synscanmount driver by Gerry Rozema.
 */

#include "skywatcherAPI.h"

#include <memory>
#include <cmath>
#include <cstdio>
#include <sstream>
#include <iomanip>
//#include <thread>
//#include <chrono>
#include <unistd.h>

void AXISSTATUS::SetFullStop()
{
    FullStop = true;
    SlewingTo = Slewing = false;
}

void AXISSTATUS::SetSlewing(bool forward, bool highspeed)
{
    FullStop = SlewingTo = false;
    Slewing = true;

    SlewingForward = forward;
    HighSpeed = highspeed;
}

void AXISSTATUS::SetSlewingTo(bool forward, bool highspeed)
{
    FullStop = Slewing = false;
    SlewingTo = true;

    SlewingForward = forward;
    HighSpeed = highspeed;
}

const double SkywatcherAPI::LOW_SPEED_MARGIN = 128.0 * SIDEREALRATE;


// Constructor

SkywatcherAPI::SkywatcherAPI()
{
    MCVersion = 0;
    FactorStepToRad[AXIS1] = FactorStepToRad[AXIS2] = 0;
    FactorRadToStep[AXIS1] = FactorRadToStep[AXIS2] = 0;
    CurrentPositions[AXIS1] = CurrentPositions[AXIS2] = 0;
    InitialPositions[AXIS1] = InitialPositions[AXIS2] = 0;
    SlewingSpeed[AXIS1] = SlewingSpeed[AXIS2] = 0;
}

// Destructor

SkywatcherAPI::~SkywatcherAPI()
{
}

// Public methods

long SkywatcherAPI::AngleToStep(AXISID Axis, double AngleInRadians)
{
    return (long)(AngleInRadians * FactorRadToStep[(int)Axis]);
}

long SkywatcherAPI::BCDstr2long(std::string &String)
{
// =020782 => 8521474
    // Funny BCD :-) string is pairs of hex chars with each pair representing a 8 bit hex number. The whole
    // string being treated as least significant hex digit pair first!
    const char *str = String.c_str();
    long value = 0;
    for (int i = 0; i < String.length(); i += 2)
    {
        long hexpair;
        sscanf(str + i, "%2lx", &hexpair);
        value += hexpair << i * 4;
    }
    return value;
}

bool SkywatcherAPI::CheckIfDCMotor()
{
    MYDEBUG(INDI::Logger::DBG_SESSION, "CheckIfDCMotor");
	// Flush the tty read buffer
	char input[20];
	int rc;
	int nbytes;

	while (true)
	{
		rc =  skywatcher_tty_read(MyPortFD, input, 20, 5, &nbytes);
		if (TTY_TIME_OUT == rc)
			break;
		if (TTY_OK != rc)
			return false;
	}

	if (TTY_OK != skywatcher_tty_write(MyPortFD, ":", 1, &nbytes))
		return false;

	rc =  skywatcher_tty_read(MyPortFD, input, 1, 5, &nbytes);

	if ((TTY_OK == rc) && (1 == nbytes) && (':' == input[0]))
	{
		IsDCMotor = true;
		return true;
	}
	if (TTY_TIME_OUT == rc)
	{
		IsDCMotor = false;
		return true;
	}

	return false;
}

bool SkywatcherAPI::GetGridPerRevolution(AXISID Axis)
{
    MYDEBUG(INDI::Logger::DBG_SESSION, "InquireGridPerRevolution");
    std::string Parameters, Response;

    if (!TalkWithAxis(Axis, 'a', Parameters, Response))
        return false;


    long tmpGearRatio = BCDstr2long(Response);

    // There is a bug in the earlier version firmware(Before 2.00) of motor controller MC001.
    // Overwrite the GearRatio reported by the MC for 80GT mount and 114GT mount.
    if ((MCVersion & 0x0000FF) == 0x80)
        tmpGearRatio = 0x162B97;		// for 80GT mount
    if ((MCVersion & 0x0000FF) == 0x82)
        tmpGearRatio = 0x205318;		// for 114GT mount

    GearRatio[(int)Axis] = tmpGearRatio;

    FactorRadToStep[(int)Axis] = tmpGearRatio / (2 * M_PI);
    FactorStepToRad[(int)Axis] = 2 * M_PI / tmpGearRatio;

    return true;
}

bool SkywatcherAPI::GetHighSpeedRatio(AXISID Axis)
{
    MYDEBUG(INDI::Logger::DBG_SESSION, "InquireHighSpeedRatio");
    std::string Parameters, Response;

    if (!TalkWithAxis(Axis, 'g', Parameters, Response))
        return false;

    long highSpeedRatio = BCDstr2long(Response);
    HighSpeedRatio[(int)Axis] = highSpeedRatio;

    return true;
}

bool SkywatcherAPI::GetPECPeriod(AXISID Axis)
{
    MYDEBUG(INDI::Logger::DBG_SESSION, "InquirePECPeriod");
    std::string Parameters, Response;

    if (!TalkWithAxis(Axis, 's', Parameters, Response))
        return false;

    long PECPeriod = BCDstr2long(Response);
    PESteps[(int)Axis] = PECPeriod;

    return true;
}

bool SkywatcherAPI::GetMotorBoardVersion(AXISID Axis)
{
    MYDEBUG(INDI::Logger::DBG_SESSION, "InquireMotorBoardVersion");
    std::string Parameters, Response;

    if (!TalkWithAxis(Axis, 'e', Parameters, Response))
        return false;

    long tmpMCVersion = BCDstr2long(Response);

    MCVersion = ((tmpMCVersion & 0xFF) << 16) | ((tmpMCVersion & 0xFF00)) | ((tmpMCVersion & 0xFF0000) >> 16);

    return true;
}

bool SkywatcherAPI::GetPosition(AXISID Axis)
{
    MYDEBUG(INDI::Logger::DBG_SESSION, "GetPosition");
    std::string Parameters, Response;
    if (!TalkWithAxis(Axis, 'j', Parameters, Response))
    	return false;

    long iPosition = BCDstr2long(Response);
    iPosition -= 0x00800000;
    CurrentPositions[(int)Axis] = StepToAngle(Axis, iPosition);

    return true;
}

bool SkywatcherAPI::GetTimerInterruptFreq(AXISID Axis)
{
    MYDEBUG(INDI::Logger::DBG_SESSION, "InquireTimerInterruptFreq");
    std::string Parameters, Response;

    if (!TalkWithAxis(Axis, 'b', Parameters, Response))
        return false;

    long TimeFreq = BCDstr2long(Response);
    StepTimerFreq[(int)Axis] = TimeFreq;

    FactorRadRateToInt[(int)Axis] = (double)(StepTimerFreq[(int)Axis]) / FactorRadToStep[(int)Axis];

    return true;
}

bool SkywatcherAPI::GetStatus(AXISID Axis)
{
    MYDEBUG(INDI::Logger::DBG_SESSION, "GetStatus");
    std::string Parameters, Response;

    if(!TalkWithAxis(Axis, 'f', Parameters, Response))
        return false;

    if ((Response[1] & 0x01) != 0)
    {
        // Axis is running
        AxesStatus[(int)Axis].FullStop = false;
        if ((Response[0] & 0x01) != 0)
        {
            AxesStatus[(int)Axis].Slewing = true;		// Axis in slewing(AstroMisc speed) mode.
            AxesStatus[(int)Axis].SlewingTo = false;
        }
        else
        {
            AxesStatus[(int)Axis].SlewingTo = true;		// Axis in SlewingTo mode.
            AxesStatus[(int)Axis].Slewing = false;
        }
    }
    else
    {

        AxesStatus[(int)Axis].FullStop = true;	// FullStop = 1;	// Axis is fully stop.
        AxesStatus[(int)Axis].Slewing = false;
        AxesStatus[(int)Axis].SlewingTo = false;
   }

    if ((Response[0] & 0x02) == 0)
        AxesStatus[(int)Axis].SlewingForward = true;	// Angle increase = 1;
    else
        AxesStatus[(int)Axis].SlewingForward = false;

    if ((Response[0] & 0x04) != 0)
        AxesStatus[(int)Axis].HighSpeed = true; // HighSpeed running mode = 1;
    else
        AxesStatus[(int)Axis].HighSpeed = false;

    if ((Response[2] & 1) == 0)
        AxesStatus[(int)Axis].NotInitialized = true;	// MC is not initialized.
    else
        AxesStatus[(int)Axis].NotInitialized = false;


    return true;
}

// Set initialization done ":F3", where '3'= Both CH1 and CH2.
bool SkywatcherAPI::InitializeMC()
{
    MYDEBUG(INDI::Logger::DBG_SESSION, "InitializeMC");
    std::string Parameters, Response;

    if (!TalkWithAxis(AXIS1, 'F', Parameters, Response))
        return false;
    if (!TalkWithAxis(AXIS2, 'F', Parameters, Response))
        return false;
    return true;
}

bool SkywatcherAPI::InitMount()
{
    MYDEBUG(INDI::Logger::DBG_SESSION, "InitMount");

	if (!CheckIfDCMotor())
		return false;

    if (!GetMotorBoardVersion(AXIS1))
        return false;

    MountCode = MCVersion & 0xFF;

    //// NOTE: Simulator settings, Mount dependent Settings

    // Inquire Gear Rate
    if (!GetGridPerRevolution(AXIS1))
        return false;
    if (!GetGridPerRevolution(AXIS2))
        return false;

    // Inquire motor timer interrup frequency
    if (!GetTimerInterruptFreq(AXIS1))
        return false;
    if (!GetTimerInterruptFreq(AXIS2))
        return false;

    // Inquire motor high speed ratio
    if (!GetHighSpeedRatio(AXIS1))
        return false;
    if (!GetHighSpeedRatio(AXIS2))
        return false;

    // Inquire PEC period
    // DC motor controller does not support PEC
    if (!IsDCMotor)
    {
//        if (!InquirePECPeriod(AXIS1);
//        if (!InquirePECPeriod(AXIS2);
    }

    // Inquire Axis Position
    if (!GetPosition(AXIS1))
        return false;
    if (!GetPosition(AXIS2))
        return false;

    // Set initial axis posiitons
    // These are used to define the arbitary zero position vector for the axis
    InitialPositions[AXIS1] = CurrentPositions[AXIS1];
    InitialPositions[AXIS2] = CurrentPositions[AXIS2];


    if (!InitializeMC())
        return false;

    // These two LowSpeedGotoMargin are calculate from slewing for 5 seconds in 128x sidereal rate
    LowSpeedGotoMargin[(int)AXIS1] = (long)(640 * SIDEREALRATE * FactorRadToStep[(int)AXIS1]);
    LowSpeedGotoMargin[(int)AXIS2] = (long)(640 * SIDEREALRATE * FactorRadToStep[(int)AXIS2]);

    // Default break steps
    BreakSteps[(int)AXIS1] = 3500;
    BreakSteps[(int)AXIS2] = 3500;


    return true;
}

bool SkywatcherAPI::InstantStop(AXISID Axis)
{
    // Request a slow stop
    MYDEBUG(INDI::Logger::DBG_SESSION, "InstantStop");
    std::string Parameters, Response;
    if (!TalkWithAxis(Axis, 'L', Parameters, Response))
    	return false;
    AxesStatus[(int)Axis].SetFullStop();
    return true;
}

void SkywatcherAPI::Long2BCDstr(long Number, std::string &String)
{
    std::stringstream Temp;
    const char *Debug;
    Temp << std::hex << std::setfill('0') << std::uppercase
        << std::setw(2) << (Number & 0xff)
        << std::setw(2) << ((Number & 0xff00) >> 8)
        << std::setw(2) << ((Number & 0xff0000) >> 16);
    Debug = Temp.str().c_str();
    String = Temp.str();
}

void SkywatcherAPI::PrepareForSlewing(AXISID Axis, double Speed)
{
    // Update the axis status
    if (!GetStatus(Axis))
        return;

    if (!AxesStatus[Axis].FullStop)
    {
        // Axis is running
        if ((AxesStatus[Axis].SlewingTo) // slew to (GOTO) in progress
            || (AxesStatus[Axis].HighSpeed) // currently high speed slewing
            || (std::abs(Speed) >= LOW_SPEED_MARGIN) // I am about to request high speed
            || ((AxesStatus[Axis].SlewingForward) && (Speed < 0)) // Direction change
            || (!(AxesStatus[Axis].SlewingForward) && (Speed > 0))) // Direction change
        {
            // I need to stop the axis first
            Stop(Axis);
        }
        else
            return; // NO need change motion mode

        // Horrible bit A POLLING LOOP !!!!!!!!!!
        while (true)
        {
            // Update status
            GetStatus(Axis);

            if (AxesStatus[Axis].FullStop)
                break;

            //std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Sleep for 1/10 second
            usleep(1000);
        }
    }

    char Direction;
    if (Speed > 0.0)
        Direction = '0';
    else
    {
        Direction = '1';
        Speed = -Speed;
    }

    if (Speed > LOW_SPEED_MARGIN)
        SetMotionMode(Axis, '3', Direction);
    else
        SetMotionMode(Axis, '1', Direction);
}

long SkywatcherAPI::RadSpeedToInt(AXISID Axis, double RateInSecondsPerRadian)
{
    return (long)(RateInSecondsPerRadian * FactorRadRateToInt[(int)Axis]);
}

bool SkywatcherAPI::SetBreakPointIncrement(AXISID Axis, long StepsCount)
{
    MYDEBUG(INDI::Logger::DBG_SESSION, "SetBreakPointIncrement");
    std::string Parameters, Response;

    Long2BCDstr(StepsCount, Parameters);

    if (!TalkWithAxis(Axis, 'M', Parameters, Response))
    	return false;

    return true;
}

bool SkywatcherAPI::SetBreakSteps(AXISID Axis, long NewBreakSteps)
{
    MYDEBUG(INDI::Logger::DBG_SESSION, "SetBreakSteps");
    std::string Parameters, Response;

    Long2BCDstr(NewBreakSteps, Parameters);

    if (!TalkWithAxis(Axis, 'U', Parameters, Response))
    	return false;

    return true;
}

bool SkywatcherAPI::SetGotoTargetIncrement(AXISID Axis, long StepsCount)
{
    MYDEBUG(INDI::Logger::DBG_SESSION, "SetGotoTargetIncrement");
    std::string Parameters, Response;

    Long2BCDstr(StepsCount, Parameters);

    if (!TalkWithAxis(Axis, 'H', Parameters, Response))
    	return false;

    return true;
}

/// Func - 0 Low speed slew to mode (goto)
/// Func - 1 Low speed slew mode
/// Func - 2 High speed slew to mode (goto)
/// Func - 3 High speed slew mode
bool SkywatcherAPI::SetMotionMode(AXISID Axis, char Func, char Direction)
{
    MYDEBUG(INDI::Logger::DBG_SESSION, "SetMotionMode");
    std::string Parameters, Response;

    Parameters.push_back(Func);
    Parameters.push_back(Direction);

    if (!TalkWithAxis(Axis, 'G', Parameters, Response))
    	return false;

    return true;
}

bool SkywatcherAPI::SetPosition(AXISID Axis, double Position)
{
    MYDEBUG(INDI::Logger::DBG_SESSION, "SetPosition");
    std::string Parameters, Response;

    long Temp = Position;
    Temp += 0x800000;
    Long2BCDstr(Temp, Parameters);

    if (!TalkWithAxis(Axis, 'L', Parameters, Response))
    	return false;

    return true;
}

bool SkywatcherAPI::SetStepPeriod(AXISID Axis, long ClockTicksPerMicrostep)
{
    MYDEBUG(INDI::Logger::DBG_SESSION, "SetStepPeriod");
    std::string Parameters, Response;

    Long2BCDstr(ClockTicksPerMicrostep, Parameters);

    if (!TalkWithAxis(Axis, 'I', Parameters, Response))
    	return false;

    return true;
}

bool SkywatcherAPI::SetSwitch(bool OnOff)
{
    MYDEBUG(INDI::Logger::DBG_SESSION, "MCSetSwitch");
    std::string Parameters, Response;

    if (OnOff)
        Parameters = "1";
    else
        Parameters = "0";

    if(!TalkWithAxis(AXIS1, 'O', Parameters, Response))
        return false;
    return true;
}

void SkywatcherAPI::Slew(AXISID Axis, double SpeedInRadiansPerSecond)
{
    // Clamp to MAX_SPEED
    if (SpeedInRadiansPerSecond > MAX_SPEED)
        SpeedInRadiansPerSecond = MAX_SPEED;
    else if (SpeedInRadiansPerSecond < -MAX_SPEED)
        SpeedInRadiansPerSecond = -MAX_SPEED;

    double InternalSpeed = SpeedInRadiansPerSecond;

    if (std::abs(InternalSpeed)<= SIDEREALRATE / 1000.0)
    {
        Stop(Axis);
        return;
    }

    // Stop motor and set motion mode if necessary
    PrepareForSlewing(Axis, InternalSpeed);

    bool Forward;
    if (InternalSpeed > 0.0)
        Forward = true;
    else
    {
        InternalSpeed = - InternalSpeed;
        Forward = false;
    }

    bool HighSpeed = false;
    if (InternalSpeed > LOW_SPEED_MARGIN)
    {
        InternalSpeed = InternalSpeed / (double)HighSpeedRatio[Axis];
        HighSpeed = true;
    }
    InternalSpeed = 1 / InternalSpeed; // Convert to seconds per radian
                                        // When I get everything working I will make some more sensible conversion routines
                                        // so I don't have to do so many conversions along the way.
    long SpeedInt = RadSpeedToInt(Axis, InternalSpeed);
    if ((MCVersion == 0x010600) || (MCVersion == 0x0010601))  // Cribbed from Mount_Skywatcher.cs
        SpeedInt -= 3;
    if (SpeedInt < 6)
        SpeedInt = 6;
    SetStepPeriod(Axis, SpeedInt);

    StartMotion(Axis);

    AxesStatus[Axis].SetSlewing(Forward, HighSpeed);
    SlewingSpeed[Axis] = SpeedInRadiansPerSecond;
}

bool SkywatcherAPI::StartMotion(AXISID Axis)
{
    MYDEBUG(INDI::Logger::DBG_SESSION, "StartMotion");
    std::string Parameters, Response;
    if (!TalkWithAxis(Axis, 'J', Parameters, Response))
    	return false;
    return true;
}

double SkywatcherAPI::StepToAngle(AXISID Axis, long Steps)
{
    return Steps * FactorStepToRad[(int)Axis];
}

bool SkywatcherAPI::Stop(AXISID Axis)
{
    // Request a slow stop
    MYDEBUG(INDI::Logger::DBG_SESSION, "Stop");
    std::string Parameters, Response;
    if (!TalkWithAxis(Axis, 'K', Parameters, Response))
    	return false;
    return true;
}

bool SkywatcherAPI::TalkWithAxis(AXISID Axis, char Command, std::string& cmdDataStr, std::string& responseStr)
{
    MYDEBUGF(INDI::Logger::DBG_SESSION, "TalkWithAxis Axis %d Command %c Data (%s)", Axis, Command, cmdDataStr.c_str());

    std::string SendBuffer;
    int bytesWritten;
    int bytesRead;
    bool StartReading = false;
    bool EndReading = false;
    bool mount_response = false;

    SendBuffer.push_back(':');
    SendBuffer.push_back(Command);
    SendBuffer.push_back(Axis == AXIS1 ? '1' : '2');
    SendBuffer.append(cmdDataStr);
    SendBuffer.push_back('\r');
    skywatcher_tty_write(MyPortFD, SendBuffer.c_str(), SendBuffer.size(), &bytesWritten);

    while (!EndReading)
    {
        char c;

        int rc = skywatcher_tty_read(MyPortFD, &c, 1, 10, &bytesRead);
        if ((rc != TTY_OK) || (bytesRead != 1))
            return false;

        if ((c == '=') || (c == '!'))
        {
            if (c == '=')
                mount_response = true;
            else
                mount_response = false;
            StartReading = true;
            continue;
        }

        if ((c == '\r') && StartReading)
        {
            EndReading = true;
            continue;
        }

        if (StartReading)
            responseStr.push_back(c);
    }
    MYDEBUGF(INDI::Logger::DBG_SESSION, "TalkWithAxis - %s Response (%s)", mount_response ? "Good" : "Bad", responseStr.c_str());
    return true;
}
