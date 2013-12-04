/*!
 * \file AlignmentSubsystemBase.cpp
 *
 * \author Roger James
 * \date 13th November 2013
 *
 * This file contains a declarations of the functionality for an alignment subsystem to be used
 * alongside the INDI::Telescope class in drivers and directly in clients.
 */


 #include "AlignmentSubsystem.h"

INDI::AlignmentSubsystemBase::SyncPointsType INDI::AlignmentSubsystemBase::SyncPoints;

bool INDI::AlignmentSubsystemBase::AppendSyncPoint(const AlignmentDatabaseEntry& CurrentValues)
{
    SyncPoints.push_back(CurrentValues);
    return true;
}

bool INDI::AlignmentSubsystemBase::InsertSyncPoint(unsigned int Offset, const AlignmentDatabaseEntry& CurrentValues)
{
    if ((Offset < 0) || (Offset > SyncPoints.size()))
        return false;
    SyncPoints.insert(SyncPoints.begin() + Offset, CurrentValues);
    return true;
}

bool INDI::AlignmentSubsystemBase::EditSyncPoint(unsigned int Offset, const AlignmentDatabaseEntry& CurrentValues)
{
    if ((Offset < 0) || (Offset >= SyncPoints.size()))
        return false;
    SyncPoints[Offset] = CurrentValues;
    return true;
}

bool INDI::AlignmentSubsystemBase::DeleteSyncPoint(unsigned int Offset)
{
    if ((Offset < 0) || (Offset >= SyncPoints.size()))
        return false;
    SyncPoints.erase(SyncPoints.begin() + Offset);
    return true;
}

bool INDI::AlignmentSubsystemBase::ClearSyncPoints()
{
    SyncPoints.clear();
    return true;
}

bool INDI::AlignmentSubsystemBase::ReadSyncPoint(unsigned int Offset, AlignmentDatabaseEntry& CurrentValues)
{

    if ((Offset < 0) || (Offset >= SyncPoints.size()))
        return false;
    CurrentValues = SyncPoints[Offset];
    return true;
}

const int INDI::AlignmentSubsystemBase::GetDatabaseSize()
{
    return SyncPoints.size();
}


