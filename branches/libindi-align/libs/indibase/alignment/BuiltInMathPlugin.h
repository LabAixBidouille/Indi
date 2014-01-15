#ifndef INDI_ALIGNMENTSUBSYSTEM_BUILTINMATHPLUGIN_H
#define INDI_ALIGNMENTSUBSYSTEM_BUILTINMATHPLUGIN_H

#include "AlignmentSubsystemForMathPlugins.h"
#include "ConvexHull.h"

#include <gsl/gsl_matrix.h>

namespace INDI {
namespace AlignmentSubsystem {

/*!
 * \class BuiltInMathPlugin
 * \brief This class implements the default math plugin.
 */
class BuiltInMathPlugin : public AlignmentSubsystemForMathPlugins, public ConvexHull
{
public:
    BuiltInMathPlugin();
    virtual ~BuiltInMathPlugin();

    virtual bool Initialise();

    virtual bool TransformCelestialToTelescope(const double RightAscension, const double Declination, TelescopeDirectionVector& TelescopeDirectionVector);

    virtual bool TransformTelescopeToCelestial(const TelescopeDirectionVector& TelescopeDirectionVector, double& RightAscension, double& Declination);

private:
    void CalculateTAKIMatrices(const TelescopeDirectionVector& Actual1, const TelescopeDirectionVector& Actual2, const TelescopeDirectionVector& Actual3,
                            const TelescopeDirectionVector& Apparent1, const TelescopeDirectionVector& Apparent2, const TelescopeDirectionVector& Apparent3,
                            gsl_matrix *pActualToApparent, gsl_matrix *pApparentToActual);

    // Offsets for single point alignment
    ln_hrz_posn SinglePointOffsetsAltAz;
    ln_equ_posn SinglePointsOffsetsRaDec;

    // Transformation matrixes for 2 and 2 sync points case
    gsl_matrix *pActualToApparentTransform;
    gsl_matrix *pApparentToActualTransform;
};

} // namespace AlignmentSubsystem
} // namespace INDI

#endif // INDI_ALIGNMENTSUBSYSTEM_BUILTINMATHPLUGIN_H
