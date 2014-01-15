#include "BuiltInMathPlugin.h"

#include <gsl/gsl_permutation.h>
#include <gsl/gsl_linalg.h>

namespace INDI {
namespace AlignmentSubsystem {

BuiltInMathPlugin::BuiltInMathPlugin()
{
    pActualToApparentTransform = gsl_matrix_alloc(3,3);
    pApparentToActualTransform = gsl_matrix_alloc(3,3);
}

BuiltInMathPlugin::~BuiltInMathPlugin()
{
    gsl_matrix_free(pActualToApparentTransform);
    gsl_matrix_free(pApparentToActualTransform);
}

bool BuiltInMathPlugin::Initialise()
{
    /// See how many entries there are in the in memory database.
    /// If just one compute offsets in the mounts frame of reference
    /// which assumed to be an abitrarily aligned spherical coordinate
    /// system, with rotational angle measured in a clockwise direction
    /// and elevation angle measured upwards from the rotational plane.
    /// The offsets are computed using a hint supplied as to the approximate
    /// orientation of the mount. This can either be ZENITH, NORTH_CELESTIAL_POLE
    /// or SOUTH_CELESTIAL_POLE,
    /// If two compute a dummy third entry and compute and store
    /// a transform matrix. If three compute a transform matrix.
    /// If four or more compute a convex hull, then matrices for each
    /// triangular facet of the hull.
    switch (GetAlignmentDatabase().size())
    {
        case 0:
            return false;

        case 1:
        {
            // Compute local horizontal coordinate offsets
            AlignmentDatabaseEntry& Entry = GetAlignmentDatabase()[0];
            ln_equ_posn RaDec;
            ln_lnlat_posn Position;
            if (!GetDatabaseReferencePosition(Position))
                return false;
            RaDec.dec = Entry.Declination;
            RaDec.ra = Entry.RightAscension;
            switch (ApproximateMountAlignment)
            {
                case ZENITH:
                    ln_hrz_posn AltAz;
                    ln_hrz_posn MountAltAz;
                   // Probably an altaz goto or a dobsonian
                    // Translate sync point RA/DEC into alt/az (clockwise from South) for the
                    // date and time of the sync point observation.
                    ln_get_hrz_from_equ(&RaDec, &Position, Entry.ObservationJulianDate, &AltAz);
                    // Translate the mount coordinates from the sync point into similar clockwise coordinate space
                    AltitudeAzimuthFromNormalisedDirectionVector(Entry.TelescopeDirection, MountAltAz);
                    SinglePointOffsetsAltAz.alt = AltAz.alt - MountAltAz.alt;
                    SinglePointOffsetsAltAz.az = AltAz.az - MountAltAz.az;
                    break;

                case NORTH_CELESTIAL_POLE:
                {
                    // THIS IS RUBBISH!!!!!!!! I need to review it.
                    // Equatorial in the northern hemisphere
                    ln_equ_posn AdjustedEquatorialCoordinate;
                    ln_equ_posn MountEquatorialCoordinate;
                    double GreenwichMeanSiderealTime = ln_get_mean_sidereal_time(Entry.ObservationJulianDate);
                    // Convert time to degrees
                    GreenwichMeanSiderealTime *= 360.0 / 24.0;
                    // Convert right ascension to local hour angle
                    // LHA = GMST + Longitude (positive east) - RA
                    AdjustedEquatorialCoordinate.ra = GreenwichMeanSiderealTime + Position.lng - RaDec.ra;
                    // N.B. this has converted the ra value from ANTI_CLOCKWISE to CLOCKWISE

                    // Convert declination to elevation measured from equatorial plane towards the north celestial pole
                    // I am really guessing here !!!!!
                    AdjustedEquatorialCoordinate.dec = 90.0 - RaDec.dec;


                    LocalHourAngleDeclinationFromTelescopeDirectionVector(Entry.TelescopeDirection, MountEquatorialCoordinate);
                    SinglePointsOffsetsRaDec.ra =  AdjustedEquatorialCoordinate.ra - MountEquatorialCoordinate.ra;
                    SinglePointsOffsetsRaDec.dec =  AdjustedEquatorialCoordinate.dec - MountEquatorialCoordinate.dec;
                    break;
                }

                case SOUTH_CELESTIAL_POLE:
                    // My brain has stalled. I assume I reverse the rotation of the hour angle calc
                    // and fudge the declination to elevation measured from the equatorial plane towards the south celestial pole
                    return false;

            }
            return true;
        }

        case 2:
        {
            // First compute local horizontal coordinates for the two sync points
            AlignmentDatabaseEntry& Entry1 = GetAlignmentDatabase()[0];
            AlignmentDatabaseEntry& Entry2 = GetAlignmentDatabase()[1];
            ln_hrz_posn ActualSyncPoint1;
            ln_hrz_posn ActualSyncPoint2;
            ln_equ_posn RaDec1;
            ln_equ_posn RaDec2;
            RaDec1.dec = Entry1.Declination;
            RaDec1.ra = Entry1.RightAscension;
            RaDec2.dec = Entry2.Declination;
            RaDec2.ra = Entry2.RightAscension;
            ln_lnlat_posn Position;
            if (!GetDatabaseReferencePosition(Position))
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
            AlignmentDatabaseEntry& Entry1 = GetAlignmentDatabase()[0];
            AlignmentDatabaseEntry& Entry2 = GetAlignmentDatabase()[1];
            AlignmentDatabaseEntry& Entry3 = GetAlignmentDatabase()[2];
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
            if (!GetDatabaseReferencePosition(Position))
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
            // Compute Hull etc.
            return true;
    }
}

void  BuiltInMathPlugin::CalculateTAKIMatrices(const TelescopeDirectionVector& Actual1, const TelescopeDirectionVector& Actual2, const TelescopeDirectionVector& Actual3,
                            const TelescopeDirectionVector& Apparent1, const TelescopeDirectionVector& Apparent2, const TelescopeDirectionVector& Apparent3,
                            gsl_matrix *pActualToApparent, gsl_matrix *pApparentToActual)
{
    // Derive the Actual to Apparent transformation matrix using TAKI's method
    gsl_matrix *pActualMatrix = gsl_matrix_alloc(3, 3);
    gsl_matrix_set(pActualMatrix, 0, 0, Actual1.x);
    gsl_matrix_set(pActualMatrix, 0, 1, Actual1.y);
    gsl_matrix_set(pActualMatrix, 0, 2, Actual1.z);
    gsl_matrix_set(pActualMatrix, 1, 0, Actual2.x);
    gsl_matrix_set(pActualMatrix, 1, 1, Actual2.y);
    gsl_matrix_set(pActualMatrix, 1, 2, Actual2.z);
    gsl_matrix_set(pActualMatrix, 2, 0, Actual3.x);
    gsl_matrix_set(pActualMatrix, 2, 1, Actual3.y);
    gsl_matrix_set(pActualMatrix, 2, 2, Actual3.z);

    gsl_matrix *pApparentMatrix = gsl_matrix_alloc(3, 3);
    gsl_matrix_set(pApparentMatrix, 0, 0, Apparent1.x);
    gsl_matrix_set(pApparentMatrix, 0, 1, Apparent1.y);
    gsl_matrix_set(pApparentMatrix, 0, 2, Apparent1.z);
    gsl_matrix_set(pApparentMatrix, 1, 0, Apparent2.x);
    gsl_matrix_set(pApparentMatrix, 1, 1, Apparent2.y);
    gsl_matrix_set(pApparentMatrix, 1, 2, Apparent2.z);
    gsl_matrix_set(pApparentMatrix, 2, 0, Apparent3.x);
    gsl_matrix_set(pApparentMatrix, 2, 1, Apparent3.y);
    gsl_matrix_set(pApparentMatrix, 2, 2, Apparent3.z);

    gsl_matrix_mul_elements(pApparentMatrix, pActualMatrix); // Result ends up in Apparent

    gsl_matrix_memcpy(pActualToApparent, pApparentMatrix);

    // Invert the matrix to get the Apparent to Actual transform
    gsl_permutation *pPermutation = gsl_permutation_alloc(3);
    int Signum;
    gsl_linalg_LU_decomp(pApparentMatrix, pPermutation, &Signum);
    gsl_linalg_LU_invert(pApparentMatrix, pPermutation, pApparentToActual);

    // Clean up
    gsl_matrix_free(pApparentMatrix);
    gsl_matrix_free(pActualMatrix);
    gsl_permutation_free(pPermutation);
}


bool BuiltInMathPlugin::TransformCelestialToTelescope(const double RightAscension, const double Declination, TelescopeDirectionVector& TelescopeDirectionVector)
{
    ln_equ_posn ActualRaDec;
    ActualRaDec.ra = RightAscension;
    ActualRaDec.dec = Declination;

    switch (GetAlignmentDatabase().size())
    {
        case 0:
            // No alignment points
            return false;

        case 1:
            // 1 alignment point. Use the stored single point offsets
            switch (ApproximateMountAlignment)
            {
                case ZENITH:
                    ln_hrz_posn ActualAltAz;
                    ln_hrz_posn ApparentAltAz;
                    ln_lnlat_posn Position;
                    if (!GetDatabaseReferencePosition(Position))
                        return false;
                    // Hmmm.. should I get the julian date from the observatory clock
                    ln_get_hrz_from_equ(&ActualRaDec, &Position, ln_get_julian_from_sys(), &ActualAltAz);

                    ApparentAltAz.alt = ActualAltAz.alt - SinglePointOffsetsAltAz.alt;
                    ApparentAltAz.az = ActualAltAz.az - SinglePointOffsetsAltAz.az;

                    TelescopeDirectionVector = TelescopeDirectionVectorFromAltitudeAzimuth(ApparentAltAz);
                    break;

                case NORTH_CELESTIAL_POLE:
                case SOUTH_CELESTIAL_POLE: // TODO Check if I need to do anything different here.
                    break;
            }
            break;

        case 2:
        case 3:
            break;

        default:
            return false;
    }
    return true;
}

bool BuiltInMathPlugin::TransformTelescopeToCelestial(const TelescopeDirectionVector& TelescopeDirectionVector, double& RightAscension, double& Declination)
{
    return false;
}

} // namespace AlignmentSubsystem
} // namespace INDI
