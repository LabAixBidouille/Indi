#include "BuiltInMathPlugin.h"

#include "DriverCommon.h"

#include <limits>
#include <iostream>
#include <gsl/gsl_permutation.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_blas.h>

namespace INDI {
namespace AlignmentSubsystem {

// Constructors

BuiltInMathPlugin::BuiltInMathPlugin()
{
    pActualToApparentTransform = gsl_matrix_alloc(3,3);
    pApparentToActualTransform = gsl_matrix_alloc(3,3);
}

// Destructor

BuiltInMathPlugin::~BuiltInMathPlugin()
{
    gsl_matrix_free(pActualToApparentTransform);
    gsl_matrix_free(pApparentToActualTransform);
}

// Public methods

bool BuiltInMathPlugin::Initialise(InMemoryDatabase* pInMemoryDatabase)
{
    MathPlugin::Initialise(pInMemoryDatabase);
    InMemoryDatabase::AlignmentDatabaseType& SyncPoints = pInMemoryDatabase->GetAlignmentDatabase();

    /// See how many entries there are in the in memory database.
    /// - If just one use a hint to mounts approximate alignment, this can either be ZENITH,
    /// NORTH_CELESTIAL_POLE or SOUTH_CELESTIAL_POLE. The hint is used to make a dummy second
    /// entry. A dummy third entry is computed from the cross product of the first two. A transform
    /// matrix is then computed.
    /// - If two make the dummy third entry and compute a transform matrix.
    /// - If three compute a transform matrix.
    /// - If four or more compute a convex hull, then matrices for each
    /// triangular facet of the hull.
    switch (SyncPoints.size())
    {
        case 0:
            // Not sure whether to return false or true here
            return true;

        case 1:
        {
            AlignmentDatabaseEntry& Entry1 = SyncPoints[0];
            ln_equ_posn RaDec;
            ln_hrz_posn ActualSyncPoint1;
            ln_lnlat_posn Position;
            if (!pInMemoryDatabase->GetDatabaseReferencePosition(Position))
                return false;
            RaDec.dec = Entry1.Declination;
            RaDec.ra = Entry1.RightAscension;
            ln_get_hrz_from_equ(&RaDec, &Position, Entry1.ObservationJulianDate, &ActualSyncPoint1);
            // Now express this coordinate as a normalised direction vector (a.k.a direction cosines)
            TelescopeDirectionVector ActualDirectionCosine1 = TelescopeDirectionVectorFromAltitudeAzimuth(ActualSyncPoint1);
            // Generate the second dummy sync point
            ln_hrz_posn DummyAltAz;
            ln_equ_posn DummyRaDec;
            TelescopeDirectionVector DummyActualDirectionCosine2;
            TelescopeDirectionVector DummyApparentDirectionCosine2;
            switch (ApproximateMountAlignment)
            {
                case ZENITH:
                    DummyAltAz.alt = 90.0;
                    DummyAltAz.az = 0.0;
                    break;

                case NORTH_CELESTIAL_POLE:
                    DummyRaDec.ra = 0.0;
                    DummyRaDec.dec = 0.0;
#ifdef USE_INITIAL_JULIAN_DATE
                    ln_get_hrz_from_equ(&DummyRaDec, &Position, Entry1.ObservationJulianDate, &DummyAltAz);
#else
                    ln_get_hrz_from_equ(&DummyRaDec, &Position, ln_get_julian_from_sys(), &DummyAltAz);
#endif
                    break;

                case SOUTH_CELESTIAL_POLE:
                    DummyRaDec.ra = 0.0;
                    DummyRaDec.dec = 180.0;
#ifdef USE_INITIAL_JULIAN_DATE
                    ln_get_hrz_from_equ(&DummyRaDec, &Position, Entry1.ObservationJulianDate, &DummyAltAz);
#else
                    ln_get_hrz_from_equ(&DummyRaDec, &Position, ln_get_julian_from_sys(), &DummyAltAz);
#endif
                    break;
            }
            DummyActualDirectionCosine2 = TelescopeDirectionVectorFromAltitudeAzimuth(DummyAltAz);
            // Cheat - make actual and apparent the same
            DummyApparentDirectionCosine2 = DummyActualDirectionCosine2;
            CalculateTAKIMatrices(ActualDirectionCosine1, DummyActualDirectionCosine2, ActualDirectionCosine1 * DummyActualDirectionCosine2,
                                Entry1.TelescopeDirection, DummyApparentDirectionCosine2, Entry1.TelescopeDirection * DummyApparentDirectionCosine2,
                                pActualToApparentTransform, pApparentToActualTransform);
            return true;
        }
        case 2:
        {
            // First compute local horizontal coordinates for the two sync points
            AlignmentDatabaseEntry& Entry1 = SyncPoints[0];
            AlignmentDatabaseEntry& Entry2 = SyncPoints[1];
            ln_hrz_posn ActualSyncPoint1;
            ln_hrz_posn ActualSyncPoint2;
            ln_equ_posn RaDec1;
            ln_equ_posn RaDec2;
            RaDec1.dec = Entry1.Declination;
            RaDec1.ra = Entry1.RightAscension;
            RaDec2.dec = Entry2.Declination;
            RaDec2.ra = Entry2.RightAscension;
            ln_lnlat_posn Position;
            if (!pInMemoryDatabase->GetDatabaseReferencePosition(Position))
                return false;
            ln_get_hrz_from_equ(&RaDec1, &Position, Entry1.ObservationJulianDate, &ActualSyncPoint1);
            ln_get_hrz_from_equ(&RaDec2, &Position, Entry2.ObservationJulianDate, &ActualSyncPoint2);

            // Now express these coordinates as normalised direction vectors (a.k.a direction cosines)
            TelescopeDirectionVector ActualDirectionCosine1 = TelescopeDirectionVectorFromAltitudeAzimuth(ActualSyncPoint1);
            TelescopeDirectionVector ActualDirectionCosine2 = TelescopeDirectionVectorFromAltitudeAzimuth(ActualSyncPoint2);

            // The third direction vectors is generated by taking the cross product of the first two
            CalculateTAKIMatrices(ActualDirectionCosine1, ActualDirectionCosine2, ActualDirectionCosine1 * ActualDirectionCosine2,
                                Entry1.TelescopeDirection, Entry2.TelescopeDirection, Entry1.TelescopeDirection * Entry2.TelescopeDirection,
                                pActualToApparentTransform, pApparentToActualTransform);
            return true;
        }

        case 3:
        {
            // First compute local horizontal coordinates for the three sync points
            AlignmentDatabaseEntry& Entry1 = SyncPoints[0];
            AlignmentDatabaseEntry& Entry2 = SyncPoints[1];
            AlignmentDatabaseEntry& Entry3 = SyncPoints[2];
            ln_hrz_posn ActualSyncPoint1;
            ln_hrz_posn ActualSyncPoint2;
            ln_hrz_posn ActualSyncPoint3;
            ln_equ_posn RaDec1;
            ln_equ_posn RaDec2;
            ln_equ_posn RaDec3;
            RaDec1.dec = Entry1.Declination;
            RaDec1.ra = Entry1.RightAscension;
            RaDec2.dec = Entry2.Declination;
            RaDec2.ra = Entry2.RightAscension;
            RaDec3.dec = Entry3.Declination;
            RaDec3.ra = Entry3.RightAscension;
            ln_lnlat_posn Position;
            if (!pInMemoryDatabase->GetDatabaseReferencePosition(Position))
                return false;
            ln_get_hrz_from_equ(&RaDec1, &Position, Entry1.ObservationJulianDate, &ActualSyncPoint1);
            ln_get_hrz_from_equ(&RaDec2, &Position, Entry2.ObservationJulianDate, &ActualSyncPoint2);
            ln_get_hrz_from_equ(&RaDec3, &Position, Entry3.ObservationJulianDate, &ActualSyncPoint3);

            // Now express these coordinates as normalised direction vectors (a.k.a direction cosines)
            TelescopeDirectionVector ActualDirectionCosine1 = TelescopeDirectionVectorFromAltitudeAzimuth(ActualSyncPoint1);
            TelescopeDirectionVector ActualDirectionCosine2 = TelescopeDirectionVectorFromAltitudeAzimuth(ActualSyncPoint2);
            TelescopeDirectionVector ActualDirectionCosine3 = TelescopeDirectionVectorFromAltitudeAzimuth(ActualSyncPoint3);

            CalculateTAKIMatrices(ActualDirectionCosine1, ActualDirectionCosine2, ActualDirectionCosine3,
                                Entry1.TelescopeDirection, Entry2.TelescopeDirection, Entry3.TelescopeDirection,
                                pActualToApparentTransform, pApparentToActualTransform);
            return true;
        }


        default:
        {
            ln_lnlat_posn Position;
            if (!pInMemoryDatabase->GetDatabaseReferencePosition(Position))
                return false;

            // Compute Hulls etc.
            ActualConvexHull.Reset();
            ApparentConvexHull.Reset();
            ActualDirectionCosines.clear();

            // Add a dummy point at the nadir
            ActualConvexHull.MakeNewVertex(0.0, 0.0, -1.0, 0);
            ApparentConvexHull.MakeNewVertex(0.0, 0.0, -1.0, 0);

            int VertexNumber = 1;
            // Add the rest of the vertices
            for (InMemoryDatabase::AlignmentDatabaseType::const_iterator Itr = SyncPoints.begin(); Itr != SyncPoints.end(); Itr++)
            {
                ln_equ_posn RaDec;
                ln_hrz_posn ActualSyncPoint;
                RaDec.dec = (*Itr).Declination;
                RaDec.ra = (*Itr).RightAscension;
                ln_get_hrz_from_equ(&RaDec, &Position, (*Itr).ObservationJulianDate, &ActualSyncPoint);
                // Now express this coordinate as normalised direction vectors (a.k.a direction cosines)
                TelescopeDirectionVector ActualDirectionCosine = TelescopeDirectionVectorFromAltitudeAzimuth(ActualSyncPoint);
                ActualDirectionCosines.push_back(ActualDirectionCosine);
                ActualConvexHull.MakeNewVertex(ActualDirectionCosine.x, ActualDirectionCosine.y, ActualDirectionCosine.z, VertexNumber);
                ApparentConvexHull.MakeNewVertex((*Itr).TelescopeDirection.x, (*Itr).TelescopeDirection.y, (*Itr).TelescopeDirection.z, VertexNumber);
                VertexNumber++;
            }
            // I should only need to do this once but it is easier to do it twice
            ActualConvexHull.DoubleTriangle();
            ActualConvexHull.ConstructHull();
            ActualConvexHull.EdgeOrderOnFaces();
            ApparentConvexHull.DoubleTriangle();
            ApparentConvexHull.ConstructHull();
            ApparentConvexHull.EdgeOrderOnFaces();

            // Make the matrices
            tFace CurrentFace = ActualConvexHull.faces;
            if (NULL != CurrentFace)
            {
                do
                {
                    CalculateTAKIMatrices(ActualDirectionCosines[CurrentFace->vertex[0]->vnum],
                                        ActualDirectionCosines[CurrentFace->vertex[1]->vnum],
                                        ActualDirectionCosines[CurrentFace->vertex[2]->vnum],
                                        SyncPoints[CurrentFace->vertex[0]->vnum].TelescopeDirection,
                                        SyncPoints[CurrentFace->vertex[1]->vnum].TelescopeDirection,
                                        SyncPoints[CurrentFace->vertex[2]->vnum].TelescopeDirection,
                                        CurrentFace->pMatrix, NULL);
                    CurrentFace = CurrentFace->next;
                }
                while (CurrentFace != faces);
            }

            // One of these days I will optimise this
            CurrentFace = ApparentConvexHull.faces;
            if (NULL != CurrentFace)
            {
                do
                {
                    CalculateTAKIMatrices(SyncPoints[CurrentFace->vertex[0]->vnum].TelescopeDirection,
                                        SyncPoints[CurrentFace->vertex[1]->vnum].TelescopeDirection,
                                        SyncPoints[CurrentFace->vertex[2]->vnum].TelescopeDirection,
                                        ActualDirectionCosines[CurrentFace->vertex[0]->vnum],
                                        ActualDirectionCosines[CurrentFace->vertex[1]->vnum],
                                        ActualDirectionCosines[CurrentFace->vertex[2]->vnum],
                                        CurrentFace->pMatrix, NULL);
                    CurrentFace = CurrentFace->next;
                }
                while (CurrentFace != faces);
            }

#ifdef CONVEX_HULL_DEBUGGING
            ActualConvexHull.PrintObj("ActualHull.obj");
            ApparentConvexHull.PrintObj("ApparentHull.obj");
#endif
            return true;
        }
    }
}

bool BuiltInMathPlugin::TransformCelestialToTelescope(const double RightAscension, const double Declination, double JulianOffset,
                                                        TelescopeDirectionVector& ApparentTelescopeDirectionVector)
{
    ln_equ_posn ActualRaDec;
    ln_hrz_posn ActualAltAz;
    ActualRaDec.ra = RightAscension * 360.0 / 24.0;
    ActualRaDec.dec = Declination;
    ln_lnlat_posn Position;

    if ((NULL == pInMemoryDatabase) || !pInMemoryDatabase->GetDatabaseReferencePosition(Position)) // Should check that this the same as the current observing position
        return false;

    InMemoryDatabase::AlignmentDatabaseType& SyncPoints = pInMemoryDatabase->GetAlignmentDatabase();
    switch (SyncPoints.size())
    {
        case 0:
        {
            // 0 sync points
            ln_hrz_posn ApparentAltAz;
#ifdef USE_INITIAL_JULIAN_DATE
            ln_get_hrz_from_equ(&ActualRaDec, &Position, SyncPoints[0].ObservationJulianDate, &ActualAltAz);
#else
            ln_get_hrz_from_equ(&ActualRaDec, &Position, ln_get_julian_from_sys() + JulianOffset, &ApparentAltAz);
#endif

            ApparentTelescopeDirectionVector = TelescopeDirectionVectorFromAltitudeAzimuth(ApparentAltAz);

            switch (ApproximateMountAlignment)
            {
                case ZENITH:
                    break;

                case NORTH_CELESTIAL_POLE:
                    // Rotate the TDV coordinate system clockwise (negative) around the y axis by 90 minus
                    // the (positive)observatory latitude. The vector itself is rotated anticlockwise
                    ApparentTelescopeDirectionVector.RotateAroundY(Position.lat - 90.0);
                    break;

                case SOUTH_CELESTIAL_POLE:
                    // Rotate the TDV coordinate system anticlockwise (positive) around the y axis by 90 plus
                    // the (negative)observatory latitude. The vector itself is rotated clockwise
                    ApparentTelescopeDirectionVector.RotateAroundY(Position.lat + 90.0);
                    break;
            }
            break;
        }
        case 1:
        case 2:
        case 3:
        {
#ifdef USE_INITIAL_JULIAN_DATE
            ln_get_hrz_from_equ(&ActualRaDec, &Position, SyncPoints[0].ObservationJulianDate, &ActualAltAz);
#else
            ln_get_hrz_from_equ(&ActualRaDec, &Position, ln_get_julian_from_sys() + JulianOffset, &ActualAltAz);
#endif
            TelescopeDirectionVector ActualVector = TelescopeDirectionVectorFromAltitudeAzimuth(ActualAltAz);
            ln_hrz_posn ApparentAltAz;
            gsl_vector *pGSLActualVector = gsl_vector_alloc(3);
            gsl_vector_set(pGSLActualVector, 0, ActualVector.x);
            gsl_vector_set(pGSLActualVector, 1, ActualVector.y);
            gsl_vector_set(pGSLActualVector, 2, ActualVector.z);
            gsl_vector *pGSLApparentVector = gsl_vector_alloc(3);
            MatrixVectorMultiply(pActualToApparentTransform, pGSLActualVector, pGSLApparentVector);
            ApparentTelescopeDirectionVector.x = gsl_vector_get(pGSLApparentVector, 0);
            ApparentTelescopeDirectionVector.y = gsl_vector_get(pGSLApparentVector, 1);
            ApparentTelescopeDirectionVector.z = gsl_vector_get(pGSLApparentVector, 2);
            ApparentTelescopeDirectionVector.Normalise();
            gsl_vector_free(pGSLActualVector);
            gsl_vector_free(pGSLApparentVector);
            break;
        }

        default:
        {
#ifdef USE_INITIAL_JULIAN_DATE
            ln_get_hrz_from_equ(&ActualRaDec, &Position, SyncPoints[0].ObservationJulianDate, &ActualAltAz);
#else
            ln_get_hrz_from_equ(&ActualRaDec, &Position, ln_get_julian_from_sys(), &ActualAltAz);
#endif
            TelescopeDirectionVector ActualVector = TelescopeDirectionVectorFromAltitudeAzimuth(ActualAltAz);
            // Scale the actual telescope direction vector to make sure it traverses the unit sphere.
            TelescopeDirectionVector ScaledActualVector = ActualVector * 2.0;
            // Shoot the scaled vector in the into the list of actual facets
            // and use the conversuion matrix from the one it intersects
            tFace CurrentFace = ActualConvexHull.faces;
            if (NULL != CurrentFace)
            {
                do
                {
                    if (RayTriangleIntersection(ScaledActualVector,
                                                ActualDirectionCosines[CurrentFace->vertex[0]->vnum],
                                                ActualDirectionCosines[CurrentFace->vertex[1]->vnum],
                                                ActualDirectionCosines[CurrentFace->vertex[2]->vnum]))
                        break;
                    CurrentFace = CurrentFace->next;
                }
                while (CurrentFace != faces);
                if (CurrentFace == faces)
                    return false;
            }
            else
                return false;

            // OK - got an intersection - CurrentFace is pointing at the face
            ln_hrz_posn ApparentAltAz;
            gsl_vector *pGSLActualVector = gsl_vector_alloc(3);
            gsl_vector_set(pGSLActualVector, 0, ActualVector.x);
            gsl_vector_set(pGSLActualVector, 1, ActualVector.y);
            gsl_vector_set(pGSLActualVector, 2, ActualVector.z);
            gsl_vector *pGSLApparentVector = gsl_vector_alloc(3);
            MatrixVectorMultiply(CurrentFace->pMatrix, pGSLActualVector, pGSLApparentVector);
            ApparentTelescopeDirectionVector.x = gsl_vector_get(pGSLApparentVector, 0);
            ApparentTelescopeDirectionVector.y = gsl_vector_get(pGSLApparentVector, 1);
            ApparentTelescopeDirectionVector.z = gsl_vector_get(pGSLApparentVector, 2);
            ApparentTelescopeDirectionVector.Normalise();
            gsl_vector_free(pGSLActualVector);
            gsl_vector_free(pGSLApparentVector);
            break;
        }
    }
    return true;
}

bool BuiltInMathPlugin::TransformTelescopeToCelestial(const TelescopeDirectionVector& ApparentTelescopeDirectionVector, double& RightAscension, double& Declination)
{
    ln_lnlat_posn Position;

    if ((NULL == pInMemoryDatabase) || !pInMemoryDatabase->GetDatabaseReferencePosition(Position)) // Should check that this the same as the current observing position
        return false;

    InMemoryDatabase::AlignmentDatabaseType& SyncPoints = pInMemoryDatabase->GetAlignmentDatabase();
    switch (SyncPoints.size())
    {
        case 0:
        {
            // 0 sync points
            TelescopeDirectionVector RotatedTDV(ApparentTelescopeDirectionVector);
            switch (ApproximateMountAlignment)
            {
                case ZENITH:
                    break;

                case NORTH_CELESTIAL_POLE:
                    // Rotate the TDV coordinate system anticlockwise (positive) around the y axis by 90 minus
                    // the (positive)observatory latitude. The vector itself is rotated clockwise
                    RotatedTDV.RotateAroundY(90.0 - Position.lat);
                    break;

                case SOUTH_CELESTIAL_POLE:
                    // Rotate the TDV coordinate system clockwise (negative) around the y axis by 90 plus
                    // the (negative)observatory latitude. The vector itself is rotated anticlockwise
                    RotatedTDV.RotateAroundY(-90.0 - Position.lat);
                    break;
            }
            ln_hrz_posn ApparentAltAz;
            ln_equ_posn ActualRaDec;
            AltitudeAzimuthFromTelescopeDirectionVector(RotatedTDV, ApparentAltAz);
#ifdef USE_INITIAL_JULIAN_DATE
            ln_get_equ_from_hrz(&ApparentAltAz, &Position, SyncPoints[0].ObservationJulianDate, &ActualRaDec);
#else
            ln_get_equ_from_hrz(&ApparentAltAz, &Position, ln_get_julian_from_sys(), &ActualRaDec);
#endif
            RightAscension = ActualRaDec.ra * 24.0 / 360.0;
            Declination = ActualRaDec.dec;
            break;
        }
        case 1:
        case 2:
        case 3:
        {
            gsl_vector *pGSLApparentVector = gsl_vector_alloc(3);
            gsl_vector_set(pGSLApparentVector, 0, ApparentTelescopeDirectionVector.x);
            gsl_vector_set(pGSLApparentVector, 1, ApparentTelescopeDirectionVector.y);
            gsl_vector_set(pGSLApparentVector, 2, ApparentTelescopeDirectionVector.z);
            gsl_vector *pGSLActualVector = gsl_vector_alloc(3);
            MatrixVectorMultiply(pApparentToActualTransform, pGSLApparentVector, pGSLActualVector);
            TelescopeDirectionVector ActualTelescopeDirectionVector;
            ActualTelescopeDirectionVector.x = gsl_vector_get(pGSLActualVector, 0);
            ActualTelescopeDirectionVector.y = gsl_vector_get(pGSLActualVector, 1);
            ActualTelescopeDirectionVector.z = gsl_vector_get(pGSLActualVector, 2);
            ActualTelescopeDirectionVector.Normalise();
            ln_hrz_posn ActualAltAz;
            AltitudeAzimuthFromTelescopeDirectionVector(ActualTelescopeDirectionVector, ActualAltAz);
            ln_equ_posn ActualRaDec;
#ifdef USE_INITIAL_JULIAN_DATE
            ln_get_equ_from_hrz(&ActualAltAz, &Position, SyncPoints[0].ObservationJulianDate, &ActualRaDec);
#else
            ln_get_equ_from_hrz(&ActualAltAz, &Position, ln_get_julian_from_sys(), &ActualRaDec);
#endif
            RightAscension = ActualRaDec.ra;
            Declination = ActualRaDec.dec;
            gsl_vector_free(pGSLActualVector);
            gsl_vector_free(pGSLApparentVector);
            break;
        }

        default:
        {
            // Scale the apparent telescope direction vector to make sure it traverses the unit sphere.
            TelescopeDirectionVector ScaledApparentVector = ApparentTelescopeDirectionVector * 2.0;
            // Shoot the scaled vector in the into the list of apparent facets
            // and use the conversuion matrix from the one it intersects
            tFace CurrentFace = ApparentConvexHull.faces;
            if (NULL != CurrentFace)
            {
                do
                {
                    if (RayTriangleIntersection(ScaledApparentVector,
                                                SyncPoints[CurrentFace->vertex[0]->vnum].TelescopeDirection,
                                                SyncPoints[CurrentFace->vertex[1]->vnum].TelescopeDirection,
                                                SyncPoints[CurrentFace->vertex[2]->vnum].TelescopeDirection))
                        break;
                    CurrentFace = CurrentFace->next;
                }
                while (CurrentFace != faces);
                if (CurrentFace == faces)
                    return false;
            }
            else
                return false;

            // OK - got an intersection - CurrentFace is pointing at the face
            gsl_vector *pGSLApparentVector = gsl_vector_alloc(3);
            gsl_vector_set(pGSLApparentVector, 0, ApparentTelescopeDirectionVector.x);
            gsl_vector_set(pGSLApparentVector, 1, ApparentTelescopeDirectionVector.y);
            gsl_vector_set(pGSLApparentVector, 2, ApparentTelescopeDirectionVector.z);
            gsl_vector *pGSLActualVector = gsl_vector_alloc(3);
            MatrixVectorMultiply(CurrentFace->pMatrix, pGSLApparentVector, pGSLActualVector);
            TelescopeDirectionVector ActualTelescopeDirectionVector;
            ActualTelescopeDirectionVector.x = gsl_vector_get(pGSLActualVector, 0);
            ActualTelescopeDirectionVector.y = gsl_vector_get(pGSLActualVector, 1);
            ActualTelescopeDirectionVector.z = gsl_vector_get(pGSLActualVector, 2);
            ActualTelescopeDirectionVector.Normalise();
            ln_hrz_posn ActualAltAz;
            AltitudeAzimuthFromTelescopeDirectionVector(ActualTelescopeDirectionVector, ActualAltAz);
            ln_equ_posn ActualRaDec;
#ifdef USE_INITIAL_JULIAN_DATE
            ln_get_equ_from_hrz(&ActualAltAz, &Position, SyncPoints[0].ObservationJulianDate, &ActualRaDec);
#else
            ln_get_equ_from_hrz(&ActualAltAz, &Position, ln_get_julian_from_sys(), &ActualRaDec);
#endif
            RightAscension = ActualRaDec.ra;
            Declination = ActualRaDec.dec;
            gsl_vector_free(pGSLActualVector);
            gsl_vector_free(pGSLApparentVector);
            break;
        }
    }
    return true;
}

// Private methods

void  BuiltInMathPlugin::CalculateTAKIMatrices(const TelescopeDirectionVector& Alpha1, const TelescopeDirectionVector& Alpha2, const TelescopeDirectionVector& Alpha3,
                            const TelescopeDirectionVector& Beta1, const TelescopeDirectionVector& Beta2, const TelescopeDirectionVector& Beta3,
                            gsl_matrix *pAlphaToBeta, gsl_matrix *pBetaToAlpha)
{
    // Derive the Actual to Apparent transformation matrix using TAKI's method
    gsl_matrix *pAlphaMatrix = gsl_matrix_alloc(3, 3);
    gsl_matrix_set(pAlphaMatrix, 0, 0, Alpha1.x);
    gsl_matrix_set(pAlphaMatrix, 1, 0, Alpha1.y);
    gsl_matrix_set(pAlphaMatrix, 2, 0, Alpha1.z);
    gsl_matrix_set(pAlphaMatrix, 0, 1, Alpha2.x);
    gsl_matrix_set(pAlphaMatrix, 1, 1, Alpha2.y);
    gsl_matrix_set(pAlphaMatrix, 2, 1, Alpha2.z);
    gsl_matrix_set(pAlphaMatrix, 0, 2, Alpha3.x);
    gsl_matrix_set(pAlphaMatrix, 1, 2, Alpha3.y);
    gsl_matrix_set(pAlphaMatrix, 2, 2, Alpha3.z);

    gsl_matrix *pBetaMatrix = gsl_matrix_alloc(3, 3);
    gsl_matrix_set(pBetaMatrix, 0, 0, Beta1.x);
    gsl_matrix_set(pBetaMatrix, 1, 0, Beta1.y);
    gsl_matrix_set(pBetaMatrix, 2, 0, Beta1.z);
    gsl_matrix_set(pBetaMatrix, 0, 1, Beta2.x);
    gsl_matrix_set(pBetaMatrix, 1, 1, Beta2.y);
    gsl_matrix_set(pBetaMatrix, 2, 1, Beta2.z);
    gsl_matrix_set(pBetaMatrix, 0, 2, Beta3.x);
    gsl_matrix_set(pBetaMatrix, 1, 2, Beta3.y);
    gsl_matrix_set(pBetaMatrix, 2, 2, Beta3.z);

    MatrixMatrixMultiply(pBetaMatrix, pAlphaMatrix, pAlphaToBeta);

    // Use pAlphaMatrix as temporary storage
    gsl_matrix_memcpy(pAlphaMatrix, pAlphaToBeta);

    // Invert the matrix to get the Apparent to Actual transform
    // use pBetaMatrix as temporary storage
    if (!MatrixInvert3x3(pAlphaToBeta, pBetaMatrix))
    {
        // pAlphaToBeta is singular and therefore is not a true transform
        // and cannot be inverted. This probably means it contains at least
        // one row or column that contains only zeroes
        gsl_matrix_set_identity(pBetaMatrix);
        ASSDEBUG("CalculateTAKIMatrices - AlphaToBeta matrix is singular!");
        IDMessage(NULL, "Calculated Celestial to Telescope transformation matrix is singular (not a true transform).");
    }

    if (NULL != pBetaToAlpha)
        gsl_matrix_memcpy(pBetaToAlpha, pBetaMatrix);

    // Clean up
    gsl_matrix_free(pBetaMatrix);
    gsl_matrix_free(pAlphaMatrix);
}

void BuiltInMathPlugin::Dump3x3(gsl_matrix *pMatrix)
{
    std::cerr << "Row 0 " << gsl_matrix_get(pMatrix, 0, 0) << " " << gsl_matrix_get(pMatrix, 0, 1) << " " << gsl_matrix_get(pMatrix, 0, 2) << '\n'
    << "Row 1 " << gsl_matrix_get(pMatrix, 1, 0) << " " << gsl_matrix_get(pMatrix, 1, 1) << " " << gsl_matrix_get(pMatrix, 1, 2) << '\n'
    << "Row 2 " << gsl_matrix_get(pMatrix, 2, 0) << " " << gsl_matrix_get(pMatrix, 2, 1) << " " << gsl_matrix_get(pMatrix, 2, 2) << '\n';

}


/// Use gsl to compute the inverse of a 3x3 matrix
bool BuiltInMathPlugin::MatrixInvert3x3(gsl_matrix *pInput, gsl_matrix *pInversion)
{
    bool Retcode = true;
    gsl_permutation *pPermutation = gsl_permutation_alloc(3);
    gsl_matrix *pDecomp = gsl_matrix_alloc(3,3);
    int Signum;

    gsl_matrix_memcpy(pDecomp, pInput);

    gsl_linalg_LU_decomp(pDecomp, pPermutation, &Signum);

    // Test for singularity
    if (0 == gsl_linalg_LU_det(pDecomp, Signum))
    {
        Retcode = false;
    }
    else
        gsl_linalg_LU_invert(pDecomp, pPermutation, pInversion);

    gsl_matrix_free(pDecomp);
    gsl_permutation_free(pPermutation);

    return Retcode;
}

/// Use gsl blas support to multiply two matrices together and put the result in a third.
/// For our purposes all the matrices should be 3 by 3.
void BuiltInMathPlugin::MatrixMatrixMultiply(gsl_matrix *pA, gsl_matrix *pB, gsl_matrix *pC)
{
    // Zeroise the output matrix
    gsl_matrix_set_zero(pC);

    gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, pA, pB, 0.0, pC);
}

/// Use gsl blas support to multiply a matrix by a vector and put the result in another vector
/// For our purposes the the matrix should be 3x3 and vector 3.
void BuiltInMathPlugin::MatrixVectorMultiply(gsl_matrix *pA, gsl_vector *pB, gsl_vector *pC)
{
    // Zeroise the output vector
    gsl_vector_set_zero(pC);

    gsl_blas_dgemv(CblasNoTrans, 1.0, pA, pB, 0.0, pC);
}

bool BuiltInMathPlugin::RayTriangleIntersection(TelescopeDirectionVector& Ray,
                                                TelescopeDirectionVector& TriangleVertex1,
                                                TelescopeDirectionVector& TriangleVertex2,
                                                TelescopeDirectionVector& TriangleVertex3)
{
    // Use Möller-Trumbore

    //Find vectors for two edges sharing V1
    TelescopeDirectionVector Edge1 = TriangleVertex2 - TriangleVertex1;
    TelescopeDirectionVector Edge2 = TriangleVertex3 - TriangleVertex1;

    TelescopeDirectionVector P = Ray * Edge2; // cross product
    double Determinant = Edge1 ^ P; // dot product
    double InverseDeterminant = 1.0 / Determinant;

    // if the determinant is negative the triangle is backfacing
    // if the determinant is close to 0, the ray misses the triangle
    if (Determinant < std::numeric_limits<double>::epsilon())
        return false;

    // I use zero as ray origin so
    TelescopeDirectionVector T = TriangleVertex1;

    // Calculate the u parameter
    double u = (T ^ P) * InverseDeterminant;

    if (u < 0.0 || u > 1.0)
        //The intersection lies outside of the triangle
        return false;

    //Prepare to test v parameter
    TelescopeDirectionVector Q = T * Edge1;

    //Calculate v parameter and test bound
    double v = (Ray ^ Q) * InverseDeterminant;

    if (v < 0.0 || u + v  > 1.0)
        //The intersection lies outside of the triangle
        return false;

    double t = (Edge2 ^ Q) * InverseDeterminant;

    if(t > std::numeric_limits<double>::epsilon())
    {
        //ray intersection
        return true;
    }

    // No hit, no win
    return false;
}


} // namespace AlignmentSubsystem
} // namespace INDI
