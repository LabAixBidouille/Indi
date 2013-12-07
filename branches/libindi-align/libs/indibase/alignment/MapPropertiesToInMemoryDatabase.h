/*!
 * \file MapPropertiesToInMemoryDatabase.h
 *
 * \author Roger James
 * \date 13th November 2013
 *
 */

#ifndef INDI_ALIGNMENTSUBSYSTEM_MAPPROPERTIESTOINMEMORYDATABASE_H
#define INDI_ALIGNMENTSUBSYSTEM_MAPPROPERTIESTOINMEMORYDATABASE_H

#include "indibase/inditelescope.h"
#include "InMemoryDatabase.h"

namespace INDI {
namespace AlignmentSubsystem {

/*!
 * \class MapPropertiesToInMemoryDatabase
 * \brief An entry in the sync point database is defined by the following INDI properties
 * - ALIGNMENT_POINT_ENTRY_OBSERVATION_JULIAN_DATE\n
 *   The Julian date of the sync point observation (number)
 * - ALIGNMENT_POINT_ENTRY_OBSERVATION_LOCAL_SIDEREAL_TIME\n
 *   The local sidereal time of the sync point observation (number)
 * - ALIGNMENT_POINT_ENTRY_RA\n
 *   The right ascension of the sync point (number)
 * - ALIGNMENT_POINT_ENTRY_DEC\n
 *   The declination of the sync point (number)
 * - ALIGNMENT_POINT_ENTRY_VECTOR_X\n
 *   The x component of the telescope direction vector of the sync point (number)
 * - ALIGNMENT_POINT_ENTRY_VECTOR_Y\n
 *   The y component of the telescope direction vector of the sync point (number)
 * - ALIGNMENT_POINT_ENTRY_VECTOR_Z\n
 *   The z component of the telescope direction vector of the sync point (number)
 * - ALIGNMENT_POINT_ENTRY_PRIVATE\n
 *   An optional binary blob for communication between the client and the math plugin
 * .
 * The database is accessed using the following properties
 * - ALIGNMENT_POINTSET_SIZE\n
 *   The count of the number of sync points in the set (number)
 * - ALIGNMENT_POINTSET_CURRENT_ENTRY\n
 *   A zero based number that sets/shows the current entry (number)
 *   Only valid if ALIGNMENT_POINTSET_SIZE is greater than zero
 * - ALIGNMENT_POINTSET_ACTION\n
 *   Whenever this switch is written to one of the following actions is taken
 *   - APPEND\n
 *     Append a new entry to the set.
 *   - INSERT\n
 *     Insert a new entry at the pointer.
 *   - EDIT\n
 *     Overwrites the entry at the pointer.
 *   - DELETE\n
 *     Delete the entry at the pointer.
 *   - CLEAR\n
 *     Delete all entries.
 *   - READ\n
 *     Read the entry at the pointer.
 *   - READ INCREMENT\n
 *     Increment the pointer after reading the entry.
 *   - LOAD DATABASE\n
 *     Load the databse from local storage.
 *   - SAVE DATABASE\n
 *     Save the database to local storage.
 *
 */
class MapPropertiesToInMemoryDatabase : public InMemoryDatabase
{
public:
    virtual ~MapPropertiesToInMemoryDatabase() {}

    /** \brief Initilize alignment database properties. It is recommended to call this function within initProperties() of your primary device
        \param[in] deviceName Name of the primary device
    */
    void InitProperties(Telescope* pTelescope);

    /** \brief Call this function whenever a client updates a number property. The function will
     * handle any alignment database related properties.
     * \param[in] name vector property name
     * \param[in] values value as passed by the client
     * \param[in] names names as passed by the client
     * \param[in] n number of values and names pair to process.
    */
    void ProcessNumberProperties(Telescope* pTelescope, const char *name, double values[], char *names[], int n);

    /** \brief Call this function whenever a client updates a switch property. The function will
     * handle any alignment database related properties.
     * \param[in] name vector property name
     * \param[in] values value as passed by the client
     * \param[in] names names as passed by the client
     * \param[in] n number of values and names pair to process.
    */
    void ProcessSwitchProperties(Telescope* pTelescope, const char *name, ISState *states, char *names[], int n);

    /** \brief Call this function whenever a client updates a blob property. The function will
     * handle any alignment database related properties.
     * \param[in] name vector property name
     * \param[in] values value as passed by the client
     * \param[in] names names as passed by the client
     * \param[in] n number of values and names pair to process.
    */
    void ProcessBlobProperties(Telescope* pTelescope, const char *name, int sizes[], int blobsizes[], char *blobs[], char *formats[], char *names[], int n);

private:
    enum AlignmentPointSetEnum {ENTRY_OBSERVATION_JULIAN_DATE, ENTRY_OBSERVATION_LOCAL_SIDEREAL_TIME,
                                ENTRY_RA, ENTRY_DEC, ENTRY_VECTOR_X, ENTRY_VECTOR_Y, ENTRY_VECTOR_Z};
    INumber AlignmentPointSetEntry[7];
    INumberVectorProperty AlignmentPointSetEntryV;
    IBLOB AlignmentPointSetPrivateBinaryData;
    IBLOBVectorProperty AlignmentPointSetPrivateBinaryDataV;
    INumber AlignmentPointSetSize;
    INumberVectorProperty AlignmentPointSetSizeV;
    INumber AlignmentPointSetPointer;
    INumberVectorProperty AlignmentPointSetPointerV;
    ISwitch AlignmentPointSetAction[9];
    ISwitchVectorProperty AlignmentPointSetActionV;
    ISwitch AlignmentPointSetCommit;
    ISwitchVectorProperty AlignmentPointSetCommitV;

};

} // namespace AlignmentSubsystem
} // namespace INDI

#endif // INDI_ALIGNMENTSUBSYSTEM_MAPPROPERTIESTOINMEMORYDATABASE_H
