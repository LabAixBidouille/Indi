#include "DummyMathPlugin.h"

namespace INDI {
namespace AlignmentSubsystem {


// Standard functions required for all plugins
extern "C"
{
    DummyMathPlugin* Create()
    {
        return new DummyMathPlugin;
    }

    void Destroy(DummyMathPlugin *pPlugin)
    {
        delete pPlugin;
    }

    const char *GetDisplayName()
    {
        return "Dummy Math Plugin";
    }
}

DummyMathPlugin::DummyMathPlugin()
{
    //ctor
}

DummyMathPlugin::~DummyMathPlugin()
{
    //dtor
}

bool DummyMathPlugin::Initialise()
{
    return false;
}

bool DummyMathPlugin::TransformCelestialToTelescope(const double RightAscension, const double Declination, TelescopeDirectionVector& ApparentTelescopeDirectionVector)
{
    return false;
}

bool DummyMathPlugin::TransformTelescopeToCelestial(const TelescopeDirectionVector& ApparentTelescopeDirectionVector, double& RightAscension, double& Declination)
{
    return false;
}

} // namespace AlignmentSubsystem
} // namespace INDI
