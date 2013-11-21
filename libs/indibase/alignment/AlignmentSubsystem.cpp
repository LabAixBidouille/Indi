/*!
 * \file AlignmentSubsystem.cpp
 *
 * \author Roger James
 * \date 13th November 2013
 *
 * This file contains implementations of functions for the alignment subsystem.
 */

#include "AlignmentSubsystem.h"

void INDI::AlignmentSubsystemDriver::InitAlignmentProperties(const char *deviceName, const char* groupName)
{

    IUFillNumber(&SyncDatabaseEntry[0],"ALIGNMENT_POINT_ENTRY_OBSERVATION_DATE","Observation date (dd:mm:yy)","%010.6m",0,60000,0,0);
    IUFillNumber(&SyncDatabaseEntry[1],"ALIGNMENT_POINT_ENTRY_OBSERVATION_TIME","Observation time (hh:mm:ss)","%010.6m",0,24,0,0);
    IUFillNumber(&SyncDatabaseEntry[2],"ALIGNMENT_POINT_ENTRY_RA","Right Ascension (hh:mm:ss)","%010.6m",0,24,0,0);
    IUFillNumber(&SyncDatabaseEntry[3],"ALIGNMENT_POINT_ENTRY_DEC","Declination (dd:mm:ss)","%010.6m",-90,90,0,0);
    IUFillNumber(&SyncDatabaseEntry[4],"ALIGNMENT_POINT_ENTRY_VECTOR_X","Observation data (dd:mm:yy)","%g",0,0,0,0);
    IUFillNumber(&SyncDatabaseEntry[5],"ALIGNMENT_POINT_ENTRY_VECTOR_Y","Observation time (hh:mm:ss)","%g",0,0,0,0);
    IUFillNumber(&SyncDatabaseEntry[6],"ALIGNMENT_POINT_ENTRY_VECTOR_Z","Observation time (hh:mm:ss)","%g",0,0,0,0);
    IUFillNumberVector(&SyncDataBaseNumbers,SyncDatabaseEntry,7,deviceName,
                    "ALIGNMENT_POINT_MANDATORY_NUMBERS","Mandatory sync point numeric fields",groupName,IP_RW,60,IPS_IDLE);

    IUFillBLOB(&SyncDatabasePrivateBinaryData,"ALIGNMENT_POINT_ENTRY_PRIVATE","Private binary data (optional)","");
    IUFillBLOBVector(&SyncDatabaseBlobs,&SyncDatabasePrivateBinaryData,1,deviceName,
                    "ALIGNMENT_POINT_OPTIONAL_BINARY_BLOB","Optional sync point binary data",groupName,IP_RW,60,IPS_IDLE);
}

void INDI::AlignmentSubsystemDriver::ProcessAlignmentProperties(const char *name, double values[], char *names[], int n)
{
}
