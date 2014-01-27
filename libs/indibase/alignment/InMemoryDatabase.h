/*!
 * \file InMemoryDatabase.h
 *
 * \author Roger James
 * \date 13th November 2013
 *
 */

#ifndef INDI_ALIGNMENTSUBSYSTEM_INMEMORYDATABASE_H
#define INDI_ALIGNMENTSUBSYSTEM_INMEMORYDATABASE_H

#include "Common.h"

#include <libnova.h>
#include <vector>

namespace INDI {
namespace AlignmentSubsystem {

/*!
 * \class InMemoryDatabase
 * \brief This class provides the driver side API to the in memory alignment database.
 */
class InMemoryDatabase
{
public:
    InMemoryDatabase() : LoadDatabaseCallback(0), DatabaseReferencePositionIsValid(false) {}
    virtual ~InMemoryDatabase() {}

    typedef std::vector<AlignmentDatabaseEntry> AlignmentDatabaseType;

    // Public methods

    /** \brief Get a reference to the in memory database.
        \return A reference to the in memory database.
    */
    AlignmentDatabaseType& GetAlignmentDatabase() { return MySyncPoints; }

    bool GetDatabaseReferencePosition(ln_lnlat_posn& Position);

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

    void SetDatabaseReferencePosition(double Latitude, double Longitude);

    typedef void (*LoadDatabaseCallbackPointer_t)(void *);
    void SetLoadDatabaseCallback(LoadDatabaseCallbackPointer_t CallbackPointer, void *ThisPointer);


private:
    AlignmentDatabaseType MySyncPoints;
    ln_lnlat_posn DatabaseReferencePosition;
    bool DatabaseReferencePositionIsValid;
    LoadDatabaseCallbackPointer_t LoadDatabaseCallback;
    void *LoadDatabaseCallbackThisPointer;
};

} // namespace AlignmentSubsystem
} // namespace INDI

#endif // INDI_ALIGNMENTSUBSYSTEM_INMEMORYDATABASE_H
