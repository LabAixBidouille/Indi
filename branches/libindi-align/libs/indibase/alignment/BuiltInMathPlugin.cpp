#include "BuiltInMathPlugin.h"

namespace INDI {
namespace AlignmentSubsystem {

BuiltInMathPlugin::BuiltInMathPlugin()
{
    //ctor
}

BuiltInMathPlugin::~BuiltInMathPlugin()
{
    //dtor
}

bool BuiltInMathPlugin::Initialise()
{
    return false;
}

bool BuiltInMathPlugin::TransformCelestialToTelescope(const double RightAscension, const double Declination, TelescopeDirectionVector& TelescopeDirectionVector)
{
    return false;
}

bool BuiltInMathPlugin::TransformTelescopeToCelestial(const TelescopeDirectionVector& TelescopeDirectionVector, double& RightAscension, double& Declination)
{
    return false;
}

} // namespace AlignmentSubsystem
} // namespace INDI
