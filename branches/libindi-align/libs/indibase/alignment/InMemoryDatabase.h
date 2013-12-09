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
    virtual ~InMemoryDatabase() {}

    typedef std::vector<AlignmentDatabaseEntry> AlignmentDatabaseType;

    /** \brief Get a reference to the in memory database.
        \return A reference to the in memory database.
    */
    AlignmentDatabaseType& GetAlignmentDatabase() { return MySyncPoints; }

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

private:
    AlignmentDatabaseType MySyncPoints;
};

} // namespace AlignmentSubsystem
} // namespace INDI

#endif // INDI_ALIGNMENTSUBSYSTEM_INMEMORYDATABASE_H
