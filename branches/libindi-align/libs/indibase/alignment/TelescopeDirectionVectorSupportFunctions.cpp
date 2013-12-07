/*!
 * \file TelescopeDirectionVectorSupportFunctions.cpp
 *
 * \author Roger James
 * \date 13th November 2013
 *
 */

#include "TelescopeDirectionVectorSupportFunctions.h"

namespace INDI {
namespace AlignmentSubsystem {

const TelescopeDirectionVector TelescopeDirectionVectorSupportFunctions::TelescopeDirectionVectorFromSphericalCoordinate(const double AzimuthAngle, AzimuthAngleDirection AzimuthAngleDirection,
                                                                                            const double PolarAngle, PolarAngleDirection PolarAngleDirection)
{
    TelescopeDirectionVector Vector;

    // TODO
    return Vector;
}

void TelescopeDirectionVectorSupportFunctions::SphericalCoordinateFromTelescopeDirectionVector(const TelescopeDirectionVector TelescopeDirectionVector,
                                                                const double& AzimuthAngle, AzimuthAngleDirection AzimuthAngleDirection,
                                                                const double& PolarAngle, PolarAngleDirection PolarAngleDirection)
{
    // TODO
}

} // namespace AlignmentSubsystem
} // namespace INDI
