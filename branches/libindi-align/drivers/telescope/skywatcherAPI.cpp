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

SkywatcherAPI::SkywatcherAPI()
{
    MCVersion = 0;
    FactorStepToRad[AXIS1] = FactorStepToRad[AXIS2] = 0;
    FactorRadToStep[AXIS1] = FactorRadToStep[AXIS2] = 0;
    CurrentPositions[AXIS1] = CurrentPositions[AXIS2] = 0;
    InitialPositions[AXIS1] = InitialPositions[AXIS2] = 0;
}

SkywatcherAPI::~SkywatcherAPI()
{
    //dtor
}

bool SkywatcherAPI::TalkWithAxis(AXISID Axis, char Command, std::string& cmdDataStr, std::string& responseStr)
{
    MYDEBUGF(INDI::Logger::DBG_SESSION, "TalkWithAxis Command %c Data (%s)", Command, cmdDataStr.c_str());

    std::string SendBuffer;
    int bytesWritten;
    int bytesRead;
    bool StartReading = false;
    bool EndReading = false;

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
    MYDEBUGF(INDI::Logger::DBG_SESSION, "TalkWithAxis - good return Response (%s)", responseStr.c_str());
    return true;
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

long SkywatcherAPI::AngleToStep(AXISID Axis, double AngleInRad)
{
    return (long)(AngleInRad * FactorRadToStep[(int)Axis]);
}

double SkywatcherAPI::StepToAngle(AXISID Axis, long Steps)
{
    return Steps * FactorStepToRad[(int)Axis];
}

long SkywatcherAPI::RadSpeedToInt(AXISID Axis, double RateInRad)
{
    return (long)(RateInRad * FactorRadRateToInt[(int)Axis]);
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

bool SkywatcherAPI::MCInit()
{
    MYDEBUG(INDI::Logger::DBG_SESSION, "MCInit");

	if (!CheckIfDCMotor())
		return false;

    if (!InquireMotorBoardVersion(AXIS1))
        return false;

    MountCode = MCVersion & 0xFF;

    //// NOTE: Simulator settings, Mount dependent Settings

    // Inquire Gear Rate
    if (!InquireGridPerRevolution(AXIS1))
        return false;
    if (!InquireGridPerRevolution(AXIS2))
        return false;

    // Inquire motor timer interrup frequency
    if (!InquireTimerInterruptFreq(AXIS1))
        return false;
    if (!InquireTimerInterruptFreq(AXIS2))
        return false;

    // Inquire motor high speed ratio
    if (!InquireHighSpeedRatio(AXIS1))
        return false;
    if (!InquireHighSpeedRatio(AXIS2))
        return false;

    // Inquire PEC period
    // DC motor controller does not support PEC
    if (!IsDCMotor)
    {
//        if (!InquirePECPeriod(AXIS1);
//        if (!InquirePECPeriod(AXIS2);
    }

    // Inquire Axis Position
    if (!MCGetAxisPosition(AXIS1))
        return false;
    if (!MCGetAxisPosition(AXIS2))
        return false;

    // Set initial axis posiitons
    // These are used to define the arbitary zero position vector for the axis
    InitialPositions[AXIS1] = CurrentPositions[AXIS1];
    InitialPositions[AXIS2] = CurrentPositions[AXIS2];


    if (!InitializeMC())
        return false;

    // These two LowSpeedGotoMargin are calculate from slewing for 5 seconds in 128x sidereal rate
    LowSpeedGotoMargin[(int)AXIS1] = (long)(640 * CONSTANT::SIDEREALRATE * FactorRadToStep[(int)AXIS1]);
    LowSpeedGotoMargin[(int)AXIS2] = (long)(640 * CONSTANT::SIDEREALRATE * FactorRadToStep[(int)AXIS2]);

    // Default break steps
    BreakSteps[(int)AXIS1] = 3500;
    BreakSteps[(int)AXIS2] = 3500;


    return true;
}

bool SkywatcherAPI::MCAxisStop(AXISID Axis)
{
    // Request a slow stop
    MYDEBUG(INDI::Logger::DBG_SESSION, "MCAxisStop");
    std::string Parameters, Response;
    if (!TalkWithAxis(Axis, 'K', Parameters, Response))
    	return false;
    return true;
}

bool SkywatcherAPI::MCAxisInstantStop(AXISID Axis)
{
    // Request a slow stop
    MYDEBUG(INDI::Logger::DBG_SESSION, "MCAxisStop");
    std::string Parameters, Response;
    if (!TalkWithAxis(Axis, 'L', Parameters, Response))
    	return false;
    AxesStatus[(int)Axis].SetFullStop();
    return true;
}

bool SkywatcherAPI::MCSetAxisPosition(AXISID Axis, double Position)
{
    MYDEBUG(INDI::Logger::DBG_SESSION, "MCSetAxisPosition");
    std::string Parameters, Response;

    long Temp = Position;
    Temp += 0x800000;
    Long2BCDstr(Temp, Parameters);

    if (!TalkWithAxis(Axis, 'L', Parameters, Response))
    	return false;

    return true;
}

bool SkywatcherAPI::MCGetAxisPosition(AXISID Axis)
{
    MYDEBUG(INDI::Logger::DBG_SESSION, "MCGetAxisPosition");
    std::string Parameters, Response;
    if (!TalkWithAxis(Axis, 'j', Parameters, Response))
    	return false;

    long iPosition = BCDstr2long(Response);
    iPosition -= 0x00800000;
    CurrentPositions[(int)Axis] = StepToAngle(Axis, iPosition);

    return true;
}

bool SkywatcherAPI::MCGetAxisStatus(AXISID Axis)
{
    MYDEBUG(INDI::Logger::DBG_SESSION, "MCGetAxisStatus");
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

bool SkywatcherAPI::MCSetSwitch(bool OnOff)
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


/************************ MOTOR COMMAND SET ***************************/
// Inquire Motor Board Version ":e(*1)", where *1: '1'= CH1, '2'= CH2, '3'= Both.
bool SkywatcherAPI::InquireMotorBoardVersion(AXISID Axis)
{
    MYDEBUG(INDI::Logger::DBG_SESSION, "InquireMotorBoardVersion");
    std::string Parameters, Response;

    if (!TalkWithAxis(Axis, 'e', Parameters, Response))
        return false;

    long tmpMCVersion = BCDstr2long(Response);

    MCVersion = ((tmpMCVersion & 0xFF) << 16) | ((tmpMCVersion & 0xFF00)) | ((tmpMCVersion & 0xFF0000) >> 16);

    return true;
}

// Inquire Grid Per Revolution ":a(*2)", where *2: '1'= CH1, '2' = CH2.
bool SkywatcherAPI::InquireGridPerRevolution(AXISID Axis)
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

// Inquire Timer Interrupt Freq ":b1".
bool SkywatcherAPI::InquireTimerInterruptFreq(AXISID Axis)
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

// Inquire high speed ratio ":g(*2)", where *2: '1'= CH1, '2' = CH2.
bool SkywatcherAPI::InquireHighSpeedRatio(AXISID Axis)
{
    MYDEBUG(INDI::Logger::DBG_SESSION, "InquireHighSpeedRatio");
    std::string Parameters, Response;

    if (!TalkWithAxis(Axis, 'g', Parameters, Response))
        return false;

    long highSpeedRatio = BCDstr2long(Response);
    HighSpeedRatio[(int)Axis] = highSpeedRatio;

    return true;
}

// Inquire PEC Period ":s(*1)", where *1: '1'= CH1, '2'= CH2, '3'= Both.
bool SkywatcherAPI::InquirePECPeriod(AXISID Axis)
{
    MYDEBUG(INDI::Logger::DBG_SESSION, "InquirePECPeriod");
    std::string Parameters, Response;

    if (!TalkWithAxis(Axis, 's', Parameters, Response))
        return false;

    long PECPeriod = BCDstr2long(Response);
    PESteps[(int)Axis] = PECPeriod;

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
