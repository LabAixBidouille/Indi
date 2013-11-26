/*!
 * \file AlignmentSubsystemDriver.cpp
 *
 * \author Roger James
 * \date 13th November 2013
 *
 * This file contains implementations of functions for the alignment subsystem.
 */

#include "AlignmentSubsystem.h"

void INDI::AlignmentSubsystemDriver::InitAlignmentProperties(Telescope* ChildTelescope)
{

    IUFillNumber(&AlignmentPointSetEntry[0], "ALIGNMENT_POINT_ENTRY_OBSERVATION_DATE", "Observation date (dd:mm:yy)", "%010.6m", 0, 60000, 0, 0);
    IUFillNumber(&AlignmentPointSetEntry[1], "ALIGNMENT_POINT_ENTRY_OBSERVATION_TIME", "Observation time (hh:mm:ss)", "%010.6m", 0, 24, 0, 0);
    IUFillNumber(&AlignmentPointSetEntry[2], "ALIGNMENT_POINT_ENTRY_RA", "Right Ascension (hh:mm:ss)", "%010.6m", 0, 24, 0, 0);
    IUFillNumber(&AlignmentPointSetEntry[3]," ALIGNMENT_POINT_ENTRY_DEC", "Declination (dd:mm:ss)", "%010.6m", -90, 90, 0, 0);
    IUFillNumber(&AlignmentPointSetEntry[4], "ALIGNMENT_POINT_ENTRY_VECTOR_X", "Telescope direction vector x", "%g", 0, 0, 0, 0);
    IUFillNumber(&AlignmentPointSetEntry[5]," ALIGNMENT_POINT_ENTRY_VECTOR_Y", "Telescope direction vector y", "%g", 0, 0, 0, 0);
    IUFillNumber(&AlignmentPointSetEntry[6]," ALIGNMENT_POINT_ENTRY_VECTOR_Z", "Telescope direction vector z", "%g", 0, 0, 0, 0);
    IUFillNumberVector(&AlignmentPointSetEntryV, AlignmentPointSetEntry, 7, ChildTelescope->getDeviceName(),
                    "ALIGNMENT_POINT_MANDATORY_NUMBERS", "Mandatory sync point numeric fields", ALIGNMENT_TAB, IP_RW, 60, IPS_IDLE);
    ChildTelescope->registerProperty(&AlignmentPointSetEntryV, INDI_NUMBER);

    IUFillBLOB(&AlignmentPointSetPrivateBinaryData, "ALIGNMENT_POINT_ENTRY_PRIVATE", "Private binary data", "");
    IUFillBLOBVector(&AlignmentPointSetPrivateBinaryDataV, &AlignmentPointSetPrivateBinaryData, 1, ChildTelescope->getDeviceName(),
                    "ALIGNMENT_POINT_OPTIONAL_BINARY_BLOB", "Optional sync point binary data", ALIGNMENT_TAB, IP_RW, 60, IPS_IDLE);
    ChildTelescope->registerProperty(&AlignmentPointSetPrivateBinaryDataV, INDI_BLOB);

    IUFillNumber(&AlignmentPointSetSize, "ALIGNMENT_POINTSET_SIZE", "Size", "%g", 0, 100000, 0, 0);
    IUFillNumberVector(&AlignmentPointSetSizeV, &AlignmentPointSetSize, 1, ChildTelescope->getDeviceName(),
                    "ALIGNMENT_POINTSET_SIZE", "Current Set", ALIGNMENT_TAB, IP_RO, 60, IPS_IDLE);
    ChildTelescope->registerProperty(&AlignmentPointSetSizeV, INDI_NUMBER);

    IUFillNumber(&AlignmentPointSetPointer, "ALIGNMENT_POINTSET_CURRENT_ENTRY", "Pointer", "%g", 0, 100000, 0, 0);
    IUFillNumberVector(&AlignmentPointSetPointerV, &AlignmentPointSetPointer, 1, ChildTelescope->getDeviceName(),
                    "ALIGNMENT_POINTSET_CURRENT_ENTRY", "Current Set", ALIGNMENT_TAB, IP_RW, 60, IPS_IDLE);
    ChildTelescope->registerProperty(&AlignmentPointSetPointerV, INDI_NUMBER);

    IUFillSwitch(&AlignmentPointSetAction[0], "APPEND", "Add entries at end of set", ISS_ON);
    IUFillSwitch(&AlignmentPointSetAction[1], "INSERT", "Insert entries at current index", ISS_OFF);
    IUFillSwitch(&AlignmentPointSetAction[2], "EDIT", "Overwrite entry at current index", ISS_OFF);
    IUFillSwitch(&AlignmentPointSetAction[3], "DELETE", "Delete entry at current index", ISS_OFF);
    IUFillSwitch(&AlignmentPointSetAction[4], "CLEAR","Delete all the entries in the set", ISS_OFF);
    IUFillSwitch(&AlignmentPointSetAction[5], "READ", "Read the entry at the current pointer", ISS_OFF);
    IUFillSwitch(&AlignmentPointSetAction[6], "READ INCREMENT", "Increment the pointer after reading the entry", ISS_OFF);
    IUFillSwitchVector(&AlignmentPointSetActionV, AlignmentPointSetAction, 7, ChildTelescope->getDeviceName(),
                    "ALIGNMENT_POINTSET_ACTION", "Action to take", ALIGNMENT_TAB, IP_RW, ISR_1OFMANY, 60, IPS_IDLE);
    ChildTelescope->registerProperty(&AlignmentPointSetActionV, INDI_SWITCH);

    IUFillSwitch(&AlignmentPointSetCommit, "ALIGNMENT_POINTSET_COMMIT", "Execute the action", ISS_ON);
    IUFillSwitchVector(&AlignmentPointSetCommitV, &AlignmentPointSetCommit, 1, ChildTelescope->getDeviceName(),
                    "ALIGNMENT_POINTSET_COMMIT", "Execute", ALIGNMENT_TAB, IP_RW, ISR_1OFMANY, 60, IPS_IDLE);
    ChildTelescope->registerProperty(&AlignmentPointSetCommitV, INDI_SWITCH);
}

