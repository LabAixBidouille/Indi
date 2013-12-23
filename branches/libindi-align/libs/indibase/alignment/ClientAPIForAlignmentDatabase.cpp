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

void ClientAPIForAlignmentDatabase::ProcessNewDevice(INDI::BaseDevice *DevicePointer)
{
    Device = DevicePointer;
}

void ClientAPIForAlignmentDatabase::ProcessNewProperty(INDI::Property *PropertyPointer)
{
    int ReturnCode;

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

    // Tell the client
    if ((NULL != MandatoryNumbers)
                && (NULL != OptionalBinaryBlob)
                && (NULL != PointsetSize)
                && (NULL != CurrentEntry)
                && (NULL != Action)
                && (NULL != Commit))
    {
        SignalDriverCompletion();
    }
}

bool ClientAPIForAlignmentDatabase::AppendSyncPoint(const AlignmentDatabaseEntry& CurrentValues)
{
    WaitForDriverCompletion();
    if (APPEND != IUFindOnSwitchIndex(Action->getSwitch()))
    {
    }
}

bool ClientAPIForAlignmentDatabase::WaitForDriverCompletion()
{
    int ReturnCode;
    ReturnCode = pthread_mutex_lock(&DriverActionCompleteMutex);
    while(!DriverActionComplete)
    {
        ReturnCode = pthread_cond_wait(&DriverActionCompleteCondition, &DriverActionCompleteMutex);
        if (ReturnCode)
        {
            ReturnCode = pthread_mutex_unlock(&DriverActionCompleteMutex);
            return false;
        }
    }
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
        return false;
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
    ReturnCode = pthread_mutex_unlock(&DriverActionCompleteMutex);
    if (ReturnCode)
        return false;
    else
        return true;
}

} // namespace AlignmentSubsystem
} // namespace INDI
