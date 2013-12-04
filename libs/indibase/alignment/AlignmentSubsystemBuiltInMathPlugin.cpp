/*!
 * \file AlignmentSubsystemBuiltInMathPlugin.cpp
 *
 * \author Roger James
 * \date 13th November 2013
 *
 * This file contains a declarations of the functionality for an alignment subsystem to be used
 * alongside the INDI::Telescope class in drivers and directly in clients.
 */

#include "AlignmentSubsystemBuiltInMathPlugin.h"

bool INDI::AlignmentSubsystemBuiltInMathPlugin::TransformCelestialToTelescope(const double RightAscension, const double Declination, TelescopeDirectionVector& TelescopeDirectionVector)
{
    return false;
}

bool INDI::AlignmentSubsystemBuiltInMathPlugin::TransformTelescopeToCelestial(const TelescopeDirectionVector& TelescopeDirectionVector, double& RightAscension, double& Declination)
{
    return false;
}



