/*!
 * \file Common.h
 *
 * \author Roger James
 * \date 13th November 2013
 *
 */

#ifndef INDI_ALIGNMENTSUBSYSTEM_COMMON_H
#define INDI_ALIGNMENTSUBSYSTEM_COMMON_H

#include <memory>

namespace INDI {
namespace AlignmentSubsystem {

/*!
 * \struct TelescopeDirectionVector
 * \brief Holds a nomalised direction vector (direction cosines)
 *
 * The x y,z fields of this class should always represent a normalised (unit length)
 * vector in a right handed rectangular coordinate space.
 */
typedef struct
{
    double x;
    double y;
    double z;
} TelescopeDirectionVector;

/*!
 * \struct AlignmentDatabaseEntry
 * \brief Entry in the in memory alignment database
 *
 */
typedef struct
{
    int ObservationDate;
    double ObservationTime;
    double RightAscension;
    double Declination;
    TelescopeDirectionVector TelescopeDirection;
    std::auto_ptr<void> PrivateData;
} AlignmentDatabaseEntry;

enum AlignmentDatabaseActions { APPEND, INSERT, EDIT, DELETE, CLEAR, READ, READ_INCREMENT, LOAD_DATABASE, SAVE_DATABASE };

} // namespace AlignmentSubsystem
} // namespace INDI

#endif // INDI_ALIGNMENTSUBSYSTEM_COMMON_H
