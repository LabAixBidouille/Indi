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

#include "indilogger.h"

namespace INDI {
namespace AlignmentSubsystem {

enum AlignmentDatabaseActions { APPEND, INSERT, EDIT, DELETE, CLEAR, READ, READ_INCREMENT, LOAD_DATABASE, SAVE_DATABASE };

enum AlignmentPointSetEnum {ENTRY_OBSERVATION_JULIAN_DATE, ENTRY_RA, ENTRY_DEC, ENTRY_VECTOR_X, ENTRY_VECTOR_Y, ENTRY_VECTOR_Z};

/*!
 * \struct TelescopeDirectionVector
 * \brief Holds a nomalised direction vector (direction cosines)
 *
 * The x y,z fields of this class should normally represent a normalised (unit length)
 * vector in a right handed rectangular coordinate space. However, for convenience a number
 * a number of standard 3d vector methods are also supported.
 */
struct TelescopeDirectionVector
{
    TelescopeDirectionVector() : x(0), y(0), z(0) {}
    TelescopeDirectionVector(double X, double Y, double Z) : x(X), y(Y), z(Z) {}

    double x;
    double y;
    double z;

    // Override the * operator to return a cross product
    inline const TelescopeDirectionVector operator * (const TelescopeDirectionVector &RHS) const
    {
        TelescopeDirectionVector Result;

        Result.x = y * RHS.z - z * RHS.y;
        Result.y = z * RHS.x - x * RHS.z;
        Result.z = x * RHS.y - y * RHS.x;
        return Result;
    }

    // Override the * operator to return a scalar product
    inline const TelescopeDirectionVector operator * (const double &RHS) const
    {
        TelescopeDirectionVector Result;

        Result.x *= y * RHS;
        Result.y *= z * RHS;
        Result.z *= x * RHS;
        return Result;
    }

    // Unary scalar product
    inline const TelescopeDirectionVector& operator *= (const double &RHS)
    {
        x *= RHS;
        y *= RHS;
        z *= RHS;
        return *this;
    }

    // Binary vector subtract
    inline const TelescopeDirectionVector operator - (const TelescopeDirectionVector& RHS) const
    {
        return TelescopeDirectionVector(x - RHS.x, y - RHS.y, z - RHS.z);
    }


    // Override the ^ operator to return a dot product
    inline double operator ^ (const TelescopeDirectionVector &RHS) const
    {
        return x * RHS.x + y * RHS.y + z * RHS.z;
    }

    inline void Normalise()
    {
        double length =  sqrt(x * x + y * y + z * z);
        x /= length;
        y /= length;
        z /= length;
    }

    void RotateAroundY(double Angle);
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
