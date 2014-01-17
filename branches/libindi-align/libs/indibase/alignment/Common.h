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
#include <cstring>
#include <cmath>

namespace INDI {
namespace AlignmentSubsystem {

enum AlignmentDatabaseActions { APPEND, INSERT, EDIT, DELETE, CLEAR, READ, READ_INCREMENT, LOAD_DATABASE, SAVE_DATABASE };

enum AlignmentPointSetEnum {ENTRY_OBSERVATION_JULIAN_DATE, ENTRY_RA, ENTRY_DEC, ENTRY_VECTOR_X, ENTRY_VECTOR_Y, ENTRY_VECTOR_Z};

/*!
 * \struct TelescopeDirectionVector
 * \brief Holds a nomalised direction vector (direction cosines)
 *
 * The x y,z fields of this class should always represent a normalised (unit length)
 * vector in a right handed rectangular coordinate space.
 */
struct TelescopeDirectionVector
{
    TelescopeDirectionVector() : x(0), y(0), z(0) {}
    double x;
    double y;
    double z;

    // Override the * operator to return a cross product
    const TelescopeDirectionVector operator * (const TelescopeDirectionVector &RHS) const
    {
        TelescopeDirectionVector Result;

        Result.x = y * RHS.z - z * RHS.y;
        Result.y = z * RHS.x - x * RHS.z;
        Result.z = x * RHS.y - y * RHS.x;
        return Result;
    }

    inline void Normalise()
    {
        double length =  sqrt(x * x + y * y + z * z);
        x /= length;
        y /= length;
        z /= length;
    }
};

/*!
 * \struct AlignmentDatabaseEntry
 * \brief Entry in the in memory alignment database
 *
 */
struct AlignmentDatabaseEntry
{
    AlignmentDatabaseEntry() : ObservationJulianDate(0), RightAscension(0),
                                Declination(0), PrivateDataSize(0) {}

    AlignmentDatabaseEntry(const AlignmentDatabaseEntry& Source) : ObservationJulianDate(Source.ObservationJulianDate),
                                                                    RightAscension(Source.RightAscension),
                                                                    Declination(Source.Declination),
                                                                    PrivateDataSize(Source.PrivateDataSize)
    {
        if (0 != PrivateDataSize)
        {
            PrivateData.reset(new unsigned char[PrivateDataSize]);
            memcpy(PrivateData.get(), Source.PrivateData.get(), PrivateDataSize);
        }
    }

    double ObservationJulianDate;
    double RightAscension;
    double Declination;
    TelescopeDirectionVector TelescopeDirection;
    std::auto_ptr<unsigned char> PrivateData;
    int PrivateDataSize;
};

} // namespace AlignmentSubsystem
} // namespace INDI

#endif // INDI_ALIGNMENTSUBSYSTEM_COMMON_H
