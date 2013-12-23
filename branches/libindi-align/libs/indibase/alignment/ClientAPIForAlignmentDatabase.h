/*!
 * \file ClientAPIForAlignmentDatabase.h
 *
 * \author Roger James
 * \date 13th November 2013
 *
 */

#ifndef INDI_ALIGNMENTSUBSYSTEM_CLIENTAPIFORALIGNMENTDATABASE_H
#define INDI_ALIGNMENTSUBSYSTEM_CLIENTAPIFORALIGNMENTDATABASE_H

#include "Common.h"

#include "indibase/basedevice.h"

#include <string>

namespace INDI {
namespace AlignmentSubsystem {

/*!
 * \class ClientAPIForAlignmentDatabase
 * \brief This class provides the client API to the driver side alignment database. It communicates
 * with the driver via the INDI properties interface.
 */
class ClientAPIForAlignmentDatabase
{
public:
    ClientAPIForAlignmentDatabase();
    virtual ~ClientAPIForAlignmentDatabase();

    /** \brief Process new device message from driver. This routine should be called from within
     the newDevice handler in the client. This routine is not normally called directly but is called by
     the ProcessNewDevice function in INDI::Alignment::AlignmentSubsystemForClients which filters out calls
     from unwanted devices. TODO maybe hide this function.
        \param[in] DevicePointer A pointer to the INDI::BaseDevice object.
    */
    void ProcessNewDevice(INDI::BaseDevice *DevicePointer);

    /** \brief Process new property message from driver. This routine should be called from within
     the newProperty handler in the client. This routine is not normally called directly but is called by
     the ProcessNewProperty function in INDI::Alignment::AlignmentSubsystemForClients which filters out calls
     from unwanted devices. TODO maybe hide this function.
        \param[in] PropertyPointer A pointer to the INDI::Property object.
    */
    void ProcessNewProperty(INDI::Property *PropertyPointer);

    /** \brief Append a sync point to the database.
        \param[in] CurrentValues The entry to append.
        \return True if successful
    */
    bool AppendSyncPoint(const AlignmentDatabaseEntry& CurrentValues);

    /** \brief Insert a sync point in the database.
        \param[in] Offset Pointer to where to make then insertion.
        \param[in] CurrentValues The entry to insert.
        \return True if successful
    */
    bool InsertSyncPoint(unsigned int Offset, const AlignmentDatabaseEntry& CurrentValues);

    /** \brief Edit a sync point in the database.
        \param[in] Offset Pointer to where to make then edit.
        \param[in] CurrentValues The entry to insert.
        \return True if successful
    */
    bool EditSyncPoint(unsigned int Offset, const AlignmentDatabaseEntry& CurrentValues);

    /** \brief Delete a sync point from the database.
        \param[in] Offset Pointer to the entry to delete
        \return True if successful
    */
    bool DeleteSyncPoint(unsigned int Offset);

    /** \brief Delete all sync points from the database.
        \return True if successful
    */
    bool ClearSyncPoints();

    /** \brief Read a sync point from the database.
        \param[in] Offset Pointer to where to read from.
        \param[out] CurrentValues The entry read.
        \note Unlike the property based database access methods a read next function is not provided. The caller is
        expected to manage any position pointers (carets).
        \return True if successful
    */
    bool ReadSyncPoint(unsigned int Offset, AlignmentDatabaseEntry& CurrentValues);

    /** \brief Read a sync point from the database at the current offset and increment the caret after reading.
        \param[out] CurrentValues The entry read.
        \note Unlike the property based database access methods a read next function is not provided. The caller is
        expected to manage any position pointers (carets).
        \return True if successful
    */
    bool ReadIncrementSyncPoint(AlignmentDatabaseEntry& CurrentValues);

    /** \brief Load the database from persistent storage
        \param[in] DeviceName The name of the current device.
        \return True if successful
    */
    bool LoadDatabase(const char* DeviceName);

    /** \brief Save the database to persistent storage
        \param[in] DeviceName The name of the current device.
        \return True if successful
    */
    bool SaveDatabase(const char* DeviceName);

    /** \brief Return the number of entries in the database.
        \return The number of entries in the database
    */
    const int GetDatabaseSize();

private:
    // Synchronise with the listener thread
    bool WaitForDriverCompletion();
    bool SignalDriverCompletion();
    bool SetDriverBusy();
    pthread_cond_t DriverActionCompleteCondition;
    pthread_mutex_t DriverActionCompleteMutex;
    bool DriverActionComplete;

    INDI::BaseDevice *Device;
    INDI::Property *MandatoryNumbers;
    INDI::Property *OptionalBinaryBlob;
    INDI::Property *PointsetSize;
    INDI::Property *CurrentEntry;
    INDI::Property *Action;
    INDI::Property *Commit;
};

} // namespace AlignmentSubsystem
} // namespace INDI

#endif // INDI_ALIGNMENTSUBSYSTEM_CLIENTAPIFORALIGNMENTDATABASE_H
