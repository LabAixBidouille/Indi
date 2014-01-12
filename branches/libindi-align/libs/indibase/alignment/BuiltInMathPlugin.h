#ifndef INDI_ALIGNMENTSUBSYSTEM_BUILTINMATHPLUGIN_H
#define INDI_ALIGNMENTSUBSYSTEM_BUILTINMATHPLUGIN_H

#include "AlignmentSubsystemForMathPlugins.h"
#include "ConvexHull.h"

namespace INDI {
namespace AlignmentSubsystem {

class BuiltInMathPlugin : public AlignmentSubsystemForMathPlugins, public ConvexHull
{
public:
    BuiltInMathPlugin();
    virtual ~BuiltInMathPlugin();

    virtual bool Initialise();

    virtual bool TransformCelestialToTelescope(const double RightAscension, const double Declination, TelescopeDirectionVector& TelescopeDirectionVector);

    virtual bool TransformTelescopeToCelestial(const TelescopeDirectionVector& TelescopeDirectionVector, double& RightAscension, double& Declination);

private:
    // Offsets for single point alignment
    ln_hrz_posn SinglePointOffsetsAltAz;
    ln_equ_posn SinglePointsOffsetsRaDec;
};

} // namespace AlignmentSubsystem
} // namespace INDI

#endif // INDI_ALIGNMENTSUBSYSTEM_BUILTINMATHPLUGIN_H
