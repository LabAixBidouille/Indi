/*!
 * \file MapPropertiesToInMemoryDatabase.cpp
 *
 * \author Roger James
 * \date 13th November 2013
 *
 */

#include "MapPropertiesToInMemoryDatabase.h"

#include <cfloat>

namespace INDI {
namespace AlignmentSubsystem {

void MapPropertiesToInMemoryDatabase::InitProperties(Telescope* pTelescope)
{
    IUFillNumber(&AlignmentPointSetEntry[0], "ALIGNMENT_POINT_ENTRY_OBSERVATION_JULIAN_DATE", "Observation Julian date", "%g", 0, 60000, 0, 0);
    IUFillNumber(&AlignmentPointSetEntry[1], "ALIGNMENT_POINT_ENTRY_OBSERVATION_LOCAL_SIDEREAL_TIME", "Observation local sidereal time (hh:mm:ss.ss)", "%010.9m", 0, 24, 0, 0);
    IUFillNumber(&AlignmentPointSetEntry[2], "ALIGNMENT_POINT_ENTRY_RA", "Right Ascension (hh:mm:ss)", "%010.6m", 0, 24, 0, 0);
    IUFillNumber(&AlignmentPointSetEntry[3]," ALIGNMENT_POINT_ENTRY_DEC", "Declination (dd:mm:ss)", "%010.6m", -90, 90, 0, 0);
    IUFillNumber(&AlignmentPointSetEntry[4], "ALIGNMENT_POINT_ENTRY_VECTOR_X", "Telescope direction vector x", "%g", -FLT_MAX, FLT_MAX, 0, 0);
    IUFillNumber(&AlignmentPointSetEntry[5]," ALIGNMENT_POINT_ENTRY_VECTOR_Y", "Telescope direction vector y", "%g", -FLT_MAX, FLT_MAX, 0, 0);
    IUFillNumber(&AlignmentPointSetEntry[6]," ALIGNMENT_POINT_ENTRY_VECTOR_Z", "Telescope direction vector z", "%g", -FLT_MAX, FLT_MAX, 0, 0);
    IUFillNumberVector(&AlignmentPointSetEntryV, AlignmentPointSetEntry, 7, pTelescope->getDeviceName(),
                    "ALIGNMENT_POINT_MANDATORY_NUMBERS", "Mandatory sync point numeric fields", ALIGNMENT_TAB, IP_RW, 60, IPS_IDLE);
    pTelescope->registerProperty(&AlignmentPointSetEntryV, INDI_NUMBER);

    IUFillBLOB(&AlignmentPointSetPrivateBinaryData, "ALIGNMENT_POINT_ENTRY_PRIVATE", "Private binary data", "");
    IUFillBLOBVector(&AlignmentPointSetPrivateBinaryDataV, &AlignmentPointSetPrivateBinaryData, 1, pTelescope->getDeviceName(),
                    "ALIGNMENT_POINT_OPTIONAL_BINARY_BLOB", "Optional sync point binary data", ALIGNMENT_TAB, IP_RW, 60, IPS_IDLE);
    pTelescope->registerProperty(&AlignmentPointSetPrivateBinaryDataV, INDI_BLOB);

    IUFillNumber(&AlignmentPointSetSize, "ALIGNMENT_POINTSET_SIZE", "Size", "%g", 0, 100000, 0, 0);
    IUFillNumberVector(&AlignmentPointSetSizeV, &AlignmentPointSetSize, 1, pTelescope->getDeviceName(),
                    "ALIGNMENT_POINTSET_SIZE", "Current Set", ALIGNMENT_TAB, IP_RO, 60, IPS_IDLE);
    pTelescope->registerProperty(&AlignmentPointSetSizeV, INDI_NUMBER);

    IUFillNumber(&AlignmentPointSetPointer, "ALIGNMENT_POINTSET_CURRENT_ENTRY", "Pointer", "%g", 0, 100000, 0, 0);
    IUFillNumberVector(&AlignmentPointSetPointerV, &AlignmentPointSetPointer, 1, pTelescope->getDeviceName(),
                    "ALIGNMENT_POINTSET_CURRENT_ENTRY", "Current Set", ALIGNMENT_TAB, IP_RW, 60, IPS_IDLE);
    pTelescope->registerProperty(&AlignmentPointSetPointerV, INDI_NUMBER);

    IUFillSwitch(&AlignmentPointSetAction[0], "APPEND", "Add entries at end of set", ISS_ON);
    IUFillSwitch(&AlignmentPointSetAction[1], "INSERT", "Insert entries at current index", ISS_OFF);
    IUFillSwitch(&AlignmentPointSetAction[2], "EDIT", "Overwrite entry at current index", ISS_OFF);
    IUFillSwitch(&AlignmentPointSetAction[3], "DELETE", "Delete entry at current index", ISS_OFF);
    IUFillSwitch(&AlignmentPointSetAction[4], "CLEAR","Delete all the entries in the set", ISS_OFF);
    IUFillSwitch(&AlignmentPointSetAction[5], "READ", "Read the entry at the current pointer", ISS_OFF);
    IUFillSwitch(&AlignmentPointSetAction[6], "READ INCREMENT", "Increment the pointer after reading the entry", ISS_OFF);
    IUFillSwitch(&AlignmentPointSetAction[7], "LOAD DATABASE", "Load the alignment database from local storage", ISS_OFF);
    IUFillSwitch(&AlignmentPointSetAction[8], "SAVE DATABASE", "Save the alignment database to local storage", ISS_OFF);
    IUFillSwitchVector(&AlignmentPointSetActionV, AlignmentPointSetAction, 9, pTelescope->getDeviceName(),
                    "ALIGNMENT_POINTSET_ACTION", "Action to take", ALIGNMENT_TAB, IP_RW, ISR_1OFMANY, 60, IPS_IDLE);
    pTelescope->registerProperty(&AlignmentPointSetActionV, INDI_SWITCH);

    IUFillSwitch(&AlignmentPointSetCommit, "ALIGNMENT_POINTSET_COMMIT", "Execute the action", ISS_ON);
    IUFillSwitchVector(&AlignmentPointSetCommitV, &AlignmentPointSetCommit, 1, pTelescope->getDeviceName(),
                    "ALIGNMENT_POINTSET_COMMIT", "Execute", ALIGNMENT_TAB, IP_RW, ISR_1OFMANY, 60, IPS_IDLE);
    pTelescope->registerProperty(&AlignmentPointSetCommitV, INDI_SWITCH);
}

void MapPropertiesToInMemoryDatabase::ProcessNumberProperties(Telescope* pTelescope, const char *name, double values[], char *names[], int n)
{
    DEBUGFDEVICE(pTelescope->getDeviceName(), INDI::Logger::DBG_DEBUG, "ProcessAlignmentNumberProperties - name(%s)", name);
    if (strcmp(name, AlignmentPointSetEntryV.name) == 0)
    {
        AlignmentPointSetEntryV.s = IPS_OK;
        IUUpdateNumber(&AlignmentPointSetEntryV, values, names, n);
        //  Update client display
        IDSetNumber(&AlignmentPointSetEntryV, NULL);
    } else if (strcmp(name, AlignmentPointSetPointerV.name) == 0)
    {
        AlignmentPointSetPointerV.s = IPS_OK;
        IUUpdateNumber(&AlignmentPointSetPointerV, values, names, n);
        //  Update client display
        IDSetNumber(&AlignmentPointSetPointerV, NULL);
    }
}


void MapPropertiesToInMemoryDatabase::ProcessSwitchProperties(Telescope* pTelescope, const char *name, ISState *states, char *names[], int n)
{
    DEBUGFDEVICE(pTelescope->getDeviceName(), INDI::Logger::DBG_DEBUG, "ProcessAlignmentSwitchProperties - name(%s)", name);
    AlignmentDatabaseType& AlignmentDatabase = GetAlignmentDatabase();
    if (strcmp(name, AlignmentPointSetActionV.name) == 0)
    {
        AlignmentPointSetActionV.s=IPS_OK;
        IUUpdateSwitch(&AlignmentPointSetActionV, states, names, n);

        //  Update client display
        IDSetSwitch(&AlignmentPointSetActionV, NULL);
    } else if (strcmp(name, AlignmentPointSetCommitV.name) == 0)
    {
        unsigned int Offset = AlignmentPointSetPointer.value;
        AlignmentPointSetCommitV.s=IPS_OK;

        // Perform the database action
        AlignmentDatabaseEntry CurrentValues;
        CurrentValues.ObservationDate = AlignmentPointSetEntry[ENTRY_OBSERVATION_JULIAN_DATE].value;
        CurrentValues.ObservationTime = AlignmentPointSetEntry[ENTRY_OBSERVATION_LOCAL_SIDEREAL_TIME].value;
        CurrentValues.RightAscension = AlignmentPointSetEntry[ENTRY_RA].value;
        CurrentValues.ObservationTime = AlignmentPointSetEntry[ENTRY_DEC].value;
        CurrentValues.TelescopeDirection.x = AlignmentPointSetEntry[ENTRY_VECTOR_X].value;
        CurrentValues.TelescopeDirection.y = AlignmentPointSetEntry[ENTRY_VECTOR_Y].value;
        CurrentValues.TelescopeDirection.z = AlignmentPointSetEntry[ENTRY_VECTOR_Z].value;

        if (AlignmentPointSetAction[APPEND].s == ISS_ON)
        {
            AlignmentDatabase.push_back(CurrentValues);
            AlignmentPointSetSize.value = AlignmentDatabase.size();
            //  Update client display
            IDSetNumber(&AlignmentPointSetSizeV, NULL);
        }
        else if (AlignmentPointSetAction[INSERT].s == ISS_ON)
        {
            if ((Offset < 0) || (Offset > AlignmentDatabase.size()))
                AlignmentPointSetCommitV.s=IPS_ALERT;
            else
            {
                AlignmentDatabase.insert(AlignmentDatabase.begin() + Offset, CurrentValues);
                AlignmentPointSetSize.value = AlignmentDatabase.size();
                //  Update client display
                IDSetNumber(&AlignmentPointSetSizeV, NULL);
            }
        }
        else if (AlignmentPointSetAction[EDIT].s == ISS_ON)
        {
            if ((Offset < 0) || (Offset >= AlignmentDatabase.size()))
                AlignmentPointSetCommitV.s=IPS_ALERT;
            else
                AlignmentDatabase[Offset] = CurrentValues;
        }
        else if (AlignmentPointSetAction[DELETE].s == ISS_ON)
        {
            if ((Offset < 0) || (Offset >= AlignmentDatabase.size()))
                AlignmentPointSetCommitV.s=IPS_ALERT;
            else
            {
                AlignmentDatabase.erase(AlignmentDatabase.begin() + Offset);
                AlignmentPointSetSize.value = AlignmentDatabase.size();
                //  Update client display
                IDSetNumber(&AlignmentPointSetSizeV, NULL);
            }
        }
        else if (AlignmentPointSetAction[CLEAR].s == ISS_ON)
        {
            // AlignmentDatabaseType().swap(AlignmentDatabase); // Do it this wasy to force a reallocation
            AlignmentDatabase.clear();
            AlignmentPointSetSize.value = 0;
            //  Update client display
            IDSetNumber(&AlignmentPointSetSizeV, NULL);
        }
        else if ((AlignmentPointSetAction[READ].s == ISS_ON) || (AlignmentPointSetAction[READ_INCREMENT].s == ISS_ON))
        {
            if ((Offset < 0) || (Offset >= AlignmentDatabase.size()))
                AlignmentPointSetCommitV.s=IPS_ALERT;
            else
            {
                AlignmentPointSetEntry[ENTRY_OBSERVATION_JULIAN_DATE].value = AlignmentDatabase[Offset].ObservationDate;
                AlignmentPointSetEntry[ENTRY_OBSERVATION_LOCAL_SIDEREAL_TIME].value = AlignmentDatabase[Offset].ObservationTime;
                AlignmentPointSetEntry[ENTRY_RA].value = AlignmentDatabase[Offset].RightAscension;
                AlignmentPointSetEntry[ENTRY_DEC].value = AlignmentDatabase[Offset].ObservationTime;
                AlignmentPointSetEntry[ENTRY_VECTOR_X].value = AlignmentDatabase[Offset].TelescopeDirection.x;
                AlignmentPointSetEntry[ENTRY_VECTOR_Y].value = AlignmentDatabase[Offset].TelescopeDirection.y;
                AlignmentPointSetEntry[ENTRY_VECTOR_Z].value= AlignmentDatabase[Offset].TelescopeDirection.z;

                //  Update client display
                IDSetNumber(&AlignmentPointSetEntryV, NULL);
                if (AlignmentPointSetAction[READ_INCREMENT].s == ISS_ON)
                {
                    AlignmentPointSetPointer.value++;
                    IDSetNumber(&AlignmentPointSetPointerV, NULL);
                }
            }
        }
        else if (AlignmentPointSetAction[LOAD_DATABASE].s == ISS_ON)
        {
            LoadDatabase(pTelescope->getDeviceName());
            AlignmentPointSetSize.value = AlignmentDatabase.size();
            //  Update client display
            IDSetNumber(&AlignmentPointSetSizeV, NULL);
        }
        else if (AlignmentPointSetAction[SAVE_DATABASE].s == ISS_ON)
        {
            SaveDatabase(pTelescope->getDeviceName());
        }

        //  Update client display
        IUUpdateSwitch(&AlignmentPointSetCommitV, states, names, n);
        IDSetSwitch(&AlignmentPointSetCommitV, NULL);
    }
}


void MapPropertiesToInMemoryDatabase::ProcessBlobProperties(Telescope* pTelescope, const char *name, int sizes[], int blobsizes[], char *blobs[], char *formats[], char *names[], int n)
{
    // No BLOB support yet
}

} // namespace AlignmentSubsystem
} // namespace INDI
