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

    virtual bool TransformCelestialToTelescope(const double RightAscension, const double Declination, TelescopeDirectionVector& ApparentTelescopeDirectionVector);

    virtual bool TransformTelescopeToCelestial(const TelescopeDirectionVector& ApparentTelescopeDirectionVector, double& RightAscension, double& Declination);

private:
    void CalculateTAKIMatrices(const TelescopeDirectionVector& Alpha1, const TelescopeDirectionVector& Alpha2, const TelescopeDirectionVector& Alpha3,
                            const TelescopeDirectionVector& Beta1, const TelescopeDirectionVector& Beta2, const TelescopeDirectionVector& Beta3,
                            gsl_matrix *pAlphaToBeta, gsl_matrix *pBetaToAlpha);

    void MatrixInvert3x3(gsl_matrix *pInput, gsl_matrix *pInversion);
    void MatrixMatrixMultipy(gsl_matrix *pA, gsl_matrix *pB, gsl_matrix *pC);
    void MatrixVectorMultipy(gsl_matrix *pA, gsl_vector *pB, gsl_vector *pC);
    bool RayTriangleIntersection(TelescopeDirectionVector& Ray, TelescopeDirectionVector& TriangleVertex1,
                                                                TelescopeDirectionVector& TriangleVertex2,
                                                                TelescopeDirectionVector& TriangleVertex3);

#if 0
    // Offsets for single point alignment
    ln_hrz_posn SinglePointOffsetsAltAz;
    ln_equ_posn SinglePointsOffsetsRaDec;
#endif

    // Transformation matrixes for 1, 2 and 2 sync points case
    gsl_matrix *pActualToApparentTransform;
    gsl_matrix *pApparentToActualTransform;

    // Convex hulls for 4+ sync points case
    ConvexHull ActualConvexHull;
    ConvexHull ApparentConvexHull;
    // Actual direction cosines for the 4+ case
    std::vector<TelescopeDirectionVector> ActualDirectionCosines;

};

} // namespace AlignmentSubsystem
} // namespace INDI

#endif // INDI_ALIGNMENTSUBSYSTEM_BUILTINMATHPLUGIN_H
