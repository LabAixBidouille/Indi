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
    /// \brief Default constructor
    BuiltInMathPlugin();

    /// \brief Virtual desctructor
    virtual ~BuiltInMathPlugin();

    /// \brief Override for the base class virtual function
    virtual bool Initialise(InMemoryDatabase* pInMemoryDatabase);

    /// \brief Override for the base class virtual function
    virtual bool TransformCelestialToTelescope(const double RightAscension, const double Declination, double JulianOffset,
                                                    TelescopeDirectionVector& ApparentTelescopeDirectionVector);

    /// \brief Override for the base class virtual function
    virtual bool TransformTelescopeToCelestial(const TelescopeDirectionVector& ApparentTelescopeDirectionVector, double& RightAscension, double& Declination);

private:
    /// \brief Calculate tranformation matrices from the supplied vectors using Taki's method
    /// \param[in] Alpha1 Pointer to the first coordinate in the alpha reference frame
    /// \param[in] Alpha2 Pointer to the second coordinate in the alpha reference frame
    /// \param[in] Alpha3 Pointer to the third coordinate in the alpha reference frame
    /// \param[in] Beta1 Pointer to the first coordinate in the beta reference frame
    /// \param[in] Beta2 Pointer to the second coordinate in the beta reference frame
    /// \param[in] Beta3 Pointer to the third coordinate in the beta reference frame
    /// \param[in] pAlphaToBeta Pointer to a matrix to receive the Alpha to Beta transformation matrix
    /// \param[in] pBetaToAlpha Pointer to a matrix to receive the Beta to Alpha transformation matrix
    void CalculateTAKIMatrices(const TelescopeDirectionVector& Alpha1, const TelescopeDirectionVector& Alpha2, const TelescopeDirectionVector& Alpha3,
                            const TelescopeDirectionVector& Beta1, const TelescopeDirectionVector& Beta2, const TelescopeDirectionVector& Beta3,
                            gsl_matrix *pAlphaToBeta, gsl_matrix *pBetaToAlpha);

    /// \brief Calculate the inverse of the supplied matrix
    /// \param[in] pInput Pointer to the input matrix
    /// \param[in] pInversion Pointer to a matrix to receive the inversion
    void MatrixInvert3x3(gsl_matrix *pInput, gsl_matrix *pInversion);

    /// \brief Multiply matrix A by matrix B and put the result in C
    void MatrixMatrixMultipy(gsl_matrix *pA, gsl_matrix *pB, gsl_matrix *pC);

    /// \brief Multiply matrix A by vector B and put the result in vector C
    void MatrixVectorMultipy(gsl_matrix *pA, gsl_vector *pB, gsl_vector *pC);

    /// \brief Test if a ray intersects a triangle in 3d space
    /// \param[in] Ray The ray vector
    /// \param[in] TriangleVertex1 The first vertex of the triangle
    /// \param[in] TriangleVertex2 The second vertex of the triangle
    /// \param[in] TriangleVertex3 The third vertex of the triangle
    /// \note The order of the vertices determine whether the triangle is facing away from or towards the origin.
    /// Intersection with triangles facing the origin will be ignored.
    bool RayTriangleIntersection(TelescopeDirectionVector& Ray, TelescopeDirectionVector& TriangleVertex1,
                                                                TelescopeDirectionVector& TriangleVertex2,
                                                                TelescopeDirectionVector& TriangleVertex3);

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
