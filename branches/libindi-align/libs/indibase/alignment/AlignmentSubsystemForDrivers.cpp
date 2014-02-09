/*!
 * \file AlignmentSubsystemForDrivers.cpp
 *
 * \author Roger James
 * \date 13th November 2013
 *
 */

#include "AlignmentSubsystemForClients.h"

#include "AlignmentSubsystemForDrivers.h"

namespace INDI {
namespace AlignmentSubsystem {

AlignmentSubsystemForDrivers::AlignmentSubsystemForDrivers()
{
    // Set up the in memory database pointer for math plugins
    SetCurrentInMemoryDatabase(this);
    // Fix up the database load callback
    SetLoadDatabaseCallback(&MyDatabaseLoadCallback, this);
}

// Public methods

void AlignmentSubsystemForDrivers::InitProperties(Telescope* pTelescope)
{
    MapPropertiesToInMemoryDatabase::InitProperties(pTelescope);
    MathPluginManagement::InitProperties(pTelescope);
}

void AlignmentSubsystemForDrivers::ProcessBlobProperties(Telescope* pTelescope, const char *name, int sizes[], int blobsizes[], char *blobs[], char *formats[], char *names[], int n)
{
    MapPropertiesToInMemoryDatabase::ProcessBlobProperties(pTelescope, name, sizes, blobsizes, blobs, formats, names, n);
}

void AlignmentSubsystemForDrivers::ProcessNumberProperties(Telescope* pTelescope, const char *name, double values[], char *names[], int n)
{
    MapPropertiesToInMemoryDatabase::ProcessNumberProperties(pTelescope, name, values, names, n);
}

void AlignmentSubsystemForDrivers::ProcessSwitchProperties(Telescope* pTelescope, const char *name, ISState *states, char *names[], int n)
{
    MapPropertiesToInMemoryDatabase::ProcessSwitchProperties(pTelescope, name, states, names, n);
    MathPluginManagement::ProcessSwitchProperties(pTelescope, name, states, names, n);
}

void AlignmentSubsystemForDrivers::ProcessTextProperties(Telescope* pTelescope, const char *name, char *texts[], char *names[], int n)
{
    MathPluginManagement::ProcessTextProperties(pTelescope, name, texts, names, n);
}

void AlignmentSubsystemForDrivers::SaveConfigProperties(FILE *fp)
{
    MathPluginManagement::SaveConfigProperties(fp);
}

void AlignmentSubsystemForDrivers::SetApproximateMountAlignmentFromMountType(MountType_t Type)
{
    if (EQUATORIAL == Type)
    {
        ln_lnlat_posn Position;
        if (GetDatabaseReferencePosition(Position))
        {
            if (Position.lat >= 0)
                SetApproximateMountAlignment(NORTH_CELESTIAL_POLE);
            else
                SetApproximateMountAlignment(SOUTH_CELESTIAL_POLE);
        }
        //else
        // TODO some kind of error!!!
    }
    else
        SetApproximateMountAlignment(ZENITH);
}

// Private methods

void AlignmentSubsystemForDrivers::MyDatabaseLoadCallback(void *ThisPointer)
{
    ((AlignmentSubsystemForDrivers*)ThisPointer)->Initialise((AlignmentSubsystemForDrivers*)ThisPointer);
}

} // namespace AlignmentSubsystem
} // namespace INDI