void INDI::AlignmentSubsystemDriver::ProcessAlignmentNumberProperties(Telescope* pTelescope, const char *name, double values[], char *names[], int n)
{
    DEBUGFDEVICE(pTelescope->getDeviceName(), INDI::Logger::DBG_DEBUG, "ProcessAlignmentNumberProperties - name(%s)", name);
    if (strcmp(name, AlignmentPointSetEntryV.name) == 0)
    {
        AlignmentPointSetEntryV.s=IPS_OK;
        IUUpdateNumber(&AlignmentPointSetEntryV, values, names, n);
        //  Update client display
        IDSetNumber(&AlignmentPointSetEntryV, NULL);
    } else if (strcmp(name, AlignmentPointSetPointerV.name) == 0)
    {
        AlignmentPointSetPointerV.s=IPS_OK;
        IUUpdateNumber(&AlignmentPointSetPointerV, values, names, n);
        //  Update client display
        IDSetNumber(&AlignmentPointSetPointerV, NULL);
    }
}

void INDI::AlignmentSubsystemDriver::ProcessAlignmentSwitchProperties(Telescope* pTelescope, const char *name, ISState *states, char *names[], int n)
{
    DEBUGFDEVICE(pTelescope->getDeviceName(), INDI::Logger::DBG_DEBUG, "ProcessAlignmentSwitchProperties - name(%s)", name);
    if (strcmp(name, AlignmentPointSetActionV.name) == 0)
    {
        AlignmentPointSetActionV.s=IPS_OK;
        IUUpdateSwitch(&AlignmentPointSetActionV, states, names, n);
        //  Update client display
        IDSetSwitch(&AlignmentPointSetActionV, NULL);
    } else if (strcmp(name, AlignmentPointSetCommitV.name) == 0)
    {
        // Perform the databse action

        AlignmentPointSetCommitV.s=IPS_OK;
        IUUpdateSwitch(&AlignmentPointSetCommitV, states, names, n);
        //  Update client display
        IDSetSwitch(&AlignmentPointSetCommitV, NULL);
    }
}

void INDI::AlignmentSubsystemDriver::ProcessAlignmentBlobProperties(Telescope* pTelescope, const char *name, int sizes[], int blobsizes[], char *blobs[], char *formats[], char *names[], int n)
{
    DEBUGFDEVICE(pTelescope->getDeviceName(), INDI::Logger::DBG_DEBUG, "ProcessAlignmentBlobProperties - name(%s)", name);
}

bool INDI::AlignmentSubsystemDriver::AppendSyncPoint(const double ObservationTime, const double RightAscension, const double Declination,
                    const TelescopeDirectionVector& TelescopeDirectionVector)
{
    return false;
}

bool INDI::AlignmentSubsystemDriver::InsertSyncPoint(unsigned int Offset, const double ObservationTime, const double RightAscension, const double Declination,
                    const TelescopeDirectionVector& TelescopeDirectionVector)
{
    return false;
}

bool INDI::AlignmentSubsystemDriver::EditSyncPoint(unsigned int Offset, const double ObservationTime, const double RightAscension, const double Declination,
                    const TelescopeDirectionVector& TelescopeDirectionVector)
{
    return false;
}

bool INDI::AlignmentSubsystemDriver::DeleteSyncPoint(unsigned int Offset)
{
    return false;
}

bool INDI::AlignmentSubsystemDriver::ClearSyncPoints()
{
    return false;
}

bool INDI::AlignmentSubsystemDriver::ReadSyncPoint(unsigned int Offset, const double ObservationTime, const double RightAscension, const double Declination,
                    const TelescopeDirectionVector& TelescopeDirectionVector)
{
    return false;
}

bool INDI::AlignmentSubsystemDriver::ReadNextSyncPoint(const double ObservationTime, const double RightAscension, const double Declination,
                    const TelescopeDirectionVector& TelescopeDirectionVector)
{
    return false;
}

