/*!
 * \file AlignmentSubsystemBuiltInMathPlugin.h
 *
 * \author Roger James
 * \date 13th November 2013
 *
 * This file contains a declarations of the functionality for an alignment subsystem to be used
 * alongside the INDI::Telescope class in drivers and directly in clients.
 */

#ifndef INDI_BUILTINMATHPLUGIN_H
#define INDI_BUILTINMATHPLUGIN_H

#include "AlignmentSubsystem.h"


namespace INDI
{
    class AlignmentSubsystemBuiltInMathPlugin;
}

class INDI::AlignmentSubsystemBuiltInMathPlugin : public INDI::AlignmentSubsystemMathPlugin
{
    virtual bool TransformCelestialToTelescope(const double RightAscension, const double Declination, TelescopeDirectionVector& TelescopeDirectionVector) = 0;
    virtual bool TransformTelescopeToCelestial(const TelescopeDirectionVector& TelescopeDirectionVector, double& RightAscension, double& Declination) = 0;
};


#endif // INDI_BUILTINMATHPLUGIN_H
