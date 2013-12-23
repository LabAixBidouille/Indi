/*!
 * \file ClientAPIForAlignmentDatabase.cpp
 *
 * \author Roger James
 * \date 13th November 2013
 *
 */

#include "ClientAPIForAlignmentDatabase.h"

#include <cstring>

namespace INDI {
namespace AlignmentSubsystem {

ClientAPIForAlignmentDatabase::ClientAPIForAlignmentDatabase() :   Device(NULL),
                                    MandatoryNumbers(NULL),
                                    OptionalBinaryBlob(NULL),
                                    PointsetSize(NULL),
                                    CurrentEntry(NULL),
                                    Action(NULL),
                                    Commit(NULL),
                                    DriverActionComplete(false)
{
    pthread_cond_init(&DriverActionCompleteCondition, NULL);
    pthread_mutex_init(&DriverActionCompleteMutex, NULL);
}

 ClientAPIForAlignmentDatabase::~ClientAPIForAlignmentDatabase()
 {
    pthread_cond_destroy(&DriverActionCompleteCondition);
    pthread_mutex_destroy(&DriverActionCompleteMutex);
 }

void ClientAPIForAlignmentDatabase::Initialise(INDI::BaseClient *BaseClient)
{
    ClientAPIForAlignmentDatabase::BaseClient = BaseClient;
}

void ClientAPIForAlignmentDatabase::ProcessNewDevice(INDI::BaseDevice *DevicePointer)
{
    Device = DevicePointer;
}

void ClientAPIForAlignmentDatabase::ProcessNewProperty(INDI::Property *PropertyPointer)
{
    int ReturnCode;
    bool GotOneOfMine = true;

    if (!strcmp(PropertyPointer->getName(), "ALIGNMENT_POINT_MANDATORY_NUMBERS"))
        MandatoryNumbers = PropertyPointer;
    else if (!strcmp(PropertyPointer->getName(), "ALIGNMENT_POINT_OPTIONAL_BINARY_BLOB"))
        OptionalBinaryBlob = PropertyPointer;
    else if (!strcmp(PropertyPointer->getName(), "ALIGNMENT_POINTSET_SIZE"))
        PointsetSize = PropertyPointer;
    else if (!strcmp(PropertyPointer->getName(), "ALIGNMENT_POINTSET_CURRENT_ENTRY"))
        CurrentEntry = PropertyPointer;
    else if (!strcmp(PropertyPointer->getName(), "ALIGNMENT_POINTSET_ACTION"))
        Action = PropertyPointer;
    else if (!strcmp(PropertyPointer->getName(), "ALIGNMENT_POINTSET_COMMIT"))
        Commit = PropertyPointer;
    else
        GotOneOfMine = false;

    // Tell the client when all the database proeprties have been set up
    if (GotOneOfMine
                && (NULL != MandatoryNumbers)
                && (NULL != OptionalBinaryBlob)
                && (NULL != PointsetSize)
                && (NULL != CurrentEntry)
                && (NULL != Action)
                && (NULL != Commit))
    {
        // The DriverActionComplete state variable is initialised to false
        // So I need to call this to set it to true and signal anyone
        // waiting for the driver to initialise etc.
        SignalDriverCompletion();
    }
}

void ClientAPIForAlignmentDatabase::ProcessNewSwitch(ISwitchVectorProperty *SwitchVectorProperty)
{
    if (!strcmp(SwitchVectorProperty->name, "ALIGNMENT_POINTSET_ACTION"))
    {
        if ((IPS_OK == SwitchVectorProperty->s) || (IPS_ALERT == SwitchVectorProperty->s))
            SignalDriverCompletion();
    }
}

void ClientAPIForAlignmentDatabase::ProcessNewNumber(INumberVectorProperty *NumberVectorProperty)
{
    if (!strcmp(NumberVectorProperty->name, "ALIGNMENT_POINT_MANDATORY_NUMBERS"))
    {
        if ((IPS_OK == NumberVectorProperty->s) || (IPS_ALERT == NumberVectorProperty->s))
            SignalDriverCompletion();
    }
}

bool ClientAPIForAlignmentDatabase::AppendSyncPoint(const AlignmentDatabaseEntry& CurrentValues)
{
    // Wait for driver to initialise if neccessary
    WaitForDriverCompletion();

    ISwitchVectorProperty *pSwitch = Action->getSwitch();
    INumberVectorProperty *pNumber = MandatoryNumbers->getNumber();


    if (APPEND != IUFindOnSwitchIndex(pSwitch))
    {
        // Request Append mode
        IUResetSwitch(pSwitch);
        pSwitch->sp[APPEND].s = ISS_ON;
        SetDriverBusy();
        BaseClient->sendNewSwitch(pSwitch);
        WaitForDriverCompletion();
        if (IPS_OK != pSwitch->s)
        {
            IDLog("AppendSyncPoint - Bad Action switch state %s\n", pstateStr(pSwitch->s));
            return false;
        }
    }

    pNumber->np[ENTRY_OBSERVATION_JULIAN_DATE].value = CurrentValues.ObservationDate;
    pNumber->np[ENTRY_OBSERVATION_LOCAL_SIDEREAL_TIME].value  = CurrentValues.ObservationTime;
    pNumber->np[ENTRY_RA].value  = CurrentValues.RightAscension;
    pNumber->np[ENTRY_DEC].value  = CurrentValues.Declination;
    pNumber->np[ENTRY_VECTOR_X].value  = CurrentValues.TelescopeDirection.x;
    pNumber->np[ENTRY_VECTOR_Y].value  = CurrentValues.TelescopeDirection.y;
    pNumber->np[ENTRY_VECTOR_Z].value  = CurrentValues.TelescopeDirection.z;
    SetDriverBusy();
    BaseClient->sendNewNumber(pNumber);
    WaitForDriverCompletion();
    if (IPS_OK != pNumber->s)
    {
        IDLog("AppendSyncPoint - Bad mandatory numbers state %s\n", pstateStr(pSwitch->s));
        return false;
    }
    return true;
}

bool ClientAPIForAlignmentDatabase::WaitForDriverCompletion()
{
    int ReturnCode;
    ReturnCode = pthread_mutex_lock(&DriverActionCompleteMutex);
    while(!DriverActionComplete)
    {
        IDLog("WaitForDriverCompletion - Waiting\n");
        ReturnCode = pthread_cond_wait(&DriverActionCompleteCondition, &DriverActionCompleteMutex);
        IDLog("WaitForDriverCompletion - Back from wait ReturnCode = %d\n", ReturnCode);
        if (ReturnCode)
        {
            ReturnCode = pthread_mutex_unlock(&DriverActionCompleteMutex);
            return false;
        }
    }
    IDLog("WaitForDriverCompletion - Finished waiting\n");
    ReturnCode = pthread_mutex_unlock(&DriverActionCompleteMutex);
    if (ReturnCode)
        return false;
    else
        return true;
}

bool ClientAPIForAlignmentDatabase::SignalDriverCompletion()
{
    int ReturnCode;
    ReturnCode = pthread_mutex_lock(&DriverActionCompleteMutex);
    if (ReturnCode)
        return false;
    DriverActionComplete = true;
    ReturnCode = pthread_cond_signal(&DriverActionCompleteCondition);
    if (ReturnCode)
    {
        ReturnCode = pthread_mutex_unlock(&DriverActionCompleteMutex);
        return false;
    }
    IDLog("SignalDriverCompletion\n");
    ReturnCode = pthread_mutex_unlock(&DriverActionCompleteMutex);
    if (ReturnCode)
        return false;
    else
        return true;
}

bool ClientAPIForAlignmentDatabase::SetDriverBusy()
{
    int ReturnCode;
    ReturnCode = pthread_mutex_lock(&DriverActionCompleteMutex);
    if (ReturnCode)
        return false;
    DriverActionComplete = false;
    IDLog("SetDriverBusy\n");
    ReturnCode = pthread_mutex_unlock(&DriverActionCompleteMutex);
    if (ReturnCode)
        return false;
    else
        return true;
}

} // namespace AlignmentSubsystem
} // namespace INDI
